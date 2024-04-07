/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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
#include "rep_AppSettingsProxy_source.h"

class AlarmList;

class AppSettings : public AppSettingsProxySource
{
    Q_OBJECT

public:
    explicit AppSettings(QObject* parent = nullptr);
    ~AppSettings() override;

    int androidApiLevel() const override;

    bool advancedMode() const override;
    void setAdvancedMode(bool newValue) override;

    bool developerMode() const override;
    void setDeveloperMode(bool newValue) override;

    bool idleMode() const override;
    void setIdleMode(bool newValue) override;

    bool autoReconnect() const override;
    void setAutoReconnect(bool newValue) override;

    bool alwaysSendToAll() const override;
    void setAlwaysSendToAll(bool alwaysSendToAll) override;

    QStringList idleCategories() const override;
    void setIdleCategories(QStringList newCategories) override;
    void addIdleCategory(const QString& category) override;
    void removeIdleCategory(const QString& category) override;

    int idleMinPause() const override;
    void setIdleMinPause(int pause) override;

    int idleMaxPause() const override;
    void setIdleMaxPause(int pause) override;

    bool fakeTailMode() const override;
    void setFakeTailMode(bool fakeTailMode) override;

    QStringList moveLists() const override;
    QStringList moveList() const override;
    void setActiveMoveList(const QString& moveListName) override;
    void addMoveList(const QString& moveListName) override;
    void removeMoveList(const QString& moveListName) override;
    void addMoveListEntry(int index, const QString& entry, QStringList devices) override;
    void removeMoveListEntry(int index) override;

    AlarmList* alarmListImpl() const;
    QVariantList alarmList() const override;
    void addAlarm(const QString& alarmName) override;
    void removeAlarm(const QString& alarmName) override;
    QVariantMap activeAlarm() const override;
    void setActiveAlarmName(const QString& alarmName) override;
    void changeAlarmName(const QString& newName) override;
    void setAlarmTime(const QDateTime& time) override;
    void setAlarmCommands(const QStringList& commands) override;
    void addAlarmCommand(int index, const QString& command, QStringList devices) override;
    void removeAlarmCommand(int index) override;

    QStringList availableLanguages() const override;
    QString languageOverride() const override;
    void setLanguageOverride ( QString languageOverride ) override;

    /**
     * A map of the available command files, with the following structure:
     * [QString (filename)] => QVariantMap [QString (title)] => QString - A short title for the file as interpreted from the contents on load
     *                                     [QString (description)] => QString - The long-form description of the file as interpreted from the contents on load
     *                                     [QString (contents)] => QString - The actual contents of the file
     *                                     [QString (isEditable)] => bool - Whether or not the contents can be changed (false when the file is a built-in)
     *                                     [QString (isValid)] => bool - Whether or not the contents are valid json/crumpet
     */
    QVariantMap commandFiles() const override;
    void addCommandFile(const QString& filename, const QString& content) override;
    // Changing and removing things not marked as Editable will fail silently (and commandFiles will simply not change)
    void removeCommandFile(const QString& filename) override;
    // Changing the content will reset the title and description, but only if it is valid (or they will be retained in the current session)
    void setCommandFileContents(const QString& filename, const QString& content) override;
    void renameCommandFile(const QString& filename, const QString& newFilename) override;

    void shutDownService() override;
private:
    class Private;
    Private* d;

    void loadAlarmList();
    void saveAlarmList();

private Q_SLOTS:
    void onAlarmListChanged();
};

#endif
