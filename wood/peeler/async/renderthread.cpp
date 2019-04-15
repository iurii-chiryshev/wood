#include "renderthread.h"
namespace async{
RenderThread::RenderThread(QThread *parent): QThread(parent),
    m_viewer(0)
{

}

RenderThread::~RenderThread()
{
     if (m_viewer) m_viewer->setDone(true);
     wait();
}

void RenderThread::setViewer(osgViewer::Viewer *pviewer)
{
    m_viewer = pviewer;
}


void RenderThread::run()
{
    if (m_viewer) m_viewer->run();
}


} //async
