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

#include <QCoreApplication>
#include <QVector>
#include <QDebug>

#include "PhoneEventList.h"
#include "PhoneEvent.h"
#include "CommandQueue.h"

class PhoneEventList::Private
{
public:
    static QVector<QString> m_eventStrings;

    QList<PhoneEvent*> list;
    CommandQueue* commandQueue = nullptr;

    Private()
    {
    }
};

/// If you change the m_eventStrings property
/// then you also should change the PhoneEventList::Event enum,
/// PhoneEventList::getEventDisplayName() method,
/// PhoneEventList() constructor
/// and PhonecallReceiver.java
QVector<QString> PhoneEventList::Private::m_eventStrings = {
    "IncomingCallReceived",
    "IncomingCallAnswered",
    "IncomingCallEnded",
    "OutgoingCallStarted",
    "OutgoingCallEnded",
    "MissedCall"
};

QString PhoneEventList::getEventName(PhoneEventList::Event event)
{
    int index = static_cast<int>(event);

    if (index >= 0 && index < PhoneEventList::Private::m_eventStrings.size()) {
        return PhoneEventList::Private::m_eventStrings.at(index);
    }

    qWarning() << QString("Unable to recognize the phone event '%1'").arg(index);

    return QString();
}

QString PhoneEventList::getEventDisplayName(PhoneEventList::Event event)
{
    // We use switch/case instead of predefined list of strings
    // because we should return translated strings,
    // and the switch is the simplest way to do that

    switch (event) {
    case (Event::IncomingCallReceived):
        return QCoreApplication::translate("PhoneEventList", "Incoming Call Received");
    case (Event::IncomingCallAnswered):
        return QCoreApplication::translate("PhoneEventList", "Incoming Call Answered");
    case (Event::IncomingCallEnded):
        return QCoreApplication::translate("PhoneEventList", "Incoming Call Ended");
    case (Event::OutgoingCallStarted):
        return QCoreApplication::translate("PhoneEventList", "Outgoing Call Started");
    case (Event::OutgoingCallEnded):
        return QCoreApplication::translate("PhoneEventList", "Outgoing Call Ended");
    case (Event::MissedCall):
        return QCoreApplication::translate("PhoneEventList", "Missed Call");
    default:
        qWarning() << QString("Unable to recognize the phone event '%1'").arg(static_cast<int>(event));
        return QCoreApplication::translate("PhoneEventList", "Unknown Event");
    }
}

PhoneEventList::PhoneEventList(QObject* parent)
    : QObject(parent),
      d(new Private())
{
    addPhoneEvent(new PhoneEvent(Event::IncomingCallReceived, this));
    addPhoneEvent(new PhoneEvent(Event::IncomingCallAnswered, this));
    addPhoneEvent(new PhoneEvent(Event::IncomingCallEnded, this));
    addPhoneEvent(new PhoneEvent(Event::OutgoingCallStarted, this));
    addPhoneEvent(new PhoneEvent(Event::OutgoingCallEnded, this));
    addPhoneEvent(new PhoneEvent(Event::MissedCall, this));
}

PhoneEventList::~PhoneEventList()
{
    delete d;
}

void PhoneEventList::handle(const PhoneEvent *event)
{
    if (!event) {
        qWarning() << "Unable to handle null phone event";
        return;
    }

    if (event->isEmpty()) {
        return;
    }

    d->commandQueue->clear();
    d->commandQueue->pushCommands(event->commands());
}

void PhoneEventList::handle(PhoneEventList::Event event)
{
    for (const PhoneEvent *phoneEvent : d->list) {
        if (phoneEvent->phoneEvent() == event) {
            handle(phoneEvent);
            return;
        }
    }

    qWarning() << "Unable to recognize the phone event type:" << static_cast<int>(event);
}

void PhoneEventList::handle(const QString &eventName)
{
    PhoneEvent *event = phoneEvent(eventName);

    if (event) {
        handle(event);
        return;
    }

    qWarning() << "Unable to recognize the phone event type:" << eventName;
}

int PhoneEventList::size() const
{
    return d->list.size();
}

PhoneEvent* PhoneEventList::at(int index) const
{
    if (index >= 0 && index < d->list.count()) {
        return d->list.at(index);
    };

    return nullptr;
}

bool PhoneEventList::exists(const QString &eventName) const
{
    return phoneEvent(eventName);
}

PhoneEvent* PhoneEventList::phoneEvent(const QString& eventName) const
{
    for (PhoneEvent* phoneEvent : d->list) {
        if (phoneEvent->name().compare(eventName, Qt::CaseInsensitive) == 0) {
            return phoneEvent;
        }
    }

    return nullptr;
}

int PhoneEventList::phoneEventIndex(const QString& eventName) const
{
    for (int i = 0; i < d->list.size(); ++i) {
        if (d->list.at(i)->name().compare(eventName, Qt::CaseInsensitive) == 0) {
            return i;
        }
    }

    return -1;;
}

void PhoneEventList::addPhoneEvent(PhoneEvent* phoneEvent)
{
    if (d->list.contains(phoneEvent)) {
        qWarning() << "The phone event already exists in Phone Event List";
        return;
    }

    if (exists(phoneEvent->name())) {
        qWarning() << "The phone event already exists in Phone Event List";
        return;
    }

    connect(phoneEvent, &PhoneEvent::phoneEventChanged, this, &PhoneEventList::listChanged);

    d->list.push_back(phoneEvent);
}

void PhoneEventList::setPhoneEventCommands(const QString& phoneEventName, const QStringList& commands)
{
    PhoneEvent* phoneEvent = this->phoneEvent(phoneEventName);

    if (phoneEvent) {
        phoneEvent->setCommands(commands);
    } else {
        qWarning() << QString("The phone event '%1' does not exist").arg(phoneEventName);
    }
}

void PhoneEventList::addPhoneEventCommand(const QString& phoneEventName, int index, const QString& command)
{
    PhoneEvent* phoneEvent = this->phoneEvent(phoneEventName);

    if (phoneEvent) {
        phoneEvent->addCommand(index, command);
    } else {
        qWarning() << QString("The phone event '%1' does not exist").arg(phoneEventName);
    }
}

void PhoneEventList::removePhoneEventCommand(const QString& phoneEventName, int index)
{
    PhoneEvent* phoneEvent = this->phoneEvent(phoneEventName);

    if (phoneEvent) {
        phoneEvent->removeCommand(index);
    } else {
        qWarning() << QString("The phone event '%1' does not exist").arg(phoneEventName);
    }
}

QVariantList PhoneEventList::toVariantList() const
{
    QVariantList result;

    for (const PhoneEvent* phoneEvent : d->list) {
        result.push_back(phoneEvent->toVariantMap());
    }

    return result;
}

QVariantMap PhoneEventList::getPhoneEventVariantMap(const QString& phoneEventName)
{
    if (phoneEventName.isEmpty()) {
        return QVariantMap();
    }

    PhoneEvent* phoneEvent = this->phoneEvent(phoneEventName);

    if (phoneEvent) {
        return phoneEvent->toVariantMap();
    } else {
        qWarning() << QString("The phone event '%1' does not exist").arg(phoneEventName);
        return QVariantMap();
    }
}

void PhoneEventList::setCommandQueue(CommandQueue* commandQueue)
{
    d->commandQueue = commandQueue;
}
