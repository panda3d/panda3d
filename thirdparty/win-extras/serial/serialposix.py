#!/usr/bin/env python
#Python Serial Port Extension for Win32, Linux, BSD, Jython
#module for serial IO for POSIX compatible systems, like Linux
#see __init__.py
#
#(C) 2001-2003 Chris Liechti <cliechti@gmx.net>
# this is distributed under a free software license, see license.txt
#
#parts based on code from Grant B. Edwards  <grante@visi.com>:
#  ftp://ftp.visi.com/users/grante/python/PosixSerial.py
# references: http://www.easysw.com/~mike/serial/serial.html

import sys, os, fcntl, termios, struct, select, errno
from serialutil import *

VERSION = "$Revision: 1.1 $".split()[1]     #extract CVS version

#Do check the Python version as some constants have moved.
if (sys.hexversion < 0x020100f0):
    import TERMIOS
else:
    TERMIOS = termios

if (sys.hexversion < 0x020200f0):
    import FCNTL
else:
    FCNTL = fcntl

#try to detect the os so that a device can be selected...
plat = sys.platform.lower()

if   plat[:5] == 'linux':    #Linux (confirmed)
    def device(port):
        return '/dev/ttyS%d' % port

elif plat == 'cygwin':       #cywin/win32 (confirmed)
    def device(port):
        return '/dev/com%d' % (port + 1)

elif plat     == 'openbsd3': #BSD (confirmed)
    def device(port):
        return '/dev/ttyp%d' % port

elif plat[:3] == 'bsd' or  \
     plat[:7] == 'freebsd' or \
     plat[:7] == 'openbsd' or \
     plat[:6] == 'darwin':   #BSD (confirmed for freebsd4: cuaa%d)
    def device(port):
        return '/dev/cuaa%d' % port

elif plat[:6] == 'netbsd':   #NetBSD 1.6 testing by Erk
    def device(port):
        return '/dev/dty%02d' % port

elif plat[:4] == 'irix':     #IRIX (not tested)
    def device(port):
        return '/dev/ttyf%d' % port

elif plat[:2] == 'hp':       #HP-UX (not tested)
    def device(port):
        return '/dev/tty%dp0' % (port+1)

elif plat[:5] == 'sunos':    #Solaris/SunOS (confirmed)
    def device(port):
        return '/dev/tty%c' % (ord('a')+port)
        
elif plat[:3] == 'aix':      #aix
    def device(port):
        return '/dev/tty%d' % (port)

else:
    #platform detection has failed...
    print """don't know how to number ttys on this system.
! Use an explicit path (eg /dev/ttyS1) or send this information to
! the author of this module:

sys.platform = %r
os.name = %r
serialposix.py version = %s

also add the device name of the serial port and where the
counting starts for the first serial port.
e.g. 'first serial port: /dev/ttyS0'
and with a bit luck you can get this module running...
""" % (sys.platform, os.name, VERSION)
    #no exception, just continue with a brave attempt to build a device name
    #even if the device name is not correct for the platform it has chances
    #to work using a string with the real device name as port paramter.
    def device(portum):
        return '/dev/ttyS%d' % portnum
    #~ raise Exception, "this module does not run on this platform, sorry."

#whats up with "aix", "beos", ....
#they should work, just need to know the device names.


#load some constants for later use.
#try to use values from TERMIOS, use defaults from linux otherwise
TIOCMGET  = hasattr(TERMIOS, 'TIOCMGET') and TERMIOS.TIOCMGET or 0x5415
TIOCMBIS  = hasattr(TERMIOS, 'TIOCMBIS') and TERMIOS.TIOCMBIS or 0x5416
TIOCMBIC  = hasattr(TERMIOS, 'TIOCMBIC') and TERMIOS.TIOCMBIC or 0x5417
TIOCMSET  = hasattr(TERMIOS, 'TIOCMSET') and TERMIOS.TIOCMSET or 0x5418

#TIOCM_LE = hasattr(TERMIOS, 'TIOCM_LE') and TERMIOS.TIOCM_LE or 0x001
TIOCM_DTR = hasattr(TERMIOS, 'TIOCM_DTR') and TERMIOS.TIOCM_DTR or 0x002
TIOCM_RTS = hasattr(TERMIOS, 'TIOCM_RTS') and TERMIOS.TIOCM_RTS or 0x004
#TIOCM_ST = hasattr(TERMIOS, 'TIOCM_ST') and TERMIOS.TIOCM_ST or 0x008
#TIOCM_SR = hasattr(TERMIOS, 'TIOCM_SR') and TERMIOS.TIOCM_SR or 0x010

TIOCM_CTS = hasattr(TERMIOS, 'TIOCM_CTS') and TERMIOS.TIOCM_CTS or 0x020
TIOCM_CAR = hasattr(TERMIOS, 'TIOCM_CAR') and TERMIOS.TIOCM_CAR or 0x040
TIOCM_RNG = hasattr(TERMIOS, 'TIOCM_RNG') and TERMIOS.TIOCM_RNG or 0x080
TIOCM_DSR = hasattr(TERMIOS, 'TIOCM_DSR') and TERMIOS.TIOCM_DSR or 0x100
TIOCM_CD  = hasattr(TERMIOS, 'TIOCM_CD') and TERMIOS.TIOCM_CD or TIOCM_CAR
TIOCM_RI  = hasattr(TERMIOS, 'TIOCM_RI') and TERMIOS.TIOCM_RI or TIOCM_RNG
#TIOCM_OUT1 = hasattr(TERMIOS, 'TIOCM_OUT1') and TERMIOS.TIOCM_OUT1 or 0x2000
#TIOCM_OUT2 = hasattr(TERMIOS, 'TIOCM_OUT2') and TERMIOS.TIOCM_OUT2 or 0x4000
TIOCINQ   = hasattr(TERMIOS, 'FIONREAD') and TERMIOS.FIONREAD or 0x541B

TIOCM_zero_str = struct.pack('I', 0)
TIOCM_RTS_str = struct.pack('I', TIOCM_RTS)
TIOCM_DTR_str = struct.pack('I', TIOCM_DTR)


class Serial(SerialBase):
    """Serial port class POSIX implementation. Serial port configuration is 
    done with termios and fcntl. Runs on Linux and many other Un*x like
    systems."""

    def open(self):
        """Open port with current settings. This may throw a SerialException
           if the port cannot be opened."""
        if self._port is None:
            raise SerialException("Port must be configured before it can be used.")
        self.fd = None
        #open
        try:
            self.fd = os.open(self.portstr, os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK)
        except Exception, msg:
            self.fd = None
            raise SerialException("Could not open port: %s" % msg)
        #~ fcntl.fcntl(self.fd, FCNTL.F_SETFL, 0)  #set blocking
        
        self._reconfigurePort()
        self._isOpen = True
        #~ self.flushInput()
        
        
    def _reconfigurePort(self):
        """Set commuication parameters on opened port."""
        if self.fd is None:
            raise SerialException("Can only operate on a valid port handle")
            
        vmin = vtime = 0                #timeout is done via select
        try:
            iflag, oflag, cflag, lflag, ispeed, ospeed, cc = termios.tcgetattr(self.fd)
        except termios.error, msg:      #if a port is nonexistent but has a /dev file, it'll fail here
            raise SerialException("Could not configure port: %s" % msg)
        #set up raw mode / no echo / binary
        cflag |=  (TERMIOS.CLOCAL|TERMIOS.CREAD)
        lflag &= ~(TERMIOS.ICANON|TERMIOS.ECHO|TERMIOS.ECHOE|TERMIOS.ECHOK|TERMIOS.ECHONL|
                     TERMIOS.ISIG|TERMIOS.IEXTEN) #|TERMIOS.ECHOPRT
        for flag in ('ECHOCTL', 'ECHOKE'): #netbsd workaround for Erk
            if hasattr(TERMIOS, flag):
                lflag &= ~getattr(TERMIOS, flag)
        
        oflag &= ~(TERMIOS.OPOST)
        iflag &= ~(TERMIOS.INLCR|TERMIOS.IGNCR|TERMIOS.ICRNL|TERMIOS.IGNBRK)
        if hasattr(TERMIOS, 'IUCLC'):
            iflag &= ~TERMIOS.IUCLC
        if hasattr(TERMIOS, 'PARMRK'):
            iflag &= ~TERMIOS.PARMRK
        
        #setup baudrate
        try:
            ispeed = ospeed = getattr(TERMIOS,'B%s' % (self._baudrate))
        except AttributeError:
            raise ValueError('Invalid baud rate: %r' % self._baudrate)
        #setup char len
        cflag &= ~TERMIOS.CSIZE
        if self._bytesize == 8:
            cflag |= TERMIOS.CS8
        elif self._bytesize == 7:
            cflag |= TERMIOS.CS7
        elif self._bytesize == 6:
            cflag |= TERMIOS.CS6
        elif self._bytesize == 5:
            cflag |= TERMIOS.CS5
        else:
            raise ValueError('Invalid char len: %r' % self._bytesize)
        #setup stopbits
        if self._stopbits == STOPBITS_ONE:
            cflag &= ~(TERMIOS.CSTOPB)
        elif self._stopbits == STOPBITS_TWO:
            cflag |=  (TERMIOS.CSTOPB)
        else:
            raise ValueError('Invalid stopit specification: %r' % self._stopbits)
        #setup parity
        iflag &= ~(TERMIOS.INPCK|TERMIOS.ISTRIP)
        if self._parity == PARITY_NONE:
            cflag &= ~(TERMIOS.PARENB|TERMIOS.PARODD)
        elif self._parity == PARITY_EVEN:
            cflag &= ~(TERMIOS.PARODD)
            cflag |=  (TERMIOS.PARENB)
        elif self._parity == PARITY_ODD:
            cflag |=  (TERMIOS.PARENB|TERMIOS.PARODD)
        else:
            raise ValueError('Invalid parity: %r' % self._parity)
        #setup flow control
        #xonxoff
        if hasattr(TERMIOS, 'IXANY'):
            if self._xonxoff:
                iflag |=  (TERMIOS.IXON|TERMIOS.IXOFF) #|TERMIOS.IXANY)
            else:
                iflag &= ~(TERMIOS.IXON|TERMIOS.IXOFF|TERMIOS.IXANY)
        else:
            if self._xonxoff:
                iflag |=  (TERMIOS.IXON|TERMIOS.IXOFF)
            else:
                iflag &= ~(TERMIOS.IXON|TERMIOS.IXOFF)
        #rtscts
        if hasattr(TERMIOS, 'CRTSCTS'):
            if self._rtscts:
                cflag |=  (TERMIOS.CRTSCTS)
            else:
                cflag &= ~(TERMIOS.CRTSCTS)
        elif hasattr(TERMIOS, 'CNEW_RTSCTS'):   #try it with alternate constant name
            if self._rtscts:
                cflag |=  (TERMIOS.CNEW_RTSCTS)
            else:
                cflag &= ~(TERMIOS.CNEW_RTSCTS)
        #XXX should there be a warning if setting up rtscts (and xonxoff etc) fails??
        
        #buffer
        #vmin "minimal number of characters to be read. = for non blocking"
        if vmin < 0 or vmin > 255:
            raise ValueError('Invalid vmin: %r ' % vmin)
        cc[TERMIOS.VMIN] = vmin
        #vtime
        if vtime < 0 or vtime > 255:
            raise ValueError('Invalid vtime: %r' % vtime)
        cc[TERMIOS.VTIME] = vtime
        #activate settings
        termios.tcsetattr(self.fd, TERMIOS.TCSANOW, [iflag, oflag, cflag, lflag, ispeed, ospeed, cc])

    def close(self):
        """Close port"""
        if self._isOpen:
            if self.fd is not None:
                os.close(self.fd)
                self.fd = None
            self._isOpen = False

    def makeDeviceName(self, port):
        return device(port)

    #  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

    def inWaiting(self):
        """Return the number of characters currently in the input buffer."""
        #~ s = fcntl.ioctl(self.fd, TERMIOS.FIONREAD, TIOCM_zero_str)
        s = fcntl.ioctl(self.fd, TIOCINQ, TIOCM_zero_str)
        return struct.unpack('I',s)[0]

    def read(self, size=1):
        """Read size bytes from the serial port. If a timeout is set it may
           return less characters as requested. With no timeout it will block
           until the requested number of bytes is read."""
        if self.fd is None: raise portNotOpenError
        read = ''
        inp = None
        if size > 0:
            while len(read) < size:
                #print "\tread(): size",size, "have", len(read)    #debug
                ready,_,_ = select.select([self.fd],[],[], self._timeout)
                if not ready:
                    break   #timeout
                buf = os.read(self.fd, size-len(read))
                read = read + buf
                if self._timeout >= 0 and not buf:
                    break  #early abort on timeout
        return read

    def write(self, data):
        """Output the given string over the serial port."""
        if self.fd is None: raise portNotOpenError
        t = len(data)
        d = data
        while t > 0:
            try:
                if self._writeTimeout is not None and self._writeTimeout > 0:
                    _,ready,_ = select.select([],[self.fd],[], self._writeTimeout)
                    if not ready:
                        raise writeTimeoutError
                n = os.write(self.fd, d)
                if self._writeTimeout is not None and self._writeTimeout > 0:
                    _,ready,_ = select.select([],[self.fd],[], self._writeTimeout)
                    if not ready:
                        raise writeTimeoutError
                d = d[n:]
                t = t - n
            except OSError,v:
                if v.errno != errno.EAGAIN:
                    raise

    def flush(self):
        """Flush of file like objects. In this case, wait until all data
           is written."""
        self.drainOutput()

    def flushInput(self):
        """Clear input buffer, discarding all that is in the buffer."""
        if self.fd is None:
            raise portNotOpenError
        termios.tcflush(self.fd, TERMIOS.TCIFLUSH)

    def flushOutput(self):
        """Clear output buffer, aborting the current output and
        discarding all that is in the buffer."""
        if self.fd is None:
            raise portNotOpenError
        termios.tcflush(self.fd, TERMIOS.TCOFLUSH)

    def sendBreak(self):
        """Send break condition."""
        if self.fd is None:
            raise portNotOpenError
        termios.tcsendbreak(self.fd, 0)

    def setRTS(self,on=1):
        """Set terminal status line: Request To Send"""
        if self.fd is None: raise portNotOpenError
        if on:
            fcntl.ioctl(self.fd, TIOCMBIS, TIOCM_RTS_str)
        else:
            fcntl.ioctl(self.fd, TIOCMBIC, TIOCM_RTS_str)

    def setDTR(self,on=1):
        """Set terminal status line: Data Terminal Ready"""
        if self.fd is None: raise portNotOpenError
        if on:
            fcntl.ioctl(self.fd, TIOCMBIS, TIOCM_DTR_str)
        else:
            fcntl.ioctl(self.fd, TIOCMBIC, TIOCM_DTR_str)

    def getCTS(self):
        """Read terminal status line: Clear To Send"""
        if self.fd is None: raise portNotOpenError
        s = fcntl.ioctl(self.fd, TIOCMGET, TIOCM_zero_str)
        return struct.unpack('I',s)[0] & TIOCM_CTS != 0

    def getDSR(self):
        """Read terminal status line: Data Set Ready"""
        if self.fd is None: raise portNotOpenError
        s = fcntl.ioctl(self.fd, TIOCMGET, TIOCM_zero_str)
        return struct.unpack('I',s)[0] & TIOCM_DSR != 0

    def getRI(self):
        """Read terminal status line: Ring Indicator"""
        if self.fd is None: raise portNotOpenError
        s = fcntl.ioctl(self.fd, TIOCMGET, TIOCM_zero_str)
        return struct.unpack('I',s)[0] & TIOCM_RI != 0

    def getCD(self):
        """Read terminal status line: Carrier Detect"""
        if self.fd is None: raise portNotOpenError
        s = fcntl.ioctl(self.fd, TIOCMGET, TIOCM_zero_str)
        return struct.unpack('I',s)[0] & TIOCM_CD != 0

    # - - platform specific - - - -

    def drainOutput(self):
        """internal - not portable!"""
        if self.fd is None: raise portNotOpenError
        termios.tcdrain(self.fd)

    def nonblocking(self):
        """internal - not portable!"""
        if self.fd is None:
            raise portNotOpenError
        fcntl.fcntl(self.fd, FCNTL.F_SETFL, FCNTL.O_NONBLOCK)

    def fileno(self):
        """For easier of the serial port instance with select.
           WARNING: this function is not portable to different platforms!"""
        if self.fd is None: raise portNotOpenError
        return self.fd

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
