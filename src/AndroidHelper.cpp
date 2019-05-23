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

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QCoreApplication>
#include <QDebug>
#include <jni.h>

#include "AndroidHelper.h"
#include "AppSettings.h"
#include "rep_SettingsProxy_replica.h"
#include "PhoneEventList.h"

#ifdef Q_OS_ANDROID

AppSettings *AndroidHelper::m_appSettings = nullptr;
QSharedPointer<SettingsProxyReplica> AndroidHelper::m_appSettingsReplica;

extern "C"
{

JNIEXPORT void JNICALL Java_org_thetailcompany_digitail_TailService_phoneCallHandler__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring callTypeString)
{
    Q_UNUSED(obj)

    const QString callType = AndroidHelper::convertJStringToQString(env, callTypeString);

    QMetaObject::invokeMethod(qApp,
        [callType] { AndroidHelper::handlePhoneCall(callType); },
        Qt::QueuedConnection,
        nullptr);
}

} // extern "C"

void AndroidHelper::initStatic(AppSettings *appSettings)
{
    m_appSettings = appSettings;
}

void AndroidHelper::initStatic(QSharedPointer<SettingsProxyReplica> appSettingsReplica)
{
    m_appSettingsReplica = appSettingsReplica;
}

QString AndroidHelper::convertJStringToQString(JNIEnv *env, jstring str)
{
	  if (str == 0)
	  {
		    return QString();
	  }

	  int length = env->GetStringLength(str);
	  if (length == 0)
	  {
		    return QString();
	  }

	  const jchar* strPtr = env->GetStringChars(str, 0);
	  if (strPtr == 0)
	  {
		    return QString();
	  }

	  QString ret = QString(reinterpret_cast<const QChar *>(strPtr), length);
	  env->ReleaseStringChars(str, strPtr);

    return ret;
}

void AndroidHelper::handlePhoneCall(const QString &callType)
{
    qDebug() << "Handle phone call event:" << callType;

    if (m_appSettings) {
        qDebug() << "APP SETTINGS IS NOT NULL";
        m_appSettings->handlePhoneEvent(callType);
		    return;
    } else if (m_appSettingsReplica) {
        qDebug() << "APP SETTINGS REPLICA IS NOT NULL";
        m_appSettingsReplica->handlePhoneEvent(callType);
		    return;
    }
}

#endif // Q_OS_ANDROID
