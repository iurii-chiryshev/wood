/**
 * @file
 * @brief Заголовочный файл с классом ply загрузчика
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * */

#ifndef OSGPLYLOADER_H
#define OSGPLYLOADER_H

#include <QObject>
#include <QFileDialog>
#include <QDebug>

#include "async/task.h"
#include "fitting/model.h"
#include "util/osg.h"
#include "util/pcl.h"

#include "baseloader.h"

namespace io{
using namespace async;
using namespace fitting;
using namespace util;
/**
 * @brief The SceneMaker class
 * Класс загрузчика облака точек в ply формате
 */
class SceneMaker: public Task
{
    Q_OBJECT
public:
    /**
     * @brief SceneMaker
     * @param loader
     * @param parent
     */
    explicit SceneMaker(BaseLoader * loader,Task *parent = 0);

    virtual ~SceneMaker();

private:

    QSharedPointer<BaseLoader> m_loaderPtr;
    /**
     * @brief m_osgCloudPtr
     * osg облако точек (smart point)
     */
    OSG::GeodePtr m_osgCloudPtr;

    OSG::NodePtr m_osgBboxPtr;

    OSG::CameraPtr m_osgTextcamPtr;

    void make(const PCL::PointCloud &cloud);


public:
    /**
     * @brief run
     * AbstractTask interface
     */
    void run();
    /**
     * @brief getName
     * @return
     * AbstractTask interface
     */
    QString getName();

    /**
     * @brief getPclCloudPtr
     * @return pcl облака точек
     */
    PCL::PointCloudPtr getPclCloudPtr() const;
    /**
     * @brief getOsgSwitchPtr
     * @return osg облака точек
     */
    OSG::SwitchPtr getOsgSwitchPtr(bool cloud = true, bool bbox = true) const;
};

} //io
#endif // OSGPLYLOADER_H
