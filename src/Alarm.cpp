#include "Alarm.h"

class Alarm::Private
{
public:
    Private()
    {}

    Private(const QString &name)
        : name(name)
    {}

    QString name;
    QDateTime time;

    //TODO: It would be great to represent commands as objects, not strings
    QStringList commands;
};

Alarm::Alarm(QObject *parent)
    : QObject(parent),
      d(new Private())
{
}

Alarm::Alarm(const QString &name, QObject *parent)
    : QObject(parent),
      d(new Private(name))
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

void Alarm::setName(const QString &name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged();
    }
}

QDateTime Alarm::time() const
{
    return d->time;
}

void Alarm::setTime(const QDateTime &time)
{
    if (d->time != time) {
        d->time = time;
        emit timeChanged();
    }
}

QStringList Alarm::commands() const
{
    return d->commands;
}

void Alarm::setCommands(const QStringList &commands)
{
    d->commands = commands;
    emit commandsChanged();
}
