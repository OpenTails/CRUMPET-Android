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

#include "GearMitailMini.h"

#include <KLocalizedString>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QTimer>

#include "AppSettings.h"

class GearMitailMini::Private {
public:
    Private(GearMitailMini* qq)
        : q(qq)
    {
        knownFirmwareMessages[QLatin1String{"VER 4.0.3"}] = i18nc("A message displayed to the user when firmware version 4.03 is installed on their gear, with a description of why they should upgrade, and how", "Your MiTail Mini currently has version 4.03 firmware installed, which is outdated and has some known issues! We would <b>strongly recommend</b> updating to the newest firmware which fixes them! Head over to Settings and find the Firmware section to perform the update.");
    }
    ~Private() {}
    GearMitailMini* q{nullptr};
    DeviceModel * parentModel{nullptr};

    QHash<QString, QString> knownFirmwareMessages;
    QString version{"(unknown)"};
    int batteryLevel{0};

    QString currentCall;
    QString currentSubCall;
    QStringList callQueue;

    QLowEnergyController* btControl{nullptr};
    QLowEnergyService* deviceService{nullptr};
    QLowEnergyCharacteristic deviceCommandWriteCharacteristic;
    QLowEnergyCharacteristic deviceCommandReadCharacteristic;

    QLowEnergyService* batteryService{nullptr};
    QLowEnergyCharacteristic batteryCharacteristic;
    QLowEnergyCharacteristic deviceChargingReadCharacteristic;

    QTimer pingTimer;
    QBluetoothUuid deviceServiceUuid{QLatin1String{"3af2108b-d066-42da-a7d4-55648fa0a9b6"}};
    QBluetoothUuid deviceCommandReadCharacteristicUuid{QLatin1String("{c6612b64-0087-4974-939e-68968ef294b0}")};
    QBluetoothUuid deviceCommandWriteCharacteristicUuid{QLatin1String("{5bfd6484-ddee-4723-bfe6-b653372bbfd6}")};
    QBluetoothUuid deviceChargingReadCharacteristicUuid{QLatin1String("{5073792e-4fc0-45a0-b0a5-78b6c1756c91}")};

    int reconnectThrottle{0};
    void reconnectDevice(QObject* context)
    {
        QTimer::singleShot(0, context, [this] {
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
        case QLowEnergyService::DiscoveringServices:
            qDebug() << q->name() << q->deviceID() << "Discovering services...";
            break;
        case QLowEnergyService::ServiceDiscovered:
        {
            qDebug() << q->name() << q->deviceID() << "Service discovered.";

            foreach(const QLowEnergyCharacteristic& leChar, deviceService->characteristics()) {
                qDebug() << q->name() << q->deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
            }

            deviceCommandWriteCharacteristic = deviceService->characteristic(deviceCommandWriteCharacteristicUuid);
            if (!deviceCommandWriteCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "MiTail Mini command writing characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), i18nc("A message when sent when attempting to connect to a device which does not have a specific expected feature", "It looks like this device is not a MiTail Mini (could not find the main device writing characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            deviceCommandReadCharacteristic = deviceService->characteristic(deviceCommandReadCharacteristicUuid);
            if (!deviceCommandReadCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "MiTail Mini command reading characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), i18nc("A message when sent when attempting to connect to a device which does not have a specific expected feature", "It looks like this device is not a MiTail Mini (could not find the main device reading characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            q->commandModel->clear();

            // Get the descriptor, and turn on notifications
            QLowEnergyDescriptor commandUpdateDescriptor = deviceCommandReadCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (deviceCommandReadCharacteristic.properties() & QLowEnergyCharacteristic::Notify) {
                deviceService->writeDescriptor(commandUpdateDescriptor, QByteArray::fromHex("0100"));
            }
            if (deviceCommandReadCharacteristic.properties() & QLowEnergyCharacteristic::Indicate) {
                deviceService->writeDescriptor(commandUpdateDescriptor, QByteArray::fromHex("0200"));
            }

            reconnectThrottle = 0;
            emit q->isConnectedChanged(q->isConnected());
            deviceService->writeCharacteristic(deviceCommandWriteCharacteristic, "VER"); // Ask for the version, and then react to the response...

            break;
        }
        default:
            //nothing for now
            break;
        }
    }

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        qDebug() << q->name() << q->deviceID() << "Current call is supposed to be" << currentCall << "and characteristic" << characteristic.uuid() << "NOTIFIED value change" << newValue;

        if (deviceCommandReadCharacteristicUuid == characteristic.uuid()) {
            QString theValue(newValue);
            if (theValue.endsWith("\x00")) {
                theValue = theValue.left(theValue.length());
            }
            QStringList stateResult = theValue.split(' ');
            if (theValue == QLatin1String{"System is busy now"}) {
                // Postpone what we attempted to send a few moments before trying again, as the device is currently busy
                QTimer::singleShot(1000, q, [this](){ q->sendMessage(currentSubCall); });
            }
            else if (stateResult[0] == QLatin1String{"VER"}) {
                q->reloadCommands();
                version = newValue;
                emit q->versionChanged(newValue);
                q->setKnownFirmwareMessage(knownFirmwareMessages.value(version, QLatin1String{}));
                pingTimer.start();
                if (firmwareProgress > -1) {
                    if (otaVersion == newValue) {
                        // successful update get!
                        q->deviceBlockingMessage(i18nc("Title of the message box shown to the user upon a successful firmware upgrade", "Upgrade Successful"), i18nc("Message shown to the user when a firmware update completed successfully", "Congratulations, your gear has been successfully updated to version %1!", version));
                    } else {
                        // sadface, update failed...
                        q->deviceBlockingMessage(i18nc("Title of the message box shown to the user upon an unsuccessful firmware upgrade", "Update Failed"), i18nc("Message shown to the user when a firmware update failed", "<p>Sorry, but the upgrade failed. Most often this is due to the transfer being corrupted during the upload process itself, which is why your gear has a safe fallback to just go back to your old firmware version upon a failure. You can try the update again by clicking the Install button gain.</p>"));
                    }
                    q->setProgressDescription("");
                    q->setDeviceProgress(-1);
                    firmwareProgress = -1;
                    firmwareChunk.clear();
                } else {
                    // Logic here is, the user explicitly picks what to do when disconnecting the app from a tail
                    q->sendMessage("STOPNPM");
                }
            }
            else if (stateResult[0] == QLatin1String{"GLOWTIP"}) {
                if (stateResult[1] == QLatin1String{"TRUE"}) {
                    q->setHasLights(true);
                } else {
                    q->setHasLights(false);
                }
            }
            else if (stateResult[0] == QLatin1String{"PONG"} || stateResult[0] == QLatin1String{"OK"}) {
                if (currentCall != QLatin1String{"PING"}) {
                    qWarning() << q->name() << q->deviceID() << "We got an out-of-order response for a ping";
                }
            }
            else if (theValue.startsWith(QLatin1String{"MiTail Mini started"})) {
                qDebug() << q->name() << q->deviceID() << "MiTail Mini detected the connection";
            }
            else if (stateResult[0] == QLatin1String("OTA") || firmwareProgress > -1) {
                qDebug() << "Firmware update is happening...";
            }
            else if (stateResult.last() == QLatin1String{"BEGIN"}) {
                q->commandModel->setRunning(currentCall, true);
                // ****************************************************
                // ******************* EARLY RETURN *******************
                // ****************************************************
                return;
            }
            else if (stateResult.last() == QLatin1String{"END"}) {
                // If we've got more in the queue, send the next bit of the command
                if (callQueue.length() > 0) {
                    int pauseDuration{0};
                    QString message = callQueue.takeFirst();
                    while (message.startsWith(QLatin1String{"PAUSE"})) {
                        QStringList pauseCommand = message.split(QChar{' '});
                        int pause = pauseCommand.value(1).toInt();
                        pauseDuration += pause;
                        message = callQueue.takeFirst();
                        qDebug() << q->name() << q->deviceID() << "Found a pause, so we're now waiting" << pauseDuration << "milliseconds";
                    }
                    if (pauseDuration > 0) {
                        // Just in case some funny person stuck a pause at the end...
                        if (message.length() > 0) {
                            // Clamp the max single pause duration to 3000 ms (the conceptual human moment)
                            QTimer::singleShot(qMax(3000, pauseDuration), q, [this, message](){ q->sendMessage(message); });
                        }
                    }
                    else {
                        q->sendMessage(message);
                    }
                    // ****************************************************
                    // ******************* EARLY RETURN *******************
                    // ****************************************************
                    return;
                } else {
                    // If the queue is empty, we're done
                    q->commandModel->setRunning(currentCall, false);
                }
            }
            else {
                qDebug() << q->name() << q->deviceID() << "Unexpected response: Did not understand" << newValue;
            }
        }
        currentCall.clear();
        emit q->currentCallChanged(currentCall);
    }

    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        if (firmwareProgress > -1) {
            if (firmwareProgress < firmware.size()) {
                static const int MTUSize{500}; // evil big size for a start, hopefully should be ok, but let's see if we get any reports...
                firmwareChunk = firmware.mid(firmwareProgress, MTUSize);
                firmwareProgress += firmwareChunk.size();
                deviceService->writeCharacteristic(deviceCommandWriteCharacteristic, firmwareChunk);
                q->setDeviceProgress(1 + (99 * (firmwareProgress / (double)firmware.size())));
                qDebug() << q->name() << q->deviceID() << "Uploading firmware:" << 1 + (99 * (firmwareProgress / (double)firmware.size())) << "%, or" << firmwareProgress << "of" << firmware.size() << "The newValue value was of length" << newValue.length();
            } else {
                // we presumably just rebooted...
                qDebug() << "We presumably just rebooted?";
            }
        } else {
            qDebug() << q->name() << q->deviceID() << "Characteristic written:" << characteristic.uuid() << newValue;
            currentCall = newValue;
            emit q->currentCallChanged(currentCall);
        }
    }


    QByteArray firmware;
    QString otaVersion;
    QUrl firmwareUrl;
    QString firmwareMD5;
    QByteArray firmwareChunk;
    int firmwareProgress{-1};

    enum DownloadOperation {
        NoDownloadOperation,
        DownloadingOTAInformation,
        DownloadingOTAData,
    };
    DownloadOperation downloadOperation{NoDownloadOperation};
    QNetworkAccessManager qnam;
    QPointer<QNetworkReply> networkReply;
    void handleRedirect(QNetworkReply *reply)
    {
        QNetworkAccessManager *qnam = reply->manager();
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }
        const QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (!possibleRedirectUrl.isEmpty()) {
            const QUrl redirectUrl = reply->url().resolved(possibleRedirectUrl);
            if (redirectUrl == reply->url()) {
                // no infinite redirections thank you very much
                reply->deleteLater();
                return;
            }
            reply->deleteLater();
            QNetworkRequest request(possibleRedirectUrl);
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
            networkReply = qnam->get(request);
            connect(networkReply.data(), &QNetworkReply::downloadProgress, q, [this](quint64 received, quint64 total){
                if (total > 0) {
                    q->setDeviceProgress(100 * (received/(double)total));
                } else {
                    q->setDeviceProgress(0);
                }
            });
            connect(networkReply.data(), &QNetworkReply::finished, q, [this]() {
                handleFinished(networkReply);
            });
        }
    }
    void handleFinished(QNetworkReply* reply) {
        reply->deleteLater();
        if (reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull()) {
            // Then we are doing the thing
            QByteArray downloadedData = reply->readAll();
            if (downloadOperation == DownloadingOTAInformation) {
                // The OTA information is a json object with three properties (the version, the md5sum, and the url for the firmware payload)
                QJsonDocument document = QJsonDocument::fromJson(downloadedData);
                if (document.isObject()) {
                    QJsonObject fwInfoObj = document.object();
                    firmwareUrl = fwInfoObj.value("url").toString();
                    firmwareMD5 = fwInfoObj.value("md5sum").toString();
                    otaVersion = fwInfoObj.value("version").toString();
                    if (otaVersion == version) {
                        q->deviceMessage(q->deviceID(), i18nc("Message shown to the user when they already have the newest firmware installed", "You already have the newest version of the firmware installed on your gear, congratulations!"));
                    }
                    Q_EMIT q->hasAvailableOTAChanged();
                } else {
                    qDebug() << q->name() << q->deviceID() << downloadedData;
                    q->deviceMessage(q->deviceID(), i18nc("Warning message for when the firmware update information file did not contain a JSON object", "The file used to determine information about new firmware versions did not contain the expected format of data. This is likely a temporary error, or a connection issue. If you run into this problem repeatedly, please get in touch."));
                }
            } else if (downloadOperation == DownloadingOTAData) {
                q->setDeviceProgress(0);
                q->setProgressDescription(i18nc("Message for when we are checking the downloaded firmware data", "Checking integrity of the downloaded firmware data..."));
                q->setOTAData(firmwareMD5, downloadedData);
            }
            q->setDeviceProgress(-1);
            q->setProgressDescription(QString{});
            downloadOperation = NoDownloadOperation;
        } else {
            // If it has contents, we're redirecting
            handleRedirect(reply);
        }
    }
};

GearMitailMini::GearMitailMini(const QBluetoothDeviceInfo& info, DeviceModel * parent)
    : GearBase(info, parent)
    , d(new Private(this))
{
    d->parentModel = parent;
    setSupportsOTA(true);
    setHasLights(true); // Just in case someone has an old firmware loaded
    setHasShutdown(true);
    setHasNoPhoneMode(true);
    setNoPhoneModeGroups({
        i18nc("Name of the calm and relaxed group as used for no phone group selection", "Calm and Relaxed"),
        i18nc("Name of the fast and excited group as used for no phone group selection", "Fast and Excited"),
        i18nc("Name of the frustrated and tense group as used for no phone group selection", "Frustrated and Tense"),
    });

    // The battery timer also functions as a keepalive call. If it turns
    // out to be a problem that we pull the battery this often, we can
    // add a separate ping keepalive functon.
    connect(&d->pingTimer, &QTimer::timeout,
            [this](){ if(d->currentCall.isEmpty() && d->firmwareProgress == -1) { sendMessage("PING"); } });

    d->pingTimer.setTimerType(Qt::VeryCoarseTimer);
    d->pingTimer.setInterval(60000 / 2);
    d->pingTimer.setSingleShot(false);
}

GearMitailMini::~GearMitailMini()
{
    delete d;
}

void GearMitailMini::connectDevice()
{
    if(d->btControl) {
        disconnectDevice();
    }

    d->btControl = QLowEnergyController::createCentral(deviceInfo, this);
    d->btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);

    if(d->deviceService) {
        d->deviceService->deleteLater();
        d->deviceService = nullptr;
    }

    connect(d->btControl, &QLowEnergyController::serviceDiscovered,
        [this](const QBluetoothUuid &gatt){
            qDebug() << name() << deviceID() << "service discovered" << gatt;
        });

    connect(d->btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << name() << deviceID()<< "Done!";

                // Main control service
                d->deviceService = d->btControl->createServiceObject(d->deviceServiceUuid);
                if (!d->deviceService) {
                    qWarning() << "Cannot create QLowEnergyService for " << d->deviceServiceUuid;
                    emit deviceMessage(deviceID(), i18nc("Warning message when a fault occurred during a connection attempt", "An error occurred while connecting to your MiTail Mini (the main service object could not be created). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }

                connect(d->deviceService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){ d->serviceStateChanged(newState); });
                connect(d->deviceService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicChanged(info, value); });
                connect(d->deviceService, &QLowEnergyService::characteristicWritten, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicWritten(info, value); });
                connect(d->deviceService, QOverload<QLowEnergyService::ServiceError>::of(&QLowEnergyService::error), this, [this](QLowEnergyService::ServiceError newError){
                    qDebug() << name() << deviceID() << "Error occurred for service:" << newError;
                    if (newError == QLowEnergyService::CharacteristicWriteError && d->firmwareProgress > -1) {
                        // This will usually be the android error GATT_INVALID_ATTRIBUTE_LENGTH, which should not usually happen
                        // but as we can't actually read the MTU size out of QLowEnergyCharacteristic until 6.2, we'll just
                        // have to... do a thing and ask people to report back for now.
                        QTimer::singleShot(10000, this, [this](){
                            setDeviceProgress(-1);
                            setProgressDescription("");
                        });
                        setDeviceProgress(0);
                        setProgressDescription(i18nc("Message asking people to tell us when a firmware update failed, and that this is the error they got", "<p><b>Update Failed!</b></p><p>We have tried to update your firmware too rapidly for your device, and have had to abort. If you are getting this error:</p><p>Firstly, don't worry, your gear is safe.</p><p>Secondly, please contact us on info@thetailcompany.com and tell us that you got this error.</p>"));
                        d->firmwareProgress = -1;
                        d->firmwareChunk.clear();
                    }
                });
                d->deviceService->discoverDetails();

                // Battery service
                d->batteryService = d->btControl->createServiceObject(QBluetoothUuid::BatteryService);
                if (!d->batteryService) {
                    qWarning() << "Failed to create battery service";
                    emit deviceMessage(deviceID(), i18nc("Warning message when the battery information is unavailable on a device", "An error occurred while connecting to your MiTail Mini (the battery service was not available). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }
                else {
                    connect(d->batteryService, &QLowEnergyService::characteristicRead, this, [this](const QLowEnergyCharacteristic &, const QByteArray &value){
                        if (value.length() > 0) {
                            d->batteryLevel = (int)value.at(0) / 20;
                            setBatteryLevelPercent((int)value.at(0));
                            qDebug() << name() << deviceID() << "Updated battery to" << value;
                            emit batteryLevelChanged(d->batteryLevel);
                        }
                    });
                    connect(d->batteryService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic& characteristic, const QByteArray& value){
                        if (characteristic.uuid() == d->deviceChargingReadCharacteristicUuid) {
                            QString theValue(value);
                            if (theValue.endsWith("\x00")) {
                                theValue = theValue.left(theValue.length());
                            }
                            QStringList stateResult = theValue.split(' ');
                            if (stateResult[0] == QLatin1String{"CHARGING"}) {
                                if (stateResult[1] == QLatin1String{"ON"}) {
                                    setChargingState(1);
                                } else if (stateResult[1] == QLatin1String{"FULL"}) {
                                    setChargingState(2);
                                } else {
                                    setChargingState(0);
                                }
                            }
                        }
                        else {
                            if (value.length() > 0) {
                                d->batteryLevel = (int)value.at(0) / 20;
                                setBatteryLevelPercent((int)value.at(0));
                                emit batteryLevelChanged(d->batteryLevel);
                            }
                        }
                    });
                    connect(d->batteryService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){
                        switch (newState) {
                        case QLowEnergyService::DiscoveringServices:
                            qDebug() << name() << deviceID() << "Discovering battery services...";
                            break;
                        case QLowEnergyService::ServiceDiscovered:
                        {
                            qDebug() << name() << deviceID() << "Battery service discovered";

                            foreach(const QLowEnergyCharacteristic& leChar, d->batteryService->characteristics()) {
                                qDebug() << name() << deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
                            }

                            d->batteryCharacteristic = d->batteryService->characteristic(QBluetoothUuid::BatteryLevel);
                            if (!d->batteryCharacteristic.isValid()) {
                                qDebug() << name() << deviceID() << "MiTail Mini battery level characteristic not found, this is bad";
                                deviceMessage(deviceID(), i18nc("Warning message when the battery information is unavailable on the device", "It looks like this device is not a MiTail Mini (could not find the battery level characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                                disconnectDevice();
                                break;
                            }

                            // Get the descriptor, and turn on notifications
                            QLowEnergyDescriptor batteryDescriptor = d->batteryCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                            if (!batteryDescriptor.isValid()) {
                                qDebug() << "This is bad, no battery descriptor...";
                            }
                            d->batteryService->writeDescriptor(batteryDescriptor, QByteArray::fromHex("0100"));

                            d->deviceChargingReadCharacteristic = d->batteryService->characteristic(d->deviceChargingReadCharacteristicUuid);
                            if (!d->deviceChargingReadCharacteristic.isValid()) {
                                qDebug() << name() << deviceID() << "Couldn't get the charging state characteristic - this is fine for old tails, so not getting angry about this";
                            }
                            batteryDescriptor = d->deviceChargingReadCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                            if (d->deviceChargingReadCharacteristic.properties() & QLowEnergyCharacteristic::Notify) {
                                d->batteryService->writeDescriptor(batteryDescriptor, QByteArray::fromHex("0100"));
                            }

                            d->batteryService->readCharacteristic(d->batteryCharacteristic);

                            break;
                        }
                        default:
                            //nothing for now
                            break;
                        }
                    });
                    d->batteryService->discoverDetails();
                }
            });

    connect(d->btControl, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
        this, [this](QLowEnergyController::Error error) {
            qDebug() << name() << deviceID() << "Cannot connect to remote device." << error;

            switch(error) {
                case QLowEnergyController::UnknownError:
                    emit deviceMessage(deviceID(), i18nc("Warning that some unknown error happened", "An error occurred. If you are trying to connect to your MiTail Mini, make sure it is on and close to this device."));
                    break;
                case QLowEnergyController::RemoteHostClosedError:
                    emit deviceMessage(deviceID(), i18nc("Warning that the device disconnected itself", "The MiTail Mini closed the connection."));
                    break;
                case QLowEnergyController::ConnectionError:
                    if (d->firmwareProgress > -1) {
                        emit deviceMessage(deviceID(), i18nc("Warning that some connection failure occurred (usually due to low signal strength)", "Failed to connect to your MiTail Mini. Please try again (perhaps move it closer?)"));
                    } else {
                        QTimer::singleShot(2000, this, &GearMitailMini::connectDevice);
                    }
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
        if (d->firmwareProgress > -1) {
            qDebug() << name() << deviceID() << "Rebooting after firmware installation, say as much and then wait and try a reconnection...";
            QTimer::singleShot(5000, this, [this](){
                if (!isConnected()) {
                    setDeviceProgress(0);
                    setProgressDescription(i18nc("Message shown to the user when a firmware upload has completed and the device has rebooted itself", "Attempting to reconnect to your gear..."));
                    connectDevice();
                }
            });
            setDeviceProgress(0);
            setProgressDescription(i18nc("Message shown to the user after firmware upload has completed and the tail is expected to reboot", "Firmware upload complete, waiting for your gear to reboot automatically before attempting to reconnect..."));
        } else {
            qDebug() << name() << deviceID() << "LowEnergy controller disconnected";
            emit deviceMessage(deviceID(), i18nc("Warning that the device itself disconnected during operation (usually due to turning off from low power)", "The MiTail Mini closed the connection, either by being turned off or losing power. Remember to charge your gear!"));
        }
        disconnectDevice();
    });

    // Connect
    d->btControl->connectToDevice();
}

void GearMitailMini::disconnectDevice()
{
    d->pingTimer.stop();
    if (d->btControl) {
        d->btControl->deleteLater();
        d->btControl = nullptr;
    }
    if (d->deviceService) {
        d->deviceService->deleteLater();
        d->deviceService = nullptr;
    }
    if (d->batteryService) {
        d->batteryService->deleteLater();
        d->batteryService = nullptr;
    }
    commandModel->clear();
    commandShorthands.clear();
//     emit commandModelChanged();
//     commandQueue->clear(); // FIXME Clear commands for this device only
//     emit commandQueueChanged();
    d->batteryLevel = 0;
    emit batteryLevelChanged(0);
    emit isConnectedChanged(isConnected());
}

bool GearMitailMini::isConnected() const
{
    return d->btControl;
}

QString GearMitailMini::version() const
{
    return d->version;
}

int GearMitailMini::batteryLevel() const
{
    return d->batteryLevel;
}

QString GearMitailMini::currentCall() const
{
    return d->currentCall;
}

QString GearMitailMini::deviceID() const
{
    return deviceInfo.address().toString();
}

void GearMitailMini::sendMessage(const QString &message)
{
    if (d->firmwareProgress == -1) {
        QString actualMessage{message};
        if (commandShorthands.contains(message)) {
            actualMessage = commandShorthands[message];
        }

        if (d->deviceCommandWriteCharacteristic.isValid() && d->deviceService) {
            QString actualCall{actualMessage};
            if (actualMessage.contains(';')) {
                d->callQueue = actualMessage.split(';');
                actualCall = d->callQueue.takeFirst();
            }
            if (actualMessage != message) {
                // As we're translating, we need to manually set this message as running and not trust the device to tell us
                commandModel->setRunning(message, true);
            }

            d->currentSubCall = actualCall;
            d->deviceService->writeCharacteristic(d->deviceCommandWriteCharacteristic, actualCall.toUtf8());
            d->currentCall = message;
            emit currentCallChanged(message);
        }
    }
}

QStringList GearMitailMini::defaultCommandFiles() const
{
    return QStringList{QLatin1String{":/commands/mitailmini-builtin.crumpet"}};
}

void GearMitailMini::checkOTA()
{
    if (d->downloadOperation == Private::NoDownloadOperation) {
        setDeviceProgress(0);
        setProgressDescription(i18nc("Message shown alongside a progress bar when downloading update information", "Downloading firmware update information"));
        d->firmwareUrl.clear();
        d->firmwareMD5.clear();
        d->otaVersion.clear();
        d->firmware.clear();
        Q_EMIT hasAvailableOTAChanged();
        Q_EMIT hasOTADataChanged();
        d->downloadOperation = Private::DownloadingOTAInformation;
        QNetworkRequest request(QUrl("https://thetailcompany.com/fw/mitail"));
        d->networkReply = d->qnam.get(request);
        connect(d->networkReply.data(), &QNetworkReply::finished, this, [this]() { d->handleFinished(d->networkReply.data()); });
    }
}

bool GearMitailMini::hasAvailableOTA()
{
    if (!d->otaVersion.isEmpty() && d->version != d->otaVersion) {
        // this will need thought... comparing the version strings like this will not work
        return true;
    }
    return false;
}

QString GearMitailMini::otaVersion()
{
    return d->otaVersion;
}

void GearMitailMini::downloadOTAData()
{
    if (d->downloadOperation == Private::NoDownloadOperation) {
        setDeviceProgress(0);
        setProgressDescription(i18nc("Message shown along a progress bar when downloading the firmware payload itself", "Downloading firmware update from The Tail Company's website..."));
        d->firmware.clear();
        Q_EMIT hasOTADataChanged();
        d->downloadOperation = Private::DownloadingOTAData;
        QNetworkRequest request(d->firmwareUrl);
        d->networkReply = d->qnam.get(request);
        connect(d->networkReply.data(), &QNetworkReply::downloadProgress, this, [this](quint64 received, quint64 total){ if (total > 0) { setDeviceProgress(100 * (received/(double)total)); } else { setDeviceProgress(0); } });
        connect(d->networkReply.data(), &QNetworkReply::finished, this, [this]() { d->handleFinished(d->networkReply.data()); });
    }
}

void GearMitailMini::setOTAData(const QString& md5sum, const QByteArray& firmware)
{
    QString calculatedSum = QString(QCryptographicHash::hash(firmware, QCryptographicHash::Md5).toHex());
    if (md5sum == calculatedSum) {
        d->firmware = firmware;
    } else {
        deviceMessage(deviceID(), i18nc("", "The downloaded firmware update did not contain what we expected. This is commonly due to a problem with the download itself having failed, and you should simply try again. If it continues to fail, please get in touch with us and we can try and work something out!"));
        qWarning() << name() << deviceID() << "Downloaded firmware has md5sum" << calculatedSum << "and based on the remote info, we expected" << md5sum;
        d->firmware.clear();
    }
    d->firmwareProgress = -1;
    Q_EMIT hasOTADataChanged();
}

bool GearMitailMini::hasOTAData()
{
    return d->firmware.length() > 0;
}

void GearMitailMini::startOTA()
{
    setDeviceProgress(0);
    setProgressDescription(i18nc("Message shown during firmware update processes", "Uploading firmware to your gear. Please keep your devices very near each other, and make sure both have plenty of charge (or plug in a charger now). Once completed, your gear will restart and disconnect from this device. Once rebooted, you will be able to connect to it again."));
    // send "OTA (length of firmware in bytes) (md5sum)"
    QString otaInitialiser = QString("OTA %1 %2").arg(d->firmware.length()).arg(d->firmwareMD5);
    d->firmwareProgress = 0;
    d->deviceService->writeCharacteristic(d->deviceCommandWriteCharacteristic, otaInitialiser.toUtf8());
    // next step will happen in Private::characteristicChanged
}
