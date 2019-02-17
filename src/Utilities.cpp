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

#include "Utilities.h"

class Utilities::Private
{
public:
    Private()
        : connectionManagerProxy(nullptr)
    {}
    BTConnectionManagerProxyReplica* connectionManagerProxy;
};

Utilities::Utilities(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

Utilities::~Utilities()
{
    delete d;
}

void Utilities::getCommand(QString command)
{
    QVariantMap map;
    map.insert("command", command);
    d->connectionManagerProxy->pushCommand(map);
}

void Utilities::setConnectionManager(BTConnectionManagerProxyReplica* connectionManagerProxy)
{
    if(d->connectionManagerProxy) {
        this->disconnect(d->connectionManagerProxy);
    }
    d->connectionManagerProxy = connectionManagerProxy;
    if(d->connectionManagerProxy) {
        connect(d->connectionManagerProxy, &BTConnectionManagerProxyReplica::commandChanged, this, &Utilities::commandGotten);
    }
}
