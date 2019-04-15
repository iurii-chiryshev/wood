/**
 * @file
 * @brief Заголовочный файл с классом выполнителя операций в отдельном потоке
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * */

#ifndef BACKGROUNDWORKER_H
#define BACKGROUNDWORKER_H

#include <QObject>
#include <QThreadPool>
#include <QDebug>
#include <QSharedPointer>
#include <QRunnable>
#include "core/singleton.h"
#include "task.h"

namespace async{
/**
 * @brief The BackgroundWorker class
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * Позволяет выполнять операции в отдельном, выделенном потоке.
 */
using namespace core;

class BackgroundWorker : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief start
     * @param[in] task указатель на задачу
     * @note Не удаляйте самостоятельно указатель на задачу. Используются умные указатели.
     * Положить задачу в очередь на исполнение.
     * Притом неважно есть свободный поток или нет.
     * Задача запустится как только кто-нибудь освободится.
     */
    void start(Task *task = Task::Empty());
    /**
     * @brief tryStart
     * @param task
     * @note Не удаляйте самостоятельно указатель на задачу. Используются умные указатели.
     * @return true - если есть свободный поток для выполнения
     * Попытаться положить и выполнить задачу в очередь.
     */
    bool tryStart(Task *task = Task::Empty());

signals:
    /**
     * @brief started
     * @param[in] name имя задачи
     * Сигнал о начале выполнения задачи
     */
    void started(const Task::Ptr &task);
    /**
     * @brief changed
     * @param[in] name
     * @param[in] percent
     */
    void changed(QString name, int percent = -1);
    /**
     * @brief finished
     * @param[in] task умный указатель на задачу, которая была выполнена
     * Сигнал о завершении задачи
     */
    void finished(const Task::Ptr &task);

private slots:

private:
    /**
     * @brief m_threadPool
     * пул потоков - исполнителей фоновых зачач
     */
    QThreadPool m_threadPool;
    /**
     * @brief createRunnable
     * @param[in] task умный указатель на задачу, которая была выполнена
     * @return runnable object
     * Обернуть задачу в QRunnable интерфейс, т.к. QThreadPool работает только с ним.
     */
    QRunnable* createRunnable(Task *task = Task::Empty());

    /**
     * @brief BackgroundWorker
     * @param parent
     * ctor
     */
    explicit BackgroundWorker(QObject *parent = 0);

    ~BackgroundWorker();

    friend class Singleton<BackgroundWorker>;
};

#define BGW Singleton<BackgroundWorker>::instance()


/**
 * @brief The TaskWrap class
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * Обертка над задачей - он непосредственно выполняет задачу в потоке.
 * Proxy (Шаблон проектирования)
 */
class RunnableTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    /**
     * @brief RunnableTask
     * @param[in] task
     * @param[in] parent
     * ctor
     */
    explicit RunnableTask(Task *task = Task::Empty(),QObject *parent = 0);
    /**
     * @brief ~TaskWrap
     * dtor
     */
    virtual ~RunnableTask();

    /**
     * @brief run
     * implementation QRunnable
     */
    void run();

signals:
    /**
     * @brief started
     * @param name имя задачи
     * Задача началась
     */
    void started(const Task::Ptr &task);
    /**
     * @brief changed
     * @param[in] name
     * @param[in] percent
     * Изменилась подзадача
     */
    void changed(QString name, int percent = -1);
    /**
     * @brief finished
     * @param[in] task задача
     * Задача завершилась
     */
    void finished(const Task::Ptr& task);

private slots:

private:
    /**
     * @brief m_task
     * сама задача
     */
    Task::Ptr m_taskPtr;


public:
};

} //async
#endif // BACKGROUNDWORKER_H
