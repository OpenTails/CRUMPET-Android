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

#ifndef PHONE_EVENT_H
#define PHONE_EVENT_H

#include <QObject>

#include "PhoneEventList.h"

/**
 * @brief The PhoneEvent class represents phone events moves.
 * See also PhoneEventList class.
 */
class PhoneEvent : public QObject
{
    Q_OBJECT

    //TODO: It seems we do not need Q_PROPERTIES here because in qml we use QVariantList and QVariantMap
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QStringList commands READ commands WRITE setCommands NOTIFY commandsChanged)

public:
    explicit PhoneEvent(PhoneEventList::Event event,
                        QObject* parent = nullptr);

    explicit PhoneEvent(PhoneEventList::Event event,
                        const QStringList& commands,
                        QObject* parent);
    ~PhoneEvent();

    PhoneEventList::Event phoneEvent() const;
    QString name() const;
    QString displayName() const;

    bool isEmpty() const;

    QStringList commands() const;
    void setCommands(const QStringList& commands);

    void addCommand(int index, const QString& command);
    void removeCommand(int index);

    QVariantMap toVariantMap() const;

public slots:

signals:
    void commandsChanged();
    void phoneEventChanged();

private:
    //TODO: There is no any reason to use PIMPL idiom at these classes
    class Private;
    Private* d;
};

#endif // PHONE_EVENT_H
