/*
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

#ifndef ANDROID_HELPER_H
#define ANDROID_HELPER_H

#include <QString>
#include <QSharedPointer>

#ifdef Q_OS_ANDROID

class AppSettings;
class SettingsProxyReplica;

/**
 * AndroidHelper is used to communicate with Java side on Android
 */
class AndroidHelper
{
public:
    static void initStatic(AppSettings *appSettings);
    static void initStatic(QSharedPointer<SettingsProxyReplica> appSettingsReplica);
    static QString convertJStringToQString(JNIEnv *env, jstring str);
    static void handlePhoneCall(const QString &callType);

private:
    static AppSettings *m_appSettings;
    static QSharedPointer<SettingsProxyReplica> m_appSettingsReplica;
};

#endif // Q_OS_ANDROID
#endif // ANDROID_HELPER_H
