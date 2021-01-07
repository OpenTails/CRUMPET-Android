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
#include "CommandQueue.h"

#include <QTimer>
#include <QSensorGestureManager>

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
        // load commands for this gesture
        // load devices for this gesture
        qDebug() << "IMPLEMENT ME - loading for" << gestureId;
    }
    void save() {
        // save commands and devices for this gesture
        qDebug() << "IMPLEMENT ME - saving for" << gestureId;
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
        , appSettings(nullptr)
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
    AppSettings* appSettings;
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
            if (enabled) {
                i.value()->sensor->startDetection();
            } else {
                i.value()->sensor->stopDetection();
            }
        }
        Q_EMIT q->enabledChanged(enabled);
    }

    void gestureDetected(const QString& gestureId) {
        qDebug() << gestureId;
        CommandQueue* commandQueue = qobject_cast<CommandQueue*>(connectionManager->commandQueue());
        if (commandQueue->count() == 0 && commandQueue->currentCommandRemainingMSeconds() == 0) {
            GestureDetails* gesture = gestures.value(gestureId);
            if (gesture && !gesture->command.isEmpty()) {
                commandQueue->pushCommand(gesture->command, gesture->devices);
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

void GestureController::setAppSettings(AppSettings* settings)
{
    if(d->appSettings) {
        d->appSettings->disconnect(this);
    }
    d->appSettings = settings;
    connect(d->appSettings, &AppSettings::idleModeChanged, this, [this](bool idleMode){
        if (d->connectionManager) {
            d->setEnabled(idleMode && d->connectionManager->isConnected());
        }
    });
}

void GestureController::setConnectionManager(BTConnectionManager* connectionManager)
{
    if(d->connectionManager) {
        d->connectionManager->disconnect(this);
    }
    d->connectionManager = connectionManager;
    connect(d->connectionManager, &BTConnectionManager::isConnectedChanged, this, [this](bool isConnected){
        if (d->appSettings) {
            d->setEnabled(isConnected && d->appSettings->idleMode());
        }
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
