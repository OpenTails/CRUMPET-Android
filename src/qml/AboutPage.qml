/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   This file based on sample code from Kirigami
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

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.6 as Kirigami
import org.thetailcompany.digitail 1.0

Kirigami.AboutPage {
    aboutData: {
        "displayName" : "DIGITAiL Controller",
        "productName" : "digitail/controller",
        "programLogo" : ":/logo.svg",
        "componentName" : "digitail",
        "shortDescription" : "Controller App for DIGITAiL animatronic tails\nDIGITAiL designed and created by The Tail Company",
        "homepage" : "https://thetailcompany.com/",
        "bugAddress" : "info@thetailcompany.com",
        "version" : "1.0",
        "otherText" : connectionManager.isConnected ? qsTr("Connected tail is version %1").arg(connectionManager.commandModel.tailVersion) : "",
        "authors" : [
            {
                "name" : "Dan Leinir Turthra Jensen",
                "task" : "Lead Developer",
                "emailAddress" : "admin@leinir.dk",
                "webAddress" : "https://leinir.dk/",
                "ocsUsername" : "leinir"
            }
        ],
        "credits" : [],
        "translators" : [],
        "licenses" : [
            {
                "name" : "GPL v3",
                "text" : "long, boring, license text",
                "spdx" : "GPL-3.0"
            }
        ],
        "copyrightStatement" : "© 2018-2019 The Tail Company Community",
        "desktopFileName" : "org.thetailcompany.digitail"
        }
}

