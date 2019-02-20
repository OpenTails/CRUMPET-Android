#ifndef ALARMLIST_H
#define ALARMLIST_H

#include <QAbstractListModel>

class Alarm;

/**
 * @brief The AlarmList class represents collection of all alarm moves.
 * See also Alarm class.
 *
 * Please note that now alarm names should be unique,
 * if we try to rename an alarm or add new alarm with the existed name
 * alarmExisted() signal will be emitted.
 *
 * Now we use the AlarmList class via AppSettings,
 * but later we may want to use it as a separated model via QAbstractItemModelReplica.
 */
class AlarmList : public QAbstractListModel
{
    Q_OBJECT

    //TODO: It seems we do not need Q_PROPERTIES here because in qml we use QVariantList and QVariantMap
    Q_PROPERTY(int size READ size NOTIFY listChanged)

public:
    enum Roles {
        AlarmRole = Qt::UserRole + 1,
    };

    explicit AlarmList(QObject *parent = nullptr);
    ~AlarmList();

    int size() const;
    Alarm* at(int index) const;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool exists(const QString& name) const;
    Alarm* alarm(const QString& name) const;
    int alarmIndex(const QString& name) const;

    void addAlarm(const QString& name, const QDateTime& time, const QStringList& commands);

    /**
     * Add a new alarm to the model.
     * The new alarm is added at the start of the unsorted model
     * The model takes ownership of the alarm, and deletion should not
     * be done manually.
     * @param alarm The new alarm
     */
    void addAlarm(Alarm* alarm);
    void addAlarm(const QString& alarmName);

    void removeAlarm(Alarm* alarm);
    void removeAlarm(const QString& alarmName);
    void removeAlarmByIndex(int index);

    void changeAlarmName(const QString& oldName, const QString& newName);
    void setAlarmTime(const QString& alarmName, const QDateTime& time);
    void setAlarmCommands(const QString& alarmName, const QStringList& commands);
    void addAlarmCommand(const QString& alarmName, int index, const QString& command);
    void removeAlarmCommand(const QString& alarmName, int index);

    QVariantList toVariantList() const;

public slots:

signals:
    void listChanged();
    void save();

    /// If alarm with the same name exists we emit alarmExisted() signal
    void alarmExisted(const QString& name);

    /// If we can not find an alarm with the name we emit alarmNotExisted() signal
    void alarmNotExisted(const QString& name);

private:
    //TODO: There is no any reason to use PIMPL idiom at these classes
    class Private;
    Private* d;
};

#endif // ALARMLIST_H
