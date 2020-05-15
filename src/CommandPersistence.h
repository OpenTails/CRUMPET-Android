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
     * The base name of the file used for storing the data in this object.
     * The actual location will depend on the system, but is subdirectory
     * "commands" in the first writable location from
     * QStandardPaths::AppLocalDataLocation
     */
    Q_PROPERTY(QString filename READ filename WRITE setFilename NOTIFY filenameChanged);
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
     * Set the the title, description, and commands list based on the json contained
     * within the string.
     *
     * @note, unless the string is invalid json, this will reset
     * those three properties, even if the document is empty.
     *
     * @param json A string which should contain a fully formed json document
     * @return Whether or not the deserialisation was successful
     * @see error()
     * @see read()
     */
    bool deserialize(const QString& json);
    /**
     * Get the json serialised version of what is currently stored in this class.
     * This is also what will be stored in the file when you write it.
     * @return A json document in text format
     * @see write()
     */
    QString serialized() const;

    /**
     * Request that data is read from a previously set location.
     *
     * If the operation is successfully started, it will clear the
     * commands, title, and description. If the operation is
     * successfully completed, these will be repopulated
     * with the data read from the file at the set location.
     *
     * @return Whether or not the operation was successfully completed
     * @see setLocation(QUrl)
     * @see error(QString)
     */
    bool read();
    /**
     * Request that the commands, title, and description previously
     * set are written to the also previously set location.
     *
     * @return Whether or not the operation was successfully completed
     * @see setLocation(QUrl)
     * @see error(QString)
     */
    bool write();

    /**
     * Emitted when an error occurs during any process. It does not
     * specify a severity level, only that an error occurred.
     * @param message A human-readable message describing the error
     */
    Q_SIGNAL void error(QString message);
    /**
     * The error message most recently emitted by error(QString).
     * Useful in case you wish to make a more serious point of showing
     * the error after a failed completion.
     * @return The most recently emitted error message
     */
    QString error() const;

    QString filename() const;
    void setFilename(const QString &filename);
    Q_SIGNAL void filenameChanged();

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
