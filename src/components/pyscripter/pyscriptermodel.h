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
    void send(const QVariant& data);
    void send(const QVariant& data, int ndx);
    void scriptLoaded();

signals:
    void requestRedraw();
    void receive(const QVariantList& list, int ndx);

private:
    void enqueueData(const QVariant &data, int ndx);

private:
    std::unique_ptr<NodePainter> _painter;
    using queueType_t = moodycamel::ReaderWriterQueue<std::shared_ptr<NodeData>>;
    std::vector<std::shared_ptr<queueType_t>> _queues;
};

#endif // PYSCRIPTERMODEL_H
