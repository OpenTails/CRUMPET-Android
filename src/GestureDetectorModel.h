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

#ifndef GESTUREDETECTORMODEL_H
#define GESTUREDETECTORMODEL_H

#include "GestureController.h"
#include <QAbstractListModel>

struct GestureDetails;
class GestureDetectorModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit GestureDetectorModel(GestureController* parent = nullptr);
    ~GestureDetectorModel() override;

    enum Roles {
        NameRole = Qt::UserRole + 1,
        IdRole,
        SensorIdRole, ///< The ID of the sensor for this gesture
        SensorNameRole, ///< Name of the sensor to be shown to the user
        SensorEnabledRole, ///< Whether or not the sensor is enabled
        SensorPinnedRole, ///< Whether or not the sensor should be shown on the front page
        CommandRole,
        DefaultCommandRole,
        DevicesModel,
        FirstInSensorRole, ///< True if the row has a different sensor ID to the previous row
        VisibleRole, ///< Whether or not this should be shown
    };

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void setAppSettings(AppSettings* settings);

    void addGesture(GestureDetails* gesture);
    GestureDetails* gesture(const QString& gestureId) const;

    void gestureDetailsChanged(GestureDetails* gesture);
    void setGestureDetails(int index, QString command, QStringList devices);
    // Index is the index of a gesture, but the state is set for all gestures with the same sensor
    void setGestureSensorPinned(int index, bool pinned);
    // Index is the index of a gesture, but the state is set for all gestures with the same sensor
    void setGestureSensorEnabled(int index, bool enabled);

    void setGestureSensorEnabled(GestureDetails* gesture, bool enabled);

private:
    class Private;
    Private* d;
};

class QSensorGesture;
class GestureController;
struct GestureDetails {
public:
    GestureDetails(QString gestureId, QSensorGesture* sensor, GestureController* q);
    ~GestureDetails();

    void load();
    void save();
    QSensorGesture* sensor() const;
    QString sensorName() const;
    QString gestureId() const;
    QString humanName() const;
    void setCommand(const QString& value);
    QString command() const;
    QString defaultCommand() const;
    void setDevices(const QStringList& value);
    QStringList devices() const;
    bool visible() const;

    void startDetection();
    void stopDetection();
    bool sensorEnabled() const;
    bool sensorPinned() const;

private:
    friend class GestureDetectorModel;
    class Private;
    Private* d;
};

#endif//GESTUREDETECTORMODEL_H
