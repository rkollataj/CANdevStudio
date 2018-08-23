#include "cansignaldata_p.h"
#include <QJsonArray>
#include <dbcparser.h>
#include <fstream>
#include <log.h>

CanSignalDataPrivate::CanSignalDataPrivate(CanSignalData* q, CanSignalDataCtx&& ctx)
    : _ctx(std::move(ctx))
    , _ui(_ctx.get<CanSignalDataGuiInt>())
    , _columnsOrder({ "id", "name", "start", "length", "type", "order", "factor", "offset", "min", "max" })
    , _columnsSettings({ "id", "name", "dlc", "ecu", "cycle", "initial value" })
    , q_ptr(q)
{
    initProps();

    _tvModelSettings.setHorizontalHeaderLabels(_columnsSettings);
    _tvModel.setHorizontalHeaderLabels(_columnsOrder);
    _tvModelFilter.setSourceModel(&_tvModel);
    _ui.initTableView(_tvModelFilter);
    _ui.initSearch(_tvModelFilter);

    _tvModelSettingsFilter.setSourceModel(&_tvModelSettings);
    _ui.initSearch(_tvModelSettingsFilter);

    _ui.setSettingsCbk([this] {
        _settings = !_settings;

        if (_settings) {
            _ui.initSettings(_tvModelSettingsFilter);
        } else {
            _ui.initTableView(_tvModelFilter);
        }
    });

    _ui.setDockUndockCbk([this] {
        _docked = !_docked;
        emit q_ptr->mainWidgetDockToggled(_ui.mainWidget());
    });

    _ui.setSettingsUpdatedCbk([this] { setMsgSettings(getMsgSettings()); });
}

void CanSignalDataPrivate::initProps()
{
    for (const auto& p : _supportedProps) {
        _props[p.first];
    }
}

ComponentInterface::ComponentProperties CanSignalDataPrivate::getSupportedProperties() const
{
    return _supportedProps;
}

CanSignalDataPrivate::msgSettings_t CanSignalDataPrivate::getMsgSettings()
{
    msgSettings_t msgSettings;

    for (int i = 0; i < _tvModelSettings.rowCount(); ++i) {
        uint32_t id = _tvModelSettings.item(i, 0)->data(Qt::DisplayRole).toString().toUInt(nullptr, 16);
        auto cyclePtr = _tvModelSettings.item(i, 4);
        auto initValPtr = _tvModelSettings.item(i, 5);
        QVariant cycle;
        QVariant initVal;

        if (cyclePtr) {
            cycle = cyclePtr->data(Qt::DisplayRole);
        }

        if (initValPtr) {
            initVal = initValPtr->data(Qt::DisplayRole);
        }

        if (cycle.isValid() || initVal.isValid()) {
            msgSettings[id] = std::make_pair(cycle.toString(), initVal.toString());
        }
    }

    return msgSettings;
}

void CanSignalDataPrivate::setMsgSettings(const msgSettings_t& msgSettings)
{
    // Clear current settings
    for (std::pair<CANmessage, std::vector<CANsignal>>&& msg : _messages) {
        msg.first.updateCycle = 0;
        msg.first.initValue = "";
    }

    for (const auto& msg : msgSettings) {
        auto msgDb = findInDb(msg.first);
        if (msgDb) {
            msgDb->first.updateCycle = msg.second.first.toUInt();
            msgDb->first.initValue = msg.second.second.toStdString();
        }
    }

    emit q_ptr->canDbUpdated(_messages);
}

QJsonObject CanSignalDataPrivate::getSettings()
{
    QJsonObject json;

    for (const auto& p : _props) {
        json[p.first] = QJsonValue::fromVariant(p.second);
    }

    QJsonArray array;

    for (const auto& msg : getMsgSettings()) {
        QJsonObject obj;
        obj["id"] = QString::number(msg.first, 16);
        obj["cycle"] = msg.second.first;
        obj["initVal"] = msg.second.second;

        array.append(obj);
    }

    json["msgSettings"] = array;

    return json;
}

void CanSignalDataPrivate::setSettings(const QJsonObject& json)
{
    for (const auto& p : _supportedProps) {
        if (json.contains(p.first))
            _props[p.first] = json[p.first].toVariant();
    }

    msgSettings_t msgSettings;
    if (json.contains("msgSettings")) {
        if (json["msgSettings"].type() == QJsonValue::Array) {
            auto rowArray = json["msgSettings"].toArray();

            for (int i = 0; i < rowArray.size(); ++i) {
                if (rowArray[i].type() == QJsonValue::Object) {
                    auto row = rowArray[i].toObject();
                    QString id, cycle, initVal;

                    if (row.contains("id")) {
                        id = row["id"].toString();
                    }

                    if (row.contains("cycle")) {
                        cycle = row["cycle"].toString();
                    }

                    if (row.contains("initVal")) {
                        initVal = row["initVal"].toString();
                    }

                    if (id.length() && (cycle.length() || initVal.length())) {
                        msgSettings[id.toUInt(nullptr, 16)] = std::make_pair(cycle, initVal);
                    }
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

    setMsgSettings(msgSettings);
}

std::string CanSignalDataPrivate::loadFile(const std::string& filename)
{
    const std::string path = filename;

    std::fstream file{ path.c_str() };

    if (!file.good()) {
        cds_error("File {} does not exists", filename);
    }

    std::string buff;
    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(buff));

    file.close();
    return buff;
}

void CanSignalDataPrivate::loadDbc(const std::string& filename)
{
    if(_currentDbcFile == filename) {
        cds_info("DBC filename not changed. Loading aborted");
        return;
    }

    CANdb::DBCParser parser;
    bool success = parser.parse(loadFile(filename));

    if (!success) {
        cds_error("Failed to load CAN DB from '{}' file", filename);
        return;
    }

    _messages = parser.getDb().messages;

    cds_info("CAN DB load successful. {} records found", _messages.size());

    _tvModel.removeRows(0, _tvModel.rowCount());
    _tvModelSettings.removeRows(0, _tvModelSettings.rowCount());

    emit q_ptr->canDbUpdated(_messages);

    for (auto& message : _messages) {
        QList<QStandardItem*> settingsList;
        uint32_t id = message.first.id;
        QString frameID = QString("0x" + QString::number(id, 16));

        settingsList.append(new QStandardItem(frameID));
        settingsList.append(new QStandardItem(message.first.name.c_str()));
        settingsList.append(new QStandardItem(QString::number(message.first.dlc)));
        settingsList.append(new QStandardItem(message.first.ecu.c_str()));

        for (auto& i : settingsList) {
            i->setEditable(false);
        }

        settingsList.append(new QStandardItem(QString::number(message.first.updateCycle)));
        settingsList.append(new QStandardItem(message.first.initValue.c_str()));

        _tvModelSettings.appendRow(settingsList);

        for (auto& signal : message.second) {
            QList<QStandardItem*> list;

            list.append(new QStandardItem(frameID));
            list.append(new QStandardItem(signal.signal_name.c_str()));
            list.append(new QStandardItem(QString::number(signal.startBit)));
            list.append(new QStandardItem(QString::number(signal.signalSize)));
            list.append(new QStandardItem(signal.valueSigned ? "signed" : "unsigned"));
            list.append(new QStandardItem(QString::number(signal.byteOrder)));
            list.append(new QStandardItem(QString::number(signal.factor)));
            list.append(new QStandardItem(QString::number(signal.offset)));
            list.append(new QStandardItem(QString::number(signal.min)));
            list.append(new QStandardItem(QString::number(signal.max)));

            for (auto& i : list) {
                i->setEditable(false);
            }

            _tvModel.appendRow(list);
        }
    }
}

std::pair<CANmessage, std::vector<CANsignal>>* CanSignalDataPrivate::findInDb(uint32_t id)
{
    for (auto& msg : _messages) {
        if (msg.first.id == id) {
            // TODO: find a better way to do this...
            return (std::pair<CANmessage, std::vector<CANsignal>>*)&msg;
        }
    }
    return nullptr;
}
