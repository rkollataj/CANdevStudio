#!/usr/bin/python3
from PySide2.QtWidgets import QApplication
from PySide2.QtCore import QTimer

# Initilize PySide2 application
app = QApplication(sys.argv)

# Create and initialize CANdevStudio internal communication module
# Note that this component does not exist outside of CANdevStudio
cdsComm = CdsComm()
cdsComm.init();

# Setup timers
def sendFrame1():
    fid = 0x100
    fp = [0x1, 0x2, 0x3]
    cdsComm.sndFrame(fid, fp)

def sendFrame2():
    fid = 0x12345
    fp = [0, 1, 2, 3, 4, 5, 6, 7]
    cdsComm.sndFrame(fid, fp)

def sendSignal1():
    # Signals loaded from:
    # 3rdParty/CANdb/tests/dbc/opendbc/tesla_can.dbc
    fid = 0x45
    sigName = "SpdCtrlLvr_Stat"
    sigVal = 0xf
    cdsComm.sndSignal(fid, sigName, sigVal)

def sendSignal2():
    # Signals loaded from:
    # 3rdParty/CANdb/tests/dbc/opendbc/tesla_can.dbc
    fid = 0x101
    sigName = "GTW_epasControlCounter"
    sigVal = 3
    cdsComm.sndSignal(fid, sigName, sigVal)

timer1 = QTimer()
timer1.timeout.connect(sendFrame1)
timer1.start(100)

timer2 = QTimer()
timer2.timeout.connect(sendFrame2)
timer2.start(200)

timer3 = QTimer()
timer3.timeout.connect(sendSignal1)
timer3.start(300)

timer4 = QTimer()
timer4.timeout.connect(sendSignal2)
timer4.start(200)

# Run PySide2 application
app.exec_()
