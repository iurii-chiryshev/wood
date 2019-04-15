#ifndef MODELCYLINDER_H
#define MODELCYLINDER_H
#include "model.h"

namespace fitting{
/**
 * @brief The ModelCylinder class
 * Модель цилиндра
 */
class ModelCylinder : public Model
{
public:
    explicit ModelCylinder(Model *parent = 0);


    // Model interface
protected:
    virtual void initModelNode(const PCL::PointCloud &inliers, const pcl::ModelCoefficients &coeff);
};
} //fitting

#endif // MODELCYLINDER_H
