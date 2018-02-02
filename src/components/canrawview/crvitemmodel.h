#ifndef __CRVITEMMODEL_H
#define __CRVITEMMODEL_H

#include <QAbstractItemModel>
#include <QLabel>

template <class T> struct type_list {
    template <std::size_t N> using type = typename std::tuple_element<N, T>::type;
};

class CRVItemModel : public QAbstractItemModel {
    Q_OBJECT

public:
    CRVItemModel(uint32_t maxSize)
        : _maxSize(maxSize)
    {
        _rowsUser.reserve(maxSize);
        _rowsStr.reserve(maxSize);
    }

    ~CRVItemModel() {}

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
    {
        //cds_debug("");
        (void)parent;

        return createIndex(row, column);
    }

    virtual QModelIndex parent(const QModelIndex& child) const override
    {
        //cds_debug("");
        (void)child;

        return {};
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        //cds_debug("Rows count {}", _rowsUser.size());
        (void)parent;

        return _rowsUser.size();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        //cds_debug("");
        (void)parent;

        return _columnHeaderItems.size();
    }

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        //cds_debug("row {}, col {}, role {}", index.row(), index.column(), role);

        switch (role) {
        case Qt::DisplayRole:
            return getRowStr(index.row(), index.column());

        case Qt::UserRole:
            return getRowUser(index.row(), index.column());
        }

        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
    {
        //cds_debug("");
        if ((orientation == Qt::Horizontal) && (_columnHeaderItems.size() > (uint32_t)section)) {
            return _columnHeaderItems[section]->data(role);
        } else {
            cds_warn("Not supported call, section {}, orient {}, role {}", section, orientation, role);
        }

        return {};
    }

    void setHorizontalHeaderLabels(const QStringList& labels)
    {
        //cds_debug("");
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

    template <typename... Args> void appendRow(Args... args)
    {
        //cds_debug("");
        std::tuple<Args...> items(args...);

        beginInsertRows({}, 0, 0);

        // clang-format off
        _rowsStr.push_back({
                strToArray<0>(std::get<0>(items)),
                strToArray<1>(std::get<1>(items)),
                strToArray<2>(std::get<2>(items)),
                strToArray<3>(std::get<3>(items)),
                strToArray<4>(std::get<4>(items)),
                strToArray<5>(std::get<5>(items)) });

        _rowsUser.push_back({
                strToUser<0>(std::get<0>(items)),
                strToUser<1>(std::get<1>(items)),
                strToUser<2>(std::get<2>(items)),
                strToUser<3>(std::get<3>(items)),
                strToUser<4>(std::get<4>(items)),
                strToUser<5>(std::get<5>(items)) });
        // clang-format on

        endInsertRows();
    }

private:
    using RowsStrTuple_t = std::tuple<std::array<char, 11>, // rowID
        std::array<char, 10>, // time
        std::array<char, 12>, // id
        std::array<char, 3>, // dir
        std::array<char, 2>, // dlc
        std::array<char, 24>>; // data

    using RowsUserTuple_t = std::tuple<uint32_t, float, uint32_t, QVariant, uint8_t, QVariant>;

    template <int N> type_list<RowsStrTuple_t>::type<N> strToArray(const QString& str)
    {
        type_list<RowsStrTuple_t>::type<N> array;
        auto&& bytes = str.toLatin1();

        std::memcpy(array.data(), bytes.data(), array.size() - 1);
        array[array.size() - 1] = '\0';

        return array;
    }

    template <int N> type_list<RowsUserTuple_t>::type<N> strToUser(const QString& str)
    {
        if (std::is_same<type_list<RowsUserTuple_t>::type<N>, uint32_t>::value) {
            return str.toUInt();
        } else if (std::is_same<type_list<RowsUserTuple_t>::type<N>, uint8_t>::value) {
            return str.toUInt();
        } else if (std::is_same<type_list<RowsUserTuple_t>::type<N>, float>::value) {
            return str.toFloat();
        }

        return {};
    }

    QVariant getRowStr(uint32_t rowNdx, uint32_t colNdx) const
    {
        if (_rowsStr.size() > rowNdx) {
            const auto& row = _rowsStr[rowNdx];

            switch (colNdx) {
            case 0:
                return QVariant(QString(std::get<0>(row).data()));

            case 1:
                return QVariant(QString(std::get<1>(row).data()));

            case 2:
                return QVariant(QString(std::get<2>(row).data()));

            case 3:
                return QVariant(QString(std::get<3>(row).data()));

            case 4:
                return QVariant(QString(std::get<4>(row).data()));

            case 5:
                return QVariant(QString(std::get<5>(row).data()));

            default:
                cds_error("Column index {} out of scope {}", _rowsStr.size(), colNdx);
            }
        } else {
            cds_error("Row index {} out of scope {}", _rowsStr.size(), rowNdx);
        }

        return {};
    }

    QVariant getRowUser(uint32_t rowNdx, uint32_t colNdx) const
    {
        if (_rowsUser.size() > rowNdx) {
            const auto& row = _rowsUser[rowNdx];

            switch (colNdx) {
            case 0:
                return QVariant(std::get<0>(row));

            case 1:
                return QVariant(std::get<1>(row));

            case 2:
                return QVariant(std::get<2>(row));

            case 3:
                return QVariant(std::get<3>(row));

            case 4:
                return QVariant(std::get<4>(row));

            case 5:
                return QVariant(std::get<5>(row));

            default:
                cds_error("Column index {} out of scope {}", _rowsUser.size(), colNdx);
            }
        } else {
            cds_error("Row index {} out of scope {}", _rowsUser.size(), rowNdx);
        }

        return {};
    }


private:
    const uint32_t _maxSize;
    std::vector<QStandardItem*> _columnHeaderItems;
    // row, time, id, dir (TX or RX + \0), dlc, data (8*2 + 7 + \0)
    std::vector<RowsUserTuple_t> _rowsUser;
    std::vector<RowsStrTuple_t> _rowsStr;
};

#endif /* !__CRVITEMMODEL_H */
