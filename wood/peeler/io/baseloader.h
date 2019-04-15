#ifndef BASELOADER_H
#define BASELOADER_H

#include <QObject>

#include "async/task.h"
#include "util/pcl.h"

namespace io{

using namespace async;
using namespace util;

class BaseLoader : public Task
{
    Q_OBJECT
public:
    explicit BaseLoader(Task *parent = 0);

    virtual ~BaseLoader();

    virtual PCL::PointCloudPtr getPclCloudPtr() const;



signals:

public slots:

protected:
    /**
     * @brief cloud
     * pcl облако точек (smart point)
     */
    PCL::PointCloudPtr m_pclCloudPtr;
};

}



#endif // BASELOADER_H
