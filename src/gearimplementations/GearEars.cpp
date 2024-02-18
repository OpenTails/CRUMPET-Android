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

#include "GearEars.h"

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

static const QStringList knownARevision{QLatin1String{"VER 1.0.12"}, QLatin1String{"VER 1.0.13"}, QLatin1String{"VER 1.0.14"}};
static const QStringList knownBRevision{QLatin1String{"VER 1.0.13b"}, QLatin1String{"VER 1.0.14b"}};
class GearEars::Private {
public:
    Private(GearEars* qq)
        : q(qq)
    {}
    ~Private() {}
    GearEars* q{nullptr};
    DeviceModel * parentModel{nullptr};

    QLatin1String version{"(unknown)"};
    int batteryLevel{-1};
    bool micsSwapped{false};
    ListenMode listenMode{ListenModeOff};
    bool canBalanceListening{true};
    bool hasTilt{false};
    bool tiltEnabled{false};
    int hardwareRevision{-1};

    QString currentCall;
    QString currentSubCall;
    QStringList callQueue;

    QLowEnergyController* btControl{nullptr};
    QLowEnergyService* earsService{nullptr};
    QLowEnergyCharacteristic earsCommandWriteCharacteristic;
    QLowEnergyCharacteristic earsCommandReadCharacteristic;

    QLowEnergyService* batteryService{nullptr};
    QLowEnergyCharacteristic batteryCharacteristic;

    QTimer pingTimer;
    QBluetoothUuid earsCommandWriteCharacteristicUuid{QLatin1String("{05e026d8-b395-4416-9f8a-c00d6c3781b9}")};
    QBluetoothUuid earsCommandReadCharacteristicUuid{QLatin1String("{0b646a19-371e-4327-b169-9632d56c0e84}")};

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
        case QLowEnergyService::RemoteServiceDiscovering:
            qDebug() << q->name() << q->deviceID() << "Discovering services...";
            break;
        case QLowEnergyService::RemoteServiceDiscovered:
        {
            qDebug() << q->name() << q->deviceID() << "Service discovered.";

            for(const QLowEnergyCharacteristic& leChar : earsService->characteristics()) {
                qDebug() << q->name() << q->deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
            }

            earsCommandWriteCharacteristic = earsService->characteristic(earsCommandWriteCharacteristicUuid);
            if (!earsCommandWriteCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "EarGear command writing characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), i18nc("A message when sent when attempting to connect to a device which does not have a specific expected feature", "It looks like this device is not an EarGear controller (could not find the main ears writing characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            // Get the descriptor, and turn on notifications
            QLowEnergyDescriptor earsDescriptor = earsCommandWriteCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (earsCommandWriteCharacteristic.properties() & QLowEnergyCharacteristic::Notify) {
                earsService->writeDescriptor(earsDescriptor, QByteArray::fromHex("0100"));
            }
            if (earsCommandWriteCharacteristic.properties() & QLowEnergyCharacteristic::Indicate) {
                earsService->writeDescriptor(earsDescriptor, QByteArray::fromHex("0200"));
            }

            earsCommandReadCharacteristic = earsService->characteristic(earsCommandReadCharacteristicUuid);
            if (!earsCommandReadCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "EarGear command reading characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), i18nc("A message when sent when attempting to connect to a device which does not have a specific expected feature", "It looks like this device is not an EarGear controller (could not find the main ears reading characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            q->commandModel->clear();

            // Get the descriptor, and turn on notifications
            earsDescriptor = earsCommandReadCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (earsCommandReadCharacteristic.properties() & QLowEnergyCharacteristic::Notify) {
                earsService->writeDescriptor(earsDescriptor, QByteArray::fromHex("0100"));
            }
            if (earsCommandReadCharacteristic.properties() & QLowEnergyCharacteristic::Indicate) {
                earsService->writeDescriptor(earsDescriptor, QByteArray::fromHex("0200"));
            }

            reconnectThrottle = 0;
            Q_EMIT q->isConnectedChanged(q->isConnected());
            q->sendMessage(QLatin1String{"VER"}); // Ask for the version, and then react to the response...

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

        if (earsCommandReadCharacteristicUuid == characteristic.uuid()) {
            QString theValue = QString::fromUtf8(newValue);
            if (theValue.endsWith(QString::fromUtf8("\x00"))) {
                theValue = theValue.left(theValue.length());
            }
            QStringList stateResult = theValue.split(QLatin1Char{' '});
            if (theValue == QLatin1String{"EarGear2 started"}) {
                qDebug() << q->name() << q->deviceID() << "EarGear2 has successfully started up";
            }
            else if (theValue == QLatin1String{"System is busy now"}) {
                // Postpone what we attempted to send a few moments before trying again, as the ears are currently busy
                // ...except if we're listening, at which point don't try and do this
                // if (listeningState == ListeningFull || listeningState == ListeningOn) {
                // }
                // else {
                    QTimer::singleShot(1000, q, [this](){ q->sendMessage(currentSubCall); });
                //}
            }
            else if (stateResult[0] == QLatin1String{"HWVER"}) {
                if (stateResult[1] == QLatin1String{"A"}) {
                    hardwareRevision = 1;
                }
                else if (stateResult[1] == QLatin1String{"B"}) {
                    hardwareRevision = 2;
                }
                else {
                    // This is an unknown hardware revision - this likely is a bad thing,
                    // but we then must assume there is a third that we do not know about
                    // yet - let's pass that information to the checker, so it can react.
                    // This is really only a bad thing if the user wants to update, so
                    // we can basically ignore it until time comes to attempt to update.
                    hardwareRevision = 3;
                    qDebug() << q->name() << q->deviceID() << "Unexpected hardware revision:" << stateResult[1];
                }
            }
            else if (stateResult[0] == QLatin1String{"VER"}) {
                q->reloadCommands();
                version = QLatin1String(newValue);
                Q_EMIT q->versionChanged(QString::fromUtf8(newValue));
                Q_EMIT q->supportedTiltEventsChanged();
                q->setListenMode(listenMode);
                if (q->deviceInfo.name() == QLatin1String{"EarGear"}) {
                    q->setHasShutdown(false);
                    q->setHasNoPhoneMode(false);
                }
                else {
                    q->setHasShutdown(true);
                    q->setHasNoPhoneMode(true);
                    q->setNoPhoneModeGroups({});
                    if (knownARevision.contains(version)) {
                        hardwareRevision = 1;
                    }
                    else if (knownBRevision.contains(version)) {
                        hardwareRevision = 2;
                    }
                    q->sendMessage(QLatin1String{"HWVER"});
                }
                pingTimer.start();
                if (firmwareProgress > -1) {
                    if (otaVersion == q->manuallyLoadedOtaVersion()) {
                        // We have no idea whether the update succeeded, tell the user they need to check themselves
                        Q_EMIT q->deviceBlockingMessage(i18nc("Title of the message box shown to the user upon a firmware update with an unknown outcome", "Reboot Completed"), i18nc("Message shown to the user after a reboot following a manual firmware upload", "The reboot following the firmware upload has completed and we have connected back to the device. The gear now reports %1, and we hope that is what you expected.", version));
                    } else if (otaVersion == QString::fromUtf8(newValue)) {
                        // successful update get!
                        Q_EMIT q->deviceBlockingMessage(i18nc("Title of the message box shown to the user upon a successful firmware upgrade", "Upgrade Successful"), i18nc("Message shown to the user when a firmware update completed successfully", "Congratulations, your gear has been successfully updated to version %1!", version));
                    } else {
                        // sadface, update failed...
                        Q_EMIT q->deviceBlockingMessage(i18nc("Title of the message box shown to the user upon an unsuccessful firmware upgrade", "Update Failed"), i18nc("Message shown to the user when a firmware update failed", "<p>Sorry, but the upgrade failed. Most often this is due to the transfer being corrupted during the upload process itself, which is why your gear has a safe fallback to just go back to your old firmware version upon a failure. You can try the update again by clicking the Install button gain.</p>"));
                    }
                    q->setProgressDescription(QLatin1String{""});
                    q->setDeviceProgress(-1);
                    firmwareProgress = -1;
                    firmwareChunk.clear();
                } else {
                    // Logic here is, the user explicitly picks what to do when disconnecting the app from a tail
                    q->sendMessage(QLatin1String{"STOPNPM"});
                }
            }
            else if (stateResult[0] == QLatin1String{"PONG"}) {
                if (currentCall != QLatin1String{"PING"}) {
                    qWarning() << q->name() << q->deviceID() << "We got an out-of-order response for a ping";
                }
            }
            else if (theValue == QLatin1String{"EarGear started"}) {
                qDebug() << q->name() << q->deviceID() << "EarGear detected the connection";
            } else if (theValue == QLatin1String{"POWER OFF"}) {
                q->disconnectDevice();
            }
            else if (theValue == QLatin1String{"BEGIN OTA"}) {
                qDebug() << q->name() << q->deviceID() << "Starting firmware update";
                if (firmwareProgress == -1) {
                    firmwareProgress = 0;
                }
            }
            else if (stateResult[0] == QLatin1String("OTA") || firmwareProgress > -1) {
                qDebug() << q->name() << q->deviceID() << "Firmware update is happening...";
            }
            else if (stateResult[0] == QLatin1String{"LISTEN"}) {
                ListenMode newMode = ListenModeOff;
                if (stateResult[1] != QLatin1String{"OFF"}) {
                    newMode = ListenModeOn;
                }
                if (listenMode != newMode) {
                    listenMode = newMode;
                    Q_EMIT q->listenModeChanged();
                }
            }
            else if (stateResult[0] == QLatin1String{"TILTMODE"}) {
                bool newState = false;
                if (stateResult[1] != QLatin1String{"OFF"}) {
                    newState = true;
                }
                if (tiltEnabled != newState) {
                    tiltEnabled = newState;
                    Q_EMIT q->tiltEnabledChanged();
                }
            }
            else if (stateResult[0] == QLatin1String{"TILT"}) {
                static const QLatin1String tiltLeft{"LEFT"};
                static const QLatin1String rightTilt{"RIGHT"};
                static const QLatin1String forwardTilt{"FORWARD"};
                static const QLatin1String backwardTilt{"BACKWARD"};
                static const QLatin1String neutralTilt{"NEUTRAL"};
                if (stateResult[1] == tiltLeft) {
                    Q_EMIT q->gearSensorEvent(GearBase::TiltLeftEvent);
                } else if (stateResult[1] == rightTilt) {
                    Q_EMIT q->gearSensorEvent(GearBase::TiltRightEvent);
                } else if (stateResult[1] == forwardTilt) {
                    Q_EMIT q->gearSensorEvent(GearBase::TiltForwardEvent);
                } else if (stateResult[1] == backwardTilt) {
                    Q_EMIT q->gearSensorEvent(GearBase::TiltBackwardEvent);
                } else if (stateResult[1] == neutralTilt) {
                    Q_EMIT q->gearSensorEvent(GearBase::TiltNeutralEvent);
                }
            }
            else if (currentCall == QLatin1String{"LISTEN IOS"} && theValue == QLatin1String{"DSSP END"}) {
                // This is a hack for some firmware versions, which do not report
                // their state correctly (sending instead a "DSSP END" message)
                listenMode = ListenModeOn;
                Q_EMIT q->listenModeChanged();
            }
            else if (theValue.startsWith(QLatin1String{"Noise diff:"})) {
                if (listenMode != ListenModeFull) {
                    listenMode = ListenModeFull;
                    Q_EMIT q->listenModeChanged();
                }
                q->deviceMessage(q->deviceID(), QString::fromUtf8("Noise difference levels: %1").arg(stateResult.last()));
                qDebug() << q->name() << q->deviceID() << "Updated noise difference level:" << stateResult.last();
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
                        QStringList pauseCommand = message.split(QLatin1Char{' '});
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
            else if (theValue == QLatin1String{"Mics auto balance completed"}) {
                q->deviceMessage(q->deviceID(), i18nc("Informational message for when the microphone balancing operation has completed", "Microphone balancing completed"));
            }
            else if (theValue.startsWith(QLatin1String{"MICSWAP"})) {
                if (theValue == QLatin1String{"MICSWAP: mic1-R, mic2-L"}) {
                    micsSwapped = true;
                }
                else {
                    micsSwapped = false;
                }
                Q_EMIT q->micsSwappedChanged();
            }
            else {
                qDebug() << q->name() << q->deviceID() << "Unexpected response: Did not understand" << newValue;
            }
            currentCall.clear();
            Q_EMIT q->currentCallChanged(currentCall);
        }
        else if (characteristic.uuid() == earsCommandWriteCharacteristicUuid) {
            if (firmwareProgress > -1) {
                quint32 receivedBytes;
                if (newValue.size() == 4) {
                    memcpy(&receivedBytes, newValue.data(), newValue.size());
                }
                else if (newValue.size() == 2) {
                    quint16 tempVal;
                    memcpy(&tempVal, newValue.data(), newValue.size());
                    receivedBytes = tempVal;
                }
                else if (newValue.size() == 2) {
                    quint8 tempVal;
                    memcpy(&tempVal, newValue.data(), newValue.size());
                    receivedBytes = tempVal;
                }
                if (firmwareProgress < firmware.size()) {
                    static const int MTUSize{500}; // evil big size for a start, hopefully should be ok, but let's see if we get any reports...
                    firmwareChunk = firmware.mid(firmwareProgress, MTUSize);
                    firmwareProgress += firmwareChunk.size();
                    earsService->writeCharacteristic(earsCommandWriteCharacteristic, firmwareChunk);
                    q->setDeviceProgress(1 + (99 * (firmwareProgress / (double)firmware.size())));
                    qDebug() << q->name() << q->deviceID() << "Uploading firmware:" << 1 + (99 * (firmwareProgress / (double)firmware.size())) << "%, or" << firmwareProgress << "of" << firmware.size() << "bytes, and the other end says that so far it has received" << receivedBytes;
                }
                else {
                    qDebug() << q->name() << q->deviceID() << "The gear says it has have received" << receivedBytes << "out of" << firmware.size() << "which means it should be rebooting momentarily...";
                }
            }
        }
    }

    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        qDebug() << q->name() << q->deviceID() << "Characteristic written:" << characteristic.uuid() << newValue;
        currentCall = QString::fromUtf8(newValue);
        Q_EMIT q->currentCallChanged(currentCall);
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
                    firmwareUrl = QUrl(fwInfoObj.value(QLatin1String{"url"}).toString());
                    firmwareMD5 = fwInfoObj.value(QLatin1String{"md5sum"}).toString();
                    otaVersion = fwInfoObj.value(QLatin1String{"version"}).toString();
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

GearEars::GearEars(const QBluetoothDeviceInfo& info, DeviceModel * parent)
    : GearBase(info, parent)
    , d(new Private(this))
{
    d->parentModel = parent;

    // The battery timer also functions as a keepalive call. If it turns
    // out to be a problem that we pull the battery this often, we can
    // add a separate ping keepalive functon.
    connect(&d->pingTimer, &QTimer::timeout,
            [this](){ if(d->currentCall.isEmpty() && d->firmwareProgress == -1) { sendMessage(QLatin1String{"PING"}); } });

    d->pingTimer.setTimerType(Qt::VeryCoarseTimer);
    d->pingTimer.setInterval(60000 / 2);
    d->pingTimer.setSingleShot(false);

    if (deviceInfo.name() != QLatin1String{"EarGear"}) {
        setSupportsOTA(true);
        d->canBalanceListening = false;
        Q_EMIT canBalanceListeningChanged();
        d->hasTilt = true;
        Q_EMIT hasTiltChanged();
    }
}

GearEars::~GearEars()
{
    delete d;
}

void GearEars::connectDevice()
{
    if(d->btControl) {
        disconnectDevice();
    }

    d->btControl = QLowEnergyController::createCentral(deviceInfo, this);
    d->btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);

    if(d->earsService) {
        d->earsService->deleteLater();
        d->earsService = nullptr;
    }

    connect(d->btControl, &QLowEnergyController::serviceDiscovered,
        [this](const QBluetoothUuid &gatt){
            qDebug() << name() << deviceID() << "service discovered" << gatt;
        });

    connect(d->btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << name() << deviceID()<< "Done!";

                // Main control service
                d->earsService = d->btControl->createServiceObject(QBluetoothUuid(QLatin1String("{927dee04-ddd4-4582-8e42-69dc9fbfae66}")));
                if (!d->earsService) {
                    qWarning() << name() << deviceID() << "Cannot create QLowEnergyService for {927dee04-ddd4-4582-8e42-69dc9fbfae66}";
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning message when a fault occurred during a connection attempt", "An error occurred while connecting to your EarGear (the main service object could not be created). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }

                connect(d->earsService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){ d->serviceStateChanged(newState); });
                connect(d->earsService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicChanged(info, value); });
                connect(d->earsService, &QLowEnergyService::characteristicWritten, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicWritten(info, value); });
                d->earsService->discoverDetails();

                // Battery service
                d->batteryService = d->btControl->createServiceObject(QBluetoothUuid::ServiceClassUuid::BatteryService);
                if (!d->batteryService) {
                    qWarning() << name() << deviceID() << "Failed to create battery service";
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning message when the battery information is unavailable on a device", "An error occurred while connecting to your EarGear (the battery service was not available). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }
                else {
                    connect(d->batteryService, &QLowEnergyService::characteristicRead, this, [this](const QLowEnergyCharacteristic &, const QByteArray &value){
                        if (value.length() > 0) {
                            d->batteryLevel = (int)value.at(0) / 20;
                            setBatteryLevelPercent((int)value.at(0));
                            Q_EMIT batteryLevelChanged(d->batteryLevel);
                        }
                    });
                    connect(d->batteryService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic&, const QByteArray& value){
                        if (value.length() > 0) {
                            d->batteryLevel = (int)value.at(0) / 20;
                            setBatteryLevelPercent((int)value.at(0));
                            Q_EMIT batteryLevelChanged(d->batteryLevel);
                        }
                    });
                    connect(d->batteryService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){
                        switch (newState) {
                        case QLowEnergyService::RemoteServiceDiscovering:
                            qDebug() << name() << deviceID() << "Discovering battery services...";
                            break;
                        case QLowEnergyService::RemoteServiceDiscovered:
                        {
                            qDebug() << name() << deviceID() << "Battery service discovered";

                            for(const QLowEnergyCharacteristic& leChar : d->batteryService->characteristics()) {
                                qDebug() << name() << deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
                            }

                            d->batteryCharacteristic = d->batteryService->characteristic(QBluetoothUuid::CharacteristicType::BatteryLevel);
                            if (!d->batteryCharacteristic.isValid()) {
                                qDebug() << name() << deviceID() << "EarGear battery level characteristic not found, this is bad";
                                deviceMessage(deviceID(), i18nc("Warning message when the battery information is unavailable on the device", "It looks like this device is not an EarGear controller (could not find the battery level characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                                disconnectDevice();
                                break;
                            }

                            // Get the descriptor, and turn on notifications
                            QLowEnergyDescriptor batteryDescriptor = d->batteryCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                            if (!batteryDescriptor.isValid()) {
                                qDebug() << name() << deviceID() << "This is bad, no battery descriptor...";
                            }
                            d->batteryService->writeDescriptor(batteryDescriptor, QByteArray::fromHex("0100"));
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

    connect(d->btControl, &QLowEnergyController::errorOccurred,
        this, [this](QLowEnergyController::Error error) {
            qDebug() << name() << deviceID() << "Cannot connect to remote device." << error;

            switch(error) {
                case QLowEnergyController::UnknownError:
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning that some unknown error happened", "An error occurred. If you are trying to connect to your ears, make sure the EarGear is on and close to this device."));
                    break;
                case QLowEnergyController::RemoteHostClosedError:
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning that the device disconnected itself", "The EarGear closed the connection."));
                    break;
                case QLowEnergyController::ConnectionError:
                    Q_EMIT deviceMessage(deviceID(), i18nc("Warning that some connection failure occurred (usually due to low signal strength)", "Failed to connect to your EarGear. Please try again (perhaps move it closer?)"));
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
        if (d->firmwareProgress >=  d->firmware.size()) {
            Q_EMIT deviceBlockingMessage(i18nc("Title for a message box shown after the device disconnects after completing the firmware update", "Firmware Update Completed"), i18nc("Body of a message box shown after the device disconnects after completing the firmware update", "The firmware upload has been completed, and your gear has turned itself off. If it did not turn itself back on again, close this message, turn it on manually, and then connect to it. If it turned itself back on again, you can just close this message."));
            setProgressDescription(QLatin1String{""});
            setDeviceProgress(-1);
        }
        else {
            Q_EMIT deviceMessage(deviceID(), i18nc("Warning that the device itself disconnected during operation (usually due to turning off from low power)", "The EarGear closed the connection, either by being turned off or losing power. Remember to charge your ears!"));
        }
        disconnectDevice();
    });

    // Connect
    d->btControl->connectToDevice();
}

void GearEars::disconnectDevice()
{
    d->pingTimer.stop();
    if (d->btControl) {
        d->btControl->deleteLater();
        d->btControl = nullptr;
    }
    if (d->earsService) {
        d->earsService->deleteLater();
        d->earsService = nullptr;
    }
    if (d->batteryService) {
        d->batteryService->deleteLater();
        d->batteryService = nullptr;
    }
    commandModel->clear();
    commandShorthands.clear();
//     Q_EMIT commandModelChanged();
//     commandQueue->clear(); // FIXME Clear commands for this device only
//     Q_EMIT commandQueueChanged();
    d->batteryLevel = -1;
    Q_EMIT batteryLevelChanged(0);
    Q_EMIT isConnectedChanged(isConnected());
}

bool GearEars::isConnected() const
{
    return d->btControl;
}

QString GearEars::version() const
{
    return d->version;
}

int GearEars::batteryLevel() const
{
    return d->batteryLevel;
}

QString GearEars::currentCall() const
{
    return d->currentCall;
}

GearEars::ListenMode GearEars::listenMode() const
{
    return d->listenMode;
}

void GearEars::setListenMode(const ListenMode& listenMode)
{
    switch(listenMode) {
        case ListenModeFull:
            sendMessage(QLatin1String{"LISTEN FULL"});
            break;
        case ListenModeOn:
            sendMessage(QLatin1String{"LISTEN IOS"});
            break;
        case ListenModeOff:
        default:
            sendMessage(QLatin1String{"ENDLISTEN"});
            break;
    }
}

bool GearEars::micsSwapped() const
{
    return d->micsSwapped;
}

bool GearEars::hasTilt() const
{
    return d->hasTilt;
}

bool GearEars::canBalanceListening() const
{
    return d->canBalanceListening;
}

bool GearEars::tiltEnabled() const
{
    return d->tiltEnabled;
}

void GearEars::setTiltMode(bool tiltState)
{
    if (tiltState) {
        sendMessage(QLatin1String{"TILTMODE START"});
    }
    else {
        sendMessage(QLatin1String{"ENDTILTMODE"});
    }
}

QVariantList GearEars::supportedTiltEvents()
{
    static const QStringList knownLimitedRevision{QLatin1String{"VER 1.0.12"}, QLatin1String{"VER 1.0.13"}, QLatin1String{"VER 1.0.14"}, QLatin1String{"VER 1.0.15"}, QLatin1String{"VER 1.0.16"}, QLatin1String{"VER 1.0.13b"}, QLatin1String{"VER 1.0.14b"}, QLatin1String{"VER 1.0.15"}, QLatin1String{"VER 1.0.16"}};
    static const QVariantList limitedSupportedEvents{GearBase::TiltLeftEvent, GearBase::TiltRightEvent};
    static const QVariantList fullSupportedEvents{GearBase::TiltLeftEvent, GearBase::TiltRightEvent, GearBase::TiltForwardEvent, GearBase::TiltBackwardEvent, GearBase::TiltNeutralEvent};
    if (knownLimitedRevision.contains(d->version)) {
        return limitedSupportedEvents;
    }
    return fullSupportedEvents;
}

QVariantList GearEars::supportedSoundEvents()
{
    static const QVariantList fullSupportedEvents{GearBase::SoundLeftQuietEvent, GearBase::SoundLeftLoudEvent, GearBase::SoundRightQuietEvent, GearBase::SoundRightLoudEvent, GearBase::SoundNeutralEvent};
    return fullSupportedEvents;
}

static const QLatin1Char semicolon{';'};
void GearEars::sendMessage(const QString &message)
{
    QString actualMessage{message};
    if (commandShorthands.contains(message)) {
        actualMessage = commandShorthands[message];
    }

    if (d->earsCommandWriteCharacteristic.isValid() && d->earsService) {
        QString actualCall{actualMessage};
        if (actualMessage.contains(semicolon)) {
            d->callQueue = actualMessage.split(semicolon);
            actualCall = d->callQueue.takeFirst();
        }
        if (actualMessage != message) {
            // As we're translating, we need to manually set this message as running and not trust the device to tell us
            commandModel->setRunning(message, true);
        }

        d->currentSubCall = actualCall;
        d->earsService->writeCharacteristic(d->earsCommandWriteCharacteristic, actualCall.toUtf8());
        d->currentCall = message;
        Q_EMIT currentCallChanged(message);
    }
}

QStringList GearEars::defaultCommandFiles() const
{
    if (deviceInfo.name() == QLatin1String{"EarGear"}) {
        return QStringList{QLatin1String{":/commands/eargear-base.crumpet"}};
    }
    else {
        return QStringList{QLatin1String{":/commands/eargear2-base.crumpet"}};
    }
}


void GearEars::checkOTA()
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
        QString firmwareInfoUrl;
        switch(d->hardwareRevision) {
            case 1:
                firmwareInfoUrl = QLatin1String{"https://thetailcompany.com/fw/eargear"};
                break;
            case 2:
                firmwareInfoUrl = QLatin1String{"https://thetailcompany.com/fw/eargear-b"};
                break;
            case 3:
                deviceBlockingMessage(name(), i18nc("Message shown in case the hardware revision is given by the firmware, but doesn't match one of our known ones, meaning the app is likely very outdated", "Your gear has reported a hardware revision that we do not know of. This means that your app is likely to be out of date and needs to be updated."));
                break;
        }
        if (firmwareInfoUrl.isEmpty()) {
            deviceBlockingMessage(name(), i18nc("Message shown in the unlikely case a firmware exists which does not report the expected hardware revision and which also is not known to us", "You have somehow got a firmware version which does not report the hardware revision of your ears, but which also is not known to fail to do so. This is a highly unexpected situation and we would appreciate it if you reported it directly to us at info@thetailcompany.com - thank you!"));
        } else {
            qDebug() << name() << deviceID() << "Fetching firmware information using url" << firmwareInfoUrl << "for revision" << d->hardwareRevision;
            QNetworkRequest request(QUrl{firmwareInfoUrl});
            d->networkReply = d->qnam.get(request);
            connect(d->networkReply.data(), &QNetworkReply::finished, this, [this]() { d->handleFinished(d->networkReply.data()); });
        }
    }
}

bool GearEars::hasAvailableOTA()
{
    if (!d->otaVersion.isEmpty() && d->version != d->otaVersion) {
        // this will need thought... comparing the version strings like this will not work
        return true;
    }
    return false;
}

void GearEars::setOtaVersion(const QString& version)
{
    d->otaVersion = version;
    Q_EMIT hasAvailableOTAChanged();
}

QString GearEars::otaVersion()
{
    return d->otaVersion;
}

void GearEars::downloadOTAData()
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

void GearEars::setOTAData(const QString& md5sum, const QByteArray& firmware)
{
    QString calculatedSum = QString::fromUtf8(QCryptographicHash::hash(firmware, QCryptographicHash::Md5).toHex());
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

bool GearEars::hasOTAData()
{
    return d->firmware.length() > 0;
}

void GearEars::startOTA()
{
    setDeviceProgress(0);
    setProgressDescription(i18nc("Message shown during firmware update processes", "Uploading firmware to your gear. Please keep your devices very near each other, and make sure both have plenty of charge (or plug in a charger now). Once completed, your gear will either reboot or turn itself off and disconnect from this device. Once it is started back up again, you will be able to connect to it again."));
    // send "OTA (length of firmware in bytes) (md5sum)"
    QString otaInitialiser = QString::fromUtf8("OTA %1 %2").arg(d->firmware.length()).arg(d->firmwareMD5);
    d->earsService->writeCharacteristic(d->earsCommandWriteCharacteristic, otaInitialiser.toUtf8());
    // next step will happen in Private::characteristicChanged
}
