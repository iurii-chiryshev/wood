#include "modelcylinder.h"
namespace fitting{

ModelCylinder::ModelCylinder(Model *parent): Model(parent)
{

}



static OSG::Transform* _makePartialCylinder(float height,
                                             float radius,
                                             const unsigned int parts){
    Q_ASSERT(parts >= 1);
    OSG::Transform *parent = new OSG::Transform();
    const float part_height = height / parts;
    const float half_part_height = part_height / 2;
    const float half_height = height / 2;
    // цвет всегда синий
    osg::ref_ptr<osg::Vec4Array> colors ( new osg::Vec4Array( 1,new osg::Vec4( 0, 1.f, 0, 1.f) ) );

    //линия - ось цилиндра
    osg::ref_ptr<osg::Geometry> line_geometry = new osg::Geometry;
    osg::ref_ptr<OSG::Geode> line_geode = new OSG::Geode();
    line_geometry->setColorArray(colors.get());
    line_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    // точки оси
    osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());
    const int axes_points = parts * 10;
    for(int i = 0; i < axes_points; i++){
        vertices->push_back(osg::Vec3(0,0,(float)i * height / axes_points - height/2));
    }
    line_geometry->setVertexArray( vertices.get() );
    line_geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,vertices->size()));
    line_geode->addDrawable( line_geometry );
    parent->addChild(line_geode);


    //создать один цилиндр из нескольких маленьких так,
    //чтобы его ось была там же, как если бы мы создали один цилиндр
    for(unsigned int i = 0; i < parts; i++){
        OSG::Transform *transform = new OSG::Transform();
        OSG::Geode *geode = new OSG::Geode();
        osg::ShapeDrawable *shape = new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(0,0,0),radius,part_height));
        shape->setColor((*colors)[0]);
        geode->addDrawable(shape);
        //сдвигаем
        osg::Matrix mat = osg::Matrix::translate( osg::Vec3(0, 0, i * part_height +  half_part_height - half_height) );
        transform->setMatrix( mat );
        transform->addChild(geode);
        //ось для каждого цилиндра
        parent->addChild(transform);
    }
    osg::StateSet* state = parent->getOrCreateStateSet();
    osg::Point *point = new osg::Point();
    point->setSize(2.f);
    state->setAttribute(point);
    return parent;
}

void ModelCylinder::initModelNode(const PCL::PointCloud &inliers, const pcl::ModelCoefficients &coeff)
{
    //для цилиндра должно быть 7 параметров
    Q_ASSERT(coeff.values.size() == 7);
    const int size = inliers.points.size();
    // какая-то точка, точно принадлежащая прямой
    Eigen::Vector4f line_pt  (coeff.values[0], coeff.values[1], coeff.values[2], 0);
    // вектор направления этой прямой в пространстве
    Eigen::Vector4f line_dir (coeff.values[3], coeff.values[4], coeff.values[5], 0);
    const float   ptdotdir = line_pt.dot (line_dir),
            dirdotdir = 1.0f / line_dir.dot (line_dir);
    float   t_min = std::numeric_limits<float>::max(),
            t_max = -std::numeric_limits<float>::max();
    Eigen::Vector4f max_proj, min_proj;
    // поиск границ цилиндра
    // да просто - спроецируем все точки на прямую и
    // найдем мин и мах
    for(int i = 0; i < size; i++){
        Eigen::Vector4f pt (inliers.points[i].x,
                                 inliers.points[i].y,
                                 inliers.points[i].z,
                                 1);
        // длина проекции точки pt на прямую (со знаком)
        float t = ( pt.dot (line_dir) - ptdotdir ) / dirdotdir;
        // проекция точки на прямую
        Eigen::Vector4f pt_proj = line_pt + t * line_dir;

        // ищем крайние точки
        if(t > t_max){
            max_proj = pt_proj;
            t_max = t;
        }
        if(t < t_min){
            min_proj = pt_proj;
            t_min = t;
        }
    }
    //высота
    float height = abs(t_max - t_min);
    //радиус
    float radius = coeff.values[6];
    //центр эллипса
    Eigen::Vector4f center( (max_proj + min_proj) / 2.f);

    unsigned int parts = 15;
    m_osgModelPtr = OSG::TransformPtr(_makePartialCylinder(height,radius,parts));
    // поворот цилиндра, по умолчанию он рисуется вверх по z
    // вот относительно этого направления и повернем его на нужное направление
    osg::Matrix rotate = osg::Matrix::rotate(osg::Vec3(0,0,1),osg::Vec3(line_dir[0],line_dir[1],line_dir[2]));
    // перенос цилиндра в его центр
    osg::Matrix translate = osg::Matrix::translate(osg::Vec3(center[0],center[1],center[2]));
    // применяем итоговую матрицу преобразования в таком порядке.
    m_osgModelPtr->setMatrix( rotate * translate );


    osg::StateSet* state = m_osgModelPtr->getOrCreateStateSet();
    state->setMode( GL_LIGHTING,osg::StateAttribute::OFF);

    osg::ref_ptr<osg::PolygonMode> pm = new osg::PolygonMode;
    pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::POINT);
    state->setAttribute(pm.get());

//    osg::ref_ptr<osg::Point> point = new osg::Point();
//    point->setSize(1.5f);
//    state->setAttribute(point.get());



//
//    state->setMode( GL_ALPHA_TEST, osg::StateAttribute::ON);
//    state->setMode( GL_BLEND, osg::StateAttribute::ON);
//    osg::ref_ptr<osg::Material> mat = new osg::Material;
//    mat->setAlpha(osg::Material::FRONT_AND_BACK, 0.5f);
//    mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 222 * scale, 184 * scale, 135 * scale, 1.0f ) );
//    mat->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( 0, 0, 0, 1.f ) );
//    state->setAttribute( mat.get() );
}

}//fitting
