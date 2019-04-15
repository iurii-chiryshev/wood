#ifndef FILELOADER_H
#define FILELOADER_H

#include <QObject>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>

#include "baseloader.h"

namespace io{

class FileLoader : public BaseLoader
{
public:
    explicit FileLoader(const QString &fileName = QString::Null(), BaseLoader *parent = 0);

    /**
     * @brief initFileDialog
     * @param dialog Диалог
     * Инициализация диалога
     */
    static void initFileDialog(QFileDialog &dialog);

private:
    /**
     * @brief m_filename
     * имя файла для загрузки
     */
    QString m_fileName;

    // Task interface
public:
    virtual void run();
    virtual QString getName();
    QString getFileName() const;
};

}


#endif // FILELOADER_H
