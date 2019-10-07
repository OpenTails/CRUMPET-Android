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

class FilterProxyModel::Private {
public:
    Private() {}
    bool filterBoolean{false};
};

FilterProxyModel::FilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
}

FilterProxyModel::~FilterProxyModel()
{
    delete d;
}

void FilterProxyModel::setFilterString(const QString &string)
{
    this->setFilterFixedString(string);
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);
    emit filterStringChanged();
}

QString FilterProxyModel::filterString() const
{
    return filterRegExp().pattern();
}

void FilterProxyModel::setFilterBoolean(const bool& value)
{
    d->filterBoolean = value;
    emit filterBooleanChanged();
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
        return sourceModel()->data(index, filterRole()).toString().contains(filterRegExp());
    }
}
