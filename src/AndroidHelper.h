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
#include <jni.h>

/**
 * AndroidHelper is used to communicate with Java side on Android
 */
class AndroidHelper
{
public:
    static void initStatic();
    static QString convertJStringToQString(JNIEnv *env, jstring str);

private:
};

#endif //ANDROID_HELPER_H
