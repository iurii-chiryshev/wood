#include "pcl.h"

namespace util {

void PCL::computeEigenVectors(const Eigen::Matrix3f &covariance_matrix, Eigen::Vector3f &major_axis, Eigen::Vector3f &middle_axis, Eigen::Vector3f &minor_axis, float &major_value, float &middle_value, float &minor_value)
{
    Eigen::EigenSolver <Eigen::Matrix3f > eigen_solver;
    eigen_solver.compute (covariance_matrix);

    Eigen::EigenSolver <Eigen::Matrix3f >::EigenvectorsType eigen_vectors;
    Eigen::EigenSolver <Eigen::Matrix3f >::EigenvalueType eigen_values;
    eigen_vectors = eigen_solver.eigenvectors ();
    eigen_values = eigen_solver.eigenvalues ();

    unsigned int temp = 0;
    unsigned int major_index = 0;
    unsigned int middle_index = 1;
    unsigned int minor_index = 2;

    if (eigen_values.real () (major_index) < eigen_values.real () (middle_index))
    {
        temp = major_index;
        major_index = middle_index;
        middle_index = temp;
    }

    if (eigen_values.real () (major_index) < eigen_values.real () (minor_index))
    {
        temp = major_index;
        major_index = minor_index;
        minor_index = temp;
    }

    if (eigen_values.real () (middle_index) < eigen_values.real () (minor_index))
    {
        temp = minor_index;
        minor_index = middle_index;
        middle_index = temp;
    }

    major_value = eigen_values.real () (major_index);
    middle_value = eigen_values.real () (middle_index);
    minor_value = eigen_values.real () (minor_index);

    major_axis = eigen_vectors.col (major_index).real ();
    middle_axis = eigen_vectors.col (middle_index).real ();
    minor_axis = eigen_vectors.col (minor_index).real ();

    major_axis.normalize ();
    middle_axis.normalize ();
    minor_axis.normalize ();

    float det = major_axis.dot (middle_axis.cross (minor_axis));
    if (det <= 0.0f)
    {
        major_axis (0) = -major_axis (0);
        major_axis (1) = -major_axis (1);
        major_axis (2) = -major_axis (2);
    }
}

void PCL::computeOBB(const PCL::PointCloud &cloud, Eigen::Vector3f &min_point, Eigen::Vector3f &max_point, Eigen::Vector3f &translate, Eigen::Matrix3f &rotate){

    Eigen::Vector4f centroid;
    Eigen::Matrix3f covariance_matrix;
    covariance_matrix.setZero();
    // расчет центра масс pcloud
    pcl::compute3DCentroid(cloud,centroid);
    // ковариационная матрица для pcloud
    pcl::computeCovarianceMatrix(cloud,centroid,covariance_matrix);

    Eigen::Vector3f major_axis (0.0f, 0.0f, 0.0f);
    Eigen::Vector3f middle_axis (0.0f, 0.0f, 0.0f);
    Eigen::Vector3f minor_axis (0.0f, 0.0f, 0.0f);
    float major_value = 0.0f;
    float middle_value = 0.0f;
    float minor_value = 0.0f;
    // собственные векторы и собственные значения
    computeEigenVectors(covariance_matrix,major_axis,middle_axis,minor_axis,major_value,middle_value,minor_value);


    min_point(0) = min_point(1) = min_point(2) = std::numeric_limits <float>::max ();

    max_point(0) = max_point(1) = max_point(2) = -std::numeric_limits <float>::max ();

    const unsigned int number_of_points = static_cast <unsigned int> (cloud.points.size());
    for (unsigned int i = 0; i < number_of_points; i++)
    {
        // перепроецируем каждую точку с учетом матрицы ковариации
        // ищем мин. и макс.
        float x = (cloud.points[i].x - centroid (0)) * major_axis (0) +
                (cloud.points[i].y - centroid (1)) * major_axis (1) +
                (cloud.points[i].z - centroid (2)) * major_axis (2);
        float y = (cloud.points[i].x - centroid (0)) * middle_axis (0) +
                (cloud.points[i].y - centroid (1)) * middle_axis (1) +
                (cloud.points[i].z - centroid (2)) * middle_axis (2);
        float z = (cloud.points[i].x - centroid (0)) * minor_axis (0) +
                (cloud.points[i].y - centroid (1)) * minor_axis (1) +
                (cloud.points[i].z - centroid (2)) * minor_axis (2);

        if (x <= min_point(0)) min_point(0) = x;
        if (y <= min_point(1)) min_point(1) = y;
        if (z <= min_point(2)) min_point(2) = z;

        if (x >= max_point(0)) max_point(0) = x;
        if (y >= max_point(1)) max_point(1) = y;
        if (z >= max_point(2)) max_point(2) = z;
    }
    // формируем матрицу поворота
    Eigen::Matrix3f r;
    r <<    major_axis (0), middle_axis (0), minor_axis (0),
            major_axis (1), middle_axis (1), minor_axis (1),
            major_axis (2), middle_axis (2), minor_axis (2);
    rotate = r;

    // середина отрезка Pmin Pmax
    Eigen::Vector3f shift ( (max_point(0) + min_point(0)) / 2.0f,
                            (max_point(1) + min_point(1)) / 2.0f,
                            (max_point(2) + min_point(2)) / 2.0f);
    // Pmin Pmax относительно центра
    min_point(0) -= shift (0);
    min_point(1)-= shift (1);
    min_point(2) -= shift (2);

    max_point(0) -= shift (0);
    max_point(1) -= shift (1);
    max_point(2) -= shift (2);

    Eigen::Vector3f mean ( centroid(0),
                           centroid(1),
                           centroid(2) );
    // формируем матрицу переноса
    translate = mean + rotate * shift;
}

}
