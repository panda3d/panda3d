from PandaObject import *
from DirectGeometry import *
from string import lower

class DirectLights(NodePath):
    def __init__(self, parent = None):
        # Initialize the superclass
        NodePath.__init__(self)
        # Use direct.group as default parent
        if parent == None:
            parent = direct.group
        # Create a node for the lights
        self.assign(parent.attachNewNode('DIRECT Lights'))
        # Create a light attribute 
        self.la = LightAttribute()
        # Create a list of all active lights
        self.lightList = []
        self.nodePathList = []
        self.nameList = []
        # Counts of the various types of lights
        self.ambientCount = 0
        self.directionalCount = 0
        self.pointCount = 0
        self.spotCount = 0

    def __getitem__(self, index):
        return self.lightList[index]

    def __len__(self):
        return len(self.lightList)

    def getLightNodePath(self, index):
        return self.nodePathList[index]

    def getLightName(self, index):
        return self.nameList[index]

    def create(self, type):
        type = type.lower()
        if type == 'ambient':
            self.ambientCount += 1
            light = AmbientLight('ambient_' + `self.ambientCount`)
            light.setColor(VBase4(.3,.3,.3,1))
        elif type == 'directional':
            self.directionalCount += 1
            light = DirectionalLight('directional_' + `self.directionalCount`)
            light.setColor(VBase4(1))
        elif type == 'point':
            self.pointCount += 1
            light = PointLight('point_' + `self.pointCount`)
            light.setColor(VBase4(1))
        elif type == 'spot':
            self.spotCount += 1
            light = Spotlight('spot_' + `self.spotCount`)
            light.setColor(VBase4(1))
        else:
            print 'Invalid light type'
            return None
        # Add the new light
        self.addLight(light)
        # Turn it on as a default
        self.setOn(light)
        # Return the new light
        return light

    def createDefaultLights(self):
        self.create('ambient')
        self.create('directional')

    def addLight(self, light):
        # Attach node to self
        if isinstance(light, Spotlight):
            nodePath = self.attachNewNode(light.upcastToProjectionNode())
        else:
            nodePath = self.attachNewNode(light.upcastToNamedNode())
        name = light.getName()
        # Store it in the lists
        self.lightList.append(light)
        self.nodePathList.append(nodePath)
        self.nameList.append(name)
        # Send an event to all watching objects
        messenger.send('DirectLights_addLight', [light])

    def allOn(self):
        """ Turn on all DIRECT lights """
        base.initialState.setAttribute(LightTransition.getClassType(),
                                       self.la)

    def allOff(self):
        """ Turn off all DIRECT lights """
        base.initialState.clearAttribute(LightTransition.getClassType())

    def toggle(self):
        """ Toggles light attribute, but doesn't toggle individual lights """
        if base.initialState.hasAttribute(LightTransition.getClassType()):
            self.allOff()
        else:
            self.allOn()

    def setOnNum(self, index):
        self.setOn(self.lightList[index])

    def setOffNum(self, index):
        self.setOff(self.lightList[index])

    def setOn(self, light):
        self.la.setOn(light.upcastToLight())

    def setOff(self, light):
        self.la.setOff(light.upcastToLight())


