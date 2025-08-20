#pragma once

#include <QDialog>
#include <QComboBox>
#include <QSlider>
#include <QLabel>

#include "SceneView.h"

class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);

    SceneView::Config config();
    void synchronizeConfig(const SceneView::Config& config, const SceneView::Dimmensions& dimmensions);

private:
    SceneView::Config m_config;

    QComboBox *m_zoneComboBox;
    QSlider* m_xSlider;
    QSlider* m_ySlider;
    QSlider* m_zSlider;
    QLabel* m_startXLabel;
    QLabel* m_endXLabel;
    QLabel* m_startYLabel;
    QLabel* m_endYLabel;
    QLabel* m_startZLabel;
    QLabel* m_endZLabel;

};
