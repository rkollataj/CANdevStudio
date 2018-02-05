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
        _rowsUser.reserve(_maxSize);
        _rowsStr.reserve(_maxSize);
        _uniqueRows.reserve(_maxSize);
        _uniqueRowsTlb.resize(_maxSize, -1);
        _uniqueTlbNdx = 0;
    }

    ~CRVItemModel() {}

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
    {
        // cds_debug("");
        (void)parent;

        return createIndex(row, column);
    }

    virtual QModelIndex parent(const QModelIndex& child) const override
    {
        // cds_debug("");
        (void)child;

        return {};
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        // cds_debug("Rows count {}", _rowsUser.size());
        (void)parent;

        if (_isUnique) {
            return _uniqueRows.size();
        } else {
            return _rowsUser.size();
        }
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        // cds_debug("");
        (void)parent;

        return _columnHeaderItems.size();
    }

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        // cds_debug("row {}, col {}, role {}", index.row(), index.column(), role);

        uint32_t rowId;

        if (_isUnique) {
            rowId = _uniqueRowsTlb[_uniqueRows[index.row()]];
        } else {
            rowId = (index.row() + _rowShift) % _maxSize;
        }

        switch (role) {
        case Qt::DisplayRole:
            return getRowStr(rowId, index.column());

        case Qt::UserRole:
            return getRowUser(rowId, index.column());
        }

        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
    {
        // cds_debug("");
        if ((orientation == Qt::Horizontal) && (_columnHeaderItems.size() > (uint32_t)section)) {
            return _columnHeaderItems[section]->data(role);
        } else {
            cds_warn("Not supported call, section {}, orient {}, role {}", section, orientation, role);
        }

        return {};
    }

    void setHorizontalHeaderLabels(const QStringList& labels)
    {
        // cds_debug("");
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

    void clear()
    {
        beginResetModel();

        _rowsStr.clear();
        _rowsUser.clear();
        _uniqueMap.clear();
        _uniqueRows.clear();
        _uniqueRowsTlb.clear();
        _uniqueRowsTlb.resize(_maxSize, -1);
        _rowShift = 0;
        _uniqueTlbNdx = 0;

        endResetModel();
    }

    void toggleFilter()
    {
        _isUnique = !_isUnique;

        beginResetModel();
        endResetModel();
    }

    uint32_t findTlbNdx()
    {
        for (uint32_t ndx = 0; ndx < _uniqueRowsTlb.size(); ++ndx) {
            // -1 means that TLB slot is unused
            if (_uniqueRowsTlb[ndx] == -1) {
                _uniqueTlbNdx = ndx;
                return _uniqueTlbNdx;
            }
        }

        cds_error("Failed to find free TLB. Clearing view");
        clear();

        return 0;
    }

    void updateTlbVal(uint32_t id, uint32_t val)
    {
        uint32_t tlbNdx = 0;
        if (!_uniqueMap.count(id)) {
            tlbNdx = findTlbNdx();
            _uniqueMap[id] = tlbNdx;
            _uniqueRows.push_back(tlbNdx);
        } else {
            tlbNdx = _uniqueMap[id];
        }

        _uniqueRowsTlb[tlbNdx] = val;
    }

    void removeUniqueRow(uint32_t val)
    {
        for (uint32_t i = 0; i < _uniqueRows.size(); ++i) {
            if (_uniqueRows[i] == val) {
                _uniqueRows.erase(_uniqueRows.begin() + i);
                // Mark TLB entry empty
                _uniqueRowsTlb[val] = -1;
                cds_warn("Clear TLB {}", val);
                return;
            }
        }

        cds_error("Unique element not found. Clearing view");
        clear();
    }

    template <typename... Args> void appendRow(Args... args)
    {
        // cds_debug("");
        std::tuple<Args...> items(args...);

        if (_rowsStr.size() >= _maxSize) {

            // cds_info("Removing row");

            beginRemoveRows({}, 0, 0);

            if (_rowShift == _maxSize) {
                _rowShift = 0;
            }

            uint32_t id = strToUser<2>(std::get<2>(items));
            uint32_t idOld = std::get<2>(_rowsUser[_rowShift]);

            // we are about to overwrite a row that contains unique frame.
            // Remove reference completely to avoid displaying wrong data.
            // We don't have to remove element if newId == idOld
            if ((id != idOld) && (_uniqueRowsTlb[_uniqueMap[idOld]] == _rowShift)) {
                removeUniqueRow(_uniqueMap[idOld]);
                _uniqueMap.erase(idOld);
            }

            updateTlbVal(id, _rowShift);

            // clang-format off
            _rowsStr[_rowShift] = {
                    strToArray<0>(std::get<0>(items)),
                    strToArray<1>(std::get<1>(items)),
                    strToArray<2>(std::get<2>(items)),
                    strToArray<3>(std::get<3>(items)),
                    strToArray<4>(std::get<4>(items)),
                    strToArray<5>(std::get<5>(items)) };

            _rowsUser[_rowShift] = {
                    strToUser<0>(std::get<0>(items)),
                    strToUser<1>(std::get<1>(items)),
                    id,
                    strToUser<3>(std::get<3>(items)),
                    strToUser<4>(std::get<4>(items)),
                    strToUser<5>(std::get<5>(items)) };
            // clang-format on

            ++_rowShift;

            endRemoveRows();

            beginInsertRows({}, 0, 0);
            endInsertRows();
        } else {
            beginInsertRows({}, _rowsStr.size(), _rowsStr.size());

            uint32_t id = strToUser<2>(std::get<2>(items));

            updateTlbVal(id, _rowsStr.size());

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
                    id,
                    strToUser<3>(std::get<3>(items)),
                    strToUser<4>(std::get<4>(items)),
                    strToUser<5>(std::get<5>(items)) });
            // clang-format on

            endInsertRows();
        }
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
        switch (N) {
        case 0:
        case 4:
            return str.toUInt();

        case 1:
            return str.toFloat();

        case 2:
            return str.toUInt(nullptr, 16);
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
    bool _isUnique{ false };
    const uint32_t _maxSize;
    std::vector<QStandardItem*> _columnHeaderItems;
    // row, time, id, dir (TX or RX + \0), dlc, data (8*2 + 7 + \0)
    std::vector<RowsUserTuple_t> _rowsUser;
    std::vector<RowsStrTuple_t> _rowsStr;
    std::map<uint32_t, uint32_t> _uniqueMap;
    std::vector<uint32_t> _uniqueRows;
    std::vector<int32_t> _uniqueRowsTlb;
    uint32_t _rowShift{ 0 };
    uint32_t _uniqueTlbNdx{ 0 };
};

#endif /* !__CRVITEMMODEL_H */
