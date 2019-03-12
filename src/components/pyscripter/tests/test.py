from PythonQt.private import QCanBusFrame, PortTypes

def outTypes():
    return [(PortTypes.RawFrame, "OUT1")]

def inTypes():
    return [ (PortTypes.RawFrame, "inPort1Cbk", "P1"), (PortTypes.RawFrame, "inPort2Cbk") ]

def timerTypes():
    return [ (100, "timer1"), (200, "timer2") ]

def timer1():
    frame = QCanBusFrame()
    frame.setFrameId(0x123)
    out.send(frame, 1)

def timer2():
    frame = QCanBusFrame()
    frame.setFrameId(0x234)
    out.send(frame, 1)

def inPort1Cbk(frame, dir, status):
    out.send(frame, 1)
    print("Port 1:", frame.frameId())

def inPort2Cbk(frame, dir, status):
    out.send(frame, 1)
    print("Port 2:", frame.frameId())


