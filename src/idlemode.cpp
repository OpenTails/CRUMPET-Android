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

#include "idlemode.h"
#include "commandqueue.h"

#include <QRandomGenerator>

class IdleMode::Private {
public:
    Private()
        : appSettings(nullptr)
        , connectionManager(nullptr)
    {}
    ~Private() {}
    AppSettings* appSettings;
    BTConnectionManager* connectionManager;

    void push() {
        if(connectionManager && appSettings && appSettings->idleMode()) {
            qDebug() << "Pushing command to the queue for casual mode";
            CommandQueue* queue = qobject_cast<CommandQueue*>(connectionManager->commandQueue());
            TailCommandModel* commands = qobject_cast<TailCommandModel*>(connectionManager->commandModel());
            const QStringList categories = appSettings->idleCategories();
            if(queue && commands) {
                if(queue->count() == 0 && appSettings->idleMode() == true && categories.count() > 0) {
                    TailCommandModel::CommandInfo* command = commands->getRandomCommand(categories);
                    if(command) {
                        queue->pushCommand(command->command);
                        queue->pushPause(QRandomGenerator::global()->bounded(appSettings->idleMinPause(), appSettings->idleMaxPause() + 1) * 1000);
                    }
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
    connect(d->appSettings, &AppSettings::idleModeChanged, this, [this](){ d->push(); });
    connect(d->appSettings, &AppSettings::idleCategoriesChanged, this, [this](){ d->push(); });
}

void IdleMode::setConnectionManager(BTConnectionManager* connectionManager)
{
    if(d->connectionManager) {
        d->connectionManager->disconnect(this);
    }
    d->connectionManager = connectionManager;
    connect(d->connectionManager, &BTConnectionManager::commandQueueCountChanged, this, [this](){ d->push(); });
}
