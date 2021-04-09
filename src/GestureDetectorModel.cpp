/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 3, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>
 */

#include "GestureDetectorModel.h"
#include "GestureController.h"

#include <QSettings>
#include <QSensorGesture>

class GestureDetectorModel::Private {
public:
    Private() {}
    QList<GestureDetails*> entries;
};

GestureDetectorModel::GestureDetectorModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
}

GestureDetectorModel::~GestureDetectorModel()
{
    delete d;
}

QHash<int, QByteArray> GestureDetectorModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {NameRole, "name"},
        {SensorIdRole,  "sensorId"},
        {SensorNameRole, "sensorName"},
        {IdRole, "id"},
        {CommandRole, "command"},
        {DevicesModel, "devices"},
        {FirstInSensorRole, "firstInSensor"}
    };
    return roles;
}

QVariant GestureDetectorModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    if (checkIndex(index)) {
        GestureDetails* gesture = d->entries.value(index.row());
        switch(role) {
            case NameRole:
                result.setValue(gesture->humanName());
                break;
            case SensorIdRole:
                result.setValue(gesture->sensor()->validIds().first());
                break;
            case SensorNameRole:
                result.setValue(gesture->sensor()->validIds().first());
                break;
            case IdRole:
                result.setValue(gesture->gestureId());
                break;
            case CommandRole:
                result.setValue(gesture->command());
                break;
            case DevicesModel:
                result.setValue(gesture->devices());
                break;
            case FirstInSensorRole:
            {
                GestureDetails* previousGesture{nullptr};
                if (index.row() > 0) {
                    previousGesture = d->entries.value(index.row() - 1);
                    result.setValue(previousGesture->sensor()->validIds().first() != gesture->sensor()->validIds().first());
                } else {
                    result.setValue(true);
                }
                break;
            }
            default:
                break;
        }
    }
    return result;
}

int GestureDetectorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->entries.count();
}

void GestureDetectorModel::addGesture(GestureDetails* gesture)
{
    beginInsertRows(QModelIndex(), d->entries.count(), d->entries.count());
    d->entries << gesture;
    endInsertRows();
}

void GestureDetectorModel::gestureDetailsChanged(GestureDetails* gesture)
{
    QModelIndex idx = createIndex(d->entries.indexOf(gesture), 0);
    dataChanged(idx, idx);
}

void GestureDetectorModel::setGestureDetails(int index, QString command, QStringList devices)
{
    GestureDetails* gesture = d->entries.value(index);
    gesture->setCommand(command);
    gesture->setDevices(devices);
}

class GestureDetails::Private {
public:
    Private() {}
    GestureController* controller;
    QString gestureId; // The ID used by the gesture manager to identify each gesture
    QString humanName; // A human readable name
    QSensorGesture* sensor{nullptr}; // The sensor used for detection of this gesture
    QString sensorHumanName;
    QString command; // The command we will use to send to the devices when this gesture is recognised - if this is empty, recognition is turned off for this gesture
    QStringList devices; // The list of devices to send to - if this list is empty, we send to all devices
};

GestureDetails::GestureDetails(QString gestureId, QSensorGesture* sensor, GestureController* q)
    : d(new Private)
{
    static const QHash<QString, QString> sensorNames = {
        {QLatin1String("shakeLeft"), QLatin1String("Shake Left")},
        {QLatin1String("shakeRight"), QLatin1String("Shake Right")},
        {QLatin1String("shakeUp"), QLatin1String("Shake Up")},
        {QLatin1String("shakeDown"), QLatin1String("Shake Down")},
        {QLatin1String("twistLeft"), QLatin1String("Twist Left")},
        {QLatin1String("twistRight"), QLatin1String("Twist Right")},
        {QLatin1String("pickup"), QLatin1String("Pick Up")},
        {QLatin1String("turnover"), QLatin1String("Turn Over")}
    };

    d->controller = q;
    d->gestureId = gestureId;

    d->humanName = gestureId;
    const QString& humanName = sensorNames.value(gestureId);
    if (!humanName.isEmpty()) {
        d->humanName = humanName;
    }
    // Automagically capitalise the first letter
    d->humanName.replace(0, 1, d->humanName[0].toUpper());

    d->sensor = sensor;
    d->sensorHumanName = sensor->validIds().first();
    // Automagically capitalise the first letter
    d->sensorHumanName.replace(0, 1, d->sensorHumanName[0].toUpper());

    load();
}

GestureDetails::~GestureDetails() {
    d->sensor->deleteLater();
}

void GestureDetails::load() {
    QSettings settings;
    settings.beginGroup("Gestures");
    d->command = settings.value(QString("%1/command").arg(d->gestureId), QString{}).toString();
    d->devices = settings.value(QString("%1/devices").arg(d->gestureId), QStringList{}).toStringList();
    settings.endGroup();
}

void GestureDetails::save() {
    QSettings settings;
    settings.beginGroup("Gestures");
    settings.setValue(QString("%1/command").arg(d->gestureId), d->command);
    settings.setValue(QString("%1/devices").arg(d->gestureId), d->devices);
    settings.endGroup();
    settings.sync();
}

QSensorGesture * GestureDetails::sensor() const
{
    return d->sensor;
}

QString GestureDetails::gestureId() const
{
    return d->gestureId;
}

QString GestureDetails::humanName() const
{
    return d->humanName;
}

void GestureDetails::setCommand(const QString& value) {
    d->command = value;
    d->controller->model()->gestureDetailsChanged(this);
    save();
    // Optimisation possibility: Stop and start detection for sensors where all gestures have no command to run...
}

QString GestureDetails::command() const
{
    return d->command;
}

void GestureDetails::setDevices(const QStringList& value) {
    d->devices = value;
    d->controller->model()->gestureDetailsChanged(this);
    save();
}

QStringList GestureDetails::devices() const
{
    return d->devices;
}
