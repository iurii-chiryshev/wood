/**
 * @file
 * @brief Заголовочный файл класса отрисовщика osg сцены
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * */

#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QObject>
#include <QThread>
#include <osgViewer/Viewer>
namespace async{
/**
 * @brief The RenderThread class
 * @note Не используется в QT ver. > 5
 * Класс поток-отрисовщик для osg сцены
 */
class RenderThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief RenderThread
     * @param parent
     */
    explicit RenderThread(QThread *parent = Q_NULLPTR);
    /**
     * @brief ~RenderThread
     */
    virtual ~RenderThread();
    /**
     * @brief setViewer
     * @param pviewer
     */
    void setViewer(osgViewer::Viewer *pviewer);

private:
    /**
     * @brief m_viewer
     */
    osgViewer::Viewer* m_viewer;


    // QThread interface
protected:
    /**
     * @brief run
     */
    virtual void run();
};
} //async
#endif // RENDERTHREAD_H
