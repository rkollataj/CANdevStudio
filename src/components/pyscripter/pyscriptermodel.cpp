#include "pyscriptermodel.h"
#include "pyscripterplugin.h"
#include <log.h>

namespace {

// clang-format off
const std::map<PortType, std::vector<NodeDataType>> portMappings = {
    { PortType::In, 
        {
            //{CanRawData{}.type() }
        }
    },
    { PortType::Out, 
        {
            //{CanRawData{}.type() }
        }
    }
};
// clang-format on

} // namespace

PyScripterModel::PyScripterModel()
    : ComponentModel("PyScripter")
    , _painter(std::make_unique<NodePainter>(PyScripterPlugin::PluginType::sectionColor()))
{
    _label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    _label->setFixedSize(75, 25);
    _label->setAttribute(Qt::WA_TranslucentBackground);
}

QtNodes::NodePainterDelegate* PyScripterModel::painterDelegate() const
{
    return _painter.get();
}

unsigned int PyScripterModel::nPorts(PortType portType) const
{
    return portMappings.at(portType).size();
}

NodeDataType PyScripterModel::dataType(PortType portType, PortIndex ndx) const
{
    if (portMappings.at(portType).size() > static_cast<uint32_t>(ndx)) {
        return portMappings.at(portType)[ndx];
    }

    cds_error("No port mapping for ndx: {}", ndx);
    return { };
}

std::shared_ptr<NodeData> PyScripterModel::outData(PortIndex)
{
    // example
    // return std::make_shared<CanRawData>(_frame, _direction, _status);

    return { };
}

void PyScripterModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex)
{
    // example
    // if (nodeData) {
    //     auto d = std::dynamic_pointer_cast<CanRawData>(nodeData);
    //     assert(nullptr != d);
    //     emit sendFrame(d->frame());
    // } else {
    //     cds_warn("Incorrect nodeData");
    // }
    (void) nodeData;
}
