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

#ifndef ALARM_H
#define ALARM_H

#include <QObject>
#include <QDateTime>

class AlarmList;

/**
 * @brief The Alarm class represents alarm moves.
 * See also AlarmList class.
 */
class Alarm : public QObject
{
    Q_OBJECT

    //TODO: It seems we do not need Q_PROPERTIES here because in qml we use QVariantList and QVariantMap
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QDateTime time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(QStringList commands READ commands WRITE setCommands NOTIFY commandsChanged)

public:
    explicit Alarm(QObject *parent = nullptr);
    explicit Alarm(const QString& name, QObject* parent = nullptr);

    explicit Alarm(const QString& name,
                   const QDateTime& time,
                   const QStringList& commands,
                   QObject* parent);
    ~Alarm() override;

    QString name() const;

    QDateTime time() const;
    void setTime(const QDateTime& time);

    QStringList commands() const;
    void setCommands(const QStringList& commands);

    void addCommand(int index, const QString& command);
    void removeCommand(int index);

    QVariantMap toVariantMap() const;

public Q_SLOTS:

Q_SIGNALS:
    void nameChanged();
    void timeChanged();
    void commandsChanged();

    void alarmChanged();

private:
    //TODO: There is no any reason to use PIMPL idiom at these classes
    class Private;
    Private* d;

    void setName(const QString& name);

    /// We can change name only in AlarmList class
    friend class AlarmList;
};

#endif // ALARM_H
