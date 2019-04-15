#include "osg.h"

namespace util {

OSG::Camera *OSG::createCamera(int x, int y, int w, int h) {
    osg::DisplaySettings* ds =    osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits =    new osg::GraphicsContext::Traits;
    traits->windowDecoration = false;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext(    new osgQt::GraphicsWindowQt(traits.get()) );
    camera->setClearColor( osg::Vec4(0.5f,0.5f,0.5f,1.f) );
    camera->setViewport( new osg::Viewport(    0, 0, traits->width, traits->height) );
    camera->setProjectionMatrixAsPerspective(    30.0f, static_cast<double>(traits->width)/      static_cast<double>(traits->height), 1.0f, 10000.0f );
    return camera.release();
}

OSG::Camera *OSG::createHUDCamera(double left, double right, double bottom, double top)
{
    OSG::CameraPtr camera_ptr = new osg::Camera;
    camera_ptr->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera_ptr->setClearMask( GL_DEPTH_BUFFER_BIT );
    camera_ptr->setRenderOrder( osg::Camera::POST_RENDER );
    camera_ptr->setAllowEventFocus( false );
    camera_ptr->setProjectionMatrix(    osg::Matrix::ortho2D(left, right, bottom, top) );
    camera_ptr->getOrCreateStateSet()->setMode(    GL_LIGHTING, osg::StateAttribute::OFF );
    return camera_ptr.release();
}

void OSG::blend(const osg::Vec4Array &src, osg::Vec4Array &dst,const osg::Vec4f &bg, float t /*= 0.5f*/)
{
    Q_ASSERT(&src != &dst);
    dst.clear();
    dst.reserve(src.size());
    //loop
    for(osg::Vec4Array::const_iterator it = src.begin(); it != src.end(); ++it){
        const osg::Vec4f &fg = *it;
        osg::Vec4f v = osg::componentMultiply(fg,fg) * (1 - t) + osg::componentMultiply(bg,bg) * t;
        dst.push_back(osg::Vec4f(sqrtf(v.r()),
                                 sqrtf(v.g()),
                                 sqrtf(v.b()),
                                 sqrtf(v.a())));
    }
}

void OSG::cloud2vertices(const PCL::PointCloud &cloud, osg::Vec3Array &vertices, osg::Vec4Array &colors)
{
    const int size = cloud.points.size();
    vertices.clear(); vertices.reserve(size);
    colors.clear(); colors.reserve(size);
    for (int i=0; i<size; i++) {
        vertices.push_back (osg::Vec3 (cloud.points[i].x, cloud.points[i].y, cloud.points[i].z));
        uint32_t rgb_val_;
        memcpy(&rgb_val_, &(cloud.points[i].rgba), sizeof(uint32_t));

        uint32_t red,green,blue;
        blue=rgb_val_ & 0x000000ff;
        rgb_val_ = rgb_val_ >> 8;
        green=rgb_val_ & 0x000000ff;
        rgb_val_ = rgb_val_ >> 8;
        red=rgb_val_ & 0x000000ff;

        colors.push_back (osg::Vec4f ((float)red/255.0f, (float)green/255.0f, (float)blue/255.0f,1.0f));
    }
}

osg::Geometry *OSG::makeGeometry(const PCL::PointCloud &cloud, const osg::Vec4f &bg, float t){
    osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry());
    osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
    osg::ref_ptr<osg::Vec4Array> tmp_colors(new osg::Vec4Array()),
            colors(new osg::Vec4Array());
    OSG::cloud2vertices(cloud,*vertices,*tmp_colors);
    if( (bg == osg::Vec4f()) || (t < 10e-6) ){
        //не указан цвет, с каким подмешивать, или маленький коэффициент значит не хотят
        colors = tmp_colors;
    }else{
        //подмешаем
        OSG::blend(*tmp_colors,*colors,bg,t);
    }
    geometry->setVertexArray (vertices.get());
    geometry->setColorArray (colors.get());
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,vertices->size()));
    return geometry.release();
}

OSG::Text *OSG::makeText(const osg::Vec3 &pos,
                         const std::string &content,
                         float size,
                         const osg::Vec4 &color) {
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setDataVariance( osg::Object::DYNAMIC );
    //text->setFont( font.get() );
    text->setCharacterSize( size );
    text->setColor(color);
    text->setAxisAlignment( osgText::TextBase::XY_PLANE );
    text->setPosition( pos );
    text->setText( content );
    return text.release();
}

OSG::Geode *OSG::makeTextGeoge(const osg::Vec3 &pos,
                               const std::string &content,
                               float size,
                               const osg::Vec4 &color)
{
    OSG::GeodePtr g_ptr(new OSG::Geode());
    g_ptr->addDrawable(makeText(pos,content,size,color));
    return g_ptr.release();
}

OSG::NodePtr OSG::makeBBox(const PCL::PointCloud &cloud, osg::Vec3 &size)
{

    size.set(0,0,0);
    OSG::GeodePtr geode = OSG::GeodePtr(new OSG::Geode());
    OSG::TransformPtr node = OSG::TransformPtr(new OSG::Transform());
    node->addChild(geode);
    if(cloud.points.size() < 2) return node;

    //Расчет моментов инерции
    Eigen::Vector3f min_point, max_point, e_translate;
    Eigen::Matrix3f e_rotate;
    PCL::computeOBB(cloud,min_point, max_point, e_translate, e_rotate);
    //    pcl::MomentOfInertiaEstimation<PCL::Point> feature_extractor;
    //    feature_extractor.setInputCloud (cloud_ptr);
    //    feature_extractor.compute();
    //    feature_extractor.getOBB (min_point, max_point, e_translate, e_rotate);

    //цвет
    osg::ref_ptr<osg::Vec4Array> colors ( new osg::Vec4Array( 1,new osg::Vec4( 0.93f, 0.67f, 0.49f, 1.f) ) );

    //  точки куба
    osg::Vec3 points[8] = {
        osg::Vec3(min_point(0), min_point(1), min_point(2)),
        osg::Vec3(min_point(0), min_point(1), max_point(2)),
        osg::Vec3(max_point(0), min_point(1), max_point(2)),
        osg::Vec3(max_point(0), min_point(1), min_point(2)),
        osg::Vec3(min_point(0), max_point(1), min_point(2)),
        osg::Vec3(min_point(0), max_point(1), max_point(2)),
        osg::Vec3(max_point(0), max_point(1), max_point(2)),
        osg::Vec3(max_point(0), max_point(1), min_point(2))
    };
    osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());
    for(int i = 0 ; i < 8 ; i++){
        vertices->push_back(points[i]);
    }
    // линии куба
    osg::ref_ptr<osg::DrawElementsUInt> lines = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES);
    lines->push_back(0); lines->push_back(1);
    lines->push_back(0); lines->push_back(3);
    lines->push_back(0); lines->push_back(4);
    lines->push_back(4); lines->push_back(5);
    lines->push_back(4); lines->push_back(7);
    lines->push_back(1); lines->push_back(5);
    lines->push_back(5); lines->push_back(6);
    lines->push_back(6); lines->push_back(7);
    lines->push_back(1); lines->push_back(2);
    lines->push_back(3); lines->push_back(7);
    lines->push_back(2); lines->push_back(3);
    lines->push_back(2); lines->push_back(6);

    //поворот перенос
    Eigen::Quaternionf q (e_rotate);
    osg::Matrix rotate = osg::Matrix::rotate(osg::Quat(q.x(),q.y(),q.z(),q.w()));
    osg::Matrix translate = osg::Matrix::translate(osg::Vec3(e_translate(0),e_translate(1),e_translate(2)));
    // применяем итоговую матрицу преобразования в таком порядке.
    node->setMatrix( rotate * translate );

    // заполняем
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setColorArray(colors.get());
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    geom->setVertexArray( vertices.get() );
    geom->addPrimitiveSet( lines.get() );
    osg::StateSet* state = geom->getOrCreateStateSet();
    state->setMode( GL_LIGHTING,osg::StateAttribute::OFF);
    osg::ref_ptr<osg::PolygonMode> pm = new osg::PolygonMode;
    pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
    state->setAttribute(pm.get());
    geode->addDrawable( geom );

    size.set(max_point(0) - min_point(0),
             max_point(1) - min_point(1),
             max_point(2) - min_point(2));
    return node;
}

osg::StateSet *OSG::makeStateSet(float point_size)
{
    osg::StateSet *state = new osg::StateSet();
    // размер точек и пр. ерунда
    osg::Point *point = new osg::Point();
    point->setSize(point_size);
    state->setAttribute(point);
    state->setMode( GL_LIGHTING,osg::StateAttribute::OFF);
    return state;

}

}//namespace util
