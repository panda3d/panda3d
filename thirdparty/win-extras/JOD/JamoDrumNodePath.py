"""
NodePath for Jam-o-Drum model

@author: Ben Buchwald <bb2@alumni.cmu.edu>
Last Updated: 11/29/2005

@var DRUMPAD_RADIUS: constant representing the radius of the drumpad as a percentage
                     of the radius of the table
@type DRUMPAD_RADIUS: float
@var SPINNER_RADIUS: constant representing the radius of the spinner as a percentage
                     of the radius of the table
@type SPINNER_RADIUS: float
"""

from direct.showbase.DirectObject import DirectObject
from direct.showbase.Audio3DManager import Audio3DManager
from direct.gui.DirectGui import *
from direct.task import Task
import math
from pandac.PandaModules import NodePath
from pandac.PandaModules import Texture, CardMaker, DepthOffsetAttrib, OrthographicLens

DRUMPAD_RADIUS = .167
SPINNER_RADIUS = .254

class JamoDrumNodePath(NodePath):
    """
    A C{NodePath} of a heirarchy of a Jam-o-Drum. Creating one of these sets up
    the camera as well for displaying an orthographic view of this Jam-o-drum
    
    @ivar stations: list of 4 L{Station} objects representing each Jam-o-Drum station
    @type stations: L{Station}[]
    """
    def __init__(self,table=None,mask=None):
        """
        @keyword table: filename of a table texture. See table_template.psd. Either
                        paint anywhere inside the mask for a complete background
                        or turn off the pads and spinner and paint in the table circle
                        for just a table texture that will have spinners and pads
                        put on top of it.
        @type mask: str
        @keyword mask: filename of a mask texture of the non-Jam-o-Drum area. probably
                       jod_mask.png that comes with the Jam-o-Drum library.
        @type mask: str
        """
        NodePath.__init__(self,"JamoDrum")
        
        totalHeight = max(1.0,math.sqrt(2)/4.0+SPINNER_RADIUS)*2
        
        cm = CardMaker("card")
        cm.setFrame(-1,1,-1,1)
        self.tableCard = self.attachNewNode(cm.generate())
        self.tableCard.setP(-90)
        self.tableCard.setScale(4.0/3.0)
        self.tableCard.setLightOff()
        self.tableCard.setBin("background",0)
        self.tableCard.setDepthTest(0)
        self.tableCard.setDepthWrite(0)
        self.tableCard.hide()
        
        if (table):
            self.setTableTexture(loader.loadTexture(table))
                       
        
        if (mask):
            cm = CardMaker("JOD Mask")
            cm.setFrame(-4.0/3.0,4.0/3.0,-4.0/3.0,4.0/3.0)
            self.mask = aspect2d.attachNewNode(cm.generate())
            #self.mask.setP(-90)
            self.mask.setTexture(loader.loadTexture(mask),1)
            self.mask.setTransparency(1)
            self.mask.setDepthTest(0)
        else:
            self.mask = None
        
        self.stations = []
        for i in range(4):
            station = Station(self,i)
            station.reparentTo(self)
            self.stations.append(station)
        
        self.reparentTo(render)
        base.disableMouse()
        self.lens = OrthographicLens()
        self.lens.setFilmSize(totalHeight*base.getAspectRatio(),totalHeight)
        base.cam.node().setLens(self.lens)
        camera.setPosHpr(0,0,10.0, 0,-90,0)
        base.setBackgroundColor(0,0,0)
        
        self.audio3d = Audio3DManager(base.sfxManagerList[0],self)
        self.audio3d.setDropOffFactor(0)
    # end __init__
    
    def setTableTexture(self,texture,scale=None):
        """
        sets the background texture on the table

        @param texture: C{Texture} or image file to show on the table
        @type texture: C{Texture} or string
        @keyword scale: scale of the texture. Default is set for the table_template.psd
        @type scale: float
        """
        if (not isinstance(texture,Texture)):
            texture = loader.loadTexture(texture)
        self.tableCard.setTexture(texture)
        if (scale):
            self.tableCard.setScale(scale)
        self.showTable()
    # end setTableTexture
    
    def showTable(self):
        """
        displays the table's background texture
        """
        self.tableCard.show()
    # end showTable
    
    def hideTable(self):
        """
        hides the table's background texture
        """
        self.tableCard.hide()
    # end hideSpinner
    
    def loadSfx(self,file,object=None):
        """
        load a sound with positional audio support
        
        @param file: wav file to load. Must be mono.
        @type file: string
        @keyword object: object to attach sound to
        @type object: C{NodePath}
        @return: a Panda sound object
        @rtype: C{AudioSound}        
        """
        sfx = self.audio3d.loadSfx(file)
        if (object):
            self.attachSoundToObject(sfx,object)
        return sfx
    # loadSfx
    
    def attachSoundToObject(self,sound,object):
        """
        attach a positional sound to an object it will come from
        
        @param sound: positional sound object to attach
        @type sound: C{AudioSound}
        @param object: object sound should come from
        @type object: C{NodePath}
        """
        self.audio3d.attachSoundToObject(sound,object)
    # end attachSoundToObject
    
    def detachSound(self,sound):
        """
        detach a positional sound from it's object. It will no longer move.
        
        @param sound: positional sound object to detach
        @type sound: C{AudioSound}
        """
        self.audio3d.detachSound(sound)
    # end detachSound
# end class JamoDrum

class Station(NodePath):
    """
    A C{NodePath} to a specific Jam-o-Drum station. This C{NodePath} should contain
    subparts call pad and spinner. This is automatically created by L{JamoDrumNodePath}
    and should not be created by hand.
    
    @ivar index: index of this station (0-3)
    @type index: int
    @ivar pad: C{NodePath} to the pad subpart
    @type pad: C{NodePath}
    @ivar spinner: C{NodePath} to the spinner subpart
    @type spinner: C{NodePath}
    """
    def __init__(self,jod,index):
        """
        @param jod: containing Jam-o-Drum
        @type jod: L{JamoDrumNodePath}
        @param index: index of the station
        @type index: int
        """
        NodePath.__init__(self,"station%02d"%index)
        self.jod = jod
        self.index = index
        angle = 45.0-self.index*90.0
        self.setPos(math.cos((angle-90.0)*math.pi/180.0),math.sin((angle-90.0)*math.pi/180.0),0)
        self.setH(angle)
        
        cm = CardMaker("card")
        cm.setFrame(-1,1,-1,1)

        self.spinner = self.attachNewNode("spinner")
        self.spinnerCard = self.spinner.attachNewNode(cm.generate())
        self.spinnerCard.setTransparency(1)
        self.spinnerCard.setBin("background",1)
        self.spinnerCard.node().setAttrib(DepthOffsetAttrib.make(1))
        self.spinnerCard.setScale(1.0/3.0)
        self.spinnerCard.setP(-90)
        self.spinnerCard.setLightOff()
        self.spinnerCard.hide()
        
        self.pad = self.attachNewNode("pad")
        self.padCard = self.pad.attachNewNode(cm.generate())
        self.padCard.setTransparency(1)
        self.padCard.setBin("background",2)
        self.padCard.node().setAttrib(DepthOffsetAttrib.make(2))
        self.padCard.setScale(1.0/3.0)
        self.padCard.setP(-90)
        self.padCard.setLightOff()
        self.padCard.hide()
    # end __init__
    
    def getParent(self):
        """
        get the containing L{JamoDrumNodePath}
        
        @return: the containing Jam-o-Drum
        @rtype: L{JamoDrumNodePath}
        """
        return self.jod
    # end getParent
    
    def setSpinnerTexture(self,texture,scale=None):
        """
        sets the background texture on the spinner
        
        @param texture: C{Texture} or image file to show on the spinner
        @type texture: C{Texture} or string
        @keyword scale: scale of the texture. Default is set for the spinner_template.psd
        @type scale: float
        """
        if (not isinstance(texture,Texture)):
            texture = loader.loadTexture(texture)
        self.spinnerCard.setTexture(texture)
        if (scale):
            self.spinnerCard.setScale(scale)
        self.showSpinner()
    # end setSpinnerTexture
    
    def setPadTexture(self,texture,scale=None):
        """
        sets the background texture on the pad

        @param texture: C{Texture} or image file to show on the pad
        @type texture: C{Texture} or string
        @keyword scale: scale of the texture. Default is set for the spinner_template.psd
        @type scale: float
        """
        if (not isinstance(texture,Texture)):
            texture = loader.loadTexture(texture)
        self.padCard.setTexture(texture)
        if (scale):
            self.padCard.setScale(scale)
        self.showPad()
    # end setPadTexture
    
    def showSpinner(self):
        """
        displays the spinner's background texture
        """
        self.spinnerCard.show()
    # end showSpinner
    
    def showPad(self):
        """
        displays the pad's background texture
        """
        self.padCard.show()
    # end showPad
    
    def hideSpinner(self):
        """
        hides the spinner's background texture
        """
        self.spinnerCard.hide()
    # end hideSpinner
    
    def hidePad(self):
        """
        hide the pad's background texture
        """
        self.padCard.hide()
    # end hidePad
    
    def loadSfx(self,file,object=None):
        """
        load a sound with positional audio support and attach it to this station
        
        @param file: wav file to load. Must be mono.
        @type file: string
        @keyword object: object to attach sound to. default: this station
        @type object: C{NodePath}
        @return: a Panda sound object
        @rtype: C{AudioSound}
        """
        return self.jod.loadSfx(file,object or self)
    #end loadSfx

    def attachSound(self,sound,object=None):
        """
        attach a positional sound to this station
        
        @param sound: positional sound object to attach
        @type sound: C{AudioSound}
        @keyword object: object sound should come from. default: this station
        @type object: C{NodePath}
        """
        self.jod.attachSoundToObject(sound,object or self)
    # end attachSound
# end class Station


