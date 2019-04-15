#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_inputPointCloudPtr(new PCL::PointCloud()),
    m_inputSwitchPtr(new OSG::Switch()),
    m_outputSwitchPtr(new OSG::Switch())
{
    ui->setupUi(this);

    // osg viever на центральную часть главного окна
    m_viewer = new OsgMultiViewerWidget();
    ui->centralLayout->addWidget(m_viewer);

    // инициализация прогресса как модального окна
    // min = max = 0 - на Windows прогресс крутится постоянно
    m_progressDlg = new QProgressDialog("", "", 0, 0, this);
    m_progressDlg->setCancelButton(NULL);
    m_progressDlg->setAutoClose(false);
    m_progressDlg->setWindowModality(Qt::WindowModal);
    m_progressDlg->setModal(true);
    m_progressDlg->close();

    //инициализация лога
    ui->textEdit->setTextColor(Qt::blue);

    // подцепляемся к сигналам фонового woker-а
    QObject::connect(&BGW,
            &BackgroundWorker::started,
            this,
            &MainWindow::slot_bgw_started);
    QObject::connect(&BGW,
            &BackgroundWorker::finished,
            this,
            &MainWindow::slot_bgw_finished);
    QObject::connect(&BGW,
            &BackgroundWorker::changed,
            this,
            &MainWindow::slot_bgw_changed);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog dialog(this, tr("Open File"));

    FileLoader::initFileDialog(dialog);
    if(dialog.exec() == QDialog::Accepted){
        BGW.start( new SceneMaker( new FileLoader( dialog.selectedFiles().first() ) ) );
    }
}

void MainWindow::slot_bgw_started(const Task::Ptr &taskPtr)
{
#ifndef NO_BACKGROUND
    m_progressDlg->setWindowTitle(taskPtr->getName());
    m_progressDlg->setLabelText("");
    m_progressDlg->show();
#endif
}

void MainWindow::slot_bgw_changed(QString str, int val)
{
#ifndef NO_BACKGROUND
    m_progressDlg->setLabelText(str);
    //if(val >= 0) m_progressDlg->setValue(val);
    m_progressDlg->show();
#endif
}

void MainWindow::slot_bgw_finished(const Task::Ptr &taskPtr)
{
    m_progressDlg->close();
    Task* task = taskPtr.data();
    if(task->inherits(SceneMaker::staticMetaObject.className())){
        //задача открытия ply файла
        SceneMaker *opl = qobject_cast<SceneMaker *>(task);
        m_inputPointCloudPtr = opl->getPclCloudPtr();
        m_inputSwitchPtr = opl->getOsgSwitchPtr(ui->actionShowSourceCloud->isChecked(),
                                        ui->actionShowBBox->isChecked());
        m_viewer->setDrawData(0,m_inputSwitchPtr.get());
        m_viewer->setDrawData(1,new OSG::Switch());

    }else if(task->inherits(Fitter::staticMetaObject.className())){
        // задача расчета модели
        Fitter *fitter = qobject_cast<Fitter *>(task);
        m_outputSwitchPtr = fitter->getModelPtr()->getOsgSwitchPtr(ui->actionShowInliersCloud->isChecked(),
                                                 ui->actionShowOutliersCloud->isChecked(),
                                                 ui->actionShowModel->isChecked());
        m_viewer->setDrawData(1,m_outputSwitchPtr.get());
    }

}


void MainWindow::on_actionShowInliersCloud_triggered(bool checked)
{
    updateOutputSwitch(0,checked);
}

void MainWindow::on_actionShowOutliersCloud_triggered(bool checked)
{
    updateOutputSwitch(1,checked);
}

void MainWindow::on_actionShowModel_triggered(bool checked)
{
    updateOutputSwitch(2,checked);
}

void MainWindow::updateOutputSwitch(unsigned int index, bool value)
{
    if(m_outputSwitchPtr->getNumChildren() > index){
        m_outputSwitchPtr->setValue(index,value);
        //todo refhesh
    }
}

void MainWindow::updateInputSwitch(unsigned int index, bool value)
{
    if(m_inputSwitchPtr->getNumChildren() > index){
        m_inputSwitchPtr->setValue(index,value);
        //todo refhesh
    }
}

void MainWindow::initFitterParams(FitterParams &params)
{
    // запихать все параметры из ui
    params.methodType = ui->comboBox_method->currentIndex();
    const int model_id = ui->comboBox_model->currentIndex();
    params.modelType = model_id == 0 ? pcl::SACMODEL_CYLINDER : (model_id == 1 ? pcl::SACMODEL_CONE : -1 ) ;
    params.threshold = ui->doubleSpinBox_thres->value();
    params.optimizeCoefficients = ui->checkBox_opt_coeff->isChecked();
    params.radiusMin = ui->doubleSpinBox_min_radi->value();
    params.radiusMax = ui->doubleSpinBox_max_radi->value();
    params.samplesRadius = ui->doubleSpinBox_sample_radi->value();
    params.maxIterations = ui->spinBox_max_iter->value();
    params.probability = ui->doubleSpinBox_prob->value();
    const bool is_knn_scope = ui->radioButton_knn->isChecked();
    const double ns_val = is_knn_scope ? ui->spinBox_knn->value() : ui->doubleSpinBox_rsearch->value();
    params.normalsSearchScope = is_knn_scope ? SEARCHSCOPE_KNN : SEARCHSCOPE_RADII;
    params.normalsSearchValue = ns_val;
    // углы переводим в радианы
    params.angleMin = DEG2RAD(ui->doubleSpinBox_min_angle->value());
    params.angleMax = DEG2RAD(ui->doubleSpinBox_max_angle->value());
}

void MainWindow::on_actionDoneModel_triggered()
{
    // запускаем fitter
    FitterParams params;
    initFitterParams(params);
    ui->textEdit->append(QString::fromUtf8(params.toString().c_str()));
    BGW.start(new Fitter(m_inputPointCloudPtr,params));
}

void MainWindow::on_actionShowBBox_triggered(bool checked)
{
    updateInputSwitch(1,checked);
}

void MainWindow::on_actionCreate_triggered()
{
    ModelParams params;
    BGW.start( new SceneMaker( new ModelLoader( params ) ) );

}
