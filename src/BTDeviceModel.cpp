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

#include "BTDeviceModel.h"
#include "AppSettings.h"
#include "BTDevice.h"
#include "BTDeviceTail.h"
#include "BTDeviceFake.h"
#include "BTDeviceEars.h"
#include "BTDeviceMitail.h"

class BTDeviceModel::Private
{
public:
    Private(BTDeviceModel* qq)
        : q(qq)
    {
        readDeviceNames();
    }
    ~Private() {}
    BTDeviceModel* q;
    BTDeviceFake* fakeDevice{nullptr};

    void readDeviceNames();

    AppSettings* appSettings{nullptr};
    QList<BTDevice*> devices;

    void notifyDeviceDataChanged(BTDevice* device, int role)
    {
        int pos = devices.indexOf(device);
        if(pos > -1) {
            QModelIndex idx(q->index(pos));
            q->dataChanged(idx, idx, QVector<int>{role});
        }
    }
};

BTDeviceModel::BTDeviceModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    d->fakeDevice = new BTDeviceFake(QBluetoothDeviceInfo(QBluetoothAddress(QString("FA:KE:TA:IL")), QString("FAKE"), 0), this);
}

BTDeviceModel::~BTDeviceModel()
{
    delete d;
}

void BTDeviceModel::Private::readDeviceNames()
{
    if (!appSettings) {
        return;
    }

    const QVariantMap map = appSettings->deviceNames();
    for (QVariantMap::const_iterator it = map.cbegin(); it != map.cend(); ++it) {
        for (BTDevice* device : devices) {
            if (device->deviceID() == it.key()) {
                device->setName(it.value().toString());
                break;
            }
        }
    }
}

AppSettings *BTDeviceModel::appSettings() const
{
    return d->appSettings;
}

void BTDeviceModel::setAppSettings(AppSettings *appSettings)
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

QHash< int, QByteArray > BTDeviceModel::roleNames() const
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
    };
    return roles;
}

QVariant BTDeviceModel::data(const QModelIndex& index, int role) const
{
    QVariant value;
    if(index.isValid() && index.row() > -1 && index.row() < d->devices.count()) {
        BTDevice* device = d->devices.at(index.row());
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
                value = (qobject_cast<BTDeviceEars*>(device) != nullptr);
                break;
            case ListeningState:
            {
                int listeningState = 0;
                BTDeviceEars* ears = qobject_cast<BTDeviceEars*>(device);
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
                BTDeviceEars* ears = qobject_cast<BTDeviceEars*>(device);
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
            default:
                break;
        }
    };
    return value;
}

int BTDeviceModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return d->devices.count();
}

bool BTDeviceModel::isConnected() const
{
    for (BTDevice* device : d->devices) {
        if (device->isConnected()) {
            return true;
        }
    }
    return false;
}

void BTDeviceModel::addDevice(const QBluetoothDeviceInfo& deviceInfo)
{
    BTDevice* newDevice{nullptr};
    if (deviceInfo.name() == QLatin1String{"(!)Tail1"}) {
        newDevice = new BTDeviceTail(deviceInfo, this);
    } else if (deviceInfo.name() == QLatin1String{"mitail"}) {
        newDevice = new BTDeviceMitail(deviceInfo, this);
    } else if (deviceInfo.name() == QLatin1String{"EarGear"}) {
        newDevice = new BTDeviceEars(deviceInfo, this);
    } else {
        qDebug() << "Found an unsupported device" << deviceInfo.name();
    }
    if (newDevice) {
        addDevice(newDevice);
    }
}

void BTDeviceModel::addDevice(BTDevice* newDevice)
{
    // It feels a little dirty to do it this way...
    static const QStringList acceptedDeviceNames{
        QLatin1String{"EarGear"},
        QLatin1String{"mitail"},
        QLatin1String{"(!)Tail1"},
        QLatin1String{"FAKE"}
    };
    if(acceptedDeviceNames.contains(newDevice->deviceInfo.name())) {
        for(const BTDevice* device : d->devices) {
            if(device->deviceID() == newDevice->deviceID()) {
                // Don't add the same device twice. Thanks bt discovery. Thiscovery.
                newDevice->deleteLater();
                return;
            }
        }

        // Device type specifics
        BTDeviceEars* ears = qobject_cast<BTDeviceEars*>(newDevice);
        if (ears) {
            connect(ears, &BTDeviceEars::listenModeChanged, this, [this, newDevice](){
                d->notifyDeviceDataChanged(newDevice, ListeningState);
            });
            connect(ears, &BTDeviceEars::micsSwappedChanged, this, [this, newDevice](){
                d->notifyDeviceDataChanged(newDevice, MicsSwapped);
            });
        }

        // General stuff
        connect(newDevice, &BTDevice::deviceMessage, this, &BTDeviceModel::deviceMessage);
        connect(newDevice, &BTDevice::deviceBlockingMessage, this, &BTDeviceModel::deviceBlockingMessage);
        connect(newDevice, &BTDevice::isConnectedChanged, this, [this, newDevice](bool isConnected){
            if (isConnected) {
                emit deviceConnected(newDevice);
            } else {
                emit deviceDisconnected(newDevice);
            }
        });
        connect(newDevice, &BTDevice::progressDescriptionChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, ProgressDescription);
        });
        connect(newDevice, &BTDevice::deviceProgressChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, OperationInProgress);
            d->notifyDeviceDataChanged(newDevice, DeviceProgress);
        });
        connect(newDevice, &BTDevice::supportsOTAChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, SupportsOTA);
        });
        connect(newDevice, &BTDevice::hasAvailableOTAChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, HasAvailableOTA);
            d->notifyDeviceDataChanged(newDevice, OTAVersion);
        });
        connect(newDevice, &BTDevice::hasOTADataChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, HasOTAData);
        });
        connect(newDevice, &BTDevice::batteryLevelChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, BatteryLevel);
        });
        connect(newDevice, &BTDevice::enabledCommandsFilesChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, EnabledCommandsFiles);
        });
        connect(newDevice, &BTDevice::currentCallChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, CurrentCall);
        });
        connect(newDevice, &BTDevice::activeCommandTitlesChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, ActiveCommandTitles);
        });
        connect(newDevice, &BTDevice::versionChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, DeviceVersion);
        });
        connect(newDevice, &BTDevice::isConnectedChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, IsConnected);
            emit isConnectedChanged(isConnected());
        });
        connect(newDevice, &BTDevice::nameChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, Name);
        });
        connect(newDevice, &BTDevice::checkedChanged, this, [this, newDevice](){
            d->notifyDeviceDataChanged(newDevice, Checked);
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
        d->readDeviceNames();

        beginInsertRows(QModelIndex(), 0, 0);
        d->devices.insert(0, newDevice);
        emit deviceAdded(newDevice);
        emit countChanged();
        endInsertRows();
    }
}

void BTDeviceModel::removeDevice(BTDevice* device)
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

int BTDeviceModel::count()
{
    return d->devices.count();
}

BTDevice* BTDeviceModel::getDevice(const QString& deviceID) const
{
    for(BTDevice* device : d->devices) {
        if(device->deviceID() == deviceID) {
            return device;
        }
    }
    return nullptr;
}

BTDevice * BTDeviceModel::getDeviceById ( int index ) const
{
    if (index > -1 && index < d->devices.count()) {
        return d->devices[index];
    }
    return nullptr;
}

void BTDeviceModel::updateItem(const QString& deviceID)
{
    d->readDeviceNames();
    for(int idx = 0; idx < d->devices.count(); ++idx) {
        if(d->devices[idx]->deviceID() == deviceID) {
            emit dataChanged(index(idx, 0), index(idx, 0));
        }
    }
}

QString BTDeviceModel::getDeviceID(int deviceIndex) const
{
    if(deviceIndex >= 0 && deviceIndex < d->devices.count()) {
        return d->devices.at(deviceIndex)->deviceID();
    }
    return QLatin1String();
}

void BTDeviceModel::sendMessage(const QString& message, const QStringList& deviceIDs)
{
    for (BTDevice* device : d->devices) {
        // If there's no devices requested, send to everybody
        if (deviceIDs.count() == 0 || deviceIDs.contains(device->deviceID())) {
            device->sendMessage(message);
        }
    }
}
