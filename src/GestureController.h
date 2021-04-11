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

#ifndef GESTURECONTROLLER_H
#define GESTURECONTROLLER_H

#include "rep_GestureControllerProxy_source.h"

#include "AppSettings.h"

class BTConnectionManager;
class GestureDetectorModel;

/**
 * When enabled, the Gesture Controller will listen to gestures, and when one is identified
 * the associated command will be sent to the devices enabled for that gesture.
 * @see AppSettings
 */
class GestureController : public GestureControllerProxySource
{
    Q_OBJECT
public:
    explicit GestureController(QObject* parent = nullptr);
    ~GestureController() override;

    void setConnectionManager(BTConnectionManager* connectionManager);

    Q_SLOT void setGestureDetails(int index, QString command, QStringList devices) override;
    // Index is the index of a gesture, but the state is set for all gestures with the same sensor
    Q_SLOT void setGestureSensorPinned(int index, bool pinned) override;
    // Index is the index of a gesture, but the state is set for all gestures with the same sensor
    Q_SLOT void setGestureSensorEnabled(int index, bool enabled) override;

    GestureDetectorModel* model() const;
protected:
    // This is here because QSensorGesture does not support modern connect statements
    Q_SLOT void gestureDetected(const QString& gestureId);

private:
    class Private;
    Private* d;
};

#endif//GESTURECONTROLLER_H
