#pragma once

#include <QObject>
#include "Singleton.h"

class EventHandler
        : public QObject
        , public Singleton<EventHandler>
{
    Q_OBJECT

private:
    EventHandler();
    ~EventHandler() override = default;
    friend class Singleton<EventHandler>;

signals:
    void dicomReadStarted();
    void surfaceBuildStarted();
    void smoothingSurfaceStarted();
    void dicomParseComplete();
    void interactionOccured();
    void parseDicomProgressChanged(const double&);
    void pushStringToTerminal(const QString&);
};
