#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QDebug>
#include <QSharedPointer>

#include "util/osg.h"
#include "util/pcl.h"



namespace fitting{
using namespace pcl;
using namespace util;

class Fitter;

/**
 * @brief The Model class
 * Базовый класс параметрической модели.
 * От него порождаются классы более-менее осмысленных (реальных) классов
 * типа цилиндр, усеченый конус и пр. со своими алгоритмами расчета и представлления моделей.
 */
class Model : public QObject
{
    Q_OBJECT
public:
    friend class Fitter;

    typedef QSharedPointer<Model> Ptr;

    /**
     * @brief Model
     * @param parent
     * ctor
     */
    explicit Model(QObject *parent = 0);

    /**
     * @brief getSwitch
     * @param[in] inliers вкл/выкл видимость модельные точки облака
     * @param[in] outliers вкл/выкл видимость выбросы
     * @param[in] model вкл/выкл видимость представления модели
     * @note То ли баг, то ли еще какая фигня, но если все выставить в false, потом вообще ничего нельзя включить.
     * @return
     * Выдать ноду-переключатель (osg представление).
     * Модель, если она сформируется, содержит точки по которым она построиласть,
     * точки - выбросы (шумы) и саму модель. Все это возвращается с тем, что бы клиент где-то у себя
     * мог показывать/скрывать отдельные элементы сцены.
     *
     */
    OSG::SwitchPtr getOsgSwitchPtr(bool inliers = true, bool outliers = true, bool model = true) const;

signals:

public slots:

protected:
    /**
     * @brief m_incloud_geode
     * osg облака точек, вошедшие в модель (inliers)
     */
    OSG::GeodePtr m_osgInliersPtr;
    /**
     * @brief m_outcloud_geode
     * osg облака точек не вошедшие в модель (outliers)
     */
    OSG::GeodePtr m_osgOutliersPtr;

    /**
     * @brief m_model_geode
     * osg модель (цилиндр, обобщенный цилиндр, конус, усеченный конус и пр.)
     */
    OSG::TransformPtr m_osgModelPtr;

    /**
     * @brief initInOutliersNodes
     * @param[in] inliers plc точки inliers
     * @param[int] outliers pcl точки outliers
     * инициализазия двух облаков точек inliers/outliers
     * т.е. конвертируем PCL структуры в OSG ноды
     */
    void initInOutliersNodes(const PCL::PointCloud &inliers, const PCL::PointCloud &outliers);

    /**
     * @brief initModelNode
     * @param inliers точки, вошедшие в модель
     * @param coeff параметры модели (массив)
     * Сформировать представление результирующей модели по точкам входящим в модель.
     */
    virtual void initModelNode(const PCL::PointCloud &inliers,const pcl::ModelCoefficients &coeff) = 0;

};
} // fitting
#endif // MODEL_H
