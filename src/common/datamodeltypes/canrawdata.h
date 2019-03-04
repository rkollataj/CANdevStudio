#ifndef __CANRAWDATA_H
#define __CANRAWDATA_H

#include "cdsdatabase.h"
#include <QCanBusFrame>
#include <QObject>
#include <nodes/NodeDataModel>

using QtNodes::NodeDataType;

/**
 *   @brief The enum class describing frame direction
 */
enum class Direction { RX, TX, Uninitialized };

Q_DECLARE_METATYPE(QCanBusFrame);
Q_DECLARE_METATYPE(Direction);
Q_DECLARE_METATYPE(QtNodes::NodeDataType);

/**
 *   @brief The class describing data model used as output for CanDevice node
 */
class CanRawData : public CdsDataBase {
public:
    CanRawData(){};
    CanRawData(QCanBusFrame const& frame, Direction const direction = Direction::TX, bool status = true)
        : _frame(frame)
        , _direction(direction)
        , _status(status)
    {
    }

    CanRawData(const QVariantList& list)
    {
        fromVariant(list);
    }

    /**
     *   @brief  Used to get data type id and displayed text for ports
     *   @return NodeDataType of rawview
     */
    NodeDataType type() const override
    {
        return NodeDataType{ "rawframe", "RAW" };
    }

    /**
     *   @brief  Used to get frame
     */
    QCanBusFrame frame() const
    {
        return _frame;
    };

    /**
     *   @brief  Used to get direction
     */
    Direction direction() const
    {
        return _direction;
    };

    /**
     *   @brief  Used to get status
     */
    bool status() const
    {
        return _status;
    };

    QVariantList toVariant() override
    {
        QVariantList list;
        QVariant vf;
        QVariant vd;

        vf.setValue(_frame);
        vd.setValue(_direction);

        list << vf;
        list << vd;
        list << _status;

        return list;
    }

    void fromVariant(const QVariantList& list) override
    {
        if (list.size() == 3) {
            _frame = list[0].value<QCanBusFrame>();
            _direction = list[1].value<Direction>();
            _status = list[2].value<bool>();
        }
    }

private:
    QCanBusFrame _frame;
    Direction _direction;
    bool _status; // used only for frameSent, ignored for frameReceived
};

class CanRawDataDecorator : public QObject {
    Q_OBJECT

public slots:
    NodeDataType* static_CanRawData_type()
    {
        return new NodeDataType(CanRawData().type());
    }
};
#endif /* !__CANRAWDATA_H */
