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
#include "GestureDetectorModel.h"
#include "WalkingSensorGestureReconizer.h"

#include <QSensorGestureManager>
#include <QSettings>
#include <QTimer>

class GestureController::Private {
public:
    Private(GestureController* qq)
        : q(qq)
        , connectionManager(nullptr)
    {
        model = new GestureDetectorModel(qq);
        QSensorGestureManager manager;

        auto walkingSensor = new WalkingSensorGestureReconizer;
        manager.registerSensorGestureRecognizer(walkingSensor);

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
                GestureDetails* gesture = new GestureDetails(signalName, sensor, q);
                gestures[signalName] = gesture;
                model->addGesture(gesture);
            }
        }
    }
    ~Private() {
        gestures.clear();
    }
    GestureController* q;
    GestureDetectorModel* model;
    BTConnectionManager* connectionManager;

    // This holds the gestures by their detector IDs (see gestureDetected(QString))
    QHash<QString, GestureDetails*> gestures;

    void gestureDetected(const QString& gestureId) {
        qDebug() << gestureId << "detected";
        GestureDetails* gesture = gestures.value(gestureId);
        if (gesture && !gesture->command().isEmpty()) {
            qDebug() << "We have a gesture with a command set, send that to our required devices, which are (empty means all):" << gesture->devices();
            BTDeviceModel* deviceModel = qobject_cast<BTDeviceModel*>(connectionManager->deviceModel());
            // First get the command from the core model...
            BTDeviceCommandModel* commandModel = qobject_cast<BTDeviceCommandModel*>(connectionManager->commandModel());
            CommandInfo cmd = commandModel->getCommand(gesture->command());
            for (int i = 0 ; i < deviceModel->count() ; ++i) {
                BTDevice* device = deviceModel->getDeviceById(i);
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
