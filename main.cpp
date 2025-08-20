#include "widget.h"

#include <QApplication>
#include <QVector>
#include <QItemSelection>
#include <QMetaType>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QString>
#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QItemSelection>("QItemSelection");

    QString path = ":/GUI/styles";
    QStringList fileNames;
    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        fileNames << it.next();

    QString style;
    for (auto name : fileNames)
    {
        QFile file(name);
        if (!file.open(QFile::ReadOnly)) continue;
        style += QLatin1String(file.readAll());
    }
    qApp->setStyleSheet(style);

    Widget w;
    w.show();
    return a.exec();
}
