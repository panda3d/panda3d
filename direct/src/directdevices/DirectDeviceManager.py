""" Class used to create and control vrpn devices """
from PandaObject import *

class DirectDeviceManager(VrpnClient, PandaObject):
    def __init__(self, server = None):
        # Determine which server to use
        if server != None:
            # One give as constructor argument
            self.server = server
        else:
            # Check config file, if that fails, use default
            self.server = base.config.GetString('vrpn-server', 'spacedyne')
        
        # Create a vrpn client
        VrpnClient.__init__(self, self.server)
    
    def createButtons(self, device):
        return DirectButtons(self, device)
    
    def createAnalogs(self, device):
        return DirectAnalogs(self, device)

    def createDials(self, device):
        return DirectDials(self, device)

    def createTimecodeReader(self, device):
        return DirectTimecodeReader(self, device)

class DirectButtons(ButtonNode, PandaObject):
    buttonCount = 0
    def __init__(self, vrpnClient, device):
        # Keep track of number of buttons created
        DirectButtons.buttonCount += 1
        # Create a unique name for this button object
        self.name = 'DirectButtons-' + `DirectButtons.buttonCount`
        # Create a new button node for the given device
        ButtonNode.__init__(self, vrpnClient, device)
        # Attach node to data graph
        self.nodePath = base.dataRoot.attachNewNode(self)
    
    def __getitem__(self, index):
        if (index < 0) | (index > self.getNumButtons()):
            raise IndexError
        return self.getButtonState(index)
    
    def __len__(self):
        return self.getNumButtons()
    
    def enable(self):
        self.nodePath.reparentTo(base.dataRoot)
    
    def disable(self):
        self.nodePath.reparentTo(base.dataUnused)
    
    def getName(self):
        return self.name
    
    def getNodePath(self):
        return self.nodePath
    
    def __repr__(self):
        str = self.name + ': '
        for val in self:
            str = str + '%d' % val + ' '
        return str

class DirectAnalogs(AnalogNode, PandaObject):
    analogCount = 0
    def __init__(self, vrpnClient, device):
        # Keep track of number of analogs created
        DirectAnalogs.analogCount += 1
        # Create a unique name for this analog object
        self.name = 'DirectAnalogs-' + `DirectAnalogs.analogCount`
        # Create a new analog node for the given device
        AnalogNode.__init__(self, vrpnClient, device)
        # Attach node to data graph
        self.nodePath = base.dataRoot.attachNewNode(self)
    
    def __getitem__(self, index):
        if (index < 0) | index > self.getNumControls():
            raise IndexError
        return self.getControlState(index)
    
    def __len__(self):
        return self.getNumControls()
    
    def enable(self):
        self.nodePath.reparentTo(base.dataRoot)
    
    def disable(self):
        self.nodePath.reparentTo(base.dataUnused)
    
    def getName(self):
        return self.name
    
    def getNodePath(self):
        return self.nodePath
    
    def __repr__(self):
        str = self.name + ': '
        for val in self:
            str = str + '%.3f' % val + ' '
        return str

class DirectDials(DialNode, PandaObject):
    dialCount = 0
    def __init__(self, vrpnClient, device):
        # Keep track of number of dials created
        DirectDials.dialCount += 1
        # Create a unique name for this dial object
        self.name = 'DirectDials-' + `DirectDials.dialCount`
        # Create a new dial node for the given device
        DialNode.__init__(self, vrpnClient, device)
        # Attach node to data graph
        self.nodePath = base.dataRoot.attachNewNode(self)
    
    def __getitem__(self, index):
        if (index < 0) | (index > self.getNumDials()):
            raise IndexError
        return self.readDial(index)
    
    def __len__(self):
        return self.getNumDials()
    
    def enable(self):
        self.nodePath.reparentTo(base.dataRoot)
    
    def disable(self):
        self.nodePath.reparentTo(base.dataUnused)
    
    def getName(self):
        return self.name
    
    def getNodePath(self):
        return self.nodePath
    
    def __repr__(self):
        str = self.name + ': '
        for val in self:
            str = str + '%.3f' % val + ' '
        return str

class DirectTimecodeReader(AnalogNode, PandaObject):
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
        self.nodePath = base.dataRoot.attachNewNode(self)
    
    def enable(self):
        self.nodePath.reparentTo(base.dataRoot)
    
    def disable(self):
        self.nodePath.reparentTo(base.dataUnused)
    
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
        self.minutes = ((((timeBits & 0x0F0000) >> 16) * 10) +
                        ((timeBits & 0xF00000) >> 20))
        self.hours = ((((timeBits & 0xF0000000) >> 24) * 10) +
                      ((timeBits & 0xF0000000) >> 28))
        self.totalSeconds = ((self.hours * 3600) +
                             (self.minutes * 60) +
                             self.seconds +
                             (self.frames / 30.0))
        return (self.hours, self.minutes, self.seconds, self.frames,
                self.totalSeconds)
    
    def __repr__(self):
        str = ('%s: %d:%d:%d:%d' % ((self.name,) + self.getTime()[:-1]))
        return str
