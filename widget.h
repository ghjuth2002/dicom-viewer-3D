#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QObject>

#include "EventsModel.h"
#include "ConfigDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void openDicom();

private:
    EventsModel* m_eventsModel;
    ConfigDialog* m_configDialog;

    Ui::Widget *ui;
};
#endif // WIDGET_H
