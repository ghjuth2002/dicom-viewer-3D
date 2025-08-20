#include "EventsModel.h"
#include "EventHandler.h"

#include <QAbstractListModel>
#include <QVector>
#include <QDebug>

EventsModel::EventsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QObject::connect(&EventHandler::instance(), &EventHandler::dicomReadStarted, [this] () {
        appendData("Dicom folder read");
    });
    QObject::connect(&EventHandler::instance(), &EventHandler::surfaceBuildStarted, [this] () {
        appendData("Surface build");
    });
    QObject::connect(&EventHandler::instance(), &EventHandler::smoothingSurfaceStarted, [this] () {
        appendData("Smooth surface");
    });
    QObject::connect(&EventHandler::instance(), &EventHandler::dicomParseComplete, [this] () {
        appendData("Parse dicom complete");
    });

    QObject::connect(&EventHandler::instance(), &EventHandler::parseDicomProgressChanged, [this] (const double& value) {
        const int max = 20; int i = 0;
        QString track;
        while (i++ < max * value)
            track += "-";
        const int ind = m_data.length() - 1;
        QString newStr = m_data[ind];
        QStringList list = newStr.split(" ");
        if (list.last().contains("-"))
            list.pop_back();
        newStr.clear();
        for (auto &el : list)
            if (!el.isEmpty())
            newStr += el.append(" ");
        newStr += "  " + track;
        m_data[ind] = newStr;
        emit dataChanged(this->index(ind), this->index(ind), { Qt::UserRole });
    });

    QObject::connect(&EventHandler::instance(), &EventHandler::pushStringToTerminal, [this] (const QString& value) {
        appendData(value);
    });
}

int EventsModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

QVariant EventsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) return {};
    return m_data.at(index.row());
}

void EventsModel::appendData(const QString &value)
{
    m_data.append(value);
    emit dataChanged(this->index(0), this->index(m_data.size() - 1), { Qt::UserRole });
}
