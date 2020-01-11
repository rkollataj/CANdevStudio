#include "psmessage.h"
#include <QCanBusFrame>

PsMessageType PsMessage::type()
{
    return _msgType;
}

std::vector<uint8_t> PsMessage::toArray()
{
    return _data;
}

PsMessage PsMessage::fromFrame(const QCanBusFrame& frame)
{
    PsMessage msg;
    msg.insert(PsMessageType::FRAME_MESSAGE);
    msg.insert(frame.frameId());
    msg.insert(frame.payload().data(), frame.payload().size());

    return msg;
}

bool PsMessage::toFrame(uint32_t& id, std::vector<uint8_t>& payload)
{
    bool ret = false;

    PsMessageType msgType = static_cast<PsMessageType>(reinterpret_cast<int32_t*>(_data.data())[0]);

    if (msgType == PsMessageType::FRAME_MESSAGE) {
        id = reinterpret_cast<uint32_t*>(_data.data())[1];
        payload.insert(payload.end(), _data.data()[8], _data.size() - 8);
        ret = true;
    }

    return ret;
}

PsMessage PsMessage::fromData(const std::vector<uint8_t>& data)
{
    PsMessage msg;
    msg.insert(data.data(), data.size());

    msg._msgType = static_cast<PsMessageType>(reinterpret_cast<int32_t*>(msg._data.data())[0]);

    return msg;
}

PsMessage PsMessage::createCloseMessage()
{
    PsMessage msg;
    msg.insert(PsMessageType::CLOSE_MESSAGE);

    return msg;
}

