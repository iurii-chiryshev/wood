/**
 * @file
 * @brief Заголовочный файл с классом главного окна
 * @author Chiryshev Iurii <iurii.chiryshev@mail.ru>
 * */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QProgressDialog>
#include "io/scenemaker.h"
#include "io/fileloader.h"
#include "io/modelloader.h"
#include "ui/osgviewerwidget.h"
#include "ui/osgmultiviewerwidget.h"
#include "async/backgroundworker.h"
#include "fitting/fitter.h"
#include "fitting/modelcylinder.h"

namespace Ui {
class MainWindow;
}

using namespace async;
using namespace io;
using namespace fitting;
using namespace core;
/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief MainWindow
     * @param parent
     * ctor
     */
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    /**
     * @brief on_actionOpen_triggered
     * обработчик тыканья "Файл-Открыть"
     */
    void on_actionOpen_triggered();
    /**
     * @brief slot_bgw_started
     * @param str
     * Обработчик запуска фоновой задачи
     */
    void slot_bgw_started(const Task::Ptr &taskPtr);
    /**
     * @brief slot_bgw_changed
     * @param str
     * @param val
     * Обработчик изменения фоновой задачи
     */
    void slot_bgw_changed(QString str, int val);
    /**
     * @brief slot_bgw_finished
     * @param taskPtr
     * Обработчик завершения фоновой задачи
     */
    void slot_bgw_finished(const Task::Ptr& taskPtr);

    void on_actionShowInliersCloud_triggered(bool checked);

    void on_actionShowOutliersCloud_triggered(bool checked);

    void on_actionShowModel_triggered(bool checked);

    void on_actionDoneModel_triggered();

    void on_actionShowBBox_triggered(bool checked);

    void on_actionCreate_triggered();

private:
    /**
     * @brief ui
     */
    Ui::MainWindow *ui;

    PCL::PointCloudPtr m_inputPointCloudPtr;


    OSG::SwitchPtr m_inputSwitchPtr;
    OSG::SwitchPtr m_outputSwitchPtr;

    /**
     * @brief m_progressDlg
     * Диалог с иникакцией выполнения чего-то в фоне
     */
    QProgressDialog* m_progressDlg;
    /**
     * @brief m_osgViewer
     * вижет для 3d отрисовки через osg
     */
    OsgMultiViewerWidget* m_viewer;


    void updateOutputSwitch(unsigned int index, bool value);
    void updateInputSwitch(unsigned int index, bool value);
    /**
     * @brief initFitterParams
     * @param[in] params
     * Инициализация параметров
     */
    void initFitterParams(FitterParams &params);
};


#endif // MAINWINDOW_H
