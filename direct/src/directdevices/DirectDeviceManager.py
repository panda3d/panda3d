""" Class used to create and control vrpn devices """

from direct.showbase.DirectObject import DirectObject
from pandac.PandaModules import *

ANALOG_MIN = -0.95
ANALOG_MAX = 0.95
ANALOG_DEADBAND = 0.125
ANALOG_CENTER = 0.0

try:
    myBase = base
except:
    myBase = simbase

class DirectDeviceManager(VrpnClient, DirectObject):
    def __init__(self, server = None):

        # Determine which server to use
        if server != None:
            # One given as constructor argument
            self.server = server
        else:
            # Check config file, if that fails, use default
            self.server = myBase.config.GetString('vrpn-server', 'spacedyne')

        # Create a vrpn client
        VrpnClient.__init__(self, self.server)

    def createButtons(self, device):
        return DirectButtons(self, device)

    def createAnalogs(self, device):
        return DirectAnalogs(self, device)

    def createTracker(self, device):
        return DirectTracker(self, device)

    def createDials(self, device):
        return DirectDials(self, device)

    def createTimecodeReader(self, device):
        return DirectTimecodeReader(self, device)

class DirectButtons(ButtonNode, DirectObject):
    buttonCount = 0
    def __init__(self, vrpnClient, device):
        # Keep track of number of buttons created
        DirectButtons.buttonCount += 1
        # Create a unique name for this button object
        self.name = 'DirectButtons-' + `DirectButtons.buttonCount`
        # Create a new button node for the given device
        ButtonNode.__init__(self, vrpnClient, device)
        # Attach node to data graph
        self.nodePath = myBase.dataRoot.attachNewNode(self)

    def __getitem__(self, index):
        if (index < 0) or (index >= self.getNumButtons()):
            raise IndexError
        return self.getButtonState(index)

    def __len__(self):
        return self.getNumButtons()

    def enable(self):
        self.nodePath.reparentTo(myBase.dataRoot)

    def disable(self):
        self.nodePath.reparentTo(myBase.dataUnused)

    def getName(self):
        return self.name

    def getNodePath(self):
        return self.nodePath

    def __repr__(self):
        str = self.name + ': '
        for val in self:
            str = str + '%d' % val + ' '
        return str

class DirectAnalogs(AnalogNode, DirectObject):
    analogCount = 0
    def __init__(self, vrpnClient, device):
        # Keep track of number of analogs created
        DirectAnalogs.analogCount += 1
        # Create a unique name for this analog object
        self.name = 'DirectAnalogs-' + `DirectAnalogs.analogCount`
        # Create a new analog node for the given device
        AnalogNode.__init__(self, vrpnClient, device)
        # Attach node to data graph
        self.nodePath = myBase.dataRoot.attachNewNode(self)
        # See if any of the general analog parameters are dconfig'd
        self.analogDeadband = myBase.config.GetFloat('vrpn-analog-deadband',
                                                     ANALOG_DEADBAND)
        self.analogMin = myBase.config.GetFloat('vrpn-analog-min',
                                                ANALOG_MIN)
        self.analogMax = myBase.config.GetFloat('vrpn-analog-max',
                                                ANALOG_MAX)
        self.analogCenter = myBase.config.GetFloat('vrpn-analog-center',
                                                   ANALOG_CENTER)
        self.analogRange = self.analogMax - self.analogMin


    def __getitem__(self, index):
        if (index < 0) or (index >= self.getNumControls()):
            raise IndexError
        return self.getControlState(index)

    def __len__(self):
        return self.getNumControls()

    def enable(self):
        self.nodePath.reparentTo(myBase.dataRoot)

    def disable(self):
        self.nodePath.reparentTo(myBase.dataUnused)

    def normalizeWithoutCentering(self, val, minVal = -1, maxVal = 1):
        #
        # This is the old code that doesn't incorporate the centering fix
        #
        # First record sign
        if val < 0:
            sign = -1
        else:
            sign = 1
        # Zero out values in deadband
        val = sign * max(abs(val) - self.analogDeadband, 0.0)
        # Clamp value between analog range min and max and scale about center
        val = min(max(val, self.analogMin), self.analogMax)
        # Normalize values to given minVal and maxVal range
        return (((maxVal - minVal) *
                 ((val - self.analogMin) / float(self.analogRange))) + minVal)


    def normalize(self, rawValue, minVal = -1, maxVal = 1, sf = 1.0):
        aMax = self.analogMax
        aMin = self.analogMin
        center = self.analogCenter
        deadband = self.analogDeadband
        range = self.analogRange
        # Zero out values in deadband
        if (abs(rawValue-center) <= deadband):
            return 0.0
        # Clamp value between aMin and aMax and scale around center
        if (rawValue >= center):
            # Convert positive values to range 0 to 1
            val = min(rawValue * sf, aMax)
            percentVal = ((val - (center + deadband))/
                          float(aMax - (center + deadband)))
        else:
            # Convert negative values to range -1 to 0
            val = max(rawValue * sf, aMin)
            percentVal = -((val - (center - deadband))/
                           float(aMin - (center - deadband)))
        # Normalize values to given minVal and maxVal range
        return (((maxVal - minVal) * ((percentVal + 1)/2.0)) + minVal)

    def normalizeChannel(self, chan, minVal = -1, maxVal = 1, sf = 1.0):
        try:
            return self.normalize(self[chan], minVal, maxVal, sfx)
        except IndexError:
            return 0.0

    def getName(self):
        return self.name

    def getNodePath(self):
        return self.nodePath

    def __repr__(self):
        str = self.name + ': '
        for val in self:
            str = str + '%.3f' % val + ' '
        return str

class DirectTracker(TrackerNode, DirectObject):
    trackerCount = 0
    def __init__(self, vrpnClient, device):
        # Keep track of number of trackers created
        DirectTracker.trackerCount += 1
        # Create a unique name for this tracker object
        self.name = 'DirectTracker-' + `DirectTracker.trackerCount`
        # Create a new tracker node for the given device
        TrackerNode.__init__(self, vrpnClient, device)
        # Attach node to data graph
        self.nodePath = myBase.dataRoot.attachNewNode(self)

    def enable(self):
        self.nodePath.reparentTo(myBase.dataRoot)

    def disable(self):
        self.nodePath.reparentTo(myBase.dataUnused)

    def getName(self):
        return self.name

    def getNodePath(self):
        return self.nodePath

    def __repr__(self):
        return self.name

class DirectDials(DialNode, DirectObject):
    dialCount = 0
    def __init__(self, vrpnClient, device):
        # Keep track of number of dials created
        DirectDials.dialCount += 1
        # Create a unique name for this dial object
        self.name = 'DirectDials-' + `DirectDials.dialCount`
        # Create a new dial node for the given device
        DialNode.__init__(self, vrpnClient, device)
        # Attach node to data graph
        self.nodePath = myBase.dataRoot.attachNewNode(self)

    def __getitem__(self, index):
        """
        if (index < 0) or (index >= self.getNumDials()):
            raise IndexError
        """
        return self.readDial(index)

    def __len__(self):
        return self.getNumDials()

    def enable(self):
        self.nodePath.reparentTo(myBase.dataRoot)

    def disable(self):
        self.nodePath.reparentTo(myBase.dataUnused)

    def getName(self):
        return self.name

    def getNodePath(self):
        return self.nodePath

    def __repr__(self):
        str = self.name + ': '
        for i in range(self.getNumDials()):
            str = str + '%.3f' % self[i] + ' '
        return str

class DirectTimecodeReader(AnalogNode, DirectObject):
    timecodeReaderCount = 0
    def __init__(self, vrpnClient, device):
        # Keep track of number of timecodeReader created
        DirectTimecodeReader.timecodeReaderCount += 1
        # Create a unique name for this dial object
        self.name = ('DirectTimecodeReader-' +
                     `DirectTimecodeReader.timecodeReaderCount`)
        # Initialize components of timecode
        self.frames = 0
        self.seconds = 0
        self.minutes = 0
        self.hours = 0
        # Create a new dial node for the given device
        AnalogNode.__init__(self, vrpnClient, device)
        # Attach node to data graph
        self.nodePath = myBase.dataRoot.attachNewNode(self)

    def enable(self):
        self.nodePath.reparentTo(myBase.dataRoot)

    def disable(self):
        self.nodePath.reparentTo(myBase.dataUnused)

    def getName(self):
        return self.name

    def getNodePath(self):
        return self.nodePath

    def getTime(self):
        # Assume only one card, use channel 0
        timeBits = int(self.getControlState(0))
        self.frames = ((timeBits & 0xF) +
                       (((timeBits & 0xF0) >> 4) * 10))
        self.seconds = (((timeBits & 0x0F00) >> 8) +
                        (((timeBits & 0xF000) >> 12) * 10))
        self.minutes = (((timeBits & 0x0F0000) >> 16) +
                        (((timeBits & 0xF00000) >> 20) * 10))
        self.hours = (((timeBits & 0xF000000) >> 24) +
                      (((timeBits & 0xF0000000) >> 28) * 10))
        self.totalSeconds = ((self.hours * 3600) +
                             (self.minutes * 60) +
                             self.seconds +
                             (self.frames / 30.0))
        return (self.hours, self.minutes, self.seconds, self.frames,
                self.totalSeconds)

    def __repr__(self):
        str = ('%s: %d:%d:%d:%d' % ((self.name,) + self.getTime()[:-1]))
        return str















