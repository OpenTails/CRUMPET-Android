/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "GearDigitail.h"

#include <KLocalizedString>

#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include "AppSettings.h"
#include "CommandPersistence.h"

class GearDigitail::Private {
public:
    Private(GearDigitail* qq)
        : q(qq)
    {}
    ~Private() {}
    GearDigitail* q{nullptr};
    DeviceModel * parentModel{nullptr};

    QString version = QLatin1String{"(unknown)"};
    QString currentCall;
    int batteryLevel{-1};

    QLowEnergyController* btControl{nullptr};
    QLowEnergyService* tailService{nullptr};
    QLowEnergyCharacteristic tailCharacteristic;
    QLowEnergyDescriptor tailDescriptor;

    QTimer batteryTimer;
    QBluetoothUuid tailStateCharacteristicUuid{QLatin1String("{0000ffe1-0000-1000-8000-00805f9b34fb}")};

    int reconnectThrottle{0};
    void reconnectDevice(QObject* context)
    {
        QTimer::singleShot(100, context, [this] {
            if (btControl) {
                if (reconnectThrottle > 10) {
                    q->disconnectDevice();
                    reconnectThrottle = 0;
                    q->deviceMessage(q->deviceID(), i18nc("Error message shown when automatic reconnection has been attempted too often", "Attempted to reconnect too many times to %1 (%2). To connect to it, please check that it is on, charged, and near enough.", q->name(), q->deviceID()));
                    return;
                }
                qDebug() << q->name() << q->deviceID() << "Connection lost - attempting to reconnect.";
                q->deviceMessage(q->deviceID(), i18nc("A status message sent when the connection to a device has been lost, and we are attempting to connect again automatically", "Connection lost to %1, attempting to reconnect...", q->name()));
                ++reconnectThrottle;
                btControl->connectToDevice();
            }
        });
    }

    void connectToDevice()
    {
        qDebug() << q->name() << q->deviceID() << "Attempting to connect to device";
        q->connectDevice();
    }

    void serviceStateChanged(QLowEnergyService::ServiceState s)
    {
        switch (s) {
        case QLowEnergyService::RemoteServiceDiscovering:
            qDebug() << q->name() << q->deviceID() << "Discovering services...";
            break;
        case QLowEnergyService::RemoteServiceDiscovered:
        {
            qDebug() << q->name() << q->deviceID() << "Service discovered.";

            for(const QLowEnergyCharacteristic& leChar : tailService->characteristics()) {
                qDebug() << q->name() << q->deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
            }
            tailCharacteristic = tailService->characteristic(tailStateCharacteristicUuid);
            if (!tailCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "Tail characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), i18nc("A message when sent when attempting to connect to a device which does not have a specific expected feature", "It looks like this device is not a DIGITAiL (could not find the tail characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            q->commandModel->clear();

            // Get the descriptor, and turn on notifications
            tailDescriptor = tailCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            tailService->writeDescriptor(tailDescriptor, QByteArray::fromHex("0100"));

            reconnectThrottle = 0;
            Q_EMIT q->isConnectedChanged(q->isConnected());
            q->setIsConnecting(false);
            q->sendMessage(QLatin1String{"VER"}); // Ask for the tail version, and then react to the response...

            break;
        }
        default:
            //nothing for now
            break;
        }
    }

    QString previousThing;
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        qDebug() << q->name() << q->deviceID() << "Current call is" << currentCall << "and characteristic" << characteristic.uuid() << "NOTIFIED value change" << newValue;

        if (tailStateCharacteristicUuid == characteristic.uuid()) {
            if (currentCall == QLatin1String("VER")) {
                q->reloadCommands();
                version = QString::fromUtf8(newValue);
                Q_EMIT q->versionChanged(version);
                batteryTimer.start();
                q->sendMessage(QLatin1String{"BATT"});
            }
            else {
                static const QLatin1String aBegin{"BEGIN"};
                static const QLatin1String anEnd{"END"};
                QString theValue = QString::fromUtf8(newValue);
                QStringList stateResult = theValue.split(QLatin1Char{' '});
                // Return value for BATT calls is BAT and a number, from 0 to 4,
                // unfortunately without a space, so we have to specialcase it a bit
                if(theValue.startsWith(QLatin1String("BAT"))) {
                    batteryLevel = theValue.right(1).toInt();
                    q->setBatteryLevelPercent(batteryLevel * 25);
                    Q_EMIT q->batteryLevelChanged(batteryLevel);
                }
                else if(stateResult.count() == 1 && !previousThing.isEmpty()) {
                    // See comment in the next bit to see what this is about
                    previousThing += stateResult[0];
                    QStringList splitThing = previousThing.split(QLatin1Char{' '});
                    previousThing.clear();
                    q->commandModel->setRunning(splitThing[1], (splitThing[0] == aBegin));
                    qDebug() << "We had a remainder, and the split is now" << splitThing;
                }
                else if(stateResult.count() == 3 && (stateResult[0] == aBegin || stateResult[0] == anEnd)) {
                    // This is an awkward thing that happens sometimes (and especially when a command is forced to
                    // stop by starting another) - specifically, we will get two results happening in quick succession,
                    // which is the expected result from two calls, just concatenated, and then split over two changes
                    // depending on how much space the characteristic can hold (for example "END TAILS1BEGIN TAIL" and
                    // "HM", or alternatively the other way around)
                    QString theCommand;
                    QString startOrEnd;
                    if(stateResult[1].endsWith(aBegin)) {
                        theCommand = stateResult[1].left(stateResult[1].length() - 5);
                        startOrEnd = aBegin;
                    }
                    else if(stateResult[1].endsWith(anEnd)) {
                        theCommand = stateResult[1].left(stateResult[1].length() - 3);
                        startOrEnd = anEnd;
                    }
                    q->commandModel->setRunning(theCommand, (stateResult[0] == aBegin));
                    // Now let's just see whether that second thing is actually a full command or not (at which point
                    // we should be expecting another notification shortly)
                    bool fullCommand{false};
                    for(const CommandInfo& cmd : std::as_const(q->commandModel->allCommands())) {
                        if (cmd.command == stateResult[2]) {
                            fullCommand = true;
                            break;
                        }
                    }
                    if (fullCommand) {
                        q->commandModel->setRunning(stateResult[2], (startOrEnd == aBegin));
                        qDebug() << "Detected a complete squashed command, with the command" << theCommand << ", the type" << stateResult[0] << ", the startOrEnd" << startOrEnd << ", and the end command" << stateResult[2];
                    } else {
                        previousThing = QString::fromUtf8("%1 %2").arg(startOrEnd).arg(stateResult[2]);
                        qDebug() << "Detected an incomplete squashed command, with the command" << theCommand << ", the type" << stateResult[0] << ", and the remainder" << previousThing;
                    }
                }
                else if(stateResult.count() == 2) {
                    if(stateResult[0] == aBegin) {
                        q->commandModel->setRunning(stateResult[1], true);
                    }
                    else if(stateResult[0] == anEnd) {
                        q->commandModel->setRunning(stateResult[1], false);
                    }
                    else {
                        qDebug() << q->name() << q->deviceID() << "Unexpected response: The first element of the two part message should be either BEGIN or END";
                    }
                }
                else {
                    qDebug() << q->name() << q->deviceID() << "Unexpected response: The response should consist of a string of two words separated by a single space, the first word being either BEGIN or END, and the second should be the command name either just beginning its run, or having just ended its run.";
                }
            }
        }
        currentCall.clear();
        Q_EMIT q->currentCallChanged(currentCall);
    }

    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        qDebug() << q->name() << q->deviceID() << "Characteristic written:" << characteristic.uuid() << newValue;
        currentCall = QString::fromUtf8(newValue);
        Q_EMIT q->currentCallChanged(currentCall);
    }
};

GearDigitail::GearDigitail(const QBluetoothDeviceInfo& info, DeviceModel * parent)
    : GearBase(info, parent)
    , d(new Private(this))
{
    d->parentModel = parent;
    setHasLights(true);

    // The battery timer also functions as a keepalive call. If it turns
    // out to be a problem that we pull the battery this often, we can
    // add a separate ping keepalive functon.
    connect(&d->batteryTimer, &QTimer::timeout,
            [this](){ if(d->currentCall.isEmpty()) { sendMessage(QLatin1String{"BATT"}); } });

    d->batteryTimer.setTimerType(Qt::VeryCoarseTimer);
    d->batteryTimer.setInterval(60000 / 2);
    d->batteryTimer.setSingleShot(false);
}

GearDigitail::~GearDigitail()
{
    delete d;
}

void GearDigitail::connectDevice()
{
    setIsConnecting(true);
    if(d->btControl) {
        disconnectDevice();
    }

    d->btControl = QLowEnergyController::createCentral(deviceInfo, this);
    d->btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);

    if(d->tailService) {
        d->tailService->deleteLater();
        d->tailService = nullptr;
    }

    connect(d->btControl, &QLowEnergyController::serviceDiscovered,
        [this](const QBluetoothUuid &gatt){
            qDebug() << name() << deviceID() << "service discovered" << gatt;
        });

    connect(d->btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << name() << deviceID()<< "Done!";
                QLowEnergyService *service = d->btControl->createServiceObject(QBluetoothUuid(QLatin1String("{0000ffe0-0000-1000-8000-00805f9b34fb}")));

                if (!service) {
                    qWarning() << "Cannot create QLowEnergyService for {0000ffe0-0000-1000-8000-00805f9b34fb}";
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning message when the main service was not found on the device", "An error occured while connecting to your DIGITAiL (the service object could not be created). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }

                d->tailService = service;
                connect(d->tailService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){ d->serviceStateChanged(newState); });
                connect(d->tailService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicChanged(info, value); });
                connect(d->tailService, &QLowEnergyService::characteristicWritten, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicWritten(info, value); });
                d->tailService->discoverDetails();
            });

    connect(d->btControl, &QLowEnergyController::errorOccurred,
        this, [this](QLowEnergyController::Error error) {
            qDebug() << name() << deviceID() << "Cannot connect to remote device." << error;

            switch(error) {
                case QLowEnergyController::UnknownError:
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning that some unknown error happened", "An error occurred. If you are trying to connect to your DIGITAiL, make sure it is on and close to this device."));
                    break;
                case QLowEnergyController::RemoteHostClosedError:
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning that the device disconnected itself", "The DIGITAiL closed the connection."));
                    break;
                case QLowEnergyController::ConnectionError:
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning that some connection failure occurred (usually due to low signal strength)", "Failed to connect to your DIGITAiL. Please try again (perhaps move it closer?)"));
                    break;
                default:
                    break;
            }

            if (d->parentModel->appSettings()->autoReconnect()) {
                d->reconnectDevice(this);
            } else {
                disconnectDevice();
            }
        });

    connect(d->btControl, &QLowEnergyController::connected, this, [this]() {
        qDebug() << name() << deviceID() << "Controller connected. Search services...";
        d->btControl->discoverServices();
    });

    connect(d->btControl, &QLowEnergyController::disconnected, this, [this]() {
        qDebug() << name() << deviceID() << "LowEnergy controller disconnected";
        Q_EMIT deviceMessage(deviceID(), i18nc("Warning that the device itself disconnected during operation (usually due to turning off from low power)", "The DIGITAiL closed the connection, either by being turned off or losing power. Remember to charge your DIGITAiL!"));
        disconnectDevice();
        deleteLater();
    });

    // Connect
    d->btControl->connectToDevice();
}

void GearDigitail::disconnectDevice()
{
    d->batteryTimer.stop();
    d->btControl->deleteLater();
    d->btControl = nullptr;
    d->tailService->deleteLater();
    d->tailService = nullptr;
    commandModel->clear();
    commandShorthands.clear();
//     Q_EMIT commandModelChanged();
//     commandQueue->clear(); // FIXME Clear commands for this device only
//     Q_EMIT commandQueueChanged();
    d->batteryLevel = -1;
    Q_EMIT batteryLevelChanged(0);
    Q_EMIT isConnectedChanged(isConnected());
    setIsConnecting(false);
}

bool GearDigitail::isConnected() const
{
    return d->btControl;
}

QString GearDigitail::version() const
{
    return d->version;
}

int GearDigitail::batteryLevel() const
{
    return d->batteryLevel;
}

QString GearDigitail::currentCall() const
{
    return d->currentCall;
}

void GearDigitail::sendMessage(const QString &message)
{
    // Don't send out another call while we're waiting to hear back... at least for a little bit
    int i = 0;
    while(!d->currentCall.isEmpty()) {
        if(++i == 1000) {
            break;
        }
        qApp->processEvents();
    }
    if (d->tailCharacteristic.isValid() && d->tailService) {
        d->tailService->writeCharacteristic(d->tailCharacteristic, message.toUtf8());
        d->currentCall = message;
        Q_EMIT currentCallChanged(message);

        // It is unfortunate, but we actually need to do this, as we will occasionally run into situations where we cannot trust
        // the tail to report correctly when a command starts and ends. In short, in stead of getting the expected "END commandname"
        // and "BEGIN commandname" notifications, we will occasionally get both of those mashed together in a single notification
        // which further doesn't quite have everything we need (due to the character limitation on btle values), so we have to
        // do a little internal state tracking. A shame, but here we are, and we just need to deal with that.
        commandModel->setRunning(message, true);
    }
}
