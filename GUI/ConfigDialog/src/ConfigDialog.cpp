#include "ConfigDialog.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QRect>
#include <QAbstractItemModel>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_zoneComboBox(new QComboBox(this))
    , m_xSlider(new QSlider)
    , m_ySlider(new QSlider)
    , m_zSlider(new QSlider)
    , m_startXLabel(new QLabel)
    , m_endXLabel(new QLabel)
    , m_startYLabel(new QLabel)
    , m_endYLabel(new QLabel)
    , m_startZLabel(new QLabel)
    , m_endZLabel(new QLabel)
{
    setWindowTitle("Set parameters");

    QPushButton *okButton = new QPushButton("OK", this);
    okButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    m_zoneComboBox->addItems({"Lungs", "Bones", "Skin"});
    QObject::connect(m_zoneComboBox, &QComboBox::currentTextChanged, [this](const QString &text){
            if (text == "Lungs")
                m_config.zone = SceneView::Lungs;
            if (text == "Bones")
                m_config.zone = SceneView::Bones;
            if (text == "Skin")
                m_config.zone = SceneView::Skin;
        });

    QObject::connect(m_xSlider, &QSlider::valueChanged, [this] () {
        if (m_config.xMax <= m_config.xMin || m_config.yMax <= m_config.yMin || m_config.zMax <= m_config.zMin)
            return;
        m_config.xMax = m_xSlider->value();
    });
    QObject::connect(m_ySlider, &QSlider::valueChanged, [this] () {
        if (m_config.xMax <= m_config.xMin || m_config.yMax <= m_config.yMin || m_config.zMax <= m_config.zMin)
            return;
        m_config.yMax = m_ySlider->value();
    });
    QObject::connect(m_zSlider, &QSlider::valueChanged, [this] () {
        if (m_config.xMax <= m_config.xMin || m_config.yMax <= m_config.yMin || m_config.zMax <= m_config.zMin)
            return;
        m_config.zMax = m_zSlider->value();
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_zoneComboBox);

    QHBoxLayout* xLayout = new QHBoxLayout;
    QHBoxLayout* yLayout = new QHBoxLayout;
    QHBoxLayout* zLayout = new QHBoxLayout;

    m_xSlider->setOrientation(Qt::Horizontal);
    m_ySlider->setOrientation(Qt::Horizontal);
    m_zSlider->setOrientation(Qt::Horizontal);

    xLayout->addWidget(m_startXLabel);
    xLayout->addWidget(m_xSlider);
    xLayout->addWidget(m_endXLabel);
    yLayout->addWidget(m_startYLabel);
    yLayout->addWidget(m_ySlider);
    yLayout->addWidget(m_endYLabel);
    zLayout->addWidget(m_startZLabel);
    zLayout->addWidget(m_zSlider);
    zLayout->addWidget(m_endZLabel);

    layout->addLayout(xLayout);
    layout->addLayout(yLayout);
    layout->addLayout(zLayout);

    layout->addWidget(okButton);

}

SceneView::Config ConfigDialog::config()
{
    return m_config;
}

void ConfigDialog::synchronizeConfig(const SceneView::Config &config, const SceneView::Dimmensions& dimmensions)
{
    m_config = config;

    switch (m_config.zone)
    {
    case SceneView::Zone::Lungs: m_zoneComboBox->setCurrentIndex(0); break;
    case SceneView::Zone::Bones: m_zoneComboBox->setCurrentIndex(1); break;
    case SceneView::Zone::Skin : m_zoneComboBox->setCurrentIndex(2); break;
    }

    if (m_config.xMax <= m_config.xMin || m_config.yMax <= m_config.yMin || m_config.zMax <= m_config.zMin)
        return;

    m_xSlider->setMinimum(0);
    m_xSlider->setMaximum(dimmensions.x);
    m_ySlider->setMinimum(0);
    m_ySlider->setMaximum(dimmensions.y);
    m_zSlider->setMinimum(0);
    m_zSlider->setMaximum(dimmensions.z);

    m_xSlider->setValue(m_config.xMax);
    m_ySlider->setValue(m_config.yMax);
    m_zSlider->setValue(m_config.zMax);

    m_startXLabel->setText("Extraction OX:    " + QString::number(m_xSlider->minimum()));
    m_endXLabel->setText(QString::number(m_xSlider->maximum()));
    m_startYLabel->setText("Extraction OY:    " + QString::number(m_ySlider->minimum()));
    m_endYLabel->setText(QString::number(m_ySlider->maximum()));
    m_startZLabel->setText("Extraction OZ:    " + QString::number(m_zSlider->minimum()));
    m_endZLabel->setText(QString::number(m_zSlider->maximum()));


}
