#include "cansignalsender_p.h"

CanSignalSenderPrivate::CanSignalSenderPrivate(CanSignalSender* q, CanSignalSenderCtx&& ctx)
    : _ctx(std::move(ctx))
    , _ui(_ctx.get<CanSignalSenderGuiInt>())
    , _tvColumns({ "Id", "Signal name", "Value", "Loop", "Interval", "" })
    , q_ptr(q)
{
    initProps();

    _tvModel.setHorizontalHeaderLabels(_tvColumns);
    _ui.initTv(_tvModel, _signalNames);

    _ui.setDockUndockCbk([this] {
        _docked = !_docked;
        emit q_ptr->mainWidgetDockToggled(_ui.mainWidget());
    });

    _ui.setAddCbk([this] { _ui.addRow(); });

    _ui.setRemoveCbk([this] {
        QModelIndexList IndexList = _ui.getSelectedRows();
        std::list<QModelIndex> tmp = IndexList.toStdList();

        tmp.sort(); // List must to be sorted and reversed because erasing started from last row
        tmp.reverse();

        for (QModelIndex n : tmp) {
            _tvModel.removeRow(n.row()); // Delete line from table view
            // TODO: check if works when the collums was sorted before
        }
    });

    _ui.setSendCbk([this](const QString& id, const QString& name, const QVariant& val, bool checked, uint32_t ival) {
        uint32_t idNum = id.toUInt(nullptr, 16);
        std::string sigName = fmt::format("0x{:03x}{}_{}", idNum, idNum > 0x7ff ? "x" : "", name.toStdString());

        if (checked) {
            // Timer was set. Disable it.
            if (_timerMap[sigName]) {
                _timerMap[sigName]->stop();
                _timerMap[sigName] = nullptr;
            } else {
                _timerMap[sigName] = std::make_unique<QTimer>();
                _timerMap[sigName]->setInterval(ival);

                connect(_timerMap[sigName].get(), &QTimer::timeout,
                    [sigName, val, this] { emit q_ptr->sendSignal(sigName.c_str(), val); });

                if (_simStarted) {
                    _timerMap[sigName]->start();
                }
            }
        } else {
            // Timer was set. Disable it.
            if (_timerMap[sigName]) {
                _timerMap[sigName]->stop();
                _timerMap[sigName] = nullptr;
            } else {
                // single shot
                emit q_ptr->sendSignal(sigName.c_str(), val);
            }
        }
    });
}

void CanSignalSenderPrivate::initProps()
{
    for (const auto& p : _supportedProps) {
        _props[p.first];
    }
}

ComponentInterface::ComponentProperties CanSignalSenderPrivate::getSupportedProperties() const
{
    return _supportedProps;
}

QJsonObject CanSignalSenderPrivate::getSettings()
{
    QJsonObject ret;

    ret["rows"] = _ui.getRows();

    return ret;
}

void CanSignalSenderPrivate::setSettings(const QJsonObject& json)
{
    if (json.contains("rows")) {
        if (json["rows"].type() == QJsonValue::Array) {
            auto rowArray = json["rows"].toArray();

            for (int i = 0; i < rowArray.size(); ++i) {
                if (rowArray[i].type() == QJsonValue::Object) {
                    auto row = rowArray[i].toObject();
                    QString id, sig, val;

                    if (row.contains("id")) {
                        id = row["id"].toString();
                    }

                    if (row.contains("sig")) {
                        sig = row["sig"].toString();
                    }

                    if (row.contains("val")) {
                        val = row["val"].toString();
                    }

                    _ui.addRow(id, sig, val);
                } else {
                    cds_warn("rows array element expected to be object!");
                }
            }
        } else {
            cds_warn("rows expected to be array!");
        }
    } else {
        cds_info("Rows to restore not found");
    }
}
