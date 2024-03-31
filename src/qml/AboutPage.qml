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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.AboutPage {
    Digitail.FilterProxyModel {
        id: deviceFilterProxy;
        sourceModel: Digitail.DeviceModel;
        filterRole: Digitail.DeviceModelTypes.IsConnected;
        filterBoolean: true;
        function handyStringMakerThing() {
            if (count === 1) {
                return i18nc("A label which describes the hardware revision of the connected gear, when only one item is connected", "The connected gear is version %1", data(index(0, 0), Digitail.DeviceModelTypes.DeviceVersion))
            } else {
                var constructedString = "";
                var newLine = "";
                for (var i = 0; i < count; ++i) {
                    constructedString += newLine + i18nc("A label which describes the hardware revision of a piece of connected gear, when more than one item is connected", "The gear named %1 is version %2", data(index(i, 0), Digitail.DeviceModelTypes.Name), data(index(i, 0), Digitail.DeviceModelTypes.DeviceVersion));
                    newLine = "\n";
                }
                return constructedString;
            }
        }
    }
    objectName: "aboutPage";
    getInvolvedUrl: "https://github.com/OpenTails/CRUMPET-Android"
    aboutData: {
        "displayName" : "Crumpet",
        "productName" : "digitail/controller",
        "programLogo" : "qrc:/icon-round.png",
        "componentName" : "digitail",
        "shortDescription" : "Crumpet, MiTail, DIGITAiL, and EarGear designed and created by The Tail Company",
        "homepage" : "https://thetailcompany.com/",
        "bugAddress" : "contact@thetailcompany.com",
        "version" : AppVersion,
        "otherText" : deviceFilterProxy.count > 0 ? deviceFilterProxy.handyStringMakerThing() : "",
        "authors" : [
                    {
                        "name" : "MT at The Tail Company",
                        // TODO Reenable this when the about page wraps the person delegate properly
                        //"task" : i18nc("Task description for mt", ""),
                        "emailAddress" : "contact@thetailcompany.com",
                        "webAddress" : "https://thetailcompany.com/",
                    },
                    {
                        "name" : "Dan Leinir Turthra Jensen",
                        //"task" : i18nc("Task description for leinir", "Lead Developer"),
                        "emailAddress" : "admin@leinir.dk",
                        "webAddress" : "https://leinir.dk/",
                        "ocsUsername" : "leinir"
                    },
                    {
                        "name" : "Ildar Gilmanov",
                        //"task" : i18nc("Task description for Ildar", "Qt/QML Developer"),
                        "emailAddress" : "gil.ildar@gmail.com",
                        "webAddress": ""
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
        "copyrightStatement" : "© 2018-2024 The Tail Company Community",
        "desktopFileName" : "org.thetailcompany.digitail"
    }
}

