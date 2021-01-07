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
#define GES

#include "rep_GestureControllerProxy_source.h"

#include "AppSettings.h"

class BTConnectionManager;

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

    void setAppSettings(AppSettings* settings);
    void setConnectionManager(BTConnectionManager* connectionManager);

    bool enabled() const override;
    void setEnabled(bool value) override;

    QStringList gestures() const override;

    void setCurrentGesture(QString gesture) override;
    QString currentGesture() const override;

    QString command() const override;
    void setCommand(QString value) override;
    QStringList devices() const override;
    void setDevices(QStringList value) override;

protected:
    // This is here because QSensorGesture does not support modern connect statements
    Q_SLOT void gestureDetected(const QString& gestureId);

private:
    class Private;
    Private* d;
};

#endif//GESTURECONTROLLER_H
