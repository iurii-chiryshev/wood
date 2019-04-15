#include "modelloader.h"

#include <time.h>
#include <cmath>

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/ml/ml.hpp"

namespace io{

ModelLoader::ModelLoader(const ModelParams &params, BaseLoader *parent): BaseLoader(parent),
  m_params(params)
{

}

struct CylinderCoord{
    CylinderCoord(float _phi = 0, float _r = 0, float _z = 0): phi(_phi),r(_r),z(_z){}
    float phi;
    float r;
    float z;
};

static void _makePCloud(float radius_min,
                               float radius_max,
                               float length,
                               int npoints,
                               float sector,
                               float sigma,
                               PCL::PointCloud &cloud){
   CV_Assert(radius_min > 0 && radius_max >= radius_min && length > 0 && npoints > 0 && sector >= 0 && sigma >= 0);
   cloud.width    = npoints;
   cloud.height   = 1;
   cloud.is_dense = false;
   cloud.points.resize (cloud.width * cloud.height);
   cv::RNG rng(time(0));
   const float tg = (radius_max - radius_min) / length, hlength = length / 2;
   uchar red, green, blue;
   red = green = blue = 200;
   for(int i = 0; i < cloud.points.size(); i++){
        float phi = rng.uniform(0.f,sector);
        float z = rng.uniform(0.f,length);
        float r_mean = tg * z + radius_min; // truth
        float r_std = (float)rng.gaussian((double)sigma);
        float r = i % 2 == 0 ? r_mean + r_std : std::max(r_mean - r_std, 0.f);
        //to xyz
        cloud[i].x = r * cos(phi);
        cloud[i].y = r * sin(phi);
        cloud[i].z = z;
        // + rgb
        cloud[i].r = red;
        cloud[i].g = green;
        cloud[i].b = blue;
   }
}

void ModelLoader::run()
{
    emit changed(tr("Generate model"));
    _makePCloud(m_params.radius_min,
                m_params.radius_max,
                m_params.length,
                m_params.npoints,
                m_params.sector,
                m_params.sigma,*m_pclCloudPtr);
}

QString ModelLoader::getName()
{
    return tr("Model creator");
}




ModelParams::ModelParams(float _radius_min,
                             float _radius_max,
                             float _length,
                             int _npoints,
                             float _sector,
                             float _sigma):
    radius_min(_radius_min),
    radius_max(_radius_max),
    length(_length),
    npoints(_npoints),
    sector(_sector),
    sigma(_sigma)
{}

ModelParams ModelParams::Empty()
{
    return ModelParams();
}


}
