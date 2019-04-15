#ifndef MODELCONE_H
#define MODELCONE_H

#include "model.h"


namespace fitting{

/**
 * @brief The ModelCone class
 * Модель усеченого конуса
 */
class ModelCone : public Model
{
public:
    explicit ModelCone(Model *parent = 0);

signals:

public slots:

    // Model interface
protected:
    virtual void initModelNode(const PCL::PointCloud &inliers, const ModelCoefficients &coeff);
};
} //fitting
#endif // MODELCONE_H
