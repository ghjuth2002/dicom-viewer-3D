#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <QString>
#include <QVariant>

class EventsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    EventsModel(QObject* parent = nullptr);
    ~EventsModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    void appendData(const QString& value);

private:
    QStringList m_data;
};
