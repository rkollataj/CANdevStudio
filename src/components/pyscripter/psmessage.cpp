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

PsMessage PsMessage::fromSignal(uint32_t id, const std::string& name, double value, int32_t dir)
{
    int32_t padding_4B = 0;

    PsMessage msg;
    msg.insert(PsMessageType::SIGNAL);
    msg.insert(id);
    msg.insert(dir);
    msg.insert(padding_4B);
    msg.insert(value);
    msg.insert(name.data(), name.length());

    return msg;
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

bool PsMessage::toFrame(uint32_t& id, std::vector<uint8_t>& payload, std::string& dir)
{
    bool ret = false;

    PsMessageType msgType = static_cast<PsMessageType>(reinterpret_cast<int32_t*>(_data.data())[0]);

    if (msgType == PsMessageType::FRAME) {
        id = reinterpret_cast<uint32_t*>(_data.data())[1];
        dir = dirToStr(reinterpret_cast<int32_t*>(_data.data())[2]);

        payload.insert(payload.end(), _data.data() + 12, _data.data() + _data.size());
        ret = true;
    }

    return ret;
}

bool PsMessage::toSignal(uint32_t& id, std::string& name, double& value, std::string& dir)
{
    bool ret = false;

    PsMessageType msgType = static_cast<PsMessageType>(reinterpret_cast<int32_t*>(_data.data())[0]);

    if (msgType == PsMessageType::SIGNAL) {
        id = reinterpret_cast<uint32_t*>(_data.data())[1];
        dir = dirToStr(reinterpret_cast<int32_t*>(_data.data())[2]);
        value = reinterpret_cast<double*>(_data.data())[2];

        name.insert(name.end(), _data.data() + 24, _data.data() + _data.size());
        ret = true;
    }

    return ret;
}

std::string PsMessage::dirToStr(int32_t dir)
{
    std::string ret;

    switch (dir) {
    case Direction::RX:
        ret = "RX";
        break;

    case Direction::TX:
        ret = "TX";
        break;

    default:
        ret = "Unknown";
        break;
    }

    return ret;
}
