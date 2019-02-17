#ifndef ALARM_H
#define ALARM_H

#include <QObject>
#include <QDateTime>

/**
 * @brief The Alarm class represents alarm moves.
 * See also AlarmList class.
 */
class Alarm : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QDateTime time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(QStringList commands READ commands WRITE setCommands NOTIFY commandsChanged)

public:
    explicit Alarm(QObject *parent = nullptr);
    explicit Alarm(const QString &name, QObject *parent = nullptr);
    ~Alarm();

    QString name() const;
    void setName(const QString &name);

    QDateTime time() const;
    void setTime(const QDateTime &time);

    QStringList commands() const;
    void setCommands(const QStringList &commands);

public slots:

signals:
    void nameChanged();
    void timeChanged();
    void commandsChanged();

private:
    //TODO: There is no any reason to use PIMPL idiom at these classes
    class Private;
    Private* d;
};

#endif // ALARM_H
