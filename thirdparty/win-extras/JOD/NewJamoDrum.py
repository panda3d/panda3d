"""
A generic Jam-o-Drum input interface for the Jam-o-Drum that uses the OptiPAC
for both spinners and pads.

@author: U{Ben Buchwald <bb2@alumni.cmu.edu>}
Last Updated: 2/27/2006
"""

from direct.showbase.DirectObject import DirectObject
import string, sys, md5
from pandac.PandaModules import Filename
from pandac.PandaModules import WindowProperties
from pandac.PandaModules import ConfigVariableList

class JamoDrum(DirectObject):
    """
    Class representing input from a Jam-o-Drum. To handle Jam-o-Drum input 
    accept the Panda messages JOD_SPIN_x and JOD_HIT_x where x is a number between
    0 and 3 for the 4 stations. Spin messages also pass a parameter which is the
    angle spun in degrees. Hit messages also pass a parameter which is the force
    the pad was hit with in the range 0.0-1.0 (will probably be fairly low). With
    or without actual Jam-o-Drum hardware this class will automatically respond
    to the keys (j,k,l),(s,d,f),(w,e,r), and (u,i,o) corresponding to spin left 10
    degrees, hit with full force, and spin right 10 degrees respectively for each
    of the stations. You must call L{poll} periodically to receive input from the
    real Jam-o-Drum hardware.
    """
    def __init__(self, useJOD=None):
        """
        @keyword useJOD: connected to actual drumpads and spinners to read from (default: read from config.prc)
        @type useJOD: bool
        """

        self.configPath = Filename("/c/jamoconfig.txt")
        self.logPath = Filename("/c/jamoconfig.log")
        self.clearConfig()
        self.simulate()
        self.log = sys.stdout
        self.configMissing = 0
        self.hardwareChanged = 0

        if (useJOD==None):
            useJOD = base.config.GetBool("want-jamodrum", True)

        self.useJOD = useJOD
        if (useJOD):
            self.setLog(self.logPath)
            self.devindices = range(1,base.win.getNumInputDevices())
            self.readConfigFile(self.configPath)
            self.prepareDevices()
            props = WindowProperties()
            props.setCursorHidden(1)
	    if (sys.platform == "win32"):
                props.setZOrder(WindowProperties.ZTop)
            base.win.requestProperties(props)
            self.setLog(None)


    def setLog(self, fn):
        if (self.log != sys.stdout):
            self.log.close()
            self.log = sys.stdout
        if (fn):
            try:
                self.log = open(fn.toOsSpecific(), "w")
            except:
                self.log = sys.stdout

    def generateMouseDigest(self):
        m = md5.md5()
        for i in range(base.win.getNumInputDevices()):
            m.update(base.win.getInputDeviceName(i))
            m.update("\n")
        return m.hexdigest()

    def reportDevices(self):
        for devindex in self.devindices:
            self.log.write("Encoder Detected: "+base.win.getInputDeviceName(devindex)+"\n")

    def clearConfig(self):
        self.ratio = 8.71
        self.wheelConfigs = [[0,0],[0,0],[0,0],[0,0]]
        self.padConfigs = [[0,0],[0,0],[0,0],[0,0]]

    def getIntVal(self, spec):
	try:
	    return int(spec)
	except:
	    return -1

    def setWheelConfig(self, station, axis, device):
        if (axis=="x") or (axis=="X"): axis=0
        if (axis=="y") or (axis=="Y"): axis=1
	istation = self.getIntVal(station)
	iaxis = self.getIntVal(axis)
	if (istation < 0) or (istation > 3):
            self.log.write("Wheel Config: Invalid station index "+str(station)+"\n")
	    return
	if (iaxis < 0) or (iaxis > 1):
	    self.log.write("Wheel Config: Invalid axis index "+str(axis)+"\n")
            return
        self.wheelConfigs[istation] = [iaxis, str(device)]
    
    def setPadConfig(self, station, button, device):
	istation = self.getIntVal(station)
        ibutton = self.getIntVal(button)
        if (istation < 0) or (istation > 3):
            self.log.write("Pad Config: Invalid station index "+str(station)+"\n")
            return
	if (ibutton < 0) or (ibutton > 2):
	    self.log.write("Pad Config: Invalid button index "+str(button)+"\n")
	    return
        self.padConfigs[istation] = [ibutton, device]

    def readConfigFile(self, fn):
        digest = self.generateMouseDigest()
        self.clearConfig()
	try:
	    file = open(fn.toOsSpecific(),"r")
	    lines = file.readlines()
            file.close()
	except:
            self.configMissing = 1
	    self.log.write("Could not read "+fn.toOsSpecific()+"\n")
	    return
	for line in lines:
	    line = line.strip(" \t\r\n")
	    if (line=="") or (line[0]=="#"):
		continue
	    words = line.split(" ")
	    if (words[0]=="wheel"):
		if (len(words)==4):
		    self.setWheelConfig(words[1],words[2],words[3])
		else:
		    self.log.write("Wheel Config: invalid syntax\n")
	    elif (words[0]=="pad"):
		if (len(words)==4):
		    self.setPadConfig(words[1],words[2],words[3])
		else:
		    self.log.write("Pad Config: invalid syntax\n")
	    elif (words[0]=="ratio"):
		try:
		    self.ratio = float(words[1])
		except:
		    self.log.write("Ratio Config: invalid syntax\n")
            elif (words[0]=="digest"):
                if (len(words)==2):
                    if (digest != words[1]):
                        self.hardwareChanged = 1
                else:
                    self.log.write("Digest: invalid syntax")                    
	    else:
		self.log.write("Unrecognized config directive "+line+"\n")

    def writeConfigFile(self, fn):
	try:
            file = open(fn.toOsSpecific(),"w")
            file.write("ratio "+str(self.ratio)+"\n")
            for i in range(4):
                wheelinfo = self.wheelConfigs[i]
                file.write("wheel "+str(i)+" "+str(wheelinfo[0])+" "+wheelinfo[1]+"\n")
                padinfo = self.padConfigs[i]
                file.write("pad "+str(i)+" "+str(padinfo[0])+" "+padinfo[1]+"\n")
            file.close()
        except:
            self.log.write("Could not write "+fn.toOsSpecific()+"\n")

    def findWheel(self, devaxis, devname):
        for wheelindex in range(4):
            wheelinfo = self.wheelConfigs[wheelindex]
            wheelaxis = wheelinfo[0]
            wheeldevice = wheelinfo[1]
            if (devname == wheeldevice) and (devaxis == wheelaxis):
                return wheelindex
        return -1
    
    def findPad(self, devbutton, devname):
        for padindex in range(4):
            padinfo = self.padConfigs[padindex]
            padbutton = padinfo[0]
            paddevice = padinfo[1]
            if (devname == paddevice) and (devbutton == padbutton):
                return padindex
        return -1

    def prepareDevices(self):
        """
        Each axis or button will be associated with a wheel or pad.
        Any axis or button not in the config list will be associated
        with wheel -1 or pad -1.
        """
        self.polls = []
        for devindex in range(1, base.win.getNumInputDevices()):
            devname = base.win.getInputDeviceName(devindex)
            for devaxis in range(2):
                target = self.findWheel(devaxis, devname)
                self.log.write("Axis "+str(devaxis)+" of "+devname+" controls wheel "+str(target)+"\n")
                self.polls.append([devaxis, devindex, target, 0])
            for devbutton in range(3):
                target = self.findPad(devbutton, devname)
                sig = "mousedev"+str(devindex)+"-mouse"+str(devbutton+1)
                self.log.write("Button "+str(devbutton)+" of "+devname+" controls pad "+str(target)+"\n")
                self.ignore(sig)
                self.accept(sig, self.hit, [target, 1.0])

    def simulate(self,spin=10.0,hit=1.0):
        """
        Accept keyboard keys to simulate Jam-o-Drum input.
        
        @keyword spin: degrees to spin for each keystroke (default: 10.0)
        @type spin: float
        @keyword hit: force to hit for each keystroke (default: 1.0)
        @type hit: float
        """
        self.accept('k',self.hit,[0,hit])
        self.accept('d',self.hit,[1,hit])
        self.accept('e',self.hit,[2,hit])
        self.accept('i',self.hit,[3,hit])

        self.accept('j',self.spin,[0,spin])
        self.accept('l',self.spin,[0,-spin])
        self.accept('s',self.spin,[1,spin])
        self.accept('f',self.spin,[1,-spin])
        self.accept('w',self.spin,[2,-spin])
        self.accept('r',self.spin,[2,spin])
        self.accept('u',self.spin,[3,-spin])
        self.accept('o',self.spin,[3,spin])
    # end simulate
    
    def poll(self):
        """
        Call this each frame to poll actual drumpads and spinners for input.
        If input occurs messages will be sent.
        """
        if (not self.useJOD):
            return
        
        offsets = [0.0,0.0,0.0,0.0]
        for info in self.polls:
            axis = info[0]
            devindex = info[1]
            wheel = info[2]
            last = info[3]
            if (axis == 0):
                pos = base.win.getPointer(devindex).getX()
            else:
                pos = base.win.getPointer(devindex).getY()
            if (pos != last):
                diff = (pos-last)/self.ratio
                if (wheel < 0):
                    offsets[0] += diff
                    offsets[1] += diff
                    offsets[2] += diff
                    offsets[3] += diff
                else:
                    offsets[wheel] += diff
                info[3] = pos

        for i in range(4):
            if (offsets[i] != 0.0):
                self.spin(i,offsets[i])

    
    def spin(self,station,angle):
        """
        Sends a JOD_SPIN_<station> message
        """
        sig = "JOD_SPIN_"+str(station)
        messenger.send(sig,[angle])
    
    def hit(self,station,force):
        """
        Sends a JOD_HIT_<station> message
        """
        if (station < 0):
            for station in range(4):
                sig = "JOD_HIT_"+str(station)
                messenger.send(sig,[force])
        else:
            sig = "JOD_HIT_"+str(station)
            messenger.send(sig,[force])

# end class JamoDrum
