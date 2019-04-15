#include "osgviewerwidget.h"

OsgViewerWidget::OsgViewerWidget(QWidget *parent) : QWidget(parent)
{
    this->setCamera(util::OSG::createCamera());
    //this->setSceneData(new osg::Node());
    this->addEventHandler( new osgViewer::StatsHandler );
    this->setCameraManipulator(  new osgGA::TrackballManipulator );
    this->setThreadingModel( osgViewer::Viewer::SingleThreaded );

    osgQt::GraphicsWindowQt* gw =  dynamic_cast<osgQt::GraphicsWindowQt*>( this->getCamera()->getGraphicsContext() );
    if ( gw ) {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget( gw->getGLWidget() );
        layout->setSpacing( 1 );
        layout->setMargin( 1 );
        setLayout( layout );
    }

#if QT_VERSION >= 0x050000
    // Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
    connect( &m_timer, SIGNAL(timeout()), this, SLOT(update()) );
    m_timer.start( 20 );
#else
    m_renderThread.setViewer(this);
    m_renderThread.start();
#endif

}

void OsgViewerWidget::paintEvent(QPaintEvent *event)  {
    //todo нужно проверять, чтоб окно было exposed, иначе:
    //QOpenGLContext::swapBuffers() called with non-exposed window, behavior is undefined
    this->frame();
}

void OsgViewerWidget::setPointCloud(osg::Geode *node){
    this->setSceneData(node);
}
