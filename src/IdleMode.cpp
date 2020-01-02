/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   This file based on sample code from Kirigami
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

#include "IdleMode.h"
#include "CommandQueue.h"
#include "CommandInfo.h"
#include "BTDeviceCommandModel.h"
#include "BTConnectionManager.h"

#include <QRandomGenerator>
#include <QTimer>

class IdleMode::Private {
public:
    Private()
        : appSettings(nullptr)
        , connectionManager(nullptr)
    {
        pushTimer.setInterval(0);
        pushTimer.setSingleShot(true);
        QObject::connect(&pushTimer, &QTimer::timeout, [this](){ actualPush(); });
    }
    ~Private() {}
    AppSettings* appSettings;
    BTConnectionManager* connectionManager;


    // Now we support multiple tails, we might end up in some odd situation where we get told by multiple tails
    // that we should be doing things. Let's try and avoid that, and postpone this all to the start of the event loop
    QTimer pushTimer;
    void push() {
        pushTimer.start();
    }
    void actualPush() {
        if(connectionManager && connectionManager->isConnected() && appSettings && appSettings->idleMode()) {
            CommandQueue* queue = qobject_cast<CommandQueue*>(connectionManager->commandQueue());
            BTDeviceCommandModel* commands = qobject_cast<BTDeviceCommandModel*>(connectionManager->commandModel());
            const QStringList categories = appSettings->idleCategories();
            qDebug() << "Correctly set up, able to push a command to the queue for casual mode";
            if(queue && commands) {
                if(queue->count() == 0 && appSettings->idleMode() == true && categories.count() > 0) {
                    const CommandInfo& command = commands->getRandomCommand(categories);
                    if(command.isValid()) {
                        qDebug() << "Command is valid, and we've got an empty queue. Add the command to the queue.";
                        queue->pushCommand(command.command, {});
                    }
                    queue->pushPause(QRandomGenerator::global()->bounded(appSettings->idleMinPause(), appSettings->idleMaxPause() + 1) * 1000, {});
                }
            }
        }
    }
};

IdleMode::IdleMode(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

IdleMode::~IdleMode()
{
    delete d;
}

void IdleMode::setAppSettings(AppSettings* settings)
{
    if(d->appSettings) {
        d->appSettings->disconnect(this);
    }
    d->appSettings = settings;
    connect(d->appSettings, &AppSettings::idleModeChanged, this, [this](){
        if (!d->appSettings->idleMode() && d->connectionManager) {
            CommandQueue* queue = qobject_cast<CommandQueue*>(d->connectionManager->commandQueue());
            if (queue) {
                qDebug() << "Casual mode has been disabled, clear the queue to avoid any mishaps.";
                queue->clear({});
            }
        }
        d->push();
    });
    connect(d->appSettings, &AppSettings::idleCategoriesChanged, this, [this](){ d->push(); });
}

void IdleMode::setConnectionManager(BTConnectionManager* connectionManager)
{
    if(d->connectionManager) {
        d->connectionManager->disconnect(this);
    }
    d->connectionManager = connectionManager;
    connect(d->connectionManager, &BTConnectionManager::commandQueueCountChanged, this, [this](){ d->push(); });
    connect(d->connectionManager, &BTConnectionManager::isConnectedChanged, this, [this](){
        // By the time this gets called we /should/ have an appSettings, but let's not add crashes, because
        // they're just so ugly...
        if (d->connectionManager->isConnected() && d->appSettings) {
            d->appSettings->setIdleMode(false);
        }
        d->push();
    });
    connect(qobject_cast<QAbstractItemModel*>(d->connectionManager->deviceModel()), &QAbstractItemModel::rowsInserted, this, [this](){ d->push(); });
    connect(qobject_cast<QAbstractItemModel*>(d->connectionManager->deviceModel()), &QAbstractItemModel::rowsRemoved, this, [this](){ d->push(); });
}
