#!/usr/bin/python3

from PySide2.QtWidgets import QApplication

app = QApplication(sys.argv)

cdsComm = CdsComm()
cdsComm.init();

# Loopback setup
cdsComm.rcvFrame.connect(cdsComm.sndFrame)
cdsComm.rcvSignal.connect(cdsComm.sndSignal)
app.exec_()
