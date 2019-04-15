#ifndef OSGMULTIVIEWERWIDGET_H
#define OSGMULTIVIEWERWIDGET_H

#include <QWidget>
#include<QGridLayout>
#include <QTimer>

#include "util/osg.h"

#if QT_VERSION >= 0x050000
    // Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
    #include <QTimer>
#else
    #include "async/renderthread.h"
#endif

class OsgMultiViewerWidget  : public QWidget, protected osgViewer::CompositeViewer
{
    Q_OBJECT
public:
    explicit OsgMultiViewerWidget(QWidget *parent = 0);

    /**
     * @brief setPointCloud
     * @param node
     */
    void setDrawData(unsigned int index, osg::Node* node);

protected:
    /**
     * @brief paintEvent
     * @param event
     * Переопределенный метод перерисовки виджета
     */
    virtual void paintEvent( QPaintEvent* event );

#if QT_VERSION >= 0x050000
    /**
     * @brief m_timer
     * @note Только для qt > 5
     * Кривая реализация спешиал фо кутэ 5.x
     * Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
     */
    QTimer m_timer;
#else
    /**
     * @brief m_renderThread
     * @note Правильный но не работающий метод в qt 5.x
     */
    RenderThread m_renderThread;
#endif

private:
    QWidget *createAndAddView( osg::Camera* camera );

};

#endif // OSGMULTIVIEWERWIDGET_H
