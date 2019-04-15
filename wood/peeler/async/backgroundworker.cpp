#include "backgroundworker.h"
namespace async{
/***
 *
 * BackgroundWorker
 *
 * */
BackgroundWorker::BackgroundWorker(QObject *parent) : QObject(parent)
{
    // регистрируем тип, который ходит в сигналах\слотах
    qRegisterMetaType<Task::Ptr>("Task::Ptr");
    // запускаем по одной задаче за раз
    m_threadPool.setMaxThreadCount(1);
}

BackgroundWorker::~BackgroundWorker()
{
    // ждем завершения задач в пуле
    m_threadPool.waitForDone();
}


void BackgroundWorker::start(Task *task /*= Task::Empty()*/)
{
#ifdef NO_BACKGROUND
    QSharedPointer<QRunnable> rPtr(createRunnable(task));
    rPtr->run();
#else
    m_threadPool.start(createRunnable(task));
#endif
}

bool BackgroundWorker::tryStart(Task *task /*= Task::Empty()*/)
{
#ifdef NO_BACKGROUND
    QSharedPointer<QRunnable> rPtr(createRunnable(task));
    rPtr->run();
    return true;
#else
    return m_threadPool.tryStart(createRunnable(task));
#endif

}

QRunnable *BackgroundWorker::createRunnable(Task *task /*= Task::Empty()*/)
{
    RunnableTask* proxy = new RunnableTask(task);
    proxy->setAutoDelete(true);

    // started
    QObject::connect(proxy,
                     &RunnableTask::started,
                     this,
                     &BackgroundWorker::started);
    //changed
    QObject::connect(proxy,
                     &RunnableTask::changed,
                     this,
                     &BackgroundWorker::changed);

    // finished
    QObject::connect(proxy,
                     &RunnableTask::finished,
                     this,
                     &BackgroundWorker::finished);

    return proxy;
}

/***
 *
 * RunnableTask
 *
 * */
RunnableTask::RunnableTask(Task *task/*= Task::Empty()*/, QObject *parent /*=0*/) : QObject(parent),
    m_taskPtr(task)
{
    QObject::connect(m_taskPtr.data(),
                     &Task::changed,
                     this,
                     &RunnableTask::changed);
}

RunnableTask::~RunnableTask()
{
    qDebug().nospace() << "[dtor " << this << "] disconnect changed signal from task: " << m_taskPtr.data();
    QObject::disconnect(m_taskPtr.data(),
                         &Task::changed,
                         this,
                         &RunnableTask::changed);
}

void RunnableTask::run()
{
    // сигнал о запуске задачи
    emit started(m_taskPtr);
    // выполняем
    m_taskPtr->run();
    // сигнал об останове задачи
    emit finished(m_taskPtr);
}

} //async






