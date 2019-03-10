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

std::shared_ptr<NodeData> PyScripterModel::outData(PortIndex)
{
    // example
    // return std::make_shared<CanRawData>(_frame, _direction, _status);

    return {};
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

void PyScripterModel::send(const QCanBusFrame& frame, int ndx) {}
