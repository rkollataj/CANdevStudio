#ifndef __PSMESSAGE_H
#define __PSMESSAGE_H

#include <cstdint>
#include <string>
#include <vector>

class QCanBusFrame;

enum PsMessageType { FRAME = 0, SIGNAL, CLOSE, UNKNOWN };

class PsMessage {
public:
    PsMessageType type();
    std::vector<uint8_t> toArray();
    bool toFrame(uint32_t& id, std::vector<uint8_t>& payload, std::string& dir);
    bool toSignal(uint32_t& id, std::string& name, double& value, std::string& dir);

public:
    static PsMessage fromFrame(const QCanBusFrame& frame, int32_t dir);
    static PsMessage fromFrame(uint32_t id, std::vector<uint8_t>& payload, int32_t dir);
    static PsMessage fromSignal(uint32_t id, const std::string& name, double value, int32_t dir);
    static PsMessage fromData(const std::vector<uint8_t>& data);
    static PsMessage createCloseMessage();

private:
    template <typename T, std::enable_if_t<!std::is_pointer<T>::value, int> = 0>
    void insert(T el, std::size_t size = sizeof(T))
    {
        const uint8_t* chars = reinterpret_cast<decltype(chars)>(&el);

        _data.insert(_data.end(), chars, chars + size);
    }

    template <typename T, std::enable_if_t<std::is_pointer<T>::value, int> = 0>
    void insert(T el, std::size_t size = sizeof(T))
    {
        const uint8_t* chars = reinterpret_cast<decltype(chars)>(el);

        _data.insert(_data.end(), chars, chars + size);
    }

    std::string dirToStr(int32_t dir);

private:
    PsMessageType _msgType{ PsMessageType::UNKNOWN };
    std::vector<uint8_t> _data;
};

#endif /* !__PSMESSAGE_H */
