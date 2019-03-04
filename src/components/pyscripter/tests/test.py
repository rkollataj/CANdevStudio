from PythonQt.private import QCanBusFrame

def outTypes():
    return  [
                {
                    "id" : "rawframe",
                    "name" : "RAW"
                }
            ]

def inTypes():
    return [
                {
                    "id" : "rawframe",
                    "name" : "RAW",
                    "callback": "inPort1Cbk"
                },
                {
                    "id" : "rawfddrame",
                    "name" : "DD",
                    "callback": "inPort2Cbk"
                }
            ]

def inPort1Cbk(frame, dir, status):
    out.send(frame, 1)
    print("Port 1:", frame.frameId())

def inPort2Cbk(canrawdata):
    print("Port 2:")


