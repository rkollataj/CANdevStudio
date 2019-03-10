#ifndef __OUTHANDLER_H
#define __OUTHANDLER_H

#include <QObject>

class QCanBusFrame;

class OutHandler : public QObject {
    Q_OBJECT

signals:
    void send(const QVariant& data, int ndx);
    void send(const QVariant& data);
};

#endif /* !__OUTHANDLER_H */
