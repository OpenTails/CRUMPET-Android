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

#include "WalkingSensorGestureReconizer.h"

#include <QCoreApplication>
#include <QAccelerometer>
#include <QElapsedTimer>
#include <QMutex>
#include <QThread>
#include <QTimer>

#include <algorithm>
#include <queue>
#include <vector>

#define SAMPLE_SIZE (50)

using ValueList = std::vector<qreal>;

WalkingSensorSignaller::WalkingSensorSignaller(WalkingSensor* parent)
    : QObject(parent)
{ }

WalkingSensorSignaller::~WalkingSensorSignaller() = default;

class WalkingSensorPrivate {
public:
    WalkingSensorPrivate(WalkingSensor *q)
        : q(q)
        , zValue{9.8}
        , workerThread(qApp)
        , stepCount{0}
    {
        accelerometer.setDataRate(30);
        accelerometer.setAccelerationMode(QAccelerometer::Combined);
    }
    WalkingSensor* q{nullptr};
    QTimer timer;
    QAccelerometer accelerometer;
    qreal zValue;
    QThread workerThread;
    QMutex mutex;

    int stepCount;
    ValueList zVals;
    QTimer isWalkingTimer;

    void countSteps();
};

static qreal median(const ValueList& input)
{
    auto sorted = input;
    std::sort(sorted.begin(), sorted.end());
    auto len = sorted.size();
    auto index = (len - 1) / 2;
    if (len % 2) {
        return sorted[index];
    } else {
        return (sorted[index] + sorted[index + 1]) / static_cast<qreal>(2.0);
    }
}

static std::vector<int> countZeros(const ValueList& deMeanedArray,
                            const ValueList& filteredArray)
{
    size_t i = 0;
    std::vector<int> zeroArray;
    zeroArray.resize(deMeanedArray.size());
    std::fill(zeroArray.begin(), zeroArray.end(), 0);

    while (i < (filteredArray.size() - 30)) {
        ValueList currentBuffer;
        std::copy_n(deMeanedArray.begin() + i, std::min(static_cast<std::size_t>(30), deMeanedArray.size() - i),
                    std::back_inserter(currentBuffer));
//        qDebug() << i << filteredArray.size() - 30;
        for (std::size_t bufferIndex = 0;
             bufferIndex < currentBuffer.size() - 1;
             ++bufferIndex) {
            if ((currentBuffer[bufferIndex] < 0) && (currentBuffer[bufferIndex+1] > 0)) {
                zeroArray[bufferIndex + i] = 1;
//                qDebug() << "bufferIndex" << bufferIndex
//                         << currentBuffer[bufferIndex] << currentBuffer[bufferIndex+1];
            }
        }

        i += 15;
    }
    return zeroArray;
}

ValueList deMeanValues(const ValueList& filteredArray)
{
    ValueList deMeanedArray;
    deMeanedArray.resize(filteredArray.size());
    qreal filteredArrayMean = std::accumulate(filteredArray.begin(),
                                              filteredArray.end(),
                                              static_cast<qreal>(0))
            / static_cast<qreal>(filteredArray.size());
    for (std::size_t i = 0; i < filteredArray.size(); ++i) {
        deMeanedArray[i] = filteredArray[i] - filteredArrayMean;
    }
    return deMeanedArray;
}

static ValueList medianFiltering(ValueList& zValues)
{
    ValueList filteredArray;
    for (size_t i = 0; i < zValues.size(); ++i) {
        zValues[i] = zValues[i] - 9.8; // gravity
        if (i > 10) {
            ValueList tmpBuff;
            std::copy_n(zValues.begin() + i - 10, 10,
                        std::back_inserter(tmpBuff));
            filteredArray.push_back(median(tmpBuff));
        }
    }
    return filteredArray;
}

WalkingSensor::WalkingSensor(QObject* parent)
    : GestureSensor(parent)
    , d(new WalkingSensorPrivate(this))
{
    d->isWalkingTimer.setInterval(4000);
    QObject::connect(&d->isWalkingTimer, &QTimer::timeout, [this] {
       Q_EMIT detected(QLatin1String{"walkingStopped"});
       d->stepCount = 0;
    });
    d->isWalkingTimer.setSingleShot(true);

    d->timer.moveToThread(&d->workerThread);
    d->timer.setInterval(16);
    d->timer.setSingleShot(false);
    connect(&d->timer, &QTimer::timeout,this, [this](){
        d->mutex.lock();
        d->zVals.push_back(d->zValue);
        d->mutex.unlock();
        if ((d->zVals.size() % SAMPLE_SIZE == 0) && (d->zVals.size() >= SAMPLE_SIZE)) {
            d->countSteps();
        }
        if (d->zVals.size() >= 5000) {
            ValueList tmp(d->zVals.end() - 1000, d->zVals.end());
            std::swap(d->zVals, tmp);
        }
        // Q_EMIT zValueTick(d->elapsedTimer.elapsed(), d->zValue - 9.8);
    });

    QObject::connect(&d->accelerometer, &QAccelerometer::readingChanged, this, [this](){
        auto value = d->accelerometer.reading();
        d->mutex.lock();
        d->zValue = value->z();
        d->mutex.unlock();
    });

    d->workerThread.start();
}

QStringList WalkingSensor::recognizerSignals() const
{
    static const QStringList signals{
        QLatin1String{"walkingStarted"},
        QLatin1String{"walkingStopped"},
        QLatin1String{"stepDetected"},
        QLatin1String{"evenStepDetected"},
        QLatin1String{"oddStepDetected"},
    };
    return signals;
}

QString WalkingSensor::sensorId() const
{
    static const QLatin1String id{"Tailcompany.Walking"};
    return id;
}

void WalkingSensor::startDetection()
{
    QMetaObject::invokeMethod(&d->timer, "start", Qt::QueuedConnection);

    d->accelerometer.setActive(true);
    d->accelerometer.setAlwaysOn(true);
}

void WalkingSensor::stopDetection()
{
    QMetaObject::invokeMethod(&d->timer, "stop", Qt::QueuedConnection);
    d->accelerometer.setActive(false);
    d->accelerometer.setAlwaysOn(false);
}
/*
bool WalkingSensorReading::isActive()
{
    return d->timer.isActive();
}*/

void WalkingSensorPrivate::countSteps()
{
    if (zVals.size() < SAMPLE_SIZE) {
        return;
    }
    ValueList tmp(zVals.end() - SAMPLE_SIZE, zVals.end());
//    qDebug() << QDateTime::currentDateTime() << QThread::currentThreadId();
    auto filteredArray = medianFiltering(tmp);
    auto deMeanedArray = deMeanValues(filteredArray);
    auto zeroArray = countZeros(deMeanedArray, filteredArray);

    for (std::size_t i = 0; i < zeroArray.size(); ++i) {
        auto currentNumber = zeroArray[i];

        if (currentNumber == 1) {
            for (std::size_t j = i; j < i + 20; ++j) {
                auto val = deMeanedArray[j];
                if (val > 0.3) {
                    stepCount += 1;
                    Q_EMIT q->detected(QLatin1String{"stepDetected"});
                    if (stepCount % 2) {
                        Q_EMIT q->detected(QLatin1String{"oddStepDetected"});
                    } else  {
                        Q_EMIT q->detected(QLatin1String{"evenStepDetected"});
                    }
                    if (!isWalkingTimer.isActive()) {
                        Q_EMIT q->detected(QLatin1String{"walkingStarted"});
                    }
                    isWalkingTimer.start();
                    break;
                }
            }
        }
    }
}
