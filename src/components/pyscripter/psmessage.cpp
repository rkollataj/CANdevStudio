#include "psmessage.h"
#include <QCanBusFrame>
#include <datamodeltypes/datadirection.h>

PsMessageType PsMessage::type()
{
    return _msgType;
}

std::vector<uint8_t> PsMessage::toArray()
{
    return _data;
}

PsMessage PsMessage::fromFrame(const QCanBusFrame& frame, int32_t dir)
{
    PsMessage msg;
    msg.insert(PsMessageType::FRAME);
    msg.insert(frame.frameId());
    msg.insert(dir);
    msg.insert(frame.payload().data(), frame.payload().size());

    return msg;
}

PsMessage PsMessage::fromFrame(uint32_t id, std::vector<uint8_t>& payload, int32_t dir)
{
    PsMessage msg;
    msg.insert(PsMessageType::FRAME);
    msg.insert(id);
    msg.insert(dir);
    msg.insert(payload.data(), payload.size());

    return msg;
}

bool PsMessage::toFrame(uint32_t& id, std::vector<uint8_t>& payload, std::string& dir)
{
    bool ret = false;

    PsMessageType msgType = static_cast<PsMessageType>(reinterpret_cast<int32_t*>(_data.data())[0]);

    if (msgType == PsMessageType::FRAME) {
        id = reinterpret_cast<uint32_t*>(_data.data())[1];
        int32_t dirInt = reinterpret_cast<int32_t*>(_data.data())[2];

        switch (dirInt) {
            case Direction::RX:
                dir = "RX";
                break;

            case Direction::TX:
                dir = "TX";
                break;

            default:
                dir = "Unknown";
                break;
        }

        payload.insert(payload.end(), _data.data() + 12, _data.data() + _data.size());
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
    msg.insert(PsMessageType::CLOSE);

    return msg;
}

