/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "DeviceModel.h"
#include "AppSettings.h"
#include "GearBase.h"
#include "gearimplementations/GearDigitail.h"
#include "gearimplementations/GearEars.h"
#include "gearimplementations/GearFake.h"
#include "gearimplementations/GearFlutterWings.h"
#include "gearimplementations/GearMitail.h"
#include "gearimplementations/GearMitailMini.h"

#include <KLocalizedString>

class DeviceModel::Private
{
public:
    Private(DeviceModel * qq)
        : q(qq)
    {
        readDeviceNames();
    }
    ~Private() {}
    DeviceModel * q{nullptr};
    GearBase* fakeDevice{nullptr};

    void readDeviceNames();

    AppSettings* appSettings{nullptr};
    QList<GearBase*> devices;

    void notifyDeviceDataChanged(GearBase* device, int role)
    {
        int pos = devices.indexOf(device);
        if(pos > -1) {
            QModelIndex idx(q->index(pos));
            q->dataChanged(idx, idx, QVector<int>{role});
        }
    }
};

DeviceModel::DeviceModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    d->fakeDevice = new GearFake(QBluetoothDeviceInfo(QBluetoothAddress(QString("FA:KE:TA:IL")), QString("FAKE"), 0), this);
}

DeviceModel::~DeviceModel()
{
    delete d;
}

void DeviceModel::Private::readDeviceNames()
{
    if (!appSettings) {
        return;
    }

    const QVariantMap map = appSettings->deviceNames();
    for (QVariantMap::const_iterator it = map.cbegin(); it != map.cend(); ++it) {
        for (GearBase* device : devices) {
            if (device->deviceID() == it.key()) {
                device->setName(it.value().toString());
                break;
            }
        }
    }
}

AppSettings *DeviceModel::appSettings() const
{
    return d->appSettings;
}

void DeviceModel::setAppSettings(AppSettings *appSettings)
{
    d->appSettings = appSettings;
    connect(d->appSettings, &AppSettings::deviceNamesChanged, this, [this](){ d->readDeviceNames(); });
    d->readDeviceNames();
    connect(d->appSettings, &AppSettings::fakeTailModeChanged, this, [this](bool fakeTailMode){
        if (fakeTailMode && !d->devices.contains(d->fakeDevice)) {
            addDevice(d->fakeDevice);
        } else if (!fakeTailMode && d->devices.contains(d->fakeDevice)) {
            removeDevice(d->fakeDevice);
        }
    });
    if (d->appSettings->fakeTailMode() && !d->devices.contains(d->fakeDevice)) {
        addDevice(d->fakeDevice);
    }
}

QHash< int, QByteArray > DeviceModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {Name, "name"},
        {DeviceID, "deviceID"},
        {DeviceVersion, "deviceVersion"},
        {BatteryLevel, "batteryLevel"},
        {CurrentCall, "currentCall"},
        {IsConnected, "isConnected"},
        {ActiveCommandTitles, "activeCommandTitles"},
        {Checked, "checked"},
        {HasListening, "hasListening"},
        {ListeningState, "listeningState"},
        {EnabledCommandsFiles, "enabledCommandsFiles"},
        {MicsSwapped, "micsSwapped"},
        {SupportsOTA, "supportsOTA"},
        {HasAvailableOTA, "hasAvailableOTA"},
        {HasOTAData, "hasOTAData"},
        {DeviceProgress, "deviceProgress"},
        {ProgressDescription, "progressDescription"},
        {OperationInProgress, "operationInProgress"},
        {OTAVersion, "otaVersion"},
        {HasLights, "hasLights"},
        {HasShutdown, "hasShutdown"},
        {HasNoPhoneMode, "hasNoPhoneMode"},
        {NoPhoneModeGroups, "noPhoneModeGroups"},
        {ChargingState, "chargingState"},
        {BatteryLevelPercent, "batteryLevelPercent"},
        {HasTilt, "hasTilt"},
        {CanBalanceListening, "canBalanceListening"},
        {TiltEnabled, "tiltEnabled"},
        {KnownFirmwareMessage, "knownFirmwareMessage"},
        {GestureEventValues, "gestureEventValues"},
        {GestureEventTitles, "gestureEventTitles"},
        {GestureEventCommands, "gestureEventCommands"},
        {GestureEventDevices, "gestureEventDevices"},
    };
    return roles;
}

QVariant DeviceModel::data(const QModelIndex& index, int role) const
{
    QVariant value;
    if(index.isValid() && index.row() > -1 && index.row() < d->devices.count()) {
        GearBase* device = d->devices.at(index.row());
        switch(role) {
            case Name:
                value = device->name();
                break;
            case DeviceID:
                value = device->deviceID();
                break;
            case DeviceVersion:
                value = device->version();
                break;
            case BatteryLevel:
                value = device->batteryLevel();
                break;
            case CurrentCall:
                value = device->currentCall();
                break;
            case IsConnected:
                value = device->isConnected();
                break;
            case ActiveCommandTitles:
                value = device->activeCommandTitles();
                break;
            case Checked:
                value = device->checked();
                break;
            case HasListening:
                value = (qobject_cast<GearEars*>(device) != nullptr);
                break;
            case ListeningState:
            {
                int listeningState = 0;
                GearEars* ears = qobject_cast<GearEars*>(device);
                if (ears) {
                    listeningState = ears->listenMode();
                }
                value = listeningState;
                break;
            }
            case EnabledCommandsFiles:
                value = device->enabledCommandsFiles();
                break;
            case MicsSwapped:
            {
                bool micsSwapped{false};
                GearEars* ears = qobject_cast<GearEars*>(device);
                if (ears) {
                    micsSwapped = ears->micsSwapped();
                }
                value = micsSwapped;
                break;
            }
            case SupportsOTA:
                value = device->supportsOTA();
                break;
            case HasAvailableOTA:
                value = device->hasAvailableOTA();
                break;
            case HasOTAData:
                value = device->hasOTAData();
                break;
            case DeviceProgress:
                value = device->deviceProgress();
                break;
            case ProgressDescription:
                value = device->progressDescription();
                break;
            case OperationInProgress:
                value = device->deviceProgress() > -1;
                break;
            case OTAVersion:
                value = device->otaVersion();
                break;
            case HasLights:
                value = device->hasLights();
                break;
            case HasShutdown:
                value = device->hasShutdown();
                break;
            case HasNoPhoneMode:
                value = device->hasNoPhoneMode();
                break;
            case NoPhoneModeGroups:
                value = device->noPhoneModeGroups();
                break;
            case ChargingState:
                value = device->chargingState();
                break;
            case BatteryLevelPercent:
                value = device->batteryLevelPercent();
                break;
            case HasTilt:
            {
                bool hasTilt{false};
                GearEars* ears = qobject_cast<GearEars*>(device);
                if (ears) {
                    hasTilt = ears->hasTilt();
                }
                value = hasTilt;
                break;
            }
            case CanBalanceListening:
            {
                bool canBalanceListening{false};
                GearEars* ears = qobject_cast<GearEars*>(device);
                if (ears) {
                    canBalanceListening = ears->canBalanceListening();
                }
                value = canBalanceListening;
                break;
            }
            case TiltEnabled:
            {
                bool tiltEnabled{false};
                GearEars* ears = qobject_cast<GearEars*>(device);
                if (ears) {
                    tiltEnabled = ears->tiltEnabled();
                }
                value = tiltEnabled;
                break;
            }
            case KnownFirmwareMessage:
                value = device->knownFirmwareMessage();
                break;
            case GestureEventValues: {
                static QVariantList gestureValues;
                if (gestureValues.length() == 0) {
                    static const QMetaEnum gearSensorEventEnum = GearBase::staticMetaObject.enumerator(GearBase::staticMetaObject.indexOfEnumerator("GearSensorEvent"));
                    for (int enumKey = 0; enumKey < gearSensorEventEnum.keyCount(); ++enumKey) {
                        GearBase::GearSensorEvent eventKey = static_cast<GearBase::GearSensorEvent>(gearSensorEventEnum.value(enumKey));
                        gestureValues << eventKey;
                    }
                }
                value.setValue(gestureValues);
                break; }
            case GestureEventTitles: {
                static QStringList gestureTitles;
                if (gestureTitles.length() == 0) {
                    static const QMetaEnum gearSensorEventEnum = GearBase::staticMetaObject.enumerator(GearBase::staticMetaObject.indexOfEnumerator("GearSensorEvent"));
                    static const QHash<GearBase::GearSensorEvent, QString> gearSensorEventTranslations{
                        {GearBase::GearSensorEvent::TiltLeftEvent, i18nc("Name for an event where the gear has been detected as having been tilted to the left", "Tilt Left")},
                        {GearBase::GearSensorEvent::TiltRightEvent, i18nc("Name for an event where the gear has been detected as having been tilted to the right", "Tilt Right")},
                        {GearBase::GearSensorEvent::TiltForwardEvent, i18nc("Name for an event where the gear has been detected as having been tilted forward", "Tilt Forward")},
                        {GearBase::GearSensorEvent::TiltBackwardEvent, i18nc("Name for an event where the gear has been detected as having been tilted backward", "Tilt Backward")},
                        {GearBase::GearSensorEvent::TiltNeutralEvent, i18nc("Name for an event where the gear has been returned to an upright position from having been tilted", "Return to Upright")},
                        {GearBase::GearSensorEvent::SoundNeutralEvent, i18nc("Name for an event where the sound levels have been detected as returning to ambient after detecting a sound on one or the other side", "Low Ambient Sound")},
                        {GearBase::GearSensorEvent::SoundLeftQuietEvent, i18nc("Name for an event where a small, but detectable amount of sound has been detected on the left hand side of the gear", "Quiet Sound on the Left")},
                        {GearBase::GearSensorEvent::SoundLeftLoudEvent, i18nc("Name for an event where a large amount of sound has been detected on the left hand side of the gear", "Loud Sound on the Left")},
                        {GearBase::GearSensorEvent::SoundRightQuietEvent, i18nc("Name for an event where a small, but detectable amount of sound has been detected on the right hand side of the gear", "Quiet Sound on the Right")},
                        {GearBase::GearSensorEvent::SoundRightLoudEvent, i18nc("Name for an event where a large amount of sound has been detected on the right hand side of the gear", "Loud Sound on the Right")},
                    };
                    for (int enumKey = 0; enumKey < gearSensorEventEnum.keyCount(); ++enumKey) {
                        GearBase::GearSensorEvent eventKey = static_cast<GearBase::GearSensorEvent>(gearSensorEventEnum.value(enumKey));
                        if (gearSensorEventTranslations.contains(eventKey)) {
                            gestureTitles << gearSensorEventTranslations[eventKey];
                        } else {
                            gestureTitles << gearSensorEventEnum.key(enumKey);
                        }
                    }
                }
                value = gestureTitles;
                break; }
            case GestureEventCommands: {
                static const QMetaEnum gearSensorEventEnum = GearBase::staticMetaObject.enumerator(GearBase::staticMetaObject.indexOfEnumerator("GearSensorEvent"));
                QStringList devices;
                for (int enumKey = 0; enumKey < gearSensorEventEnum.keyCount(); ++enumKey) {
                    GearBase::GearSensorEvent eventKey = static_cast<GearBase::GearSensorEvent>(gearSensorEventEnum.value(enumKey));
                    devices << device->gearSensorCommand(eventKey);
                }
                value = devices;
                break; }
            case GestureEventDevices: {
                static const QMetaEnum gearSensorEventEnum = GearBase::staticMetaObject.enumerator(GearBase::staticMetaObject.indexOfEnumerator("GearSensorEvent"));
                QStringList devices;
                for (int enumKey = 0; enumKey < gearSensorEventEnum.keyCount(); ++enumKey) {
                    GearBase::GearSensorEvent eventKey = static_cast<GearBase::GearSensorEvent>(gearSensorEventEnum.value(enumKey));
                    devices << device->gearSensorTargetDevices(eventKey);
                }
                value = devices;
                break; }
            default:
                break;
        }
    };
    return value;
}

int DeviceModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return d->devices.count();
}

bool DeviceModel::isConnected() const
{
    for (GearBase* device : d->devices) {
        if (device->isConnected()) {
            return true;
        }
    }
    return false;
}

void DeviceModel::addDevice(const QBluetoothDeviceInfo& deviceInfo)
{
    GearBase* newDevice{nullptr};
    if (deviceInfo.name() == QLatin1String{"(!)Tail1"}) {
        newDevice = new GearDigitail(deviceInfo, this);
    } else if (deviceInfo.name() == QLatin1String{"mitail"}) {
        newDevice = new GearMitail(deviceInfo, this);
    } else if (deviceInfo.name() == QLatin1String{"EarGear"}) {
        newDevice = new GearEars(deviceInfo, this);
    } else if (deviceInfo.name() == QLatin1String{"EG2"}) {
        newDevice = new GearEars(deviceInfo, this);
    } else if (deviceInfo.name() == QLatin1String{"flutter"}) {
        newDevice = new GearFlutterWings(deviceInfo, this);
    } else if (deviceInfo.name() == QLatin1String{"minitail"}) {
        newDevice = new GearMitailMini(deviceInfo, this);
    } else {
        qDebug() << "Found an unsupported device" << deviceInfo.name();
    }
    if (newDevice) {
        addDevice(newDevice);
    }
}

void DeviceModel::addDevice(GearBase* newDevice)
{
    // It feels a little dirty to do it this way...
    static const QStringList acceptedDeviceNames{
        QLatin1String{"EarGear"},
        QLatin1String{"EG2"},
        QLatin1String{"mitail"},
        QLatin1String{"minitail"},
        QLatin1String{"flutter"},
        QLatin1String{"(!)Tail1"},
        QLatin1String{"FAKE"}
    };
    if(acceptedDeviceNames.contains(newDevice->deviceInfo.name())) {
        for(const GearBase* device : d->devices) {
            if(device->deviceID() == newDevice->deviceID()) {
                // Don't add the same device twice. Thanks bt discovery. Thiscovery.
                newDevice->deleteLater();
                return;
            }
        }

        // Device type specifics
        GearEars* ears = qobject_cast<GearEars*>(newDevice);
        if (ears) {
            connect(ears, &GearEars::listenModeChanged, this, [this, newDevice](){
                d->notifyDeviceDataChanged(newDevice, ListeningState);
            });
            connect(ears, &GearEars::micsSwappedChanged, this, [this, newDevice](){
                d->notifyDeviceDataChanged(newDevice, MicsSwapped);
            });
            connect(ears, &GearEars::hasTiltChanged, this, [this, newDevice](){
                d->notifyDeviceDataChanged(newDevice, HasTilt);
            });
            connect(ears, &GearEars::canBalanceListeningChanged, this, [this, newDevice](){
                d->notifyDeviceDataChanged(newDevice, CanBalanceListening);
            });
            connect(ears, &GearEars::tiltEnabledChanged, this, [this, newDevice](){
                d->notifyDeviceDataChanged(newDevice, TiltEnabled);
            });
        }

        // General stuff
        connect(newDevice, &GearBase::deviceMessage, this, &DeviceModel::deviceMessage);
        connect(newDevice, &GearBase::deviceBlockingMessage, this, &DeviceModel::deviceBlockingMessage);
        connect(newDevice, &GearBase::isConnectedChanged, this, [this, newDevice](bool isConnected){
            if (isConnected) {
                emit deviceConnected(newDevice);
            } else {
                emit deviceDisconnected(newDevice);
            }
        });
        connect(newDevice, &GearBase::progressDescriptionChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, ProgressDescription);
        });
        connect(newDevice, &GearBase::deviceProgressChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, OperationInProgress);
            d->notifyDeviceDataChanged(newDevice, DeviceProgress);
        });
        connect(newDevice, &GearBase::supportsOTAChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, SupportsOTA);
        });
        connect(newDevice, &GearBase::hasAvailableOTAChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, HasAvailableOTA);
            d->notifyDeviceDataChanged(newDevice, OTAVersion);
        });
        connect(newDevice, &GearBase::hasOTADataChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, HasOTAData);
        });
        connect(newDevice, &GearBase::batteryLevelChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, BatteryLevel);
        });
        connect(newDevice, &GearBase::enabledCommandsFilesChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, EnabledCommandsFiles);
        });
        connect(newDevice, &GearBase::currentCallChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, CurrentCall);
        });
        connect(newDevice, &GearBase::activeCommandTitlesChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, ActiveCommandTitles);
        });
        connect(newDevice, &GearBase::versionChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, DeviceVersion);
        });
        connect(newDevice, &GearBase::isConnectedChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, IsConnected);
            emit isConnectedChanged(isConnected());
        });
        connect(newDevice, &GearBase::nameChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, Name);
        });
        connect(newDevice, &GearBase::checkedChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, Checked);
        });
        connect(newDevice, &GearBase::hasLightsChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, HasLights);
        });
        connect(newDevice, &GearBase::hasShutdownChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, HasShutdown);
        });
        connect(newDevice, &GearBase::hasNoPhoneModeChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, HasNoPhoneMode);
        });
        connect(newDevice, &GearBase::noPhoneModeGroupsChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, NoPhoneModeGroups);
        });
        connect(newDevice, &GearBase::chargingStateChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, ChargingState);
        });
        connect(newDevice, &GearBase::batteryLevelPercentChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, BatteryLevelPercent);
        });
        connect(newDevice, &GearBase::knownFirmwareMessageChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, KnownFirmwareMessage);
        });
        connect(newDevice, &GearBase::gearSensorCommandDetailsChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, GestureEventValues);
            d->notifyDeviceDataChanged(newDevice, GestureEventTitles);
            d->notifyDeviceDataChanged(newDevice, GestureEventCommands);
            d->notifyDeviceDataChanged(newDevice, GestureEventDevices);
        });
        connect(newDevice, &QObject::destroyed, this, [this, newDevice](){
            int index = d->devices.indexOf(newDevice);
            if(index > -1) {
                beginRemoveRows(QModelIndex(), index, index);
                emit deviceRemoved(newDevice);
                d->devices.removeAll(newDevice);
                endRemoveRows();
            }
        });

        beginInsertRows(QModelIndex(), 0, 0);
        d->devices.insert(0, newDevice);
        d->readDeviceNames();
        emit deviceAdded(newDevice);
        emit countChanged();
        endInsertRows();
    }
}

void DeviceModel::removeDevice(GearBase* device)
{
    int idx = d->devices.indexOf(device);
    if (idx > -1) {
        beginRemoveRows(QModelIndex(), idx, idx);
        emit deviceRemoved(device);
        device->disconnect(this);
        d->devices.removeAt(idx);
        emit countChanged();
        endRemoveRows();
    }
    if (device != d->fakeDevice) {
        device->deleteLater();
    }
}

int DeviceModel::count()
{
    return d->devices.count();
}

GearBase* DeviceModel::getDevice(const QString& deviceID) const
{
    for(GearBase* device : d->devices) {
        if(device->deviceID() == deviceID) {
            return device;
        }
    }
    return nullptr;
}

GearBase * DeviceModel::getDeviceById ( int index ) const
{
    if (index > -1 && index < d->devices.count()) {
        return d->devices[index];
    }
    return nullptr;
}

void DeviceModel::updateItem(const QString& deviceID)
{
    d->readDeviceNames();
    for(int idx = 0; idx < d->devices.count(); ++idx) {
        if(d->devices[idx]->deviceID() == deviceID) {
            emit dataChanged(index(idx, 0), index(idx, 0));
        }
    }
}

QString DeviceModel::getDeviceID(int deviceIndex) const
{
    if(deviceIndex >= 0 && deviceIndex < d->devices.count()) {
        return d->devices.at(deviceIndex)->deviceID();
    }
    return QLatin1String();
}

void DeviceModel::sendMessage(const QString& message, const QStringList& deviceIDs)
{
    for (GearBase* device : d->devices) {
        // If there's no devices requested, send to everybody
        if (deviceIDs.count() == 0 || deviceIDs.contains(device->deviceID())) {
            device->sendMessage(message);
        }
    }
}
