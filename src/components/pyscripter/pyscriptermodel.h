#ifndef PYSCRIPTERMODEL_H
#define PYSCRIPTERMODEL_H

#include "componentmodel.h"
#include "nodepainter.h"
#include <QtCore/QObject>
#include <pyscripter.h>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::PortIndex;
using QtNodes::PortType;

enum class Direction;
class QCanBusFrame;

class PyScripterModel : public ComponentModel<PyScripter, PyScripterModel> {
    Q_OBJECT

public:
    PyScripterModel();

    unsigned int nPorts(PortType portType) const override;
    NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
    std::shared_ptr<NodeData> outData(PortIndex port) override;
    void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override;
    QtNodes::NodePainterDelegate* painterDelegate() const override;

public slots:

signals:
    void requestRedraw();
    void frameReceived(const QCanBusFrame& frame, int portNdx);

private:
    std::unique_ptr<NodePainter> _painter;
};

#endif // PYSCRIPTERMODEL_H
