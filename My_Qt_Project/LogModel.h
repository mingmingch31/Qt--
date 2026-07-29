#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QObject>
#include <QString>
#include <QQmlEngine>

class LogModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)

public:
    explicit LogModel(QObject *parent = nullptr);
    QString logText() const;
    Q_INVOKABLE void appendLog(const QString &message);

signals:
    void logTextChanged();

private:
    QString m_logText;
};

#endif // LOGMODEL_H