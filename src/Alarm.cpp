#include "Alarm.h"

#include <QDebug>

class Alarm::Private
{
public:
    Private()
    {}

    Private(const QString& name)
        : name(name)
    {}

    Private(const QString& name, const QDateTime& time, const QStringList& commands)
        : name(name),
          time(time),
          commands(commands)
    {}

    QString name;
    QDateTime time;

    //TODO: It would be great to represent commands as objects, not strings
    QStringList commands;
};

Alarm::Alarm(QObject* parent)
    : QObject(parent),
      d(new Private())
{
}

Alarm::Alarm(const QString& name, QObject* parent)
    : QObject(parent),
      d(new Private(name))
{
}

Alarm::Alarm(const QString& name,
             const QDateTime& time,
             const QStringList& commands,
             QObject* parent)
    : QObject(parent),
      d(new Private(name, time, commands))
{
}

Alarm::~Alarm()
{
    delete d;
}

QString Alarm::name() const
{
    return d->name;
}

void Alarm::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged();
        emit save();
    }
}

QDateTime Alarm::time() const
{
    return d->time;
}

void Alarm::setTime(const QDateTime& time)
{
    if (d->time != time) {
        d->time = time;
        emit timeChanged();
        emit save();
    }
}

QStringList Alarm::commands() const
{
    return d->commands;
}

void Alarm::setCommands(const QStringList& commands)
{
    d->commands = commands;
    emit commandsChanged();
    emit save();
}

void Alarm::addCommand(int index, const QString& command)
{
    d->commands.insert(index, command);
    emit commandsChanged();
    emit save();
}

void Alarm::removeCommand(int index)
{
    if (index < 0 || index >= d->commands.size()) {
        qWarning() << QString("Unable to remvoe command from alarm '%1'. Index (%2) is out of the bounds.")
                      .arg(name())
                      .arg(index);
        return;
    }

    d->commands.removeAt(index);
    emit commandsChanged();
    emit save();
}

QVariantMap Alarm::toVariantMap() const
{
    QVariantMap result;

    result["name"] = name();
    result["time"] = time();
    result["commands"] = commands();

    return result;
}
