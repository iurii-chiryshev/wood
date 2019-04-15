/**
 * @file
 * @brief Заголовочный файл osg виджета.
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * */

#ifndef OSGVIEWERWIDGET_H
#define OSGVIEWERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>

#include "util/osg.h"


#if QT_VERSION >= 0x050000
    // Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
    #include <QTimer>
#else
    #include "async/renderthread.h"
#endif

/**
 * @brief The OsgViewerWidget class
 * Класс для отрисовки osg в qt
 */
class OsgViewerWidget : public QWidget, protected osgViewer::Viewer
{
    Q_OBJECT
public:
    /**
     * @brief OsgViewerWidget
     * @param parent
     * ctor
     */
    explicit OsgViewerWidget(QWidget *parent = 0);

    /**
     * @brief setPointCloud
     * @param node
     */
    void setPointCloud(osg::Geode* node);

signals:

public slots:
protected:
    /**
     * @brief paintEvent
     * @param event
     * Переопределенный метод перерисовки виджета
     */
    virtual void paintEvent( QPaintEvent* event );
private:
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

};

#endif // OSGVIEWERWIDGET_H
