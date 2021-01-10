/*
 *   Copyright 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "GestureController.h"
#include "BTConnectionManager.h"
#include "BTDeviceCommandModel.h"
#include "BTDeviceModel.h"
#include "BTDevice.h"

#include <QSensorGestureManager>
#include <QSettings>
#include <QTimer>

struct GestureDetails {
public:
    GestureDetails(QString gestureId, QSensorGesture* sensor, GestureController* q)
        : q(q)
        , gestureId(gestureId)
        , humanName(gestureId)
        , sensor(sensor)
    {
        load();
    }
    ~GestureDetails() {
        sensor->deleteLater();
    }
    GestureController* q;
    QString gestureId; // The ID used by the gesture manager to identify each gesture
    QString humanName; // A human readable name
    QSensorGesture* sensor{nullptr}; // The sensor used for detection of this gesture
    QString command; // The command we will use to send to the devices when this gesture is recognised - if this is empty, recognition is turned off for this gesture
    QStringList devices; // The list of devices to send to - if this list is empty, we send to all devices
    void load() {
        QSettings settings;
        settings.beginGroup("Gestures");
        command = settings.value(QString("%1/command").arg(gestureId), QString{}).toString();
        devices = settings.value(QString("%1/devices").arg(gestureId), QStringList{}).toStringList();
        settings.endGroup();
    }
    void save() {
        QSettings settings;
        settings.beginGroup("Gestures");
        settings.setValue(QString("%1/command").arg(gestureId), command);
        settings.setValue(QString("%1/devices").arg(gestureId), devices);
        settings.endGroup();
        settings.sync();
    }
    void setCommand(const QString& value) {
        command = value;
        Q_EMIT q->gesturesChanged(q->gestures());
        save();
        // Optimisation possibility: Stop and start detection for sensors where all gestures have no command to run...
    }
    void setDevices(const QStringList& value) {
        devices = value;
        save();
    }
};

class GestureController::Private {
public:
    Private(GestureController* qq)
        : q(qq)
        , connectionManager(nullptr)
    {
        QSensorGestureManager manager;
        const QStringList gestureIds = manager.gestureIds();
        gestures.reserve(gestureIds.count());
        for (const QString& gestureId : gestureIds) {
            QSensorGesture* sensor = new QSensorGesture(QStringList{gestureId}, q);
            // Old style connect statement, see QSensorGesture documentation about custom metaobjects
            qq->connect(sensor, SIGNAL(detected(QString)), qq, SLOT(gestureDetected(QString)));

            QStringList signalSignatures = manager.recognizerSignals(gestureId);
            for (const QString& signalSignature : signalSignatures) {
                if (signalSignature == QLatin1String("detected(QString)")) {
                    // Skip the "detected" signal, that one's not really useful
                    continue;
                }
                QString signalName = signalSignature.split(QLatin1String("(")).first();
                gestures[signalName] = new GestureDetails(signalName, sensor, q);
            }
        }
    }
    ~Private() {
        qDeleteAll(gestures);
    }
    GestureController* q;
    BTConnectionManager* connectionManager;

    // This holds the gestures by their detector IDs (see gestureDetected(QString))
    QHash<QString, GestureDetails*> gestures;

    QString currentGesture;
    GestureDetails* currentGestureDetails{nullptr};
    bool enabled{false};
    void setEnabled(bool enabled) {
        Private::enabled = enabled;
        QHash<QString, GestureDetails*>::const_iterator i;
        for (i = gestures.constBegin() ; i != gestures.constEnd() ; ++i) {
            if (enabled && connectionManager && connectionManager->isConnected()) {
                i.value()->sensor->startDetection();
            } else {
                i.value()->sensor->stopDetection();
            }
        }
        Q_EMIT q->enabledChanged(enabled);
    }

    void gestureDetected(const QString& gestureId) {
        qDebug() << gestureId << "detected";
        GestureDetails* gesture = gestures.value(gestureId);
        if (gesture && !gesture->command.isEmpty()) {
            qDebug() << "We have a gesture with a command set, send that to our required devices, which are (empty means all):" << gesture->devices;
            BTDeviceModel* deviceModel = qobject_cast<BTDeviceModel*>(connectionManager->deviceModel());
            for (int i = 0 ; i < deviceModel->count() ; ++i) {
                BTDevice* device = deviceModel->getDeviceById(i);
                qDebug() << device->deviceID() << "is connected?" << device->isConnected() << "not currently busy?" << device->currentCall().isEmpty() << "and is supposed to be a recipient of this command?" << (gesture->devices.count() == 0 || gesture->devices.contains(device->deviceID()));
                if (device->isConnected() && device->currentCall().isEmpty()
                    && (gesture->devices.count() == 0 || gesture->devices.contains(device->deviceID()))) {
                    device->sendMessage(gesture->command);
                }
            }
        }
    }
};

GestureController::GestureController(QObject* parent)
    : GestureControllerProxySource(parent)
    , d(new Private(this))
{
}

GestureController::~GestureController()
{
    delete d;
}

void GestureController::setConnectionManager(BTConnectionManager* connectionManager)
{
    if(d->connectionManager) {
        d->connectionManager->disconnect(this);
    }
    d->connectionManager = connectionManager;
    // This just makes sure to reset the enabled state on device reconnections,
    // which isn't super important, and more a case of not having it sitting around
    // trying to detect things with no devices available.
    connect(d->connectionManager, &BTConnectionManager::isConnectedChanged, this, [this](){
        d->setEnabled(d->enabled);
    });
}

bool GestureController::enabled() const
{
    return d->enabled;
}

void GestureController::setEnabled(bool value)
{
    d->setEnabled(value);
}

QStringList GestureController::gestures() const
{
    QStringList gestures;
    for (const GestureDetails* gesture : d->gestures) {
        gestures << QString("%1;%2;%3").arg(gesture->gestureId).arg(gesture->command).arg(gesture->humanName);
    }
    return gestures;
}

void GestureController::setCurrentGesture(QString gesture)
{
    if (d->currentGesture != gesture) {
        d->currentGesture = gesture;
        Q_EMIT currentGestureChanged(gesture);
        d->currentGestureDetails = d->gestures.value(gesture);
        Q_EMIT commandChanged(d->currentGestureDetails->command);
        Q_EMIT devicesChanged(d->currentGestureDetails->devices);
    }
}

QString GestureController::currentGesture() const
{
    return d->currentGesture;
}

QString GestureController::command() const
{
    if (d->currentGestureDetails) {
        return d->currentGestureDetails->command;
    }
    return QString{};
}

void GestureController::setCommand(QString value)
{
    if (d->currentGestureDetails && d->currentGestureDetails->command != value) {
        d->currentGestureDetails->setCommand(value);
        Q_EMIT commandChanged(value);
    }
}

QStringList GestureController::devices() const
{
    if (d->currentGestureDetails) {
        return d->currentGestureDetails->devices;
    }
    return QStringList{};
}

void GestureController::setDevices(QStringList value)
{
    if (d->currentGestureDetails && d->currentGestureDetails->devices != value) {
        d->currentGestureDetails->setDevices(value);
        Q_EMIT devicesChanged(value);
    }
}

void GestureController::gestureDetected(const QString& gestureId)
{
    d->gestureDetected(gestureId);
}
