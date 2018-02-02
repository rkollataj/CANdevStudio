#ifndef __CRVITEMMODEL_H
#define __CRVITEMMODEL_H

#include <QAbstractItemModel>
#include <QLabel>

class CRVItemModel : public QAbstractItemModel {
    Q_OBJECT

public:
    CRVItemModel() {}
    ~CRVItemModel() {}

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
    {
        cds_debug("");
        (void)parent;

        return createIndex(row, column);
    }

    virtual QModelIndex parent(const QModelIndex& child) const override
    {
        cds_debug("");
        (void)child;

        return {};
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        cds_debug("Rows count {}", _rows.size());
        (void)parent;

        return _rows.size();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        cds_debug("");
        (void)parent;

        return _columnHeaderItems.size();
    }

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        cds_debug("row {}, col {}, role {}", index.row(), index.column(), role);
        
        switch(role) {
            case Qt::DisplayRole:
                return {"adasda"};

            case Qt::DecorationRole:
                break;
            case Qt::FontRole:
                break;
            case Qt::TextAlignmentRole:
                break;
            case Qt::BackgroundRole:
                break;
            case Qt::ForegroundRole:
                break;
            case Qt::CheckStateRole:
                break;
        }

        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
    {
        cds_debug("");
        if ((orientation == Qt::Horizontal) && (_columnHeaderItems.size() > (uint32_t)section)) {
            return _columnHeaderItems[section]->data(role);
        } else {
            cds_warn("Not supported call, section {}, orient {}, role {}", section, orientation, role);
        }

        return {};
    }

    void setHorizontalHeaderLabels(const QStringList& labels)
    {
        cds_debug("");
        if (columnCount() < labels.count()) {
            for (int i = columnCount(); i < labels.count(); ++i) {
                _columnHeaderItems.push_back(new QStandardItem());
            }
        }

        for (int i = 0; i < labels.count(); ++i) {
            QStandardItem* item = _columnHeaderItems[i];
            item->setText(labels.at(i));
            emit headerDataChanged(Qt::Horizontal, i, i);
        }
    }

    template <typename... Args>
    void appendRow(Args... items)
    {
        cds_debug("");
        std::tuple<Args...> row(items...);

        beginInsertRows({}, 0, 0);
        _rows.push_back({});
        endInsertRows();
    }

private:
    std::vector<QStandardItem*> _columnHeaderItems;
    // row, time, id, dir (TX or RX + \0), dlc, data (8*2 + 7 + \0) 
    std::vector<std::tuple<uint32_t, float, uint32_t, std::array<char,3>, uint8_t, std::array<char, 24>>> _rows;
};

#endif /* !__CRVITEMMODEL_H */
