/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef WALKINGSENSORGESTURERECONIZER_H
#define WALKINGSENSORGESTURERECONIZER_H

#include "GestureSensor.h"

#include <QByteArray>

class WalkingSensor;
class WalkingSensorSignaller : public QObject
{
    Q_OBJECT
public:
    explicit WalkingSensorSignaller(WalkingSensor *parent);
    ~WalkingSensorSignaller() override;
Q_SIGNALS:
    void walkingStarted();
    void walkingStopped();
    void stepDetected();
    void evenStepDetected();
    void oddStepDetected();
private:
    Q_DISABLE_COPY(WalkingSensorSignaller)
};

class WalkingSensorPrivate;
class WalkingSensor : public GestureSensor {
    Q_OBJECT
public:
    explicit WalkingSensor(QObject *parent);
    QStringList recognizerSignals() const override;
    QString sensorId() const override;
    void startDetection() override;
    void stopDetection() override;
private:
    WalkingSensorPrivate* d{nullptr};
};


#endif // WALKINGSENSORGESTURERECONIZER_H
