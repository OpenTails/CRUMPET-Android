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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool advancedMode READ advancedMode WRITE setAdvancedMode NOTIFY advancedModeChanged)
    Q_PROPERTY(bool idleMode READ idleMode WRITE setIdleMode NOTIFY idleModeChanged)
    Q_PROPERTY(QStringList idleCategories READ idleCategories WRITE setIdleCategories NOTIFY idleCategoriesChanged)
    Q_PROPERTY(int idleMinPause READ idleMinPause WRITE setIdleMinPause NOTIFY idleMinPauseChanged)
    Q_PROPERTY(int idleMaxPause READ idleMaxPause WRITE setIdleMaxPause NOTIFY idleMaxPauseChanged)
public:
    explicit AppSettings(QObject* parent = 0);
    virtual ~AppSettings();

    bool advancedMode() const;
    void setAdvancedMode(bool newValue);

    bool idleMode() const;
    void setIdleMode(bool newValue);

    QStringList idleCategories() const;
    void setIdleCategories(const QStringList& newCategories);

    int idleMinPause() const;
    void setIdleMinPause(int pause);

    int idleMaxPause() const;
    void setIdleMaxPause(int pause);
Q_SIGNALS:
    void advancedModeChanged();
    void idleModeChanged();
    void idleCategoriesChanged();
    void idleMinPauseChanged();
    void idleMaxPauseChanged();

private:
    class Private;
    Private* d;
};

#endif
