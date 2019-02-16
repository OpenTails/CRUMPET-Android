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

#include "Settings.h"

#include <QSettings>

class AppSettings::Private
{
public:
    Private()
        : advancedMode(false)
        , idleMode(false)
        , idleMinPause(15)
        , idleMaxPause(60)
    {
        idleCategories << "relaxed";
    }
    ~Private() {}

    bool advancedMode;
    bool idleMode;
    QStringList idleCategories;
    int idleMinPause;
    int idleMaxPause;
    QMap<QString, QStringList> moveLists;
    QString activeMoveListName;
};

AppSettings::AppSettings(QObject* parent)
    : SettingsProxySource(parent)
    , d(new Private)
{
    QSettings settings;

    d->advancedMode = settings.value("advancedMode", d->advancedMode).toBool();
    d->idleMode = settings.value("idleMode", d->idleMode).toBool();
    d->idleCategories = settings.value("idleCategories", d->idleCategories).toStringList();
    d->idleMinPause = settings.value("idleMinPause", d->idleMinPause).toInt();
    d->idleMaxPause = settings.value("idleMaxPause", d->idleMaxPause).toInt();

    settings.beginGroup("MoveLists");
    QStringList moveLists = settings.allKeys();
    for(const QString& list : moveLists) {
        d->moveLists[list] = settings.value(list).toString().split(';');
    }
    settings.endGroup();

    auto save = [this](){
        QSettings settings;
        settings.beginGroup("MoveLists");
        QMap<QString, QStringList>::const_iterator i = d->moveLists.constBegin();
        while(i != d->moveLists.constEnd()) {
            if(!i.key().isEmpty()) {
                settings.setValue(i.key(), i.value().join(';'));
            }
            ++i;
        }
        settings.endGroup();
    };
    connect(this, &AppSettings::moveListsChanged, save);
    connect(this, &AppSettings::moveListChanged, save);

}

AppSettings::~AppSettings()
{
    delete d;
}

bool AppSettings::advancedMode() const
{
    return d->advancedMode;
}

void AppSettings::setAdvancedMode(bool newValue)
{
    qDebug() << Q_FUNC_INFO << newValue;
    if(newValue != d->advancedMode) {
        d->advancedMode = newValue;
        QSettings settings;
        settings.setValue("advancedMode", d->advancedMode);
        emit  advancedModeChanged(newValue);
    }
}

bool AppSettings::idleMode() const
{
    return d->idleMode;
}

void AppSettings::setIdleMode(bool newValue)
{
    qDebug() << Q_FUNC_INFO << newValue;
    if(newValue != d->idleMode) {
        d->idleMode = newValue;
        QSettings settings;
        settings.setValue("idleMode", d->idleMode);
        emit idleModeChanged(newValue);
    }
}

QStringList AppSettings::idleCategories() const
{
    return d->idleCategories;
}

void AppSettings::setIdleCategories(QStringList newCategories)
{
    qDebug() << Q_FUNC_INFO << newCategories;
    if(newCategories != d->idleCategories) {
        d->idleCategories = newCategories;
        QSettings settings;
        settings.setValue("idleCategories", d->idleCategories);
        emit idleCategoriesChanged(newCategories);
    }
}

void AppSettings::addIdleCategory(const QString& category)
{
    if(!d->idleCategories.contains(category)) {
        d->idleCategories << category;
        QSettings settings;
        settings.setValue("idleCategories", d->idleCategories);
        emit idleCategoriesChanged(d->idleCategories);
    }
}

void AppSettings::removeIdleCategory(const QString& category)
{
    if(d->idleCategories.contains(category)) {
        d->idleCategories.removeAll(category);
        QSettings settings;
        settings.setValue("idleCategories", d->idleCategories);
        emit idleCategoriesChanged(d->idleCategories);
    }
}

int AppSettings::idleMinPause() const
{
    return d->idleMinPause;
}

void AppSettings::setIdleMinPause(int pause)
{
    qDebug() << Q_FUNC_INFO << pause;
    if(pause != d->idleMinPause) {
        d->idleMinPause = pause;
        QSettings settings;
        settings.setValue("idleMinPause", d->idleMinPause);
        emit idleMinPauseChanged(pause);
    }
}

int AppSettings::idleMaxPause() const
{
    return d->idleMaxPause;
}

void AppSettings::setIdleMaxPause(int pause)
{
    qDebug() << Q_FUNC_INFO << pause;
    if(pause != d->idleMaxPause) {
        d->idleMaxPause = pause;
        QSettings settings;
        settings.setValue("idleMaxPause", d->idleMaxPause);
        emit idleMaxPauseChanged(pause);
    }
}

QStringList AppSettings::moveLists() const
{
    QStringList keys = d->moveLists.keys();
    // For some decidedly silly reason, we end up going from no entries
    // to two entries when adding the first one, and we then have a ghost one
    if(keys.count() > 0 && keys.at(0).isEmpty()) {
        keys.takeAt(0);
    }
    return keys;
}

void AppSettings::addMoveList(const QString& moveListName)
{
    if(!moveListName.isEmpty()) {
        d->moveLists.insert(moveListName, QStringList());
        emit moveListsChanged(moveLists());
    }
}

void AppSettings::removeMoveList(const QString& moveListName)
{
    d->moveLists.remove(moveListName);
    emit moveListsChanged(d->moveLists.keys());
}

QStringList AppSettings::moveList() const
{
    return d->moveLists[d->activeMoveListName];
}

void AppSettings::setActiveMoveList(const QString& moveListName)
{
    d->activeMoveListName = moveListName;
    emit moveListChanged(moveList());
}

void AppSettings::addMoveListEntry(int index, const QString& entry)
{
    QStringList moveList = d->moveLists[d->activeMoveListName];
    moveList.insert(index, entry);
    d->moveLists[d->activeMoveListName] = moveList;
    emit moveListChanged(this->moveList());
}

void AppSettings::removeMoveListEntry(int index)
{
    QStringList moveList = d->moveLists[d->activeMoveListName];
    moveList.removeAt(index);
    d->moveLists[d->activeMoveListName] = moveList;
    emit moveListChanged(this->moveList());
}
