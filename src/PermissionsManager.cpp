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
    void doRequest() {
        bool hasGranted = false;
#ifdef Q_OS_ANDROID
        QStringList needRequesting;
        for (const QString& permission : permissionsToRequest) {
            auto  result = QtAndroid::checkPermission(permission);
            if(result == QtAndroid::PermissionResult::Granted && !hasGranted) {
                hasGranted = true;
            } else {
                needRequesting << permission;
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
                    emit q->permissionsChanged();
                });
            }
        }
        permissionsToRequest.clear();
#else
        hasGranted = true;
#endif
        if(hasGranted) {
            emit q->permissionsChanged();
        }
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
