#include "model.h"
namespace fitting{
Model::Model(QObject *parent) : QObject(parent),
    m_osgInliersPtr(new OSG::Geode()),
    m_osgOutliersPtr(new OSG::Geode()),
    m_osgModelPtr(new OSG::Transform())
{

}

OSG::SwitchPtr Model::getOsgSwitchPtr(bool inliers /*= true*/,
                                  bool outliers /*= true*/,
                                  bool model /*= true*/) const
{
    // собираем switch node
    OSG::SwitchPtr ret(new OSG::Switch());
    ret->addChild(m_osgInliersPtr.get(), inliers);
    ret->addChild(m_osgOutliersPtr.get(), outliers);
    ret->addChild(m_osgModelPtr.get(),model);
    return ret;
}




void Model::initInOutliersNodes(const PCL::PointCloud &inliers, const PCL::PointCloud &outliers)
{
    osg::ref_ptr<osg::Geometry> in_geometry (OSG::makeGeometry(inliers/*, osg::Vec4f(0,1,0,1)*/ ) );
    osg::ref_ptr<osg::Geometry> out_geometry (OSG::makeGeometry(outliers, osg::Vec4f(1,0,0,1) ) ); // + red

    m_osgInliersPtr = OSG::GeodePtr(new OSG::Geode());
    m_osgInliersPtr->addDrawable (in_geometry.get());
    in_geometry->setStateSet(OSG::makeStateSet(1.5f));

    m_osgOutliersPtr = OSG::GeodePtr(new OSG::Geode());
    m_osgOutliersPtr->addDrawable (out_geometry.get());
    out_geometry->setStateSet(OSG::makeStateSet(1.5f));
}
} //fitting
