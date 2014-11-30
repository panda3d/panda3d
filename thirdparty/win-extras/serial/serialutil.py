#! python
#Python Serial Port Extension for Win32, Linux, BSD, Jython
#see __init__.py
#
#(C) 2001-2003 Chris Liechti <cliechti@gmx.net>
# this is distributed under a free software license, see license.txt

PARITY_NONE, PARITY_EVEN, PARITY_ODD = 'N', 'E', 'O'
STOPBITS_ONE, STOPBITS_TWO = (1, 2)
FIVEBITS, SIXBITS, SEVENBITS, EIGHTBITS = (5,6,7,8)

PARITY_NAMES = {
    PARITY_NONE: 'None',
    PARITY_EVEN: 'Even',
    PARITY_ODD:  'Odd',
}

XON  = chr(17)
XOFF = chr(19)

#Python < 2.2.3 compatibility
try:
    True
except:
    True = 1
    False = not True

class SerialException(Exception):
    """Base class for serial port related exceptions."""

portNotOpenError = SerialException('Port not open')

class SerialTimeoutException(SerialException):
    """Write timeouts give an exception"""

writeTimeoutError = SerialTimeoutException("Write timeout")

class FileLike(object):
    """An abstract file like class.
    
    This class implements readline and readlines based on read and
    writelines based on write.
    This class is used to provide the above functions for to Serial
    port objects.
    
    Note that when the serial port was opened with _NO_ timeout that
    readline blocks until it sees a newline (or the specified size is
    reached) and that readlines would never return and therefore
    refuses to work (it raises an exception in this case)!
    """

    def read(self, size): raise NotImplementedError
    def write(self, s): raise NotImplementedError

    def readline(self, size=None, eol='\n'):
        """read a line which is terminated with end-of-line (eol) character
        ('\n' by default) or until timeout"""
        line = ''
        while 1:
            c = self.read(1)
            if c:
                line += c   #not very efficient but lines are usually not that long
                if c == eol:
                    break
                if size is not None and len(line) >= size:
                    break
            else:
                break
        return line

    def readlines(self, sizehint=None, eol='\n'):
        """read a list of lines, until timeout
        sizehint is ignored"""
        if self.timeout is None:
            raise ValueError, "Serial port MUST have enabled timeout for this function!"
        lines = []
        while 1:
            line = self.readline(eol=eol)
            if line:
                lines.append(line)
                if line[-1] != eol:    #was the line received with a timeout?
                    break
            else:
                break
        return lines

    def xreadlines(self, sizehint=None):
        """just call readlines - here for compatibility"""
        return self.readlines()

    def writelines(self, sequence):
        for line in sequence:
            self.write(line)

    def flush(self):
        """flush of file like objects"""
        pass

class SerialBase(FileLike):
    """Serial port base class. Provides __init__ function and properties to
       get/set port settings."""
    
    #default values, may be overriden in subclasses that do not support all values
    BAUDRATES = (50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,
                 19200,38400,57600,115200,230400,460800,500000,576000,921600,
                 1000000,1152000,1500000,2000000,2500000,3000000,3500000,4000000)
    BYTESIZES = (FIVEBITS, SIXBITS, SEVENBITS, EIGHTBITS)
    PARITIES  = (PARITY_NONE, PARITY_EVEN, PARITY_ODD)
    STOPBITS  = (STOPBITS_ONE, STOPBITS_TWO)
    
    def __init__(self,
                 port = None,           #number of device, numbering starts at
                                        #zero. if everything fails, the user
                                        #can specify a device string, note
                                        #that this isn't portable anymore
                                        #port will be opened if one is specified
                 baudrate=9600,         #baudrate
                 bytesize=EIGHTBITS,    #number of databits
                 parity=PARITY_NONE,    #enable parity checking
                 stopbits=STOPBITS_ONE, #number of stopbits
                 timeout=None,          #set a timeout value, None to wait forever
                 xonxoff=0,             #enable software flow control
                 rtscts=0,              #enable RTS/CTS flow control
                 writeTimeout=None,     #set a timeout for writes
                 dsrdtr=None,           #None: use rtscts setting, dsrdtr override if true or false
                 ):
        """Initialize comm port object. If a port is given, then the port will be
           opened immediately. Otherwise a Serial port object in closed state
           is returned."""

        self._isOpen   = False
        self._port     = None           #correct value is assigned below trough properties
        self._baudrate = None           #correct value is assigned below trough properties
        self._bytesize = None           #correct value is assigned below trough properties
        self._parity   = None           #correct value is assigned below trough properties
        self._stopbits = None           #correct value is assigned below trough properties
        self._timeout  = None           #correct value is assigned below trough properties
        self._writeTimeout  = None           #correct value is assigned below trough properties
        self._xonxoff  = None           #correct value is assigned below trough properties
        self._rtscts   = None           #correct value is assigned below trough properties
        self._dsrdtr   = None           #correct value is assigned below trough properties
        
        #assign values using get/set methods using the properties feature
        self.port     = port
        self.baudrate = baudrate
        self.bytesize = bytesize
        self.parity   = parity
        self.stopbits = stopbits
        self.timeout  = timeout
        self.writeTimeout = writeTimeout
        self.xonxoff  = xonxoff
        self.rtscts   = rtscts
        self.dsrdtr   = dsrdtr
        
        if port is not None:
            self.open()

    def isOpen(self):
        """Check if the port is opened."""
        return self._isOpen

    #  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

    #TODO: these are not realy needed as the is the BAUDRATES etc attribute...
    #maybe i remove them before the final release...
    
    def getSupportedBaudrates(self):
        return [(str(b), b) for b in self.BAUDRATES]

    def getSupportedByteSizes(self):
        return [(str(b), b) for b in self.BYTESIZES]
    
    def getSupportedStopbits(self):
        return [(str(b), b) for b in self.STOPBITS]

    def getSupportedParities(self):
        return [(PARITY_NAMES[b], b) for b in self.PARITIES]

    #  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

    def setPort(self, port):
        """Change the port. The attribute portstr is set to a string that
           contains the name of the port."""
        
        was_open = self._isOpen
        if was_open: self.close()
        if port is not None:
            if type(port) in [type(''), type(u'')]:       #strings are taken directly
                self.portstr = port
            else:
                self.portstr = self.makeDeviceName(port)
        else:
            self.portstr = None
        self._port = port
        if was_open: self.open()
    
    def getPort(self):
        """Get the current port setting. The value that was passed on init or using
           setPort() is passed back. See also the attribute portstr which contains
           the name of the port as a string."""
        return self._port

    port = property(getPort, setPort, doc="Port setting")


    def setBaudrate(self, baudrate):
        """Change baudrate. It raises a ValueError if the port is open and the
        baudrate is not possible. If the port is closed, then tha value is
        accepted and the exception is raised when the port is opened."""
        #~ if baudrate not in self.BAUDRATES: raise ValueError("Not a valid baudrate: %r" % baudrate)
        try:
            self._baudrate = int(baudrate)
        except TypeError:
            raise ValueError("Not a valid baudrate: %r" % baudrate)
        else:
            if self._isOpen:  self._reconfigurePort()
        
    def getBaudrate(self):
        """Get the current baudrate setting."""
        return self._baudrate
        
    baudrate = property(getBaudrate, setBaudrate, doc="Baudrate setting")


    def setByteSize(self, bytesize):
        """Change byte size."""
        if bytesize not in self.BYTESIZES: raise ValueError("Not a valid byte size: %r" % bytesize)
        self._bytesize = bytesize
        if self._isOpen: self._reconfigurePort()
    
    def getByteSize(self):
        """Get the current byte size setting."""
        return self._bytesize
    
    bytesize = property(getByteSize, setByteSize, doc="Byte size setting")


    def setParity(self, parity):
        """Change parity setting."""
        if parity not in self.PARITIES: raise ValueError("Not a valid parity: %r" % parity)
        self._parity = parity
        if self._isOpen: self._reconfigurePort()
    
    def getParity(self):
        """Get the current parity setting."""
        return self._parity
    
    parity = property(getParity, setParity, doc="Parity setting")


    def setStopbits(self, stopbits):
        """Change stopbits size."""
        if stopbits not in self.STOPBITS: raise ValueError("Not a valid stopbit size: %r" % stopbits)
        self._stopbits = stopbits
        if self._isOpen: self._reconfigurePort()
    
    def getStopbits(self):
        """Get the current stopbits setting."""
        return self._stopbits
    
    stopbits = property(getStopbits, setStopbits, doc="Stopbits setting")


    def setTimeout(self, timeout):
        """Change timeout setting."""
        if timeout is not None:
            if timeout < 0: raise ValueError("Not a valid timeout: %r" % timeout)
            try:
                timeout + 1     #test if it's a number, will throw a TypeError if not...
            except TypeError:
                raise ValueError("Not a valid timeout: %r" % timeout)
        
        self._timeout = timeout
        if self._isOpen: self._reconfigurePort()
    
    def getTimeout(self):
        """Get the current timeout setting."""
        return self._timeout
    
    timeout = property(getTimeout, setTimeout, doc="Timeout setting for read()")


    def setWriteTimeout(self, timeout):
        """Change timeout setting."""
        if timeout is not None:
            if timeout < 0: raise ValueError("Not a valid timeout: %r" % timeout)
            try:
                timeout + 1     #test if it's a number, will throw a TypeError if not...
            except TypeError:
                raise ValueError("Not a valid timeout: %r" % timeout)
        
        self._writeTimeout = timeout
        if self._isOpen: self._reconfigurePort()
    
    def getWriteTimeout(self):
        """Get the current timeout setting."""
        return self._writeTimeout
    
    writeTimeout = property(getWriteTimeout, setWriteTimeout, doc="Timeout setting for write()")


    def setXonXoff(self, xonxoff):
        """Change XonXoff setting."""
        self._xonxoff = xonxoff
        if self._isOpen: self._reconfigurePort()
    
    def getXonXoff(self):
        """Get the current XonXoff setting."""
        return self._xonxoff
    
    xonxoff = property(getXonXoff, setXonXoff, doc="Xon/Xoff setting")

    def setRtsCts(self, rtscts):
        """Change RtsCts flow control setting."""
        self._rtscts = rtscts
        if self._isOpen: self._reconfigurePort()
    
    def getRtsCts(self):
        """Get the current RtsCts flow control setting."""
        return self._rtscts
    
    rtscts = property(getRtsCts, setRtsCts, doc="RTS/CTS flow control setting")

    def setDsrDtr(self, dsrdtr=None):
        """Change DsrDtr flow control setting."""
        if dsrdtr is None:
            #if not set, keep backwards compatibility and follow rtscts setting
            self._dsrdtr = self._rtscts
        else:
            #if defined independently, follow its value
            self._dsrdtr = dsrdtr
        if self._isOpen: self._reconfigurePort()
    
    def getDsrDtr(self):
        """Get the current DsrDtr flow control setting."""
        return self._dsrdtr
    
    dsrdtr = property(getDsrDtr, setDsrDtr, "DSR/DTR flow control setting")
    
    #  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

    def __repr__(self):
        """String representation of the current port settings and its state."""
        return "%s<id=0x%x, open=%s>(port=%r, baudrate=%r, bytesize=%r, parity=%r, stopbits=%r, timeout=%r, xonxoff=%r, rtscts=%r, dsrdtr=%r)" % (
            self.__class__.__name__,
            id(self),
            self._isOpen,
            self.portstr,
            self.baudrate,
            self.bytesize,
            self.parity,
            self.stopbits,
            self.timeout,
            self.xonxoff,
            self.rtscts,
            self.dsrdtr,
        )

if __name__ == '__main__':
    s = SerialBase()
    print s.portstr
    print s.getSupportedBaudrates()
    print s.getSupportedByteSizes()
    print s.getSupportedParities()
    print s.getSupportedStopbits()
    print s
