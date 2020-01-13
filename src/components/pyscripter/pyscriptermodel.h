#ifndef PYSCRIPTERMODEL_H
#define PYSCRIPTERMODEL_H

#include "componentmodel.h"
#include "nodepainter.h"
#include <QtCore/QObject>
#include <pyscripter.h>
#include <readerwriterqueue.h>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::PortIndex;
using QtNodes::PortType;

enum Direction;
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
    void rcvFrame(const QCanBusFrame& frame);

signals:
    void requestRedraw();
    void sndFrame(const QCanBusFrame& frame, Direction const direction, bool status);

private:
    std::unique_ptr<NodePainter> _painter;
    const QString _rawType{ "RAW" };
    const QString _signalType{ "SIG" };
    // 127 to use 4 blocks, 512 bytes each
    moodycamel::ReaderWriterQueue<std::shared_ptr<NodeData>> _rxQueue{ 127 };
};

#endif // PYSCRIPTERMODEL_H
