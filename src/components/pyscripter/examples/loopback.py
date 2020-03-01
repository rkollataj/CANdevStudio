#!/usr/bin/python3
from PySide2.QtWidgets import QApplication

# Initilize PySide2 application
app = QApplication(sys.argv)

# Create and initialize CANdevStudio internal communication module
# Note that this component does not exist outside of CANdevStudio
cdsComm = CdsComm()

# Loopback setup
cdsComm.rcvFrame.connect(cdsComm.sndFrame)
cdsComm.rcvSignal.connect(cdsComm.sndSignal)

# Run PySide2 application
app.exec_()
