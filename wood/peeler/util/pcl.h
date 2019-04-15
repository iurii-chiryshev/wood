/**
 * @file
 * @brief Заголовочный файл с классами-адаптерами PCL библиотеки
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * В проекте несколько библиотек работы с 3d. Одни нужны для отрисовки, другие, - для расчета.
 * Эти библиотеки огромные. Чтобы не запутатся во всем этом нужны точки входа в эти библиотеки,
 * притом, только тех вещей которые нужны
 * Адаптер над PCL lib
 * */
#ifndef UTIL_PCL_H
#define UTIL_PCL_H

#include <pcl/io/ply_io.h>
#include <pcl/point_types.h>
#include <pcl/sample_consensus/sac_model.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/common/pca.h>
#include <pcl/features/moment_of_inertia_estimation.h>

#include <pcl/filters/extract_indices.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/sample_consensus/sac_model_cylinder.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/features/normal_3d.h>
#include <pcl/search/kdtree.h>

#include <pcl/filters/voxel_grid.h>



namespace util {

/**
 * @brief The PCL struct
 * Структура с типами и методами PCL библиотеки.
 * В проекте несколько библиотек, в части расчета облак точек используется PCL.
 * Все что относится с PCL библиотеке: типы, static методы и пр.
 */
struct PCL
{
    typedef pcl::PointXYZRGBA Point;
    typedef pcl::PointCloud<Point> PointCloud;
    typedef PointCloud::Ptr PointCloudPtr;

    typedef pcl::Normal Normal;
    typedef pcl::PointCloud<Normal> NormalCloud;
    typedef NormalCloud::Ptr NormalCloudPtr;

    typedef pcl::SampleConsensusModel<Point> SAC_Model;
    typedef SAC_Model::Ptr SAC_ModelPtr;

    PCL(){}

    /**
     * @brief computeEigenVectors
     * @param[in] covariance_matrix ковариационная матрица
     * @param[out] major_axis вектор соответсвующий наибольшему собственному значению
     * @param[out] middle_axis вектор соответсвующий среднему собственному значению
     * @param[out] minor_axis вектор соответсвующий минимал. собственному значению
     * @param[out] major_value наибольшее собств. значение
     * @param[out] middle_value среднее собственное значение
     * @param[out] minor_value минимальное собственное значение
     * По ковариационной матрице рассчитать собственные значения и собственные векторы в R3
     * Смысл этих параметров примерно такой:
     * собственные значения - это длины осей трехмерного эллипсойда.
     * собственнные векторы - это системы координат, связанные с осями этого эллипсойда
     * Еще проще - это параметры на которые нужно повернуть и отмасштабировать ед. сферу,
     * чтобы получился эллипсойд.
     */
    static void computeEigenVectors (const Eigen::Matrix3f &covariance_matrix,
                                     Eigen::Vector3f &major_axis,
                                     Eigen::Vector3f &middle_axis,
                                     Eigen::Vector3f &minor_axis,
                                     float &major_value,
                                     float &middle_value,
                                     float &minor_value);


    /**
     * @brief computeOBB
     * @param[in] cloud облако точек
     * @param[out] min_point минимальная 3d точка (уже спроецированная на главные компоненты т.е. с учетом R + T)
     * @param[out] max_point каксимальная 3d точка (уже спроецированная на главные компоненты т.е. с учетом R + T)
     * @param[out] translate поворот
     * @param[out] rotate перенос
     * Расчет ограничивающего прямоугольника для облака точек
     */
    static void computeOBB(const PointCloud &cloud,
                            Eigen::Vector3f &min_point,
                            Eigen::Vector3f &max_point,
                            Eigen::Vector3f &translate,
                            Eigen::Matrix3f &rotate);
};

}



#endif // UTIL_PCL_H
