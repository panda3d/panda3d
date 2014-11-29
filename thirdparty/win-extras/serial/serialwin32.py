#! python
#Python Serial Port Extension for Win32, Linux, BSD, Jython
#serial driver for win32
#see __init__.py
#
#(C) 2001-2003 Chris Liechti <cliechti@gmx.net>
# this is distributed under a free software license, see license.txt


import win32file  # The base COM port and file IO functions.
import win32event # We use events and the WaitFor[Single|Multiple]Objects functions.
import win32con   # constants.

from serialutil import *

VERSION = "$Revision: 1.2 $".split()[1]     #extract CVS version

#from winbase.h. these should realy be in win32con
MS_CTS_ON  = 16
MS_DSR_ON  = 32
MS_RING_ON = 64
MS_RLSD_ON = 128

def device(portnum):
    """Turn a port number into a device name"""
    #the "//./COMx" format is required for devices >= 9
    #not all versions of windows seem to support this propperly
    #so that the first few ports are used with the DOS device name
    if portnum < 9:
        return 'COM%d' % (portnum+1) #numbers are transformed to a string
    else:
        return r'\\.\COM%d' % (portnum+1)

class Serial(SerialBase):
    """Serial port implemenation for Win32. This implemenatation requires a 
       win32all installation."""

    BAUDRATES = (50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,
                 19200,38400,57600,115200)

    def open(self):
        """Open port with current settings. This may throw a SerialException
           if the port cannot be opened."""
        if self._port is None:
            raise SerialException("Port must be configured before it can be used.")
        self.hComPort = None
        try:
            self.hComPort = win32file.CreateFile(self.portstr,
                   win32con.GENERIC_READ | win32con.GENERIC_WRITE,
                   0, # exclusive access
                   None, # no security
                   win32con.OPEN_EXISTING,
                   win32con.FILE_ATTRIBUTE_NORMAL | win32con.FILE_FLAG_OVERLAPPED,
                   None)
        except Exception, msg:
            self.hComPort = None    #'cause __del__ is called anyway
            raise SerialException("could not open port: %s" % msg)
        # Setup a 4k buffer
        win32file.SetupComm(self.hComPort, 4096, 4096)

        #Save original timeout values:
        self._orgTimeouts = win32file.GetCommTimeouts(self.hComPort)

        self._rtsState = win32file.RTS_CONTROL_ENABLE
        self._dtrState = win32file.RTS_CONTROL_ENABLE

        self._reconfigurePort()
        
        # Clear buffers:
        # Remove anything that was there
        win32file.PurgeComm(self.hComPort,
                            win32file.PURGE_TXCLEAR | win32file.PURGE_TXABORT |
                            win32file.PURGE_RXCLEAR | win32file.PURGE_RXABORT)

        self._overlappedRead = win32file.OVERLAPPED()
        self._overlappedRead.hEvent = win32event.CreateEvent(None, 1, 0, None)
        self._overlappedWrite = win32file.OVERLAPPED()
        #~ self._overlappedWrite.hEvent = win32event.CreateEvent(None, 1, 0, None)
        self._overlappedWrite.hEvent = win32event.CreateEvent(None, 0, 0, None)
        self._isOpen = True

    def _reconfigurePort(self):
        """Set commuication parameters on opened port."""
        if not self.hComPort:
            raise SerialException("Can only operate on a valid port handle")
        
        #Set Windows timeout values
        #timeouts is a tuple with the following items:
        #(ReadIntervalTimeout,ReadTotalTimeoutMultiplier,
        # ReadTotalTimeoutConstant,WriteTotalTimeoutMultiplier,
        # WriteTotalTimeoutConstant)
        if self._timeout is None:
            timeouts = (0, 0, 0, 0, 0)
        elif self._timeout == 0:
            timeouts = (win32con.MAXDWORD, 0, 0, 0, 0)
        else:
            timeouts = (0, 0, int(self._timeout*1000), 0, 0)
        if self._writeTimeout is None:
            pass
        elif self._writeTimeout == 0:
            timeouts = timeouts[:-2] + (0, win32con.MAXDWORD)
        else:
            timeouts = timeouts[:-2] + (0, int(self._writeTimeout*1000))
        win32file.SetCommTimeouts(self.hComPort, timeouts)

        win32file.SetCommMask(self.hComPort, win32file.EV_ERR)

        # Setup the connection info.
        # Get state and modify it:
        comDCB = win32file.GetCommState(self.hComPort)
        comDCB.BaudRate = self._baudrate

        if self._bytesize == FIVEBITS:
            comDCB.ByteSize     = 5
        elif self._bytesize == SIXBITS:
            comDCB.ByteSize     = 6
        elif self._bytesize == SEVENBITS:
            comDCB.ByteSize     = 7
        elif self._bytesize == EIGHTBITS:
            comDCB.ByteSize     = 8
        else:
            raise ValueError("Unsupported number of data bits: %r" % self._bytesize)

        if self._parity == PARITY_NONE:
            comDCB.Parity       = win32file.NOPARITY
            comDCB.fParity      = 0 # Dis/Enable Parity Check
        elif self._parity == PARITY_EVEN:
            comDCB.Parity       = win32file.EVENPARITY
            comDCB.fParity      = 1 # Dis/Enable Parity Check
        elif self._parity == PARITY_ODD:
            comDCB.Parity       = win32file.ODDPARITY
            comDCB.fParity      = 1 # Dis/Enable Parity Check
        else:
            raise ValueError("Unsupported parity mode: %r" % self._parity)

        if self._stopbits == STOPBITS_ONE:
            comDCB.StopBits     = win32file.ONESTOPBIT
        elif self._stopbits == STOPBITS_TWO:
            comDCB.StopBits     = win32file.TWOSTOPBITS
        else:
            raise ValueError("Unsupported number of stop bits: %r" % self._stopbits)
            
        comDCB.fBinary          = 1 # Enable Binary Transmission
        # Char. w/ Parity-Err are replaced with 0xff (if fErrorChar is set to TRUE)
        if self._rtscts:
            comDCB.fRtsControl  = win32file.RTS_CONTROL_HANDSHAKE
        else:
            comDCB.fRtsControl  = self._rtsState
        if self._dsrdtr:
            comDCB.fDtrControl  = win32file.DTR_CONTROL_HANDSHAKE
        else:
            comDCB.fDtrControl  = self._dtrState
        comDCB.fOutxCtsFlow     = self._rtscts
        comDCB.fOutxDsrFlow     = self._dsrdtr
        comDCB.fOutX            = self._xonxoff
        comDCB.fInX             = self._xonxoff
        comDCB.fNull            = 0
        comDCB.fErrorChar       = 0
        comDCB.fAbortOnError    = 0
        comDCB.XonChar          = XON
        comDCB.XoffChar         = XOFF

        try:
            win32file.SetCommState(self.hComPort, comDCB)
        except win32file.error, e:
            raise ValueError("Cannot configure port, some setting was wrong. Original message: %s" % e)

    #~ def __del__(self):
        #~ self.close()

    def close(self):
        """Close port"""
        if self._isOpen:
            if self.hComPort:
                #Restore original timeout values:
                win32file.SetCommTimeouts(self.hComPort, self._orgTimeouts)
                #Close COM-Port:
                win32file.CloseHandle(self.hComPort)
                self.hComPort = None
            self._isOpen = False

    def makeDeviceName(self, port):
        return device(port)

    #  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
    
    def inWaiting(self):
        """Return the number of characters currently in the input buffer."""
        flags, comstat = win32file.ClearCommError(self.hComPort)
        return comstat.cbInQue

    def read(self, size=1):
        """Read size bytes from the serial port. If a timeout is set it may
           return less characters as requested. With no timeout it will block
           until the requested number of bytes is read."""
        if not self.hComPort: raise portNotOpenError
        if size > 0:
            win32event.ResetEvent(self._overlappedRead.hEvent)
            flags, comstat = win32file.ClearCommError(self.hComPort)
            if self.timeout == 0:
                n = min(comstat.cbInQue, size)
                if n > 0:
                    rc, buf = win32file.ReadFile(self.hComPort, win32file.AllocateReadBuffer(n), self._overlappedRead)
                    win32event.WaitForSingleObject(self._overlappedRead.hEvent, win32event.INFINITE)
                    read = str(buf)
                else:
                    read = ''
            else:
                rc, buf = win32file.ReadFile(self.hComPort, win32file.AllocateReadBuffer(size), self._overlappedRead)
                n = win32file.GetOverlappedResult(self.hComPort, self._overlappedRead, 1)
                read = str(buf[:n])
        else:
            read = ''
        return read

    def write(self, s):
        """Output the given string over the serial port."""
        if not self.hComPort: raise portNotOpenError
        #print repr(s),
        if s:
            #~ win32event.ResetEvent(self._overlappedWrite.hEvent)
            err, n = win32file.WriteFile(self.hComPort, s, self._overlappedWrite)
            if err: #will be ERROR_IO_PENDING:
                # Wait for the write to complete.
                #~ win32event.WaitForSingleObject(self._overlappedWrite.hEvent, win32event.INFINITE)
                n = win32file.GetOverlappedResult(self.hComPort, self._overlappedWrite, 1)
                if n != len(s):
                    raise writeTimeoutError
                

    def flushInput(self):
        """Clear input buffer, discarding all that is in the buffer."""
        if not self.hComPort: raise portNotOpenError
        win32file.PurgeComm(self.hComPort, win32file.PURGE_RXCLEAR | win32file.PURGE_RXABORT)

    def flushOutput(self):
        """Clear output buffer, aborting the current output and
        discarding all that is in the buffer."""
        if not self.hComPort: raise portNotOpenError
        win32file.PurgeComm(self.hComPort, win32file.PURGE_TXCLEAR | win32file.PURGE_TXABORT)

    def sendBreak(self):
        """Send break condition."""
        if not self.hComPort: raise portNotOpenError
        import time
        win32file.SetCommBreak(self.hComPort)
        #TODO: how to set the correct duration??
        time.sleep(0.020)
        win32file.ClearCommBreak(self.hComPort)

    def setRTS(self,level=1):
        """Set terminal status line: Request To Send"""
        if not self.hComPort: raise portNotOpenError
        if level:
            self._rtsState = win32file.RTS_CONTROL_ENABLE
            win32file.EscapeCommFunction(self.hComPort, win32file.SETRTS)
        else:
            self._rtsState = win32file.RTS_CONTROL_DISABLE
            win32file.EscapeCommFunction(self.hComPort, win32file.CLRRTS)

    def setDTR(self,level=1):
        """Set terminal status line: Data Terminal Ready"""
        if not self.hComPort: raise portNotOpenError
        if level:
            self._dtrState = win32file.DTR_CONTROL_ENABLE
            win32file.EscapeCommFunction(self.hComPort, win32file.SETDTR)
        else:
            self._dtrState = win32file.DTR_CONTROL_DISABLE
            win32file.EscapeCommFunction(self.hComPort, win32file.CLRDTR)

    def getCTS(self):
        """Read terminal status line: Clear To Send"""
        if not self.hComPort: raise portNotOpenError
        return MS_CTS_ON & win32file.GetCommModemStatus(self.hComPort) != 0

    def getDSR(self):
        """Read terminal status line: Data Set Ready"""
        if not self.hComPort: raise portNotOpenError
        return MS_DSR_ON & win32file.GetCommModemStatus(self.hComPort) != 0

    def getRI(self):
        """Read terminal status line: Ring Indicator"""
        if not self.hComPort: raise portNotOpenError
        return MS_RING_ON & win32file.GetCommModemStatus(self.hComPort) != 0

    def getCD(self):
        """Read terminal status line: Carrier Detect"""
        if not self.hComPort: raise portNotOpenError
        return MS_RLSD_ON & win32file.GetCommModemStatus(self.hComPort) != 0

    # - - platform specific - - - -

    def setXON(self, level=True):
        """Platform specific - set flow state."""
        if not self.hComPort: raise portNotOpenError
        if level:
            win32file.EscapeCommFunction(self.hComPort, win32file.SETXON)
        else:
            win32file.EscapeCommFunction(self.hComPort, win32file.SETXOFF)

#Nur Testfunktion!!
if __name__ == '__main__':
    print __name__
    s = Serial()
    print s
    
    s = Serial(0)
    print s
    
    s.baudrate = 19200
    s.databits = 7
    s.close()
    s.port = 3
    s.open()
    print s

