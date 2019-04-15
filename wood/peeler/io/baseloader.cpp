#include "baseloader.h"

namespace io {

BaseLoader::BaseLoader(async::Task *parent): Task(parent),
    m_pclCloudPtr (new PCL::PointCloud)
{

}

BaseLoader::~BaseLoader()
{

}

PCL::PointCloudPtr BaseLoader::getPclCloudPtr() const
{
    return m_pclCloudPtr;
}

}// io


