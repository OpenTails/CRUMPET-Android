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

#include "Utilities.h"

#include <KLocalizedString>

class Utilities::Private
{
public:
    Private()
        : connectionManagerProxy(nullptr)
    {}
    BTConnectionManagerProxyReplica* connectionManagerProxy;
};

Utilities::Utilities(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

Utilities::~Utilities()
{
    delete d;
}

void Utilities::getCommand(QString command)
{
    QVariantMap map;
    map.insert(QLatin1String{"command"}, command);
    d->connectionManagerProxy->pushCommand(map);
}

void Utilities::setConnectionManager(BTConnectionManagerProxyReplica* connectionManagerProxy)
{
    if(d->connectionManagerProxy) {
        this->disconnect(d->connectionManagerProxy);
    }
    d->connectionManagerProxy = connectionManagerProxy;
    if(d->connectionManagerProxy) {
        connect(d->connectionManagerProxy, &BTConnectionManagerProxyReplica::commandChanged, this, &Utilities::commandGotten);
    }
}

static const QLatin1String stringUpperOne{"One"};
static const QLatin1String stringUpperTwo{"Two"};
static const QLatin1String stringUpperThree{"Three"};
static const QLatin1String stringUpperFour{"Four"};
static const QLatin1String stringUpperFive{"Five"};
static const QLatin1String stringUpperSix{"Six"};
static const QLatin1String stringUpperSeven{"Seven"};
static const QLatin1String stringUpperEight{"Eight"};
static const QLatin1String stringUpperNine{"Nine"};
static const QLatin1String stringRelaxed{"relaxed"};
static const QLatin1String stringExcited{"excited"};
static const QLatin1String stringTense{"tense"};
static const QLatin1String stringEarGearPoses{"eargearposes"};
static const QLatin1String stringLights{"lights"};
static const QLatin1String stringBlink{"BLINK"};
static const QLatin1String stringTriangle{"TRIANGLE"};
static const QLatin1String stringSawtooth{"SAWTOOTH"};
static const QLatin1String stringMorse{"MORSE"};
static const QLatin1String stringBeacon{"BEACON"};
static const QLatin1String stringFlame{"FLAME"};
static const QLatin1String stringStrobe{"STROBE"};
static const QLatin1String stringOn{"On"};
static const QLatin1String stringOff{"Off"};
QString Utilities::translateStaticString(const QString& stringToTranslate) const
{
    if (stringToTranslate == stringUpperOne) {
        return i18nc("The name of the first entry in any of the categories of moves", "One");
    } else if (stringToTranslate == stringUpperTwo) {
        return i18nc("The name of the second entry in any of the categories of moves", "Two");
    } else if (stringToTranslate == stringUpperThree) {
        return i18nc("The name of the third entry in any of the categories of moves", "Three");
    } else if (stringToTranslate == stringUpperFour) {
        return i18nc("The name of the fourth entry in any of the categories of moves", "Four");
    } else if (stringToTranslate == stringUpperFive) {
        return i18nc("The name of the fifth entry in any of the categories of moves", "Five");
    } else if (stringToTranslate == stringUpperSix) {
        return i18nc("The name of the sixth entry in any of the categories of moves", "Six");
    } else if (stringToTranslate == stringUpperSeven) {
        return i18nc("The name of the seventh entry in any of the categories of moves", "Seven");
    } else if (stringToTranslate == stringUpperEight) {
        return i18nc("The name of the eighth entry in any of the categories of moves", "Eight");
    } else if (stringToTranslate == stringUpperNine) {
        return i18nc("The name of the ninth entry in any of the categories of moves", "Nine");
    } else if (stringToTranslate == stringRelaxed) {
        return i18nc("The name of the calm and relaxed category of gear moves", "Calm and Relaxed");
    } else if (stringToTranslate == stringExcited) {
        return i18nc("The name of the fast and excited category of gear moves", "Fast and Excited");
    } else if (stringToTranslate == stringTense) {
        return i18nc("The name of the frustrated and tense category of gear moves", "Frustrated and Tense");
    } else if (stringToTranslate == stringEarGearPoses) {
        return i18nc("The name of the poses category of gear commands", "Poses");
    } else if (stringToTranslate == stringLights) {
        return i18nc("The name of the lights category of gear commands", "Glow Tip Patterns");
    } else if (stringToTranslate == stringBlink) {
        return i18nc("The name of the Blink command in the lights category of gear commands", "Blink");
    } else if (stringToTranslate == stringTriangle) {
        return i18nc("The name of the Triangle command in the lights category of gear commands", "Triangle");
    } else if (stringToTranslate == stringSawtooth) {
        return i18nc("The name of the Sawtooth command in the lights category of gear commands", "Sawtooth");
    } else if (stringToTranslate == stringMorse) {
        return i18nc("The name of the Morse command in the lights category of gear commands", "Morse");
    } else if (stringToTranslate == stringBeacon) {
        return i18nc("The name of the Beacon command in the lights category of gear commands", "Beacon");
    } else if (stringToTranslate == stringFlame) {
        return i18nc("The name of the Flame command in the lights category of gear commands", "Flame");
    } else if (stringToTranslate == stringStrobe) {
        return i18nc("The name of the Strobe command in the lights category of gear commands", "Strobe");
    } else if (stringToTranslate == stringOn) {
        return i18nc("The name of the On command in the lights category of gear commands", "On");
    } else if (stringToTranslate == stringOff) {
        return i18nc("The name of the Off command in the lights category of gear commands", "Off");
    }
    return stringToTranslate;
}
