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
    QJsonArray t;

    if (portType == PortType::In) {
        t = _component.inTypes();
    } else if (portType == PortType::Out) {
        t = _component.outTypes();
    }

    return t.size();
}

NodeDataType PyScripterModel::dataType(PortType portType, PortIndex ndx) const
{
    QJsonArray t;

    if (portType == PortType::In) {
        t = _component.inTypes();
    } else if (portType == PortType::Out) {
        t = _component.outTypes();
    }

    QJsonObject o;
    if (ndx < t.size()) {
        auto v = t[ndx];

        if (v.isObject()) {
            o = v.toObject();
        } else {
            cds_error("QJsonObject expected");
        }
    } else {
        cds_error("Wrong size ndx {}, t {}", ndx, t.size());
    }

    return NodeDataType{ o["id"].toString(), o["name"].toString() };
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
    } else {
        cds_error("nodeData is NULL!");
    }
}

void PyScripterModel::send(const QCanBusFrame& frame, int ndx)
{
}
