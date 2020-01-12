#ifndef __PSMESSAGE_H
#define __PSMESSAGE_H

#include <cstdint>
#include <vector>
#include <QString>

class QCanBusFrame;

enum PsMessageType { FRAME = 0, SIGNAL, CLOSE, UNKNOWN };

class PsMessage {
public:
    PsMessageType type();
    std::vector<uint8_t> toArray();
    bool toFrame(uint32_t& id, std::vector<uint8_t>& payload);

public:
    static PsMessage fromFrame(const QCanBusFrame& frame, const QString& dir);
    static PsMessage fromData(const std::vector<uint8_t>& data);
    static PsMessage createCloseMessage();

private:
    template <typename T> void insert(T el, std::size_t size = sizeof(T))
    {
        const uint8_t* chars;

        if (std::is_pointer<T>::value) {
            chars = reinterpret_cast<decltype(chars)>(el);
        } else {
            chars = reinterpret_cast<decltype(chars)>(&el);
        }

        _data.insert(_data.end(), chars, chars + size);
    }

private:
    PsMessageType _msgType{ PsMessageType::UNKNOWN };
    std::vector<uint8_t> _data;
};

#endif /* !__PSMESSAGE_H */
