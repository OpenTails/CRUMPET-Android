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

#include "FilterProxyModel.h"

#include <QTimer>

class FilterProxyModel::Private {
public:
    Private() {
        updateTimer.setInterval(1);
        updateTimer.setSingleShot(true);
    }
    bool filterBoolean{false};
    QTimer updateTimer;
};

FilterProxyModel::FilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    connect(&d->updateTimer, &QTimer::timeout, this, [this](){ QTimer::singleShot(1, this, [this](){ Q_EMIT countChanged(); }); } );
    connect(this, &QAbstractItemModel::rowsInserted, this, [this](){ d->updateTimer.start(); });
    connect(this, &QAbstractItemModel::rowsRemoved, this, [this](){ d->updateTimer.start(); });
    connect(this, &QAbstractItemModel::dataChanged, this, [this](){ d->updateTimer.start(); });
    connect(this, &QAbstractItemModel::modelReset, this, [this](){ d->updateTimer.start(); });
    connect(this, &QSortFilterProxyModel::sourceModelChanged, this, [this](){ d->updateTimer.start(); });
}

FilterProxyModel::~FilterProxyModel()
{
    delete d;
}

void FilterProxyModel::setFilterString(const QString &string)
{
    this->setFilterFixedString(string);
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);
    Q_EMIT filterStringChanged();
}

QString FilterProxyModel::filterString() const
{
    return filterRegularExpression().pattern();
}

void FilterProxyModel::setFilterBoolean(const bool& value)
{
    d->filterBoolean = value;
    Q_EMIT filterBooleanChanged();
}

bool FilterProxyModel::filterBoolean() const
{
    return d->filterBoolean;
}

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (d->filterBoolean) {
        return sourceModel()->data(index, filterRole()).toBool();
    } else {
        return sourceModel()->data(index, filterRole()).toString().contains(filterRegularExpression());
    }
}

int FilterProxyModel::count() const
{
    return rowCount();
}

int FilterProxyModel::sourceIndex(const int& filterIndex)
{
    int mappedIndex{-1};
    QModelIndex ourIndex = index(filterIndex, 0);
    if (ourIndex.isValid() && sourceModel()) {
        mappedIndex = mapToSource(ourIndex).row();
    }
    return mappedIndex;
}
