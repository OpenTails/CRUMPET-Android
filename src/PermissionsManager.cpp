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

#include "PermissionsManager.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

class PermissionsManager::Private {
public:
    Private(PermissionsManager* qq)
        : q(qq)
    {
        QObject::connect(&requestDelay, &QTimer::timeout, [this](){ doRequest(); });
        requestDelay.setInterval(500);
        requestDelay.setSingleShot(true);
    }
    PermissionsManager* q{nullptr};

    QTimer requestDelay;
    QStringList permissionsToRequest;
    bool bluetoothPermissionValid{false};
    bool hasBluetoothPermissions{false};
    void doRequest() {
#ifdef Q_OS_ANDROID
        QStringList needRequesting;
        for (const QString& permission : permissionsToRequest) {
            auto  result = QtAndroid::checkPermission(permission);
            if(result != QtAndroid::PermissionResult::Granted) {
                needRequesting << permission;
            }
        }
        if (!needRequesting.isEmpty()) {
            qDebug() << Q_FUNC_INFO << "Requesting permissions for" << needRequesting;
            QtAndroid::requestPermissions(needRequesting, [this](QtAndroid::PermissionResultMap resultHash){
                QHashIterator<QString, QtAndroid::PermissionResult> permissionsIterator(resultHash);
                while (permissionsIterator.hasNext()) {
                    permissionsIterator.next();
                    if (permissionsIterator.value() == QtAndroid::PermissionResult::Denied) {
                        qWarning() << Q_FUNC_INFO << "Permission actively denied for" << permissionsIterator.key();
                    }
                }
                invalidateKnownPermissions();
            });
        }
        permissionsToRequest.clear();
#endif
        invalidateKnownPermissions();
    }
    void invalidateKnownPermissions() {
        bluetoothPermissionValid = false;
        Q_EMIT q->permissionsChanged();
    }
};

PermissionsManager::PermissionsManager(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    QTimer::singleShot(1, this, &PermissionsManager::permissionsChanged);
}

PermissionsManager::~PermissionsManager()
{
    delete d;
}

void PermissionsManager::requestPermission(const QString& permission)
{
    d->permissionsToRequest << QString("android.permission.%1").arg(permission);
    d->requestDelay.start();
    qDebug() << Q_FUNC_INFO << "Delayed permissions request begun for" << permission;
}

void PermissionsManager::requestPermissionNow(const QString& permission)
{
    QEventLoop loop;
    d->permissionsToRequest << QString("android.permission.%1").arg(permission);
    d->doRequest();
    connect(this, &PermissionsManager::permissionsChanged, &loop, &QEventLoop::quit);
    connect(this, &PermissionsManager::permissionsChanged, &loop, [](){ qDebug() << "Permissions changed, exit event loop"; });
    qDebug() << Q_FUNC_INFO << "Starting inner event loop for immediate permissions request";
    loop.exec();
}

bool PermissionsManager::hasPermission(const QString& permission) const
{
    bool returnValue{false};
#ifdef Q_OS_ANDROID
    auto  result = QtAndroid::checkPermission(QString("android.permission.%1").arg(permission));//QString("android.permission.BLUETOOTH_SCAN"));
    if(result == QtAndroid::PermissionResult::Granted) {
        returnValue = true;
    } else {
        returnValue = false;
    }
#else
    returnValue = true;
#endif
    qDebug() << Q_FUNC_INFO << "Permission granted for" << permission << ":" << returnValue;
    return returnValue;
}

QStringList bluetoothPermissionNames() {
#ifdef Q_OS_ANDROID
    return QtAndroid::androidSdkVersion() > 30 ? QStringList{"BLUETOOTH_SCAN", "BLUETOOTH_CONNECT"} : QStringList{"ACCESS_FINE_LOCATION", "BLUETOOTH"};
#else
    return QStringList{"NOT_ANDROID_SO_NO_SCAN_PERMISSION"};
#endif
}

bool PermissionsManager::hasBluetoothPermissions() const
{
    if (d->bluetoothPermissionValid == false) {
        d->hasBluetoothPermissions = true;
        const QStringList permissions{bluetoothPermissionNames()};
        for (const QString& permission : qAsConst(permissions)) {
            if (hasPermission(permission) == false) {
                d->hasBluetoothPermissions = false;
                break;
            }
        }
        d->bluetoothPermissionValid = true;
    }
    return d->hasBluetoothPermissions;
}

void PermissionsManager::requestBluetoothPermissions()
{
    for (const QString &permission : bluetoothPermissionNames()) {
        d->permissionsToRequest << QString("android.permission.%1").arg(permission);
    }
    d->requestDelay.start();
    qDebug() << Q_FUNC_INFO << "Delayed permissions request begun for" << bluetoothPermissionNames();
}
