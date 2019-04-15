#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <QObject>
#include "baseloader.h"

namespace io{

struct ModelParams{
    ModelParams(float _radius_min = 9.0f,
                    float _radius_max = 10.0f,
                    float _length = 100.0f,
                    int _npoints = 4000,
                    float _sector = (float)M_PI,
                    float _sigma = 0.5f);
    static ModelParams Empty();
    float radius_min;
    float radius_max;
    float length;
    int npoints;
    float sector;
    float sigma;
};

class ModelLoader : public BaseLoader
{
    Q_OBJECT
public:
    explicit ModelLoader(const ModelParams& params = ModelParams::Empty(),  BaseLoader *parent = 0);

signals:

public slots:

    // Task interface
public:
    virtual void run();
    virtual QString getName();

private:
    ModelParams m_params;
};

}// io

#endif // MODELLOADER_H
