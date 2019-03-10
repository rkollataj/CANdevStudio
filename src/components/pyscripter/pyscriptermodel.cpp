#include "pyscriptermodel.h"
#include "pyscripterplugin.h"
#include <datamodeltypes/canrawdata.h>
#include <log.h>

PyScripterModel::PyScripterModel()
    : ComponentModel("PyScripter")
    , _painter(std::make_unique<NodePainter>(PyScripterPlugin::PluginType::sectionColor()))
{
    _label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    _label->setFixedSize(75, 25);
    _label->setAttribute(Qt::WA_TranslucentBackground);

    connect(this, &PyScripterModel::receive, &_component, &PyScripter::receive);
    connect(&_component, qOverload<const QVariant&>(&PyScripter::send), this,
        qOverload<const QVariant&>(&PyScripterModel::send));
    connect(&_component, qOverload<const QVariant&, int>(&PyScripter::send), this,
        qOverload<const QVariant&, int>(&PyScripterModel::send));
    connect(&_component, &PyScripter::scriptLoaded, this, &PyScripterModel::scriptLoaded);
}

QtNodes::NodePainterDelegate* PyScripterModel::painterDelegate() const
{
    return _painter.get();
}

unsigned int PyScripterModel::nPorts(PortType portType) const
{
    std::vector<QtNodes::NodeDataType> dt;

    if (portType == PortType::In) {
        dt = _component.inTypes();
    } else if (portType == PortType::Out) {
        dt = _component.outTypes();
    }

    return dt.size();
}

NodeDataType PyScripterModel::dataType(PortType portType, PortIndex ndx) const
{
    std::vector<QtNodes::NodeDataType> dt;

    if (portType == PortType::In) {
        dt = _component.inTypes();
    } else if (portType == PortType::Out) {
        dt = _component.outTypes();
    }

    if (ndx < static_cast<PortIndex>(dt.size())) {
        return dt[ndx];
    }

    cds_warn("Failed to get data type for port {}", ndx);
    return {};
}

std::shared_ptr<NodeData> PyScripterModel::outData(PortIndex port)
{
    std::shared_ptr<NodeData> ret;

    if (port < static_cast<int>(_queues.size())) {
        bool status = _queues[port]->try_dequeue(ret);

        if (!status) {
            cds_error("No data availalbe on rx queue");
        }

    } else {
        cds_error("Wrong port ndx {}, size {}", port, _queues.size());
    }

    return ret;
}

void PyScripterModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex ndx)
{
    if (nodeData) {
        auto d = std::dynamic_pointer_cast<CdsDataBase>(nodeData);

        if (d) {
            emit receive(d->toVariant(), ndx);
        } else {
            cds_error("Failed to convert nodeData to CanRawData");
        }
    }
}

void PyScripterModel::send(const QVariant& data)
{
    for (int i = 0; i < static_cast<int>(_queues.size()); ++i) {
        enqueueData(data, i);
    }
}

void PyScripterModel::send(const QVariant& data, int ndx)
{
    // Indexes in python are 1-based
    --ndx;
    enqueueData(data, ndx);
}

void PyScripterModel::enqueueData(const QVariant& data, int ndx)
{
    auto&& outTypes = _component.outTypes();

    if (ndx >= static_cast<int>(_queues.size())) {
        cds_error("Data sent to port that does not exist. Ndx {}, portCount {}", ndx, _queues.size());
        return;
    }

    if (CanRawData().type().id == outTypes[ndx].id) {
        if (data.canConvert<QCanBusFrame>()) {
            bool ret = _queues[ndx]->try_enqueue(std::make_shared<CanRawData>(data.value<QCanBusFrame>()));
            if (ret) {
                emit dataUpdated(ndx);
            } else {
                cds_error("Failed to enqueue data");
            }
        } else {
            cds_error("Cannot convert QVariant to QCanBusFrame");
        }
    } else {
        cds_error("Unsupported type");
    }
}

void PyScripterModel::scriptLoaded()
{
    auto&& outTypes = _component.outTypes();

    _queues.clear();

    // 127 to use 4 blocks, 512 bytes each
    _queues.resize(outTypes.size(), std::make_shared<queueType_t>(127));
}
