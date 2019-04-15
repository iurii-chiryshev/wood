/**
 * @file
 * @brief Заголовочный файл интерфейса "фоновой" задачи
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * */

#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QSharedPointer>
namespace async{

/**
 * @brief The Task class
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * Абстрактный класс задачи для выполнения в background-е
 */
class Task : public QObject
{
    Q_OBJECT
private:
    Q_DISABLE_COPY(Task)
public:
    /**
     * @brief AbstractTask
     * @param parent
     * ctor
     */
    explicit Task(QObject *parent = 0);
    /**
     * @brief ~Task
     * dtor
     */
    virtual ~Task();
    /**
     * @brief run
     * @code
     * // example
     * void run(){
     *  emit changed("subtask 1...",50);
     *  //do subtask1
     *  ...
     *  emit changed("subtask 2...",100);
     *  //do subtask2
     *  ...
     *  return;
     * }
     * @endcode
     * main executor
     */
    virtual void run() = 0;
    /**
     * @brief getName
     * @return
     * Получить имя задачи.
     * Это имя будет светиться возле progressbar-а для пользователя
     */
    virtual QString getName() = 0;

    /**
     * @brief Empty
     * @return
     */
    static Task *Empty();

    /**
     * @brief Ptr
     * Синоним для умного указателя
     */
    typedef QSharedPointer<Task> Ptr;

signals:
    /**
     * @brief changed
     * @param[in] name имя текущей подзадачи в рамках всей задачи
     * @param[in] percent процент выполненого
     * Вполне возможно, что задача состоит из нескольких подзадач.
     * Этот сигнал о смене подзадачи, если есть необходимость уведомить пользователя.
     */
    void changed(QString name, int percent = -1 /*< 0 not used*/);

public slots:

};

/**
 * @brief The Empty class
 * Пустая задача. Null object (Шаблон проектирования)
 */
class TaskEmpty: public Task{
public:
    /**
     * @brief TaskEmpty
     * @param parent
     * ctor
     */
    explicit TaskEmpty(Task *parent = 0);
    /**
     * @brief run
     */
    void run();
    /**
     * @brief getName
     * @return
     */
    QString getName();
};


} //async
#endif // TASK_H
