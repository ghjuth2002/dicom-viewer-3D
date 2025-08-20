#include "widget.h"
#include "./ui_widget.h"

#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QList>
#include <QRect>
#include <QModelIndex>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QApplication>
#include <QWindow>
#include <QDesktopWidget>
#include <QScreen>
#include <QPoint>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , m_eventsModel(new EventsModel)
    , m_configDialog(new ConfigDialog)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    setWindowTitle("Dicom viewer 3D");
    setProperty("mainWindow", true);

    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *fileMenu = new QMenu("File", menuBar);

    QAction *openAction = new QAction("New file", fileMenu);
    QAction *closeAction = new QAction("Clear scene", fileMenu);
    QObject::connect(openAction, &QAction::triggered, this, &Widget::openDicom);
    QObject::connect(closeAction, &QAction::triggered, ui->openGLWidget, &SceneView::removeActors);

    fileMenu->addAction(openAction);
    fileMenu->addAction(closeAction);
    menuBar->addMenu(fileMenu);

    QMenu *exportMenu = new QMenu("Export", menuBar);
    QAction *stlExportAction = new QAction("slt", exportMenu);
    QObject::connect(stlExportAction, &QAction::triggered, ui->openGLWidget, &SceneView::saveToStl);
    exportMenu->addAction(stlExportAction);
    menuBar->addMenu(exportMenu);

    QHBoxLayout *menuBarLayout = new QHBoxLayout;
    menuBarLayout->setMenuBar(menuBar);
    menuBarLayout->setMargin(10);

    ui->verticalLayout->insertLayout(0, menuBarLayout);

    QObject::connect(ui->buildButton, &QPushButton::clicked, ui->openGLWidget, &SceneView::buildModel);
    QObject::connect(ui->configButton, &QPushButton::clicked, [this] () {
        QPoint point =  QApplication::focusWindow()->geometry().center();
        m_configDialog->setGeometry(point.x() - 150, point.y() - 150, 300, 300);
        m_configDialog->synchronizeConfig(ui->openGLWidget->config(), ui->openGLWidget->dimmensions());
        if (m_configDialog->exec() == QDialog::Accepted) {
            SceneView::Config config = m_configDialog->config();
            ui->openGLWidget->setConfig(config);
        }
    });

    ui->listView->setModel(m_eventsModel);
    QObject::connect(m_eventsModel, &EventsModel::dataChanged,
                     [this] (const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
        ui->listView->setCurrentIndex(bottomRight);
    });
    ui->listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QRect rect = this->geometry();
    ui->splitter->setSizes(QList<int>{int(rect.height() * 0.9), int(rect.height() * 0.1)});
    ui->splitter->setAttribute(Qt::WA_StyledBackground, true);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::openDicom()
{
//    if (GuiSettings::instance().isModelOppened())
//        return;

     QString directory = QFileDialog::getExistingDirectory(
             nullptr,
             "Выберите каталог",
             QDir::homePath(), // Стартовый каталог
             QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks // Опции
         );

     if (!directory.isEmpty())
     {
         ui->openGLWidget->openDicomFolder(directory);
     }
}

