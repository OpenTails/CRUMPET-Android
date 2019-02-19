#ifndef ALARMLIST_H
#define ALARMLIST_H

#include <QAbstractTableModel>

class Alarm;

/**
 * @brief The AlarmList class represents collection of all alarm moves.
 * See also Alarm class.
 */
class AlarmList : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Roles {
        AlarmRole = Qt::UserRole + 1,
    };

    explicit AlarmList(QObject *parent = nullptr);
    ~AlarmList();

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

public slots:
    /**
     * Add a new alarm to the model.
     * The new alarm is added at the start of the unsorted model
     * The model takes ownership of the alarm, and deletion should not
     * be done manually.
     * @param alarm The new alarm
     */
    void addAlarm(Alarm* alarm);
    void addAlarmName(const QString &alarmName);

    void removeAlarm(Alarm* alarm);
    void removeAlarmByIndex(int index);

signals:
    void listChanged();

private:
    //TODO: There is no any reason to use PIMPL idiom at these classes
    class Private;
    Private* d;
};

#endif // ALARMLIST_H
