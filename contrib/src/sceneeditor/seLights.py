#################################################################
# seLights.py
# Written by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#################################################################
from direct.showbase.DirectObject import *
from direct.directtools import DirectUtil
from panda3d.core import *
import string


class seLight(NodePath):
    #################################################################
    # seLight(NodePath)
    # This is an object for keeping light data and let we can manipulate
    # lights as otherNopaths.
    # This idea basically is from directLight.
    # But the way directLight object worked is not we want in our
    # sceneEditor. So, we wrote one by ourself.
    #################################################################
    def __init__(self, light, parent, type,
                 lightcolor=VBase4(0.3,0.3,0.3,1),
                 specularColor = VBase4(1),
                 position = Point3(0,0,0),
                 orientation = Vec3(1,0,0),
                 constant = 1.0,
                 linear = 0.0,
                 quadratic = 0.0,
                 exponent = 0.0,
                 tag="",
                 lence = None):
        #################################################################
        # __init__(self, light, parent, type,
        #          lightcolor=VBase4(0.3,0.3,0.3,1),
        #          specularColor = VBase4(1),
        #          position = Point3(0,0,0),
        #          orientation = Vec3(1,0,0),
        #          constant = 1.0,
        #          linear = 0.0,
        #          quadratic = 0.0,
        #          exponent = 0.0,
        #          tag="",
        #          lence = None):
        # This constructor will create a light node inside it and upcast
        # this light node to itself as a nodePath.
        # Also, it will load a model as a reference to itself on the secene so that
        # user can easily recognize where is the light and can easily manipulate
        # by mouse picking and widget control.
        #################################################################
        # Initialize the super class
        NodePath.__init__(self)
        # Initialize and save values
        self.light = light
        self.type = type
        self.lightcolor=lightcolor
        self.specularColor = specularColor
        self.position = position
        self.orientation = orientation
        self.constant = constant
        self.linear = linear
        self.quadratic = quadratic
        self.exponent = exponent
        self.lence = lence
        self.active = True

        # Attach node to self
        self.LightNode=parent.attachNewNode(light)
        self.LightNode.setTag("Metadata",tag)
        if(self.type=='spot'):
            self.LightNode.setHpr(self.orientation)
            self.LightNode.setPos(self.position)
        else:
            self.LightNode.setHpr(self.orientation)
            self.LightNode.setPos(self.position)


        self.assign(self.LightNode)
        if(self.type=='spot'):
            self.helpModel = loader.loadModel( "models/misc/Spotlight" )
        elif(self.type=='point'):
            self.helpModel = loader.loadModel( "models/misc/Pointlight" )
        elif(self.type=='directional'):
            self.helpModel = loader.loadModel( "models/misc/Dirlight" )
        else:
            self.helpModel = loader.loadModel( "models/misc/sphere" )
        self.helpModel.setColor(self.lightcolor)
        self.helpModel.reparentTo(self)
        DirectUtil.useDirectRenderStyle(self.helpModel)
        if not ((self.type == 'directional')or(self.type == 'point')or(self.type == 'spot')):
            self.helpModel.hide()

    def getLight(self):
        #################################################################
        # getLight(self)
        # This function will return the light object it contains.
        #################################################################

        return self.light

    def getLightColor(self):
        #################################################################
        # getLightColor(self)
        # This function will return the color of the light color of this light node.
        #################################################################
        return self.lightcolor

    def getName(self):
        #################################################################
        # getName(self)
        # This function will return the name of this light.
        #################################################################
        return self.light.getName()

    def rename(self,name):
        #################################################################
        # rename(self, name)
        # User should specify a string object, name, as the new name of this
        # light node. This function will rename itself and light object
        # to this new name.
        #################################################################
        self.light.setName(name)
        self.setName(name)

    def getType(self):
        #################################################################
        # getType(self)
        # This function will return a string which is the type of this
        # light node. (We only have four types of light)
        #################################################################
        return self.type

    def setColor(self,color):
        #################################################################
        # setColor(self, color)
        # This function takes one input parameter, color, which is a
        # VBase4 object. This function will simply set this object into
        # light node it has to change the property of light.
        # Also, if the light type is either "directional" or "point", which
        # means it has a reference model with it, this function will also
        # change the reference model's color to input value.
        #################################################################
        self.light.setColor(color)
        self.lightcolor = color
        if (self.type == 'directional')or(self.type == 'point'):
            self.helpModel.setColor(self.lightcolor)
        return

    def getSpecColor(self):
        #################################################################
        # getSpecColor(self)
        # This function will return the specular color of the light.
        # Although you can call this function for all kinds of light,
        # it will only meanful if this light is not a ambient light.
        #################################################################
        return self.specularColor

    def setSpecColor(self,color):
        #################################################################
        # setSpecColor(self, color)
        # This function can be used to set the specular color of the light.
        # Although you can call this function for all kinds of light,
        # it will only meanful if the light type is not "ambient"
        #################################################################
        self.light.setSpecularColor(color)
        self.specularcolor = color
        return

    def getPosition(self):
        #################################################################
        # getPosition(self)
        # getPosition(self)
        # This functioln will return a Point3 object which contains
        # the x, y, z position data of this light node.
        # It only has meaning for "point Light" and "Directional light"
        #################################################################
        self.position = self.LightNode.getPos()
        return self.position

    def setPosition(self, pos):
        #################################################################
        # setPosition(self, pos)
        # This function will take a Point3 object as a input.
        # Then, this function will set the itself andd light node to the
        # target point.
        #################################################################
        self.LightNode.setPos(pos)
        self.position = pos
        return

    def getOrientation(self):
        #################################################################
        # getOrientation(self)
        # This function will return a Vec3-type object which contains the
        # orientation data of this light node
        #
        # This function will only have meaning for point light and directional light.
        #
        #################################################################
        self.orientation = self.LightNode.getHpr()
        return self.orientation

    def setOrientation(self,orient):
        #################################################################
        # setOrientation(self, orient)
        # This funtction will take a Vec3-type object as an input.
        # Then this function will set itself nad light node to face
        # the target orientation.
        #
        # This function only has meaning for point light and directional light
        # type of lights.
        #
        #################################################################
        self.LightNode.setHpr(orient)
        self.orientation = orient
        return

    def getAttenuation(self):
        #################################################################
        # getAttenuation(self)
        # This function will return a Vec3 type of object which contains
        # the constant, linear and quadratic attenuation for this light node.
        #
        # This function will only have meaning for point light and spot light
        # tyoe of lights.
        #
        #################################################################
        return Vec3(self.constant,self.linear,self.quadratic)

    def setConstantAttenuation(self, value):
        #################################################################
        # setConstantAttenuation(self, value)
        # This function will take a float number as an input.
        # Then, this function will set the Constant Atenuation value
        # to this number.
        #################################################################
        self.light.setAttenuation(Vec3(value, self.linear, self.quadratic))
        self.constant = value
        return

    def setLinearAttenuation(self, value):
        #################################################################
        # setLinearAttenuation(self, value)
        # This function will take a float number as an input.
        # Then, this function will set the Linear Atenuation value
        # to this number.
        #################################################################
        self.light.setAttenuation(Vec3(self.constant, value, self.quadratic))
        self.linear = value
        return

    def setQuadraticAttenuation(self, value):
        #################################################################
        # setQuadraticAttenuation(self, value)
        # This function will take a float number as an input.
        # Then, this function will set the Quadratic Atenuation value
        # to this number.
        #################################################################
        self.light.setAttenuation(Vec3(self.constant, self.linear, value))
        self.quadratic = value
        return

    def getExponent(self):
        #################################################################
        # getExponent(self)
        # This function will return the value of the Exponent Attenuation
        # of this light node. (float)
        #################################################################
        return self.exponent

    def setExponent(self, value):
        #################################################################
        # setExponent(self, value)
        # This function will take a float number as an input.
        # Then, this function will set the Exponent Atenuation value
        # to this number.
        #################################################################
        self.light.setExponent(value)
        self.exponent = value
        return

class seLightManager(NodePath):
    #################################################################
    # seLightManager(NodePath)
    # This is the class we used to control al lightings in our sceneEditor.
    #################################################################
    def __init__(self):
        # Initialize the superclass
        NodePath.__init__(self)
        # Create a node for the lights
        self.lnode=render.attachNewNode('Lights')
        self.assign(self.lnode)
        # Create a light attrib
        self.lightAttrib = LightAttrib.makeAllOff()
        self.lightDict = {}
        self.ambientCount = 0
        self.directionalCount = 0
        self.pointCount = 0
        self.spotCount = 0
        # Originally, we don't do this load model thing.
        # But the problem is, if we don't, then it will cause some
        # Bounding calculation error...
        self.helpModel = loader.loadModel( "models/misc/sphere" )
        self.helpModel.reparentTo(self)
        self.helpModel.hide()




    def create(self, type = 'ambient',
                 lightcolor=VBase4(0.3,0.3,0.3,1),
                 specularColor = VBase4(1),
                 position = Point3(0,0,0),
                 orientation = Vec3(1,0,0),
                 constant = 1.0,
                 linear = 0.0,
                 quadratic = 0.0,
                 exponent = 0.0,
                 tag= "",
                 name='DEFAULT_NAME'):
        #################################################################
        # create(self, type = 'ambient',
        #        lightcolor=VBase4(0.3,0.3,0.3,1),
        #        specularColor = VBase4(1),
        #        position = Point3(0,0,0),
        #        orientation = Vec3(1,0,0),
        #        constant = 1.0,
        #        linear = 0.0,
        #        quadratic = 0.0,
        #        exponent = 0.0,
        #        tag= "",
        #        name='DEFAULT_NAME')
        # As you can see, once user call this function and specify those
        # variables, this function will create a seLight node.
        # In the default, the light which just has been created will be
        # set to off.
        #################################################################
        ### create the light

        lence = None

        if type == 'ambient':
            self.ambientCount += 1
            if(name=='DEFAULT_NAME'):
                light = AmbientLight('ambient_' + repr(self.ambientCount))
            else:
                light = AmbientLight(name)

            light.setColor(lightcolor)

        elif type == 'directional':
            self.directionalCount += 1
            if(name=='DEFAULT_NAME'):
                light = DirectionalLight('directional_' + repr(self.directionalCount))
            else:
                light = DirectionalLight(name)

            light.setColor(lightcolor)
            light.setSpecularColor(specularColor)

        elif type == 'point':
            self.pointCount += 1
            if(name=='DEFAULT_NAME'):
                light = PointLight('point_' + repr(self.pointCount))
            else:
                light = PointLight(name)

            light.setColor(lightcolor)
            light.setSpecularColor(specularColor)
            light.setAttenuation(Vec3(constant, linear, quadratic))

        elif type == 'spot':
            self.spotCount += 1
            if(name=='DEFAULT_NAME'):
                light = Spotlight('spot_' + repr(self.spotCount))
            else:
                light = Spotlight(name)

            light.setColor(lightcolor)
            lence = PerspectiveLens()
            light.setLens(lence)
            light.setSpecularColor(specularColor)
            light.setAttenuation(Vec3(constant, linear, quadratic))
            light.setExponent(exponent)
        else:
            print('Invalid light type')
            return None

        # Create the seLight objects and put the light object we just created into it.
        lightNode = seLight(light,self,type,
                            lightcolor=lightcolor,
                            specularColor = specularColor,
                            position = position,
                            orientation = orientation,
                            constant = constant,
                            linear = linear,
                            quadratic = quadratic,
                            exponent = exponent,
                            tag=tag,
                            lence = lence
                                )
        self.lightDict[light.getName()] = lightNode
        self.setOn(lightNode)


        return self.lightDict.keys(),lightNode

    def addLight(self, light):
        #################################################################
        # addLight(self, light)
        # This function will put light in and save its properties in to a seLight Node
        # Attention!!
        # only Spotlight obj nneds to be specified a lens node first. i.e. setLens() first!
        #################################################################
        type = light.getType().getName().lower()

        specularColor = VBase4(1)
        position = Point3(0,0,0)
        orientation = Vec3(1,0,0)
        constant = 1.0
        linear = 0.0
        quadratic = 0.0
        exponent = 0.0
        lence = None

        lightcolor = light.getColor()
        if type == 'ambientlight':
            type = 'ambient'
            self.ambientCount += 1
        elif type == 'directionallight':
            type = 'directional'
            self.directionalCount += 1
            orientation = light.getDirection()
            position = light.getPoint()
            specularColor = light.getSpecularColor()
        elif type == 'pointlight':
            type = 'point'
            self.pointCount += 1
            position = light.getPoint()
            specularColor = light.getSpecularColor()
            Attenuation = light.getAttenuation()
            constant = Attenuation.getX()
            linear = Attenuation.getY()
            quadratic = Attenuation.getZ()
        elif type == 'spotlight':
            type = 'spot'
            self.spotCount += 1
            specularColor = light.getSpecularColor()
            Attenuation = light.getAttenuation()
            constant = Attenuation.getX()
            linear = Attenuation.getY()
            quadratic = Attenuation.getZ()
            exponent = light.getExponent()
        else:
            print('Invalid light type')
            return None

        lightNode = seLight(light,self,type,
                            lightcolor=lightcolor,
                            specularColor = specularColor,
                            position = position,
                            orientation = orientation,
                            constant = constant,
                            linear = linear,
                            quadratic = quadratic,
                            exponent = exponent,
                            lence = lence)
        self.lightDict[light.getName()] = lightNode
        self.setOn(lightNode)

        return self.lightDict.keys(),lightNode

    def delete(self, name, removeEntry = True):
        #################################################################
        # delete(self, name, removeEntry = True)
        # This function will remove light node had the same name with user input.
        # Also, you can specify the removeEntry to decide to remove the entry from the lightDict or not.
        # Normaly, you alway want to remove the entry from the dictionary. Thsi is only used for "deleteAll" function.
        #################################################################
        type = self.lightDict[name].getType()
        if type == 'ambient':
            self.ambientCount -= 1
        elif type == 'directional':
            self.directionalCount -= 1
        elif type == 'point':
            self.pointCount -= 1
        elif type == 'spot':
            self.spotCount -= 1
        self.setOff(self.lightDict[name])
        self.lightDict[name].removeChildren()
        self.lightDict[name].removeNode()
        if removeEntry:
            del self.lightDict[name]
        return self.lightDict.keys()

    def deleteAll(self):
        #################################################################
        # deleteAll(self)
        # This function will keep calling delete and put exist lights in
        # until all lights have been eliminated.
        #################################################################
        for name in self.lightDict:
            self.delete(name, removeEntry = False)

        self.lightDict.clear()

    def isLight(self,name):
        #################################################################
        # isLight(self.name)
        # Use a string as a index to check if there existing a light named "name"
        #################################################################
        return name in self.lightDict

    def rename(self,oName,nName):
        #################################################################
        # rename(self, oName, nName)
        # This function will reanem the light named "oName(String)" to
        # nName(String)
        #################################################################
        if self.isLight(oName):
            lightNode = self.lightDict[oName]
            self.lightDict[nName] = lightNode
            lightNode.rename(nName)
            del self.lightDict[oName]
            return self.lightDict.keys(),lightNode
        else:
            print('----Light Mnager: No such Light!')

    def getLightNodeList(self):
        #################################################################
        # getLightNodeList(self)
        # Return a list which contains all seLight nodes
        #################################################################
        list = []
        for name in self.lightDict:
            list.append(self.lightDict[name])
        return list

    def getLightNodeDict(self):
        #################################################################
        # getLightNodeDict(self)
        # Return the light dictionary itself.
        #
        # Attention!
        # Because it doesn't make a copy when you return a dictionary, it
        # means when you can change the value from outside and that change
        # will directly reflect back to here.
        #
        #################################################################
        return self.lightDict

    def getLightList(self):
        #################################################################
        # getLightList(self)
        # Return a list which contains names of all lights.
        #################################################################
        list = []
        for name in self.lightDict:
            list.append(name)
        return list

    def getLightNode(self,lightName):
        #################################################################
        # getLightNode(self, lightName)
        # This function will return a seLight Node using a string, lightName. as a index.
        #################################################################
        if lightName in self.lightDict:
            return self.lightDict[lightName]

    def allOn(self):
        #################################################################
        # allOb(self)
        # Enable the lighting system.
        #################################################################
        # Turn on all lighting
        render.node().setAttrib(self.lightAttrib)
        # Make sure there is a default material
        render.setMaterial(Material())

    def allOff(self):
        #################################################################
        # allOff(self)
        # Disable whole lighting system
        #################################################################
        # Turn off all lighting
        render.node().clearAttrib(LightAttrib.getClassType())

    def toggle(self):
        #################################################################
        # toggle(self)
        # Toggles light attribute, but doesn't toggle individual lights
        #################################################################
        if render.node().hasAttrib(LightAttrib.getClassType()):
            self.allOff()
        else:
            self.allOn()

    def setOn(self, lightNode):
        #################################################################
        # setOn(lightNode)
        # This function will enable the input seLight node.
        # If the light system itself is down, activate it.
        #################################################################
        self.lightAttrib = self.lightAttrib.addLight(lightNode.getLight())
        lightNode.active = True
        if render.node().hasAttrib(LightAttrib.getClassType()):
            render.node().setAttrib(self.lightAttrib)


    def setOff(self, lightNode):
        #################################################################
        # setOff(self, lightNode)
        # This function will disable the input seLight node
        # If the light system itself is down, activate it.
        #################################################################
        lightNode.active = False
        self.lightAttrib = self.lightAttrib.removeLight(lightNode.getLight())
        if render.node().hasAttrib(LightAttrib.getClassType()):
            render.node().setAttrib(self.lightAttrib)

    def getList(self):
        #################################################################
        # getList(self)
        # This function actually has the same functionality with getLightList(),
        # but this one should be more efficient.
        #################################################################
        return self.lightDict.keys()
