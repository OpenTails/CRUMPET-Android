#include "AlarmList.h"
#include "Alarm.h"

#include <QDebug>

class AlarmList::Private
{
public:
    QList<Alarm*> list;
    QHash<int, QByteArray> roles;
};

AlarmList::AlarmList(QObject *parent)
    : QAbstractTableModel(parent),
      d(new Private())
{
}

AlarmList::~AlarmList()
{
    delete d;
}

QHash<int, QByteArray> AlarmList::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[AlarmRole] = "alarm";

    return roles;
}

int AlarmList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->list.size();
}

QVariant AlarmList::data(const QModelIndex &index, int role) const
{
    if(index.isValid() && index.row() >= 0 && index.row() < d->list.count()) {
        if (role == AlarmRole) {
            return QVariant::fromValue(d->list.at(index.row()));
        }
    };

    return QVariant();
}

void AlarmList::addAlarm(Alarm *alarm)
{
    if (d->list.contains(alarm)) {
        qWarning() << "The alarm already exists in Alarm List";
        return;
    }

    beginInsertRows(QModelIndex(), 0, 0);
    alarm->setParent(this);
    d->list.insert(0, alarm);
    emit listChanged();
    endInsertRows();
}

void AlarmList::addAlarmName(const QString &alarmName)
{
    Alarm* alarm = new Alarm(alarmName, this);
    addAlarm(alarm);
}

void AlarmList::removeAlarm(Alarm *alarm)
{
    int index = d->list.indexOf(alarm);

    if (index < 0) {
        qWarning() << "Unable to find the alarm in Alarm List";
        return;
    }

    removeAlarmByIndex(index);
}

void AlarmList::removeAlarmByIndex(int index)
{
    if (index < 0 || index >= d->list.size()) {
        qWarning() << QString("Unable to delete alarm with index %1").arg(index);
    }

    beginRemoveRows(QModelIndex(), index, index);

    Alarm *alarm = d->list.at(index);
    d->list.removeAt(index);
    alarm->deleteLater();

    emit listChanged();
    endRemoveRows();
}
