#include "fileloader.h"

namespace io{

FileLoader::FileLoader(const QString &fileName, io::BaseLoader *parent): BaseLoader(parent),
    m_fileName(fileName)
{

}

void FileLoader::initFileDialog(QFileDialog &dialog)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList filesLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
        dialog.setDirectory(filesLocations.isEmpty() ? QDir::currentPath() : filesLocations.last());
    }
    dialog.setNameFilter("*.ply");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
}

QString FileLoader::getFileName() const
{
    return m_fileName;
}

void FileLoader::run()
{
    emit changed(tr("Loading..."));
    PCL::PointCloudPtr cloud_ptr (new PCL::PointCloud());
    if (pcl::io::loadPLYFile<PCL::Point>(m_fileName.toStdString(), *cloud_ptr) != -1){
        m_pclCloudPtr = cloud_ptr;
    }
}

QString FileLoader::getName()
{
    return tr("PLY loader");
}

}// io




