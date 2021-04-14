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
    GestureController* controller;
    QList<GestureDetails*> entries;
};

GestureDetectorModel::GestureDetectorModel(GestureController* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
    d->controller = parent;
}

GestureDetectorModel::~GestureDetectorModel()
{
    qDeleteAll(d->entries);
    delete d;
}

QHash<int, QByteArray> GestureDetectorModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {NameRole, "name"},
        {SensorIdRole,  "sensorId"},
        {SensorNameRole, "sensorName"},
        {SensorEnabledRole, "sensorEnabled"},
        {SensorPinnedRole, "sensorPinned"},
        {IdRole, "id"},
        {CommandRole, "command"},
        {DefaultCommandRole, "defaultCommand"},
        {DevicesModel, "devices"},
        {FirstInSensorRole, "firstInSensor"},
        {VisibleRole, "visible"}
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
                result.setValue(gesture->sensorName());
                break;
            case SensorEnabledRole:
                result.setValue(gesture->sensorEnabled());
                break;
            case SensorPinnedRole:
                result.setValue(gesture->sensorPinned());
                break;
            case IdRole:
                result.setValue(gesture->gestureId());
                break;
            case CommandRole:
                result.setValue(gesture->command());
                break;
            case DefaultCommandRole:
                result.setValue(gesture->defaultCommand());
                break;
            case DevicesModel:
                result.setValue(gesture->devices());
                break;
            case FirstInSensorRole:
            {
                GestureDetails* previousGesture = index.row() > 0 ? d->entries.value(index.row() - 1) : nullptr;
                if (previousGesture) {
                    result.setValue(previousGesture->sensor()->validIds().first() != gesture->sensor()->validIds().first());
                } else {
                    result.setValue(true);
                }
                break;
            }
            case VisibleRole:
                result.setValue(gesture->visible());
                break;
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

GestureDetails * GestureDetectorModel::gesture(const QString& gestureId) const
{
    GestureDetails* theGesture{nullptr};
    for (GestureDetails* ges : d->entries) {
        if (ges->gestureId() == gestureId) {
            theGesture = ges;
            break;
        }
    }
    return theGesture;
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
    QString defaultCommand; // The default command for this gesture (often empty, but not supposed to be used for clear, it's the sensor-wide revert)
    QStringList devices; // The list of devices to send to - if this list is empty, we send to all devices
    bool visible{true};
    bool sensorPinned{false};
    bool sensorEnabled{false};
};

void GestureDetectorModel::setGestureSensorPinned(int index, bool pinned)
{
    GestureDetails* gesture = d->entries.value(index);
    QSettings settings;
    settings.beginGroup("Sensors");
    settings.setValue(QString("%1/pinned").arg(gesture->sensorName()), pinned);
    settings.endGroup();
    settings.sync();

    for (GestureDetails* ges : d->entries) {
        if (ges->sensor() == gesture->sensor()) {
            ges->d->sensorPinned = pinned;
            gestureDetailsChanged(ges);
        }
    }
}

void toggleSensor(QSensorGesture* sensor, bool enabled)
{
    if (enabled) {
        sensor->startDetection();
    } else {
        sensor->stopDetection();
    }
}

void GestureDetectorModel::setGestureSensorEnabled(int index, bool enabled)
{
    GestureDetails* gesture = d->entries.value(index);
    QSettings settings;
    settings.beginGroup("Sensors");
    settings.setValue(QString("%1/enabled").arg(gesture->sensorName()), enabled);
    settings.endGroup();
    settings.sync();

    for (GestureDetails* ges : d->entries) {
        if (ges->sensor() == gesture->sensor()) {
            ges->d->sensorEnabled = enabled;
            gestureDetailsChanged(ges);
        }
    }
    toggleSensor(gesture->sensor(), enabled);
}

GestureDetails::GestureDetails(QString gestureId, QSensorGesture* sensor, GestureController* q)
    : d(new Private)
{
    static const QHash<QString, QString> sensorNames{
        {QLatin1String("shakeLeft"), QLatin1String("Shake Left")},
        {QLatin1String("shakeRight"), QLatin1String("Shake Right")},
        {QLatin1String("shakeUp"), QLatin1String("Shake Up")},
        {QLatin1String("shakeDown"), QLatin1String("Shake Down")},
        {QLatin1String("twistLeft"), QLatin1String("Twist Left")},
        {QLatin1String("twistRight"), QLatin1String("Twist Right")},
        {QLatin1String("pickup"), QLatin1String("Pick Up")},
        {QLatin1String("turnover"), QLatin1String("Turn Over")},
        {QLatin1String("walkingStarted"), QLatin1String("Walking Started")},
        {QLatin1String("walkingStopped"), QLatin1String("Walking Stopped")},
        {QLatin1String("stepDetected"), QLatin1String("Step Detected")},
        {QLatin1String("evenStepDetected"), QLatin1String("Even-numbered Step Detected")},
        {QLatin1String("oddStepDetected"), QLatin1String("Odd-numbered Step Detected")}
    };

    d->controller = q;
    d->gestureId = gestureId;

    d->humanName = QString("%1 gesture").arg(gestureId);
    const QString& humanName = sensorNames.value(gestureId);
    if (humanName.isEmpty()) {
        // Automagically capitalise the first letter if we've nothing better
        d->humanName.replace(0, 1, d->humanName[0].toUpper());
    } else {
        d->humanName = humanName;
    }

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
    static const QHash<QString, QString> defaultCommands{
        {QLatin1String("stepDetected"), QLatin1String("TAILS1")},
        {QLatin1String("walkingStopped"), QLatin1String("TAILHM")}
    };
    static const QStringList defaultPinned{
        QLatin1String{"QtSensors.Walking"}
    };
    d->defaultCommand = defaultCommands.value(d->gestureId, QLatin1String{});

    QSettings settings;
    settings.beginGroup("Gestures");
    d->command = settings.value(QString("%1/command").arg(d->gestureId), d->defaultCommand).toString();
    d->devices = settings.value(QString("%1/devices").arg(d->gestureId), QStringList{}).toStringList();
    settings.endGroup();
    settings.beginGroup("Sensors");
    d->sensorPinned = settings.value(QString("%1/pinned").arg(sensorName()), defaultPinned.contains(d->sensor->validIds().first())).toBool();
    d->sensorEnabled = settings.value(QString("%1/enabled").arg(sensorName()), false).toBool();
    settings.endGroup();
    toggleSensor(sensor(), d->sensorEnabled);
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

QString GestureDetails::sensorName() const
{
    QString name = d->sensor->validIds().first().split(QStringLiteral(".")).last();
    name.replace(0, 1, name[0].toUpper());
    return name;
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

QString GestureDetails::defaultCommand() const
{
    return d->defaultCommand;
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

bool GestureDetails::visible() const
{
    return d->visible;
}

bool GestureDetails::sensorEnabled() const
{
    return d->sensorEnabled;
}

bool GestureDetails::sensorPinned() const
{
    return d->sensorPinned;
}
