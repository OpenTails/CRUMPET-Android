/*
 *   Copyright 2024 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef GESTURESENSOR_H
#define GESTURESENSOR_H

#include <QSensor>

class GestureSensorPrivate;
class GestureSensor : public QObject {
    Q_OBJECT
public:
    explicit GestureSensor(QObject* parent = nullptr);
    ~GestureSensor() override;

    virtual QStringList recognizerSignals() const = 0;
    virtual QString sensorId() const = 0;
    virtual QString humanName() const = 0;

    virtual void startDetection() = 0;
    virtual void stopDetection() = 0;

    Q_SIGNAL void detected(const QString &signature);
};

#endif//GESTURESENSOR_H
