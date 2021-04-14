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

#ifndef FILTERPROXYMODEL
#define FILTERPROXYMODEL

#include <QSortFilterProxyModel>

class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterString READ filterString WRITE setFilterString NOTIFY filterStringChanged)
    Q_PROPERTY(bool filterBoolean READ filterBoolean WRITE setFilterBoolean NOTIFY filterBooleanChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit FilterProxyModel(QObject* parent = nullptr);
    ~FilterProxyModel() override;

    void setFilterString(const QString &string);
    QString filterString() const;
    Q_SIGNAL void filterStringChanged();

    void setFilterBoolean(const bool &value);
    bool filterBoolean() const;
    Q_SIGNAL void filterBooleanChanged();

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    int count() const;
    Q_SIGNAL void countChanged();

    Q_INVOKABLE int sourceIndex( const int& filterIndex );
private:
    class Private;
    Private* d;
};

#endif//FILTERPROXYMODEL
