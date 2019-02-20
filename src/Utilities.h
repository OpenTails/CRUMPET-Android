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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>
#include <QVariantMap>

#include "rep_BTConnectionManagerProxy_replica.h"

class Utilities : public QObject
{
    Q_OBJECT
public:
    ~Utilities() override;

    static Utilities* getInstance() {
        static Utilities* instance = nullptr;
        if(!instance) {
            instance = new Utilities();
        }
        return instance;
    }

    //TODO: It would be great to represent commands as objects, not strings or QVariantMap
    /**
     * An awkward way to get a command from the service via the replicated objects
     * Request a command by calling getCommand, and then listen to the commandGotten
     * signal, which will be emitted for all responses from the service (that is, likely
     * more than once per request)
     */
    Q_INVOKABLE void getCommand(QString command);
    Q_SIGNAL void commandGotten(QVariantMap command);

    void setConnectionManager(BTConnectionManagerProxyReplica* connectionManagerProxy);

private:
    explicit Utilities(QObject* parent = nullptr);
    class Private;
    Private* d;
};

#endif//UTILITIES_H
