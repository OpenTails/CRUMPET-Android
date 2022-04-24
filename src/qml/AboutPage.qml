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
import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0

Kirigami.AboutPage {
    FilterProxyModel {
        id: deviceFilterProxy;
        sourceModel: DeviceModel;
        filterRole: 262; // the isConnected role
        filterBoolean: true;
        function handyStringMakerThing() {
            if (count === 1) {
                return i18nc("A label which describes the hardware revision of the connected gear, when only one item is connected", "The connected gear is version %1", data(index(0, 0), 259))
            } else {
                var constructedString = "";
                var newLine = "";
                for (var i = 0; i < count; ++i) {
                    constructedString += newLine + i18nc("A label which describes the hardware revision of a piece of connected gear, when more than one item is connected", "The gear named %1 is version %2", data(index(i, 0), 257), data(index(i, 0), 259));
                    newLine = "\n";
                }
                return constructedString;
            }
        }
    }
    objectName: "aboutPage";
    getInvolvedUrl: "https://github.com/MasterTailer/CRUMPET"
    aboutData: {
        "displayName" : "Crumpet",
        "productName" : "digitail/controller",
        "programLogo" : "qrc:/icon-round.png",
        "componentName" : "digitail",
        "shortDescription" : "Crumpet, MiTail, DIGITAiL, and EarGear designed and created by The Tail Company",
        "homepage" : "https://thetailcompany.com/",
        "bugAddress" : "info@thetailcompany.com",
        "version" : AppVersion,
        "otherText" : deviceFilterProxy.count > 0 ? deviceFilterProxy.handyStringMakerThing() : "",
        "authors" : [
                    {
                        "name" : "Dan Leinir Turthra Jensen",
                        // TODO Reenable this when the about page wraps the person delegate properly
                        //"task" : i18nc("Task description for leinir", "Lead Developer"),
                        "emailAddress" : "admin@leinir.dk",
                        "webAddress" : "https://leinir.dk/",
                        "ocsUsername" : "leinir"
                    },
                    {
                        "name" : "Ildar Gilmanov",
                        //"task" : i18nc("Task description for Ildar", "Qt/QML Developer"),
                        "emailAddress" : "gil.ildar@gmail.com"
                    }
                ],
        "credits" : [],
        "translators" : [],
        "licenses" : [
                    {
                        "name" : "GPL v3",
                        "text" : GPLv3LicenseText,
                        "spdx" : "GPL-3.0"
                    }
                ],
        "copyrightStatement" : "© 2018-2022 The Tail Company Community",
        "desktopFileName" : "org.thetailcompany.digitail"
    }
}

