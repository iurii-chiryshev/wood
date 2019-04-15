#include "osgmultiviewerwidget.h"

OsgMultiViewerWidget::OsgMultiViewerWidget(QWidget *parent): QWidget(parent)
{
    this->setThreadingModel( osgViewer::Viewer::SingleThreaded );

    QWidget* widget1 = createAndAddView( util::OSG::createCamera());
    QWidget* widget2 = createAndAddView( util::OSG::createCamera());
    QGridLayout* grid = new QGridLayout;
    grid->addWidget( widget1, 0, 0 );
    grid->addWidget( widget2, 0, 1 );
    grid->setSpacing( 1 );
    grid->setMargin( 1 );
    setLayout( grid );

#if QT_VERSION >= 0x050000
    // Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
    connect( &m_timer, SIGNAL(timeout()), this, SLOT(update()) );
    m_timer.start( 20 );
#else
    m_renderThread.setViewer(this);
    m_renderThread.start();
#endif
}

void OsgMultiViewerWidget::setDrawData(unsigned int index, osg::Node* node)
{
    if(this->getNumViews() > index){
        this->getView(index)->setSceneData(node);
    }
}

void OsgMultiViewerWidget::paintEvent(QPaintEvent *event)
{
    this->frame();
}

QWidget *OsgMultiViewerWidget::createAndAddView(osg::Camera *camera)
{

    osgViewer::View* view = new osgViewer::View;
    this->addView( view );

    view->setCamera(camera);
    //this->setSceneData(new osg::Node());
    view->addEventHandler( new osgViewer::StatsHandler );
    view->setCameraManipulator(  new osgGA::TrackballManipulator );
    osgQt::GraphicsWindowQt* gw =  dynamic_cast<osgQt::GraphicsWindowQt*>( view->getCamera()->getGraphicsContext() );
    return gw->getGLWidget();
}


