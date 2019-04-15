#include "task.h"
namespace async{
Task::Task(QObject *parent) : QObject(parent)
{

}

Task::~Task()
{

}

Task *Task::Empty()
{
    return new TaskEmpty();
}

TaskEmpty::TaskEmpty(Task *parent): Task(parent){}

void TaskEmpty::run(){ return;/*ничего не деаем*/}

QString TaskEmpty::getName() {return tr("Empty");}

} //async
