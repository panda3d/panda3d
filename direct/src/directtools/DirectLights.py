from direct.showbase.PandaObject import *
from string import lower

class DirectLight(NodePath):
    def __init__(self, light, parent):
        # Initialize the superclass
        NodePath.__init__(self)
        # Record light and name
        self.light = light

        # Upcast the light object to its node base pointer
        if isinstance(light, Spotlight):
            node = light.upcastToLensNode()
        else:
            node = light.upcastToPandaNode()

        # Attach node to self
        self.assign(parent.attachNewNode(node))

    def getName(self):
        return self.light.getName()
    
    def getLight(self):
        return self.light

class DirectLights(NodePath):
    def __init__(self, parent = render):
        # Initialize the superclass
        NodePath.__init__(self)
        # Create a node for the lights
        self.assign(parent.attachNewNode('DIRECT Lights'))
        # Create a light attrib
        self.la = LightAttrib.makeAllOff()
        # Create a list of all active lights
        self.lightDict = {}
        # Counts of the various types of lights
        self.ambientCount = 0
        self.directionalCount = 0
        self.pointCount = 0
        self.spotCount = 0

    def __getitem__(self, name):
        return self.lightDict.get(name, None)

    def __len__(self):
        return len(self.lightDict)

    def delete(self, light):
        del self.lightDict[light.getName()]
        self.setOff(light)
        light.removeNode()

    def deleteAll(self):
        for light in self.asList():
            self.delete(light)

    def asList(self):
        return map(lambda n, s=self: s[n], self.getNameList())

    def getNameList(self):
        # Return a sorted list of all lights in the light dict
        nameList = map(lambda x: x.getName(), self.lightDict.values())
        nameList.sort()
        return nameList
    
    def create(self, type):
        type = type.lower()
        if type == 'ambient':
            self.ambientCount += 1
            light = AmbientLight('ambient-' + `self.ambientCount`)
            light.setColor(VBase4(.3,.3,.3,1))
        elif type == 'directional':
            self.directionalCount += 1
            light = DirectionalLight('directional-' + `self.directionalCount`)
            light.setColor(VBase4(1))
        elif type == 'point':
            self.pointCount += 1
            light = PointLight('point-' + `self.pointCount`)
            light.setColor(VBase4(1))
        elif type == 'spot':
            self.spotCount += 1
            light = Spotlight('spot-' + `self.spotCount`)
            light.setColor(VBase4(1))
            light.setLens(PerspectiveLens())
        else:
            print 'Invalid light type'
            return None
        # Add the new light
        directLight = DirectLight(light,self)
        self.lightDict[directLight.getName()] = directLight
        # Turn it on as a default
        self.setOn(directLight)
        # Send an event to all watching objects
        messenger.send('DIRECT_addLight', [directLight])
        # Return the new light
        return directLight

    def createDefaultLights(self):
        self.create('ambient')
        self.create('directional')

    def allOn(self):
        """ Turn on all DIRECT lights """
        render.node().setAttrib(self.la)
        # Make sure there is a default material
        render.setMaterial(Material())

    def allOff(self):
        """ Turn off all DIRECT lights """
        render.node().clearAttrib(LightAttrib.getClassType())

    def toggle(self):
        """ Toggles light attribute, but doesn't toggle individual lights """
        if render.node().hasAttrib(LightAttrib.getClassType()):
            self.allOff()
        else:
            self.allOn()

    def setOn(self, directLight):
        """ setOn(directLight) """
        self.la = self.la.addLight(directLight.getLight())
        if render.node().hasAttrib(LightAttrib.getClassType()):
            render.node().setAttrib(self.la)
            
    def setOff(self, directLight):
        """ setOff(directLight)"""
        self.la = self.la.removeLight(directLight.getLight())
        if render.node().hasAttrib(LightAttrib.getClassType()):
            render.node().setAttrib(self.la)


