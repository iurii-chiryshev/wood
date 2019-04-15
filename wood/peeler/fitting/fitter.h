#ifndef FITTER_H
#define FITTER_H

#include <QObject>
#include <QSharedPointer>

#include "model.h"
#include "modelcylinder.h"
#include "modelcone.h"
#include "async/task.h"



namespace fitting{
using namespace async;
using namespace pcl;
using namespace util;

/**
 * @brief The NormSearch enum
 * Какой параметр задается при поиске в облаке точек
 */
enum SearchScope{
    SEARCHSCOPE_KNN, // задаем количество ближайших соседей
    SEARCHSCOPE_RADII, // задаем радиус поиска
};

/**
 * @brief The FitterParams struct
 * Параметры подгонки модели
 */
struct FitterParams{

    /**
     * @brief FitterParams
     * @param _method_type метод подгонки
     * @param _model_type модель под которую подгоняем облако точек
     * @param _threshold
     * @param _optimize_coefficients
     * @param _radius_min
     * @param _radius_max
     * @param _samples_radius
     * @param _eps_angle
     * @param _axis
     * @param _max_iterations
     * @param _probability
     * @param _normals_search_value
     * @param _normals_search_scope
     * @param _angle_min
     * @param _angle_max
     * @param _random
     */
    FitterParams(int _methodType = pcl::SAC_RANSAC,
                 int _modelType = pcl::SACMODEL_CYLINDER,
                 double _threshold = 0.1,
                 bool _optimizeCoefficients = true,
                 double _radiusMin = -std::numeric_limits<double>::max(),
                 double _radiusMax = std::numeric_limits<double>::max(),
                 double _samplesRadius = 0.0,
                 double _epsAngle = 0.0,
                 const Eigen::Vector3f &_axis = Eigen::Vector3f::Zero(),
                 int _maxIterations = 10000,
                 double _probability = 0.99,
                 double _normalsSearchValue = 0.1,
                 int _normalsSearchScope = SEARCHSCOPE_RADII,
                 double _angleMin = -std::numeric_limits<double>::max(),
                 double _angleMax = std::numeric_limits<double>::max(),
                 bool _random = false);
    int methodType;
    int modelType;
    double threshold;
    bool optimizeCoefficients;
    double radiusMin;
    double radiusMax;
    double samplesRadius;
    //samples_radius_search
    double epsAngle;
    Eigen::Vector3f axis;
    int maxIterations;
    double probability;
    double normalsSearchValue;
    int normalsSearchScope;
    double angleMin;
    double angleMax;
    bool random;

    static FitterParams Empty();

    const static std::map<int,std::string> models;

    const static std::map<int,std::string> methods;

    const static std::map<int,std::string> searchScopes;

    std::string toString() const;

};

/**
 * @brief The Fitter class
 */
class Fitter : public Task
{
    Q_OBJECT
public:
    /**
     * @brief Fitter
     * @param[in] input_cloud входное облако точек
     * @param[in] params параметры подгонки
     * @param[in] parent qt предок
     */
    explicit Fitter(const PCL::PointCloudPtr &inputCloudPtr,
                    const FitterParams &params = FitterParams::Empty(),
                    Task *parent = 0);
    /**
     * @brief getModel
     * @return
     * Выдать модель.
     * В ней лежат и модельные данные (размеры, объемы и пр.)
     * и графичекое представление
     */
    Model::Ptr getModelPtr();
    /**
     * @brief getInputCloud
     * @return
     * Выдать исходное облако точек, по которому стоилась модель
     */
    PCL::PointCloudPtr getInputCloudPtr();

signals:

public slots:

protected:
    /**
     * @brief m_model
     * модель
     */
    Model::Ptr m_modelPtr;
    /**
     * @brief m_input_cloud
     * облако точек
     */
    PCL::PointCloudPtr m_inputCloudPtr;
    /**
     * @brief m_params
     * параметры для построения
     */
    FitterParams m_params;


    // Task interface
public:
    virtual void run();
    virtual QString getName();
};



} // fitting
#endif // FITTER_H
