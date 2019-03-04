#ifndef __OUTHANDLER_H
#define __OUTHANDLER_H

#include <QObject>

class QCanBusFrame;

class OutHandler : public QObject {
    Q_OBJECT

signals:
    void send(const QCanBusFrame& frame, int ndx);
};

#endif /* !__OUTHANDLER_H */
