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
#include "CommandModel.h"
#include "DeviceModel.h"
#include "GearBase.h"
#include "GestureDetectorModel.h"
#include "GestureSensor.h"
#include "WalkingSensorGestureReconizer.h"

// #include <QSensorGestureManager>
#include <QSensorManager>
#include <QSettings>
#include <QTimer>

class GestureController::Private {
public:
    Private(GestureController* qq)
        : q(qq)
        , connectionManager(nullptr)
    {
        model = new GestureDetectorModel(qq);

        QList<GestureSensor*> gestureSensors;
        gestureSensors << new WalkingSensor(q);

        for (GestureSensor *sensor : gestureSensors) {
            QObject::connect(sensor, &GestureSensor::detected, qq, &GestureController::gestureDetected);
            for (const QString& gestureName : sensor->recognizerSignals()) {
                GestureDetails* gesture = new GestureDetails(gestureName, sensor, q);
                model->addGesture(gesture);
            }
        }
    }
    ~Private() { }
    GestureController* q{nullptr};
    GestureDetectorModel* model{nullptr};
    BTConnectionManager* connectionManager{nullptr};

    void gestureDetected(const QString& gestureId) {
        qDebug() << gestureId << "detected";
        GestureDetails* gesture = model->gesture(gestureId);
        if (gesture && !gesture->command().isEmpty() && gesture->sensorEnabled()) {
            qDebug() << "We have a gesture with a command set, send that to our required devices, which are (empty means all):" << gesture->devices();
            DeviceModel * deviceModel = qobject_cast<DeviceModel *>(connectionManager->deviceModel());
            // First get the command from the core model...
            CommandModel * commandModel = qobject_cast<CommandModel *>(connectionManager->commandModel());
            CommandInfo cmd = commandModel->getCommand(gesture->command());
            for (int i = 0 ; i < deviceModel->count() ; ++i) {
                GearBase* device = deviceModel->getDeviceById(i);
                qDebug() << device->deviceID() << "of class type" << device->metaObject()->className() << "is connected?" << device->isConnected() << "is the command available?" << device->commandModel->isAvailable(cmd) << "with the command being" << cmd.command << "and is supposed to be a recipient of this command?" << (gesture->devices().count() == 0 || gesture->devices().contains(device->deviceID()));
                // Now check if the device is connected, the device model says that command is available,
                // and that it's supposed to be a recipient
                if (device->isConnected() && device->commandModel->isAvailable(cmd)
                    && (gesture->devices().count() == 0 || gesture->devices().contains(device->deviceID()))) {
                    device->sendMessage(gesture->command());
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
}

BTConnectionManager *GestureController::connectionManager() const
{
    return d->connectionManager;
}

void GestureController::gestureDetected(const QString& gestureId)
{
    d->gestureDetected(gestureId);
}

void GestureController::setGestureDetails(int index, QString command, QStringList devices)
{
    d->model->setGestureDetails(index, command, devices);
}

void GestureController::setGestureSensorEnabled(int index, bool enabled)
{
    d->model->setGestureSensorEnabled(index, enabled);
}

void GestureController::setGestureSensorPinned(int index, bool pinned)
{
    d->model->setGestureSensorPinned(index, pinned);
}

GestureDetectorModel * GestureController::model() const
{
    return d->model;
}
