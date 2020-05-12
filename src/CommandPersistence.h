/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef COMMANDPERSISTENCE_H
#define COMMANDPERSISTENCE_H

#include <QObject>
#include <QUrl>

#include "CommandInfo.h"

/**
 * A simple persistence system for command lists (including a title and description).
 */
class CommandPersistence : public QObject
{
    Q_OBJECT
    /**
     * The location of the file used for storing the data in this object.
     */
    Q_PROPERTY(QUrl location READ location WRITE setLocation NOTIFY locationChanged);
    /**
     * The (ordered) list of commands handled by this object
     */
    Q_PROPERTY(CommandInfoList commands READ commands WRITE setCommands NOTIFY commandsChanged);
    /**
     * An optional (but recommended) title for this list of commands
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged);
    /**
     * An optional (but recommended) description for this list of commands
     */
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged);
public:
    explicit CommandPersistence(QObject* parent = nullptr);
    virtual ~CommandPersistence();

    /**
     * Request that data is read from a previously set location.
     *
     * If the operation is successfully started, it will clear the
     * commands, title, and description. If the operation is
     * successfully completed, these will be repopulated
     * with the data read from the file at the set location.
     *
     * To see whether or not the operation completes successfully,
     * subscribe to the finished signal. Any errors will be emitted
     * through the error signal.
     *
     * You cannot start an operation (read or write) while another is
     * already ongoing.
     *
     * @return Whether or not the operation was successfully started
     * @see setLocation(QUrl)
     * @see error(QString)
     * @see finished(bool)
     */
    bool read();
    /**
     * Request that the commands, title, and description previously
     * set are written to the also previously set location.
     *
     * To see whether or not the operation completes successfully,
     * subscribe to the finished signal. Any errors will be emitted
     * through the error signal.
     *
     * You cannot start an operation (read or write) while another is
     * already ongoing.
     *
     * @return Whether or not the operation was successfully started
     * @see setLocation(QUrl)
     * @see error(QString)
     * @see finished(bool)
     */
    bool write();
    /**
     * Emitted when an error occurs during the process. It does not
     * specify a severity level, only that an error occurred. If it
     * was a critical error, it can be deduced from the finished signal
     * being fired with a successfulness of false.
     * @param message A human-readable message describing the error
     */
    Q_SIGNAL void error(QString message);
    /**
     * The error message most recently emitted by error(QString).
     * Useful in case you wish to make a more serious point of showing
     * the error after a failed completion (in the slot connected to
     * finished(bool) for example).
     * @return The most recently emitted error message
     */
    QString error() const;
    /**
     * Whether or not the most recently requested operation finished
     * successfully.
     */
    Q_SIGNAL void finished(bool success);

    QUrl location() const;
    void setLocation(const QUrl &location);
    Q_SIGNAL void locationChanged();

    CommandInfoList commands() const;
    void setCommands(const CommandInfoList &commands);
    Q_SIGNAL void commandsChanged();

    QString title() const;
    void setTitle(const QString &title);
    Q_SIGNAL void titleChanged();

    QString description() const;
    void setDescription(const QString &description);
    Q_SIGNAL void descriptionChanged();

private:
    class Private;
    Private* d;
};

#endif//COMMANDPERSISTENCE_H
