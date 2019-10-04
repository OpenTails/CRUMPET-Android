/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef TAILCOMMANDMODEL_H
#define TAILCOMMANDMODEL_H

#include <QAbstractListModel>

class TailCommandModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString tailVersion READ tailVersion NOTIFY tailVersionChanged)
public:
    explicit TailCommandModel(QObject* parent = nullptr);
    ~TailCommandModel() override;

    enum Roles {
        Name = Qt::UserRole + 1,
        Command,
        IsRunning,
        Category,
        Duration,
        MinimumCooldown,
        CommandIndex
    };

    // TODO Move this somewhere more useful (maybe its own location? Maybe create a POD?
    struct CommandInfo {
    public:
        CommandInfo() {}
        CommandInfo(const CommandInfo& other)
            : name(other.name)
            , command(other.command)
            , category(other.category)
            , duration(other.duration)
            , minimumCooldown(other.minimumCooldown)
        { }
        QString name;
        QString command;
        QString category;
        int duration{0}; // milliseconds
        int minimumCooldown{0}; // milliseconds

        bool isRunning{false};

        bool compare(const CommandInfo& other) {
            // Not comparing isRunning, as that isn't necessarily quite as true...
            return (
                name == other.name &&
                command == other.command &&
                category == other.category &&
                duration == other.duration &&
                minimumCooldown == other.minimumCooldown
            );
        }
    };
    typedef QList<CommandInfo*> CommandInfoList;

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Clear the model of all known commands
     */
    void clear();

    /**
     * Add a new command to the model.
     * The new command is added at the start of the unsorted model
     * The model takes ownership of the command, and deletion should not
     * be done manually.
     * @param command The new command to show in the model
     */
    void addCommand(CommandInfo* command);
    Q_SIGNAL void commandAdded(CommandInfo* command);
    /**
     * Remove a command from the model.
     * The entry will be deleted by this function, and you should not attempt to
     * use the instance afterwards.
     * If the command is not maintained by this model, it will still be deleted!
     */
    void removeCommand(CommandInfo* command);
    Q_SIGNAL void commandRemoved(CommandInfo* command);

    /**
     * Automatically fill the model with known commands for the specified version
     * of digitail.
     * @param version The version for the tail we've been connected to
     */
    void autofill(const QString& version);
    void setRunning(const QString& command, bool isRunning);

    QString tailVersion() const;
    Q_SIGNAL void tailVersionChanged();

    /**
     * Get the command at a specified index
     *
     * @param index The index of the command to fetch
     * @return A command if the index was valid, or null if not
     */
    Q_INVOKABLE TailCommandModel::CommandInfo* getCommand(int index) const;
    /**
     * Get the command with the specified actual command
     *
     * @param command The command to fetch information for
     * @return The command info instance for the specified command, or null if none was found
     */
    Q_INVOKABLE TailCommandModel::CommandInfo* getCommand(QString command) const;
    /**
     * Get a random command, picked from the currently available commands, limited
     * to commands with the category listed in includedCategories. If the list is
     * empty, any command will be listed.
     *
     * @param includedCategories A list of strings matching the categories
     * @return A random command matching one of the requested categories
     */
    Q_INVOKABLE TailCommandModel::CommandInfo* getRandomCommand(QStringList includedCategories) const;

    /**
     * Get all the commands in this model
     *
     * @return A list of all commands currently managed by this model
     */
    const TailCommandModel::CommandInfoList& allCommands() const;
private:
    class Private;
    Private* d;
};

#endif//TAILCOMMANDMODEL_H
