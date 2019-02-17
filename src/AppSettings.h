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
#include "rep_SettingsProxy_source.h"

class AppSettings : public SettingsProxySource
{
    Q_OBJECT

public:
    explicit AppSettings(QObject* parent = 0);
    virtual ~AppSettings();

    bool advancedMode() const override;
    void setAdvancedMode(bool newValue) override;

    bool idleMode() const override;
    void setIdleMode(bool newValue) override;

    QStringList idleCategories() const override;
    void setIdleCategories(QStringList newCategories) override;
    void addIdleCategory(const QString& category) override;
    void removeIdleCategory(const QString& category) override;

    int idleMinPause() const override;
    void setIdleMinPause(int pause) override;

    int idleMaxPause() const override;
    void setIdleMaxPause(int pause) override;

    QStringList moveLists() const override;
    QStringList moveList() const override;
    void setActiveMoveList(const QString& moveListName) override;
    void addMoveList(const QString& moveListName) override;
    void removeMoveList(const QString& moveListName) override;
    void addMoveListEntry(int index, const QString& entry) override;
    void removeMoveListEntry(int index) override;

private:
    class Private;
    Private* d;
};

#endif
