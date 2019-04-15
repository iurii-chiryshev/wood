#include "scenemaker.h"
#include <QStandardPaths>
namespace io{

SceneMaker::SceneMaker(BaseLoader *loader, Task *parent): Task(parent),
    m_loaderPtr(loader),
    m_osgCloudPtr (new OSG::Geode),
    m_osgBboxPtr(new OSG::Node),
    m_osgTextcamPtr(OSG::createHUDCamera())
{
    QObject::connect(m_loaderPtr.data(),
            &BaseLoader::changed,
            this,
            &SceneMaker::changed);
}

SceneMaker::~SceneMaker()
{
    QObject::disconnect(m_loaderPtr.data(),
            &BaseLoader::changed,
            this,
            &SceneMaker::changed);
}

void SceneMaker::make(const PCL::PointCloud &cloud)
{
    emit changed(tr("Converting..."));
    osg::ref_ptr<osg::Geometry> geometry (new osg::Geometry());
    osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());
    osg::ref_ptr<osg::Vec4Array> colors (new osg::Vec4Array());

    OSG::cloud2vertices(cloud,*vertices,*colors);

    geometry->setVertexArray (vertices.get());
    geometry->setColorArray (colors.get());
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,vertices->size()));

    // облако точек
    m_osgCloudPtr = OSG::GeodePtr(new OSG::Geode());
    m_osgCloudPtr->addDrawable (geometry.get());
    geometry->setStateSet(OSG::makeStateSet(1.5f));

    //ограничивающий прямоугольник
    osg::Vec3 size, pos(25.0f, 25.0f, 0.0f);
    m_osgBboxPtr = OSG::makeBBox(cloud,size);
    //текст с размерами
    std::string text;
    text.append(" H: ").append(std::to_string(size.x()));
    text.append(" W: ").append(std::to_string(size.y()));
    text.append(" D: ").append(std::to_string(size.z()));
    text.append("\n Points: ").append(std::to_string(cloud.points.size()));
    m_osgTextcamPtr->addChild(OSG::makeTextGeoge(pos,text));
}


OSG::SwitchPtr SceneMaker::getOsgSwitchPtr(bool cloud, bool bbox) const
{
    OSG::SwitchPtr ret(new OSG::Switch());
    ret->addChild(m_osgCloudPtr.get(), cloud);

    OSG::GroupPtr group_bbox(new OSG::Group);
    group_bbox->addChild(m_osgBboxPtr.get());
    group_bbox->addChild(m_osgTextcamPtr.get());
    ret->addChild(group_bbox.get(),bbox);

    return ret;
}

PCL::PointCloudPtr SceneMaker::getPclCloudPtr() const
{
    return m_loaderPtr->getPclCloudPtr();
}

void SceneMaker::run()
{
    m_loaderPtr->run();
    make(*m_loaderPtr->getPclCloudPtr());
}

QString SceneMaker::getName()
{
    return tr("Make scene");
}
} // io
