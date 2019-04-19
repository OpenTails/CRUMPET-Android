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
#include <QDebug>

#include "AndroidHelper.h"

static void phoneCallHandler(JNIEnv *env, jobject obj, jstring callTypeString)
{
    Q_UNUSED(obj)

    const QString callType = AndroidHelper::convertJStringToQString(env, callTypeString);
    qDebug() << "Inside first C++ function:" << callType;
}

void AndroidHelper::initStatic()
{
    JNINativeMethod methods[] {
        {"phoneCallHandler", "(Ljava/lang/String;)V", reinterpret_cast<void*>(phoneCallHandler)}
    };

    QAndroidJniObject javaClass("org/thetailcompany/digitail/TailService");
    QAndroidJniEnvironment env;
    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());

    env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
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