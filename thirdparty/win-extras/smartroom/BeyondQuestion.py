#! python
#
# Driver for Beyond Question Remote

__all__ = ["bqReadKeys"]

import re,sys,os

if (os.name != "nt"):
    raise "This Beyond Question driver only works under Win 2K/XP"
    

try:
    import win32file
    import win32event
    import win32con
except:
    raise "Cannot use Beyond Question driver until pywin32 has been installed"

import serial,_winreg

CODES = ["0","A","B","C","D","E","F","G","H","I","J",
        "check-ans","help-me",
        "arrow-up","arrow-left","arrow-right","arrow-down",
        "17","jump-left","jump-right","20",
        "log-on","22","23","24","25","26","27","28","29","30","31","32"]

SER = None
BUF = ""
OFFS = 0
PAT = re.compile("^R...:K.\r\n$", re.DOTALL)
LO = 0
HI = 0
MOD = 0

def listRegistryKeys(path):
    result=[]
    index=0
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ)
        while (1):
            result.append(_winreg.EnumKey(key, index))
            index = index + 1
    except: pass
    if (key!=0): _winreg.CloseKey(key)
    return result

def getRegistryKey(path, subkey):
    k1=0
    key=0
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ)
        k1, k2 = _winreg.QueryValueEx(key, subkey)
    except: pass
    if (key!=0): _winreg.CloseKey(key)
    return k1

def createPort():
    global MOD, LO, HI, SER

    # Locate the Beyond Question Application.

    key = "SOFTWARE\\Classes\\BeyondQuestion.AnswerFile\\DefaultIcon"
    iconfile = getRegistryKey(key, "")
    if (iconfile == 0):
        raise "The Beyond Question Executable is not installed properly."
    dir = os.path.dirname(iconfile)
    if (os.path.isdir(dir)==0):
        raise "The Beyond Question Executable is not installed properly."
    exe = os.path.join(dir, "Beyond Question.exe")
    ini = os.path.join(dir, "RF.ini")
    if (os.path.isfile(exe)==0):
        raise "The Beyond Question Executable is not installed properly."
    if (os.path.isfile(ini)==0):
        raise "Cannot find the Beyond Question RF.INI file in the program directory."

    # Read the Beyond Question INI file

    try:
        f = file(ini, "r")
        line1 = f.readline().rstrip("\r\n")
        line2 = f.readline().rstrip("\r\n")
        line3 = f.readline().rstrip("\r\n")
        MOD = int(line1)
        LO = int(line2)
        HI = int(line3)
        f.close()
    except:
        raise "Could not parse the Beyond Question RF.INI file."

    # Locate the appropriate USB COM device.

    remote = None
    key = "SYSTEM\\CurrentControlSet\\Services\usbser\Enum"
    for i in range(16):
        val = getRegistryKey(key, str(i))
        if (val != 0):
            if (val.startswith("USB\\Vid_0471&Pid_0888\\")):
                remote = val
    if (remote == None):
        raise "The Beyond Question USB driver or device does not seem to be installed."
    key = "SYSTEM\\CurrentControlSet\\Enum\\" + remote + "\\Device Parameters"
    port = getRegistryKey(key, "PortName")
    if (port == 0):
        raise "The Beyond Question USB driver or device does not seem to be installed."
    port = int(port[3:])-1

    # Open the serial port for reading.

    try:
        SER = serial.Serial(port=port, baudrate=38400, timeout=10)
    except:
        raise "Cannot open COM port for beyond question"

def bqReadKeys():
    global SER,BUF,OFFS,MOD,LO,HI
    result = []
    while 1:
        if (OFFS + 9 <= len(BUF)):
            pkt = BUF[OFFS:OFFS+9]
            if (PAT.match(pkt)):
                index = ord(pkt[1])+ord(pkt[2])*256+ord(pkt[3])*65536
                index = index >> 3
                button = ord(pkt[6])
                if (index >= LO) and (index <= HI):
                    index = (index % MOD) + 1
                    result.append((index, button, CODES[button]))
                OFFS += 9
            else:
                OFFS += 1
        else:
            n = SER.inWaiting()
            if (n == 0):
                return result
            BUF = BUF[OFFS:] + SER.read(n)
            OFFS = 0


createPort()
