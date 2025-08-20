#pragma once

#include <QObject>
#include<QString>
#include <QVTKOpenGLNativeWidget.h>

namespace Ui { class SceneView; }

class SceneView : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    enum Zone
    {
        Lungs,
        Bones,
        Skin
    };

    struct Config
    {
        Zone zone{ Lungs };
        int xMin{ 0 };
        int xMax{ 0 };
        int yMin{ 0 };
        int yMax{ 0 };
        int zMin{ 0 };
        int zMax{ 0 };
    };

    struct Dimmensions
    {
        int x{ 0 };
        int y{ 0 };
        int z{ 0 };
    };

    SceneView(QWidget* parent = nullptr);
    ~SceneView();

    void openDicomFolder(const QString& path);
    void setConfig(const Config& config);
    Config config();
    Dimmensions dimmensions();

public slots:
    void removeActors();
    void saveToStl();
    void buildModel();

private slots:
    void renderModelMapper();

private:
    Ui::SceneView *ui;

    class Impl;
    std::unique_ptr<Impl> d;
};
