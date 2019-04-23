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

#ifndef PHONE_EVENTS_LIST_H
#define PHONE_EVENTS_LIST_H

#include <QObject>

class PhoneEvent;
class CommandQueue;

/**
 * PhoneEventList is used to handle and manage phone events, like calls, sms and etc.
 */
class PhoneEventList : public QObject
{
    Q_OBJECT

public:
    /// If you change the Event enum
    /// then you also should change the PhoneEventList::Private::m_eventStrings property,
    /// the getEventDisplayName() method
    /// and the PhoneEventList() constructor
    enum class Event {
        IncomingCallReceived,
        IncomingCallAnswered,
        IncomingCallEnded,
        OutgoingCallStarted,
        OutgoingCallEnded,
        MissedCall
    };

    static QString getEventName(Event event);
    static QString getEventDisplayName(Event event);

    explicit PhoneEventList(QObject *parent = nullptr);
    ~PhoneEventList();

    void handle(const PhoneEvent *event);
    void handle(Event event);
    void handle(const QString &eventName);

    int size() const;
    PhoneEvent* at(int index) const;

    bool exists(const QString& eventName) const;
    PhoneEvent* phoneEvent(const QString& eventName) const;
    int phoneEventIndex(const QString& eventName) const;

    void setPhoneEventCommands(const QString& phoneEventName, const QStringList& commands);
    void addPhoneEventCommand(const QString& phoneEventName, int index, const QString& command);
    void removePhoneEventCommand(const QString& phoneEventName, int index);

    QVariantList toVariantList() const;
    QVariantMap getPhoneEventVariantMap(const QString& phoneEventName);

    void setCommandQueue(CommandQueue* commandQueue);

public slots:

signals:
    void listChanged();

private:
    class Private;
    Private* d;

    void addPhoneEvent(PhoneEvent* phoneEvent);
};

#endif // PHONE_EVENTS_LIST_H
