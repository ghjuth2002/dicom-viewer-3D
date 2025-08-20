#include "SceneView.h"
#include "EventHandler.h"

#include <string>

#include <QThread>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QList>

#include <vtkCamera.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkDICOMImageReader.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkMarchingCubes.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAlgorithm.h>
#include <vtkSTLWriter.h>
#include <vtkConnectivityFilter.h>
#include <vtkIdTypeArray.h>
#include <vtkSelectionNode.h>
#include <vtkExtractSelection.h>
#include <vtkExtractVOI.h>
#include <vtkIntArray.h>
#include <vtkCellData.h>
#include <vtkPointSet.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkFillHolesFilter.h>

// -------------------------------------------------------------
//                        Implementation
// -------------------------------------------------------------

// ---------------------- Class declaration --------------------
class SceneView::Impl
{
public:
    enum ModelToSave
    {
        AllModel,
        Layers // Currently unused
    };
    void construct(SceneView* self);
    ~Impl() {};

    void openDicomFolder(const QString& path);
    void renderModelMapper();
    void removeActors();
    void saveToStl(ModelToSave, const std::string &fileName);
    void setConfig(const Config& config);
    Config config();
    Dimmensions dimmensions();
    void buildModel();

private:
    QString m_dicomPath;

    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkCamera> m_camera;
    vtkSmartPointer<vtkActor> m_modelActor;
    vtkSmartPointer<vtkPolyDataMapper> m_dicomMapper;;
    vtkSmartPointer<vtkDICOMImageReader> m_dicomReader;
    vtkSmartPointer<vtkPolyData> m_modelPolydata;

    ModelToSave m_modelToSave{ AllModel };
    std::string m_nameToSave = "model.stl";

    Config m_config;
    Dimmensions m_dimmensions;

    class Worker;
    SceneView* m_self;
};

class SceneView::Impl::Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(SceneView::Impl* owner);

public slots:
    void parseDicom();
    void saveToStl();
    void buildLungs();
    void buildBones();
    void buildSkin();

signals:
    void finished();

private:
    SceneView::Impl* m_owner;
};

//-------------------- Interactions ---------------------------

class ProgressCallback : public vtkCommand
{
public:
    static ProgressCallback* New() { return new ProgressCallback; }

    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override
    {
        vtkAlgorithm* filter = dynamic_cast<vtkAlgorithm*>(caller);
        if (filter)
        {
            double progress = filter->GetProgress();
            emit EventHandler::instance().parseDicomProgressChanged(progress);
        }
    }
};

// ----------- SceneView::Impl implementation ------------------

void SceneView::Impl::construct(SceneView *self)
{
    m_self = self;

    m_modelActor = vtkSmartPointer<vtkActor>::New();
    m_camera = vtkSmartPointer<vtkCamera>::New();
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_dicomMapper = vtkSmartPointer<vtkPolyDataMapper> ::New();
    m_dicomReader = vtkSmartPointer<vtkDICOMImageReader> ::New();
    m_modelPolydata = vtkSmartPointer<vtkPolyData> ::New();

    m_camera->SetViewUp(0, 1, 0);
    m_camera->SetPosition(0, 0, 10);
    m_camera->SetFocalPoint(0.7, 0.7, 0.7);

    m_renderer->SetActiveCamera(m_camera);
    m_renderer->SetBackground(0.12549, 0.12549, 0.12549);
    m_self->renderWindow()->AddRenderer(m_renderer);
}

void SceneView::Impl::openDicomFolder(const QString &path)
{
    QThread* thread = new QThread;
    Worker* worker = new Worker(this);

    m_dicomPath = path;

    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started, worker, &Worker::parseDicom);
    QObject::connect(worker, &Worker::finished, thread, &QThread::quit);
    QObject::connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void SceneView::Impl::renderModelMapper()
{
    m_renderer->RemoveActor(m_modelActor);
    m_self->renderWindow()->Render();

    m_modelActor->SetMapper(m_dicomMapper);
    m_modelActor->GetMapper()->SetScalarVisibility(false);
    m_modelActor->GetProperty()->SetColor(0.9, 0.15, 0.1);

    m_renderer->AddActor(m_modelActor);
    m_renderer->ResetCamera(m_dicomMapper->GetBounds());

    m_self->renderWindow()->Render();
}

void SceneView::Impl::removeActors()
{
    m_renderer->RemoveActor(m_modelActor);
    m_self->renderWindow()->Render();
    m_modelPolydata->Initialize();
}

void SceneView::Impl::saveToStl(ModelToSave model, const std::string &fileName)
{
    if (m_modelPolydata->GetNumberOfPoints() == 0 && m_modelPolydata->GetNumberOfCells() == 0)
        return;

    m_modelToSave = model;
    m_nameToSave = fileName;

    QThread* thread = new QThread;
    Worker* worker = new Worker(this);

    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started, worker, &Worker::saveToStl);
    QObject::connect(worker, &Worker::finished, thread, &QThread::quit);
    QObject::connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void SceneView::Impl::setConfig(const Config &config)
{
    if (config.xMax <= config.xMin || config.yMax <= config.yMin || config.zMax <= config.zMin)
    {
        m_config.zone = config.zone;
        return;
    }
    m_config = config;
}

SceneView::Config SceneView::Impl::config()
{
    return m_config;
}

SceneView::Dimmensions SceneView::Impl::dimmensions()
{
    return m_dimmensions;
}

void SceneView::Impl::buildModel()
{
    if (m_config.xMax <= m_config.xMin || m_config.yMax <= m_config.yMin || m_config.zMax <= m_config.zMin)
        return;

    QThread* thread = new QThread;
    Worker* worker = new Worker(this);

    worker->moveToThread(thread);

    switch (m_config.zone)
    {
    case Lungs: QObject::connect(thread, &QThread::started, worker, &Worker::buildLungs); break;
    case Bones: QObject::connect(thread, &QThread::started, worker, &Worker::buildBones); break;
    case Skin : QObject::connect(thread, &QThread::started, worker, &Worker::buildSkin);  break;
    }

    QObject::connect(worker, &Worker::finished, thread, &QThread::quit);
    QObject::connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(worker, &Worker::finished, m_self, &SceneView::renderModelMapper);

    thread->start();
}

// ----------- SceneView::Impl::Worker implementation ----------

SceneView::Impl::Worker::Worker(Impl *owner)
{
    m_owner = owner;
}

void SceneView::Impl::Worker::parseDicom()
{
    m_owner->m_dicomReader->SetDirectoryName(m_owner->m_dicomPath.toStdString().c_str());

    emit EventHandler::instance().dicomReadStarted();
    m_owner->m_dicomReader->Update();

    vtkImageData *imageData = m_owner->m_dicomReader->GetOutput();

    int numComponents = imageData->GetPointData()->GetScalars()->GetNumberOfComponents();

    if (numComponents > 1)
    {
        qDebug() << "\n\nUnable to load multi component images, only single component data supported.";
        qDebug() << "but " << numComponents << " got";
        return;
    }

    int dims[3];
    double spacing[3];
    imageData->GetDimensions(dims);
    imageData->GetSpacing(spacing);

    m_owner->m_config.xMax = dims[0];
    m_owner->m_config.yMax = dims[1];
    m_owner->m_config.zMax = dims[2];

    m_owner->m_dimmensions.x = dims[0];
    m_owner->m_dimmensions.y = dims[1];
    m_owner->m_dimmensions.z = dims[2];

    emit EventHandler::instance().dicomParseComplete();

    emit EventHandler::instance().pushStringToTerminal("Volume:");
    emit EventHandler::instance().pushStringToTerminal("ox    " + QString::number(dims[0]*spacing[0]/10) + " cm");
    emit EventHandler::instance().pushStringToTerminal("oy    " + QString::number(dims[1]*spacing[1]/10) + " cm");
    emit EventHandler::instance().pushStringToTerminal("oz    " + QString::number(dims[2]*spacing[2]/10) + " cm");
    emit EventHandler::instance().pushStringToTerminal("Dimmensions:");
    emit EventHandler::instance().pushStringToTerminal("ox    " + QString::number(m_owner->m_dimmensions.x));
    emit EventHandler::instance().pushStringToTerminal("oy    " + QString::number(m_owner->m_dimmensions.y));
    emit EventHandler::instance().pushStringToTerminal("oz    " + QString::number(m_owner->m_dimmensions.z));

    emit finished();
}

void SceneView::Impl::Worker::saveToStl()
{
    auto stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
    stlWriter->SetFileName(&m_owner->m_nameToSave[0]);
    switch (m_owner->m_modelToSave) {
    case AllModel:
        stlWriter->SetInputData(m_owner->m_modelPolydata);      break;
    default: stlWriter->SetInputData(m_owner->m_modelPolydata); break;
    }

    emit EventHandler::instance().pushStringToTerminal("Saving to stl ...");
    stlWriter->Write();
    if (QFile::exists(QString::fromStdString(m_owner->m_nameToSave)))
        emit EventHandler::instance().pushStringToTerminal("Model saved to " + QCoreApplication::applicationDirPath() + "/" + QString::fromStdString(m_owner->m_nameToSave));
    else
        emit EventHandler::instance().pushStringToTerminal("Save error");

    emit finished();
}

void SceneView::Impl::Worker::buildLungs()
{
    const int xMin = m_owner->m_config.xMin;
    const int yMin = m_owner->m_config.yMin;
    const int zMin = m_owner->m_config.zMin;
    const int xMax = m_owner->m_config.xMax;
    const int yMax = m_owner->m_config.yMax;
    const int zMax = m_owner->m_config.zMax;

    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->DeepCopy(m_owner->m_dicomReader->GetOutput());

    vtkDataArray* scalars = imageData->GetPointData()->GetScalars();

    for (int z = zMin; z < zMax; ++z) {
        for (int y = yMin; y < yMax; ++y) {
            for (int x = xMin; x < xMax; ++x) {
                int idx = z * xMax * yMax + y * xMax + x;
                double value = scalars->GetComponent(idx, 0);
                if (value < -990)
                    scalars->SetComponent(idx, 0, 1000);
            }
        }
    }

    imageData->Modified();

    vtkSmartPointer<vtkExtractVOI> extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
    extractVOI->SetInputData(imageData);

    extractVOI->SetVOI(xMin, xMax, yMin, yMax, zMin, zMax); // Область интереса: x, y, z (все слои)
    extractVOI->SetSampleRate(2, 2, 2);         // Прореживание по оси Z (каждый 2-й слой)
    extractVOI->Update();

    auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
    marchingCubes->SetInputData(extractVOI->GetOutput());
    marchingCubes->SetValue(0, -350);

    auto isosurfaceProgressCallback = vtkSmartPointer<ProgressCallback>::New();
    marchingCubes->AddObserver(vtkCommand::ProgressEvent, isosurfaceProgressCallback);

    emit EventHandler::instance().surfaceBuildStarted();
    marchingCubes->Update();

    auto connect = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connect->SetInputData(marchingCubes->GetOutput());
    connect->SetExtractionModeToLargestRegion();
    connect->Update();

    auto sizes = vtkSmartPointer<vtkIdTypeArray>::New();
    sizes = connect->GetRegionSizes();

    vtkIdType largestRegionId = 0;
    vtkIdType maxSize = 0;
    for (vtkIdType i = 0; i < sizes->GetNumberOfTuples(); i++) {
        int size = sizes->GetValue(i);
        if (size > maxSize) {
            maxSize = size;
            largestRegionId = i;
        }
    }

    const int minSize = maxSize / 100;
    QList<int> selectedRegions;
    for (vtkIdType i = 0; i < sizes->GetNumberOfTuples(); i++) {
        int size = sizes->GetValue(i);
        if (size > minSize)
            selectedRegions.append(i);
    }

    QList<int> specificRegions;
    for (int& id : selectedRegions)
        if (id != largestRegionId)
        {
            connect->AddSpecifiedRegion(id);
            specificRegions.append(id);
        }

    if (specificRegions.size() == 0)
    {
        emit EventHandler::instance().pushStringToTerminal("Extraction parameters you set appears to not select any data");
        emit finished();
    }

    connect->SetExtractionModeToSpecifiedRegions();
    connect->Update();

    auto smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smoother->SetInputData(connect->GetOutput());
    smoother->SetNumberOfIterations(20);

    auto smoothProgressCallback = vtkSmartPointer<ProgressCallback>::New();
    smoother->AddObserver(vtkCommand::ProgressEvent, smoothProgressCallback);

    emit EventHandler::instance().smoothingSurfaceStarted();
    smoother->Update();

    m_owner->m_dicomMapper->SetInputConnection(smoother->GetOutputPort());
    m_owner->m_modelPolydata->DeepCopy(smoother->GetOutput());

    emit EventHandler::instance().pushStringToTerminal("Build lungs complete");

    emit finished();
}

void SceneView::Impl::Worker::buildBones()
{
    const int xMin = m_owner->m_config.xMin;
    const int yMin = m_owner->m_config.yMin;
    const int zMin = m_owner->m_config.zMin;
    const int xMax = m_owner->m_config.xMax;
    const int yMax = m_owner->m_config.yMax;
    const int zMax = m_owner->m_config.zMax;

    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->DeepCopy(m_owner->m_dicomReader->GetOutput());

    vtkSmartPointer<vtkExtractVOI> extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
    extractVOI->SetInputData(imageData);

    extractVOI->SetVOI(xMin, xMax, yMin, yMax, zMin, zMax);
    extractVOI->SetSampleRate(2, 2, 2);
    extractVOI->Update();

    auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
    marchingCubes->SetInputData(extractVOI->GetOutput());
    marchingCubes->SetValue(0, 100);

    auto isosurfaceProgressCallback = vtkSmartPointer<ProgressCallback>::New();
    marchingCubes->AddObserver(vtkCommand::ProgressEvent, isosurfaceProgressCallback);

    emit EventHandler::instance().surfaceBuildStarted();
    marchingCubes->Update();

    auto connect = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connect->SetInputData(marchingCubes->GetOutput());
    connect->SetExtractionModeToLargestRegion();
    connect->Update();

    auto smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smoother->SetInputData(connect->GetOutput());
    smoother->SetNumberOfIterations(20);

    auto smoothProgressCallback = vtkSmartPointer<ProgressCallback>::New();
    smoother->AddObserver(vtkCommand::ProgressEvent, smoothProgressCallback);

    emit EventHandler::instance().smoothingSurfaceStarted();
    smoother->Update();

    m_owner->m_dicomMapper->SetInputConnection(smoother->GetOutputPort());
    m_owner->m_modelPolydata->DeepCopy(smoother->GetOutput());

    emit EventHandler::instance().pushStringToTerminal("Build bones complete");

    emit finished();
}

void SceneView::Impl::Worker::buildSkin()
{
    const int xMin = m_owner->m_config.xMin;
    const int yMin = m_owner->m_config.yMin;
    const int zMin = m_owner->m_config.zMin;
    const int xMax = m_owner->m_config.xMax;
    const int yMax = m_owner->m_config.yMax;
    const int zMax = m_owner->m_config.zMax;

    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->DeepCopy(m_owner->m_dicomReader->GetOutput());

    vtkSmartPointer<vtkExtractVOI> extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
    extractVOI->SetInputData(imageData);

    extractVOI->SetVOI(xMin, xMax, yMin, yMax, zMin, zMax);
    extractVOI->SetSampleRate(2, 2, 2);
    extractVOI->Update();

    auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
    marchingCubes->SetInputData(extractVOI->GetOutput());
    marchingCubes->SetValue(0, -350);

    auto isosurfaceProgressCallback = vtkSmartPointer<ProgressCallback>::New();
    marchingCubes->AddObserver(vtkCommand::ProgressEvent, isosurfaceProgressCallback);

    emit EventHandler::instance().surfaceBuildStarted();
    marchingCubes->Update();

    auto smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smoother->SetInputData(marchingCubes->GetOutput());
    smoother->SetNumberOfIterations(20);

    auto smoothProgressCallback = vtkSmartPointer<ProgressCallback>::New();
    smoother->AddObserver(vtkCommand::ProgressEvent, smoothProgressCallback);

    emit EventHandler::instance().smoothingSurfaceStarted();
    smoother->Update();

    m_owner->m_dicomMapper->SetInputConnection(smoother->GetOutputPort());
    m_owner->m_modelPolydata->DeepCopy(smoother->GetOutput());

    emit EventHandler::instance().pushStringToTerminal("Build skin complete");

    emit finished();
}

// -------------------------------------------------------------
//                        Interface
// -------------------------------------------------------------

SceneView::SceneView(QWidget *parent)
    : QVTKOpenGLNativeWidget(parent),
      d(new Impl)
{
    d->construct(this);
}

SceneView::~SceneView()
{

}

void SceneView::openDicomFolder(const QString &path)
{
    d->openDicomFolder(path);
}

void SceneView::setConfig(const Config &config)
{
    d->setConfig(config);
}

SceneView::Config SceneView::config()
{
    return d->config();
}

SceneView::Dimmensions SceneView::dimmensions()
{
    return d->dimmensions();
}

void SceneView::removeActors()
{
    d->removeActors();
}

void SceneView::saveToStl()
{
    d->saveToStl(Impl::AllModel, "model.stl");
}

void SceneView::buildModel()
{
    d->buildModel();
}

void SceneView::renderModelMapper()
{
    d->renderModelMapper();
}

#include "SceneView.moc"
