#include "fitter.h"
namespace fitting{

/***
 *
 * FitterParams
 *
 * */

const std::map<int,std::string> FitterParams::models = {
    { pcl::SACMODEL_CYLINDER, "cylinder" },
    { pcl::SACMODEL_CONE, "cone" }
    };

const std::map<int,std::string> FitterParams::methods = {
    { pcl::SAC_RANSAC, "RANSAC"},
    { pcl::SAC_LMEDS, "LMEDS"},
    { pcl::SAC_MSAC, "MSAC"},
    { pcl::SAC_RRANSAC, "RRANSAC"},
    { pcl::SAC_RMSAC, "RMSAC"},
    { pcl::SAC_MLESAC, "MLESAC"},
    { pcl::SAC_PROSAC, "PROSAC"}
    };

const std::map<int,std::string> FitterParams::searchScopes = {
    { SEARCHSCOPE_KNN, "KNN" },
    { SEARCHSCOPE_RADII, "Radius" }
    };



FitterParams::FitterParams(int _methodType,
                           int _modelType,
                           double _threshold,
                           bool _optimizeCoefficients,
                           double _radiusMin,
                           double _radiusMax,
                           double _samplesRadius,
                           double _epsAngle,
                           const Eigen::Vector3f &_axis,
                           int _maxIterations,
                           double _probability,
                           double _normalsSearchValue,
                           int _normalsSearchScope,
                           double _angleMin,
                           double _angleMax,
                           bool _random):
    methodType(_methodType),
    modelType(_modelType),
    threshold(_threshold),
    optimizeCoefficients(_optimizeCoefficients),
    radiusMin(_radiusMin),
    radiusMax(_radiusMax),
    samplesRadius(_samplesRadius),
    epsAngle(_epsAngle),
    axis(_axis),
    maxIterations(_maxIterations),
    probability(_probability),
    normalsSearchValue(_normalsSearchValue),
    normalsSearchScope(_normalsSearchScope),
    angleMin(_angleMin),
    angleMax(_angleMax),
    random(_random)
{

}

FitterParams FitterParams::Empty()
{
    return FitterParams();
}

std::string FitterParams::toString() const
{
    std::string str;
    str.append("*******************************************\n");
    str.append("* model_type: ").append(models.at(this->modelType)).append("\n");
    str.append("* method_type: ").append(methods.at(this->methodType)).append("\n");
    str.append("* threshold: ").append(std::to_string(this->threshold)).append("\n");
    str.append("* optimize_coefficients: ").append(std::to_string(this->optimizeCoefficients)).append("\n");
    str.append("* radius_min: ").append(std::to_string(this->radiusMin)).append("\n");
    str.append("* radius_max: ").append(std::to_string(this->radiusMax)).append("\n");
    str.append("* samples_radius: ").append(std::to_string(this->samplesRadius)).append("\n");
    str.append("* eps_angle: ").append(std::to_string(this->epsAngle)).append("\n");
    str.append("* max_iterations: ").append(std::to_string(this->maxIterations)).append("\n");
    str.append("* probability: ").append(std::to_string(this->probability)).append("\n");
    str.append("* normals_search_scope: ").append(searchScopes.at(this->normalsSearchScope)).append("\n");
    str.append("* normals_search_value: ").append(std::to_string(this->normalsSearchValue)).append("\n");
    str.append("* angle_min: ").append(std::to_string(this->angleMin)).append("\n");
    str.append("* angle_max: ").append(std::to_string(this->angleMax)).append("\n");
    str.append("* random: ").append(std::to_string(this->random)).append("\n");
    str.append("*******************************************\n");
    return str;
}


/***
 *
 * Fitter
 *
 * */
Fitter::Fitter(const PCL::PointCloudPtr &inputCloudPtr,
               const FitterParams &params,
               Task *parent): Task(parent),
    m_inputCloudPtr(inputCloudPtr), m_params(params)
{

}

PCL::PointCloudPtr Fitter::getInputCloudPtr()
{
    return m_inputCloudPtr;
}

Model::Ptr Fitter::getModelPtr()
{
    return m_modelPtr;
}

void Fitter::run()
{
    //создание выходной модели
    const int model_type = m_params.modelType;
    Q_ASSERT( (model_type == pcl::SACMODEL_CYLINDER) || (model_type == pcl::SACMODEL_CONE) );
    if(model_type == pcl::SACMODEL_CYLINDER){
        m_modelPtr = ModelCylinder::Ptr(new ModelCylinder());
    }else if(model_type == pcl::SACMODEL_CONE){
        m_modelPtr = ModelCone::Ptr(new ModelCone());
    }

    //проверка на допустипость задания поиска нормалей
    const int nsearch_scope = m_params.normalsSearchScope;
    Q_ASSERT( (nsearch_scope == SEARCHSCOPE_KNN) || (nsearch_scope == SEARCHSCOPE_RADII) );

    qDebug(m_params.toString().c_str());

    //уменьшить облако точек
    emit changed(tr("downsampling the number of points..."));
    PCL::PointCloudPtr cloud_filtered;
    if(m_inputCloudPtr->points.size() > 100000){
        cloud_filtered = PCL::PointCloudPtr(new PCL::PointCloud());
        pcl::VoxelGrid<PCL::Point> sor;
        sor.setInputCloud (m_inputCloudPtr);
        sor.setLeafSize (0.1f, 0.1f, 0.1f);
        sor.filter (*cloud_filtered);
    }else{
        cloud_filtered = m_inputCloudPtr;
    }


    //создаем и инициализируем сегментацию
    emit changed(tr("fitter initialization..."));
    pcl::SACSegmentationFromNormals<PCL::Point, PCL::Normal> seg;
    seg.setOptimizeCoefficients (m_params.optimizeCoefficients);
    seg.setModelType (m_params.modelType);
    seg.setMethodType (m_params.methodType);
    //seg.setNormalDistanceWeight (0.1); // todo
    seg.setMaxIterations (m_params.maxIterations);
    seg.setDistanceThreshold (m_params.threshold);
    seg.setRadiusLimits (m_params.radiusMin, m_params.radiusMax);
    if(m_params.modelType == pcl::SACMODEL_CONE){
        seg.setMinMaxOpeningAngle(m_params.angleMin, m_params.angleMax);
    }
    seg.setInputCloud (cloud_filtered);

        // считаем и инициализируем нормали
        emit changed(tr("compute normals..."));
        pcl::NormalEstimation<PCL::Point,PCL::Normal> ne;
        ne.setInputCloud (cloud_filtered);
        pcl::search::KdTree<PCL::Point>::Ptr tree (new pcl::search::KdTree<PCL::Point> ());
        ne.setSearchMethod (tree);
        PCL::NormalCloud::Ptr cloud_normals(new PCL::NormalCloud);
        if(nsearch_scope == SEARCHSCOPE_KNN){
            ne.setKSearch((int) m_params.normalsSearchValue);
        }else if(nsearch_scope == SEARCHSCOPE_RADII){
            ne.setRadiusSearch(m_params.normalsSearchValue);
        }
        ne.compute (*cloud_normals);

    // расчитываем модель
    emit changed(tr("compute model..."));
    seg.setInputNormals (cloud_normals);
    pcl::PointIndices::Ptr indices(new pcl::PointIndices);
    pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
    seg.segment (*indices, *coefficients);

    //отделяем модельные и немодельные точки
    PCL::PointCloudPtr in_cloud(new PCL::PointCloud());
    PCL::PointCloudPtr out_cloud(new PCL::PointCloud());
    pcl::ExtractIndices<PCL::Point> extract;
    extract.setInputCloud (cloud_filtered);
    extract.setIndices (indices);
    extract.setNegative (false);
    extract.filter(*in_cloud);
    extract.setNegative (true);
    extract.filter(*out_cloud);
    m_modelPtr->initInOutliersNodes(*in_cloud,*out_cloud);
    m_modelPtr->initModelNode(*in_cloud,*coefficients);


}

QString Fitter::getName()
{
    return tr("Model fitting");
}

} // fitting


