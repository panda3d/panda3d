#!jython
#Python Serial Port Extension for Win32, Linux, BSD, Jython
#module for serial IO for Jython and JavaComm
#see __init__.py
#
#(C) 2002-2003 Chris Liechti <cliechti@gmx.net>
# this is distributed under a free software license, see license.txt

import javax.comm
from serialutil import *

VERSION = "$Revision: 1.1 $".split()[1]     #extract CVS version


def device(portnumber):
    """Turn a port number into a device name"""
    enum = javax.comm.CommPortIdentifier.getPortIdentifiers()
    ports = []
    while enum.hasMoreElements():
        el = enum.nextElement()
        if el.getPortType() == javax.comm.CommPortIdentifier.PORT_SERIAL:
            ports.append(el)
    return ports[portnumber].getName()

class Serial(SerialBase):
    """Serial port class, implemented with javax.comm and thus usable with
       jython and the appropriate java extension."""
    
    def open(self):
        """Open port with current settings. This may throw a SerialException
           if the port cannot be opened."""
        if self._port is None:
            raise SerialException("Port must be configured before it can be used.")
        if type(self._port) == type(''):      #strings are taken directly
            portId = javax.comm.CommPortIdentifier.getPortIdentifier(self._port)
        else:
            portId = javax.comm.CommPortIdentifier.getPortIdentifier(device(self._port))     #numbers are transformed to a comportid obj
        try:
            self.sPort = portId.open("python serial module", 10)
        except Exception, msg:
            self.sPort = None
            raise SerialException("Could not open port: %s" % msg)
        self._reconfigurePort()
        self._instream = self.sPort.getInputStream()
        self._outstream = self.sPort.getOutputStream()
        self._isOpen = True

    def _reconfigurePort(self):
        """Set commuication parameters on opened port."""
        if not self.sPort:
            raise SerialException("Can only operate on a valid port handle")
        
        self.sPort.enableReceiveTimeout(30)
        if self._bytesize == FIVEBITS:
            jdatabits = javax.comm.SerialPort.DATABITS_5
        elif self._bytesize == SIXBITS:
            jdatabits = javax.comm.SerialPort.DATABITS_6
        elif self._bytesize == SEVENBITS:
            jdatabits = javax.comm.SerialPort.DATABITS_7
        elif self._bytesize == EIGHTBITS:
            jdatabits = javax.comm.SerialPort.DATABITS_8
        else:
            raise ValueError("unsupported bytesize: %r" % self._bytesize)
        
        if self._stopbits == STOPBITS_ONE:
            jstopbits = javax.comm.SerialPort.STOPBITS_1
        elif stopbits == STOPBITS_ONE_HALVE:
            self._jstopbits = javax.comm.SerialPort.STOPBITS_1_5
        elif self._stopbits == STOPBITS_TWO:
            jstopbits = javax.comm.SerialPort.STOPBITS_2
        else:
            raise ValueError("unsupported number of stopbits: %r" % self._stopbits)

        if self._parity == PARITY_NONE:
            jparity = javax.comm.SerialPort.PARITY_NONE
        elif self._parity == PARITY_EVEN:
            jparity = javax.comm.SerialPort.PARITY_EVEN
        elif self._parity == PARITY_ODD:
            jparity = javax.comm.SerialPort.PARITY_ODD
        #~ elif self._parity == PARITY_MARK:
            #~ jparity = javax.comm.SerialPort.PARITY_MARK
        #~ elif self._parity == PARITY_SPACE:
            #~ jparity = javax.comm.SerialPort.PARITY_SPACE
        else:
            raise ValueError("unsupported parity type: %r" % self._parity)

        jflowin = jflowout = 0
        if self._rtscts:
            jflowin  |=  javax.comm.SerialPort.FLOWCONTROL_RTSCTS_IN
            jflowout |=  javax.comm.SerialPort.FLOWCONTROL_RTSCTS_OUT
        if self._xonxoff:
            jflowin  |=  javax.comm.SerialPort.FLOWCONTROL_XONXOFF_IN
            jflowout |=  javax.comm.SerialPort.FLOWCONTROL_XONXOFF_OUT
        
        self.sPort.setSerialPortParams(baudrate, jdatabits, jstopbits, jparity)
        self.sPort.setFlowControlMode(jflowin | jflowout)
        
        if self._timeout >= 0:
            self.sPort.enableReceiveTimeout(self._timeout*1000)
        else:
            self.sPort.disableReceiveTimeout()

    def close(self):
        """Close port"""
        if self._isOpen:
            if self.sPort:
                self._instream.close()
                self._outstream.close()
                self.sPort.close()
                self.sPort = None
            self._isOpen = False

    def makeDeviceName(self, port):
        return device(port)

    #  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

    def inWaiting(self):
        """Return the number of characters currently in the input buffer."""
        if not self.sPort: raise portNotOpenError
        return self._instream.available()

    def read(self, size=1):
        """Read size bytes from the serial port. If a timeout is set it may
           return less characters as requested. With no timeout it will block
           until the requested number of bytes is read."""
        if not self.sPort: raise portNotOpenError
        read = ''
        if size > 0:
            while len(read) < size:
                x = self._instream.read()
                if x == -1:
                    if self.timeout >= 0:
                        break
                else:
                    read = read + chr(x)
        return read

    def write(self, data):
        """Output the given string over the serial port."""
        if not self.sPort: raise portNotOpenError
        self._outstream.write(data)

    def flushInput(self):
        """Clear input buffer, discarding all that is in the buffer."""
        if not self.sPort: raise portNotOpenError
        self._instream.skip(self._instream.available())

    def flushOutput(self):
        """Clear output buffer, aborting the current output and
        discarding all that is in the buffer."""
        if not self.sPort: raise portNotOpenError
        self._outstream.flush()

    def sendBreak(self):
        """Send break condition."""
        if not self.sPort: raise portNotOpenError
        self.sPort.sendBreak()

    def setRTS(self,on=1):
        """Set terminal status line: Request To Send"""
        if not self.sPort: raise portNotOpenError
        self.sPort.setRTS(on)
        
    def setDTR(self,on=1):
        """Set terminal status line: Data Terminal Ready"""
        if not self.sPort: raise portNotOpenError
        self.sPort.setDTR(on)

    def getCTS(self):
        """Read terminal status line: Clear To Send"""
        if not self.sPort: raise portNotOpenError
        self.sPort.isCTS()

    def getDSR(self):
        """Read terminal status line: Data Set Ready"""
        if not self.sPort: raise portNotOpenError
        self.sPort.isDSR()

    def getRI(self):
        """Read terminal status line: Ring Indicator"""
        if not self.sPort: raise portNotOpenError
        self.sPort.isRI()

    def getCD(self):
        """Read terminal status line: Carrier Detect"""
        if not self.sPort: raise portNotOpenError
        self.sPort.isCD()



if __name__ == '__main__':
    s = Serial(0,
                 baudrate=19200,        #baudrate
                 bytesize=EIGHTBITS,    #number of databits
                 parity=PARITY_EVEN,    #enable parity checking
                 stopbits=STOPBITS_ONE, #number of stopbits
                 timeout=3,             #set a timeout value, None for waiting forever
                 xonxoff=0,             #enable software flow control
                 rtscts=0,              #enable RTS/CTS flow control
               )
    s.setRTS(1)
    s.setDTR(1)
    s.flushInput()
    s.flushOutput()
    s.write('hello')
    print repr(s.read(5))
    print s.inWaiting()
    del s



