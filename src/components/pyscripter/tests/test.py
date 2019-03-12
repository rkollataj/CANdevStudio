from PythonQt.private import QCanBusFrame, PortTypes

def outTypes():
    return [(PortTypes.RawFrame, "OUT1")]

def inTypes():
    return [ (PortTypes.RawFrame, "inPort1Cbk", "P1"), (PortTypes.RawFrame, "inPort2Cbk") ]

def inPort1Cbk(frame, dir, status):
    out.send(frame, 1)
    print("Port 1:", frame.frameId())

def inPort2Cbk(frame, dir, status):
    out.send(frame, 1)
    print("Port 2:", frame.frameId())


