"""PANDA3D Particle Panel"""

# Import Tkinter, Pmw, and the floater code from this directory tree.
from AppShell import *
from Tkinter import *
import Pmw
import Dial
import Floater
import VectorWidgets
import Placer
import Particles

class ParticlePanel(AppShell):
    # Override class variables
    appname = 'Particle Panel'
    frameWidth  = 300
    frameHeight = 600
    usecommandarea = 0
    usestatusarea  = 0
    
    def __init__(self, particles, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',     self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

	self.particles = particles

        AppShell.__init__(self)

        self.initialiseoptions(ParticlePanel)

    def appInit(self):
        self.widgetDict = {}

    def createInterface(self):
        # Handle to the toplevels hull
        interior = self.interior()

        # Combo box to switch between particle systems
        self.systemSelector = Pmw.ComboBox(self.menuFrame,
                                     labelpos = W,
                                     label_text = 'Particle System:',
                                     entry_width = 16,
                                     selectioncommand = self.selectSystemNamed,
                                     scrolledlist_items = ('system 0',))
        self.systemSelector.selectitem('system 0')
        self.systemSelector.pack(side = 'left', expand = 0)

        # Create the notebook pages
        notebook = Pmw.NoteBook(interior)
        notebook.pack(fill = BOTH, expand = 1)
        systemPage = notebook.add('System')
        factoryPage = notebook.add('Factory')
        emitterPage = notebook.add('Emitter')
        rendererPage = notebook.add('Renderer')

        ## SYSTEM PAGE ##
        # Create system floaters
        systemFloaterDefs = (
            ('System Pool size',
             'Size of particle pool',
             self.setSystemPoolSize,
             0.0, 1.0),
            ('System Birth rate',
             'Seconds between particle births',
             self.setSystemBirthRate,
             0.0, None),
            ('System Litter size',
             'Number of particle created at each birth',
             self.setSystemLitterSize,
             1.0, 1.0),
            ('System Litter spread',
             'Variation in litter size',
             self.setSystemLitterSpread,
             0.0, 1.0),
            ('System lifespan',
             'Age in seconds at which system should die',
             self.setSystemLifespan,
             0.0, None)
            )
        self.createFloaters(systemPage, systemFloaterDefs)
        # Checkboxes
        self.systemLocalVelocity = self.createCheckbutton(
            systemPage, 'Local velocity',
            self.toggleSystemLocalVelocity, 0)
        self.systemGrowsOlder = self.createCheckbutton(
            systemPage, 'System grows older', 
            self.toggleSystemGrowsOlder, 0)
        # Vector widgets
        pos = self.createVector3Entry(systemPage, 'Pos',
                                      'Particle system position')
        pos.command = self.setSystemPos
        pos.addMenuItem('Popup Placer Panel', Placer.Placer)
        hpr = self.createVector3Entry(systemPage, 'Hpr',
                                     'Particle system orientation',
                                      fGroup_labels = ('H', 'P', 'R'))
        hpr.command = self.setSystemHpr
        hpr.addMenuItem('Popup Placer Panel', Placer.Placer)

        ## FACTORY PAGE ##
        self.factorType = self.createOptionMenu(
            factoryPage,
            'Factory type:',
            'Select type of particle factory',
            ('Point', 'Z Spin', 'Oriented'),
            self.selectFactoryType)
        factoryWidgets = (
            ('Life span',
             'Average lifespan in seconds',
             self.setFactoryLifeSpan,
             0.0, None),
            ('Life span spread',
             'Variation in lifespan',
             self.setFactoryLifeSpanSpread,
             0.0, None),
            ('Mass',
             'Average particle mass',
             self.setFactoryParticleMass,
             0.0, None),
            ('Mass spread',
             'Variation in particle mass',
             self.setFactoryParticleMassSpread,
             0.0, None),
            ('Terminal velocity',
             'Average particle terminal velocity',
             self.setFactoryTerminalVelocity,
             0.0, None),
            ('Terminal vel. spread',
             'Variation in terminal velocity',
             self.setFactoryTerminalVelocitySpread,
             0.0, None))
        self.createFloaters(factoryPage, factoryWidgets)

        self.factoryNotebook = Pmw.NoteBook(factoryPage, tabpos = None)
        # Point page #
        factoryPointPage = self.factoryNotebook.add('Point')
        # Z spin page #
        zSpinPage = self.factoryNotebook.add('Z Spin')
        self.createAngleDial(zSpinPage, 'Initial angle',
                             'Starting angle in degrees',
                             command = self.setFactoryZSpinInitialAngle)
        self.createAngleDial(zSpinPage, 'Final angle',
                             'Final angle in degrees',
                             command = self.setFactoryZSpinFinalAngle)
        self.createAngleDial(zSpinPage, 'Angle spread',
                             'Spread of the final angle',
                             command = self.setFactoryZSpinAngleSpread)
        # Oriented page #
        orientedPage = self.factoryNotebook.add('Oriented')
        Label(orientedPage, text = 'Not implemented').pack(expand = 1,
                                                           fill = BOTH)
        self.factoryNotebook.pack(expand = 1, fill = BOTH)

        ## EMITTER PAGE ##
        self.createOptionMenu(emitterPage, 'Emitter type:',
                              'Select type of particle emitter',
			      ('Box', 'Disc', 'Line', 'Point', 'Rectangle',
                               'Ring', 'Sphere Volume', 'Sphere Surface',
                               'Tangent Ring'),
                              self.selectEmitterType)
        
        self.emitterNotebook = Pmw.NoteBook(emitterPage, tabpos = None)
        # Box page #
        boxPage = self.emitterNotebook.add('Box')
        self.createVector3Entry(boxPage, 'Point 1',
                               'Point defining emitter box',
                                command = self.setEmitterBoxPoint1)
        self.createVector3Entry(boxPage, 'Point 2',
                               'Point defining emitter box',
                                command = self.setEmitterBoxPoint2,
                                initialValue = (1.0, 1.0, 1.0))
        self.createVector3Entry(boxPage, 'Velocity vector',
                               'Initial particle velocity vector',
                                command = self.setEmitterBoxVelocityVector)
        # Disc page #
        discPage = self.emitterNotebook.add('Disc')
        self.createFloater(discPage, 'Radius', 'Radius of disc',
                           command = self.setEmitterDiscRadius)
        self.createAngleDial(discPage, 'Inner angle',
                             'Particle launch angle at center of disc',
                             command = self.setEmitterDiscInnerAngle)
        self.createFloater(discPage, 'Inner velocity',
                           'Launch velocity multiplier at center of disc',
                           command = self.setEmitterDiscInnerVelocity)
        self.createAngleDial(discPage, 'Outer angle',
                             'Particle launch angle at outer edge of disc',
                             command = self.setEmitterDiscOuterAngle)
        self.createFloater(discPage, 'Outer velocity',
                           'Launch velocity multiplier at edge of disc',
                           command = self.setEmitterDiscOuterVelocity)
        self.emitterDiscCubicLerping = self.createCheckbutton(
            discPage, 'Cubic Lerping',
            self.toggleEmitterDiscCubicLerping, 0)
        # Line page #
        linePage = self.emitterNotebook.add('Line')
        self.createVector3Entry(linePage, 'Point 1',
                               'Point defining emitter line',
                                command = self.setEmitterLinePoint1)
        self.createVector3Entry(linePage, 'Point 2',
                               'Point defining emitter line',
                                command = self.setEmitterLinePoint2,
                                initialValue = (1.0, 0.0, 0.0))
        self.createVector3Entry(linePage, 'Velocity Vector',
                               'Initial particle velocity vector',
                                command = self.setEmitterLineVelocityVector,
                                initialValue = (0.0, 0.0, 1.0))
        # Point page #
        emitterPointPage = self.emitterNotebook.add('Point')
        self.createVector3Entry(emitterPointPage, 'Position',
                               'Position of emitter point',
                                command = self.setEmitterPointPosition)
        self.createVector3Entry(emitterPointPage, 'Velocity vector',
                               'Initial particle velocity vector',
                                command = self.setEmitterPointVelocityVector,
                                initialValue = (0.0, 0.0, 1.0))
        # Rectangle #
        rectanglePage = self.emitterNotebook.add('Rectangle')
        self.createVector3Entry(rectanglePage, 'Point 1',
                               'Point defining rectangle',
                                command = self.setEmitterRectanglePoint1)
        self.createVector3Entry(rectanglePage, 'Point 2',
                               'Point defining rectangle',
                                command = self.setEmitterRectanglePoint2)
        self.createVector3Entry(
            rectanglePage, 'Velocity vector',
            'Initial particle velocity vector',
            command = self.setEmitterRectangleVelocityVector,
            initialValue = (0.0, 0.0, 1.0))
        # Ring #
        ringPage = self.emitterNotebook.add('Ring')
        self.createFloater(ringPage, 'Radius', 'Radius of ring',
                           command = self.setEmitterRingRadius)
        self.createAngleDial(ringPage, 'Angle', 'Particle launch angle',
                             command = self.setEmitterRingLaunchAngle)
        self.createFloater(ringPage, 'Magnitude',
                           'Launch velocity multiplier at outer edge of ring',
                           command = self.setEmitterRingVelocityMultiplier)
        # Sphere volume #
        sphereVolumePage = self.emitterNotebook.add('Sphere Volume')
        self.createFloater(sphereVolumePage, 'Radius',
                           'Radius of sphere',
                           command = self.setEmitterSphereVolumeRadius)
        # Sphere surface #
        sphereSurfacePage = self.emitterNotebook.add('Sphere Surface')
        self.createFloater(sphereSurfacePage, 'Radius',
                           'Radius of sphere',
                           command = self.setEmitterSphereSurfaceRadius)
        # Tangent ring # 
        tangentRingPage = self.emitterNotebook.add('Tangent Ring')
        self.createFloater(tangentRingPage, 'Radius',
                           'Radius of ring',
                           command = self.setEmitterTangentRingRadius)
        self.emitterNotebook.pack(fill = X)

        ## RENDERER PAGE ##
        self.createOptionMenu(rendererPage, 'Renderer type:',
                              'Select type of particle renderer',
                              ('Line', 'Geom', 'Point', 'Sparkle', 'Sprite'),
                              self.selectRendererType)
        self.rendererNotebook = Pmw.NoteBook(rendererPage, tabpos = None)
	# Line page #
	linePage = self.rendererNotebook.add('Line')
	self.createColorEntry(linePage, 'Head color',
				'Head color of line',
				command = self.setRendererLineHeadColor)
	self.createColorEntry(linePage, 'Tail color',
				'Tail color of line',
				command = self.setRendererLineTailColor)
        # Geom page #
        geomPage = self.rendererNotebook.add('Geom')
        f = Frame(geomPage)
        f.pack(fill = X)
        Label(f, width = 12, text = 'Geom node:').pack(side = LEFT)
        self.rendererGeomNode = StringVar()
        self.rendererGeomNodeEntry = Entry(
            f, width = 12,
            textvariable = self.rendererGeomNode)
        self.rendererGeomNodeEntry.bind('<Return>', self.setRendererGeomNode)
        self.rendererGeomNodeEntry.pack(side = LEFT, expand = 1, fill = X)
        # Point #
        rendererPointPage = self.rendererNotebook.add('Point')
        self.createFloater(rendererPointPage, 'Point size',
                           'Width and height of points in pixels',
                           command = self.setRendererPointSize)
        self.createColorEntry(rendererPointPage, 'Start color',
                               'Starting color of point',
                              command = self.setRendererPointStartColor)
        self.createColorEntry(rendererPointPage, 'End color',
                               'Ending color of point',
                              command = self.setRendererPointEndColor)
        self.createOptionMenu(rendererPointPage, 'Blend type:',
                              'Type of color blending used for particle',
                              ('ONE_COLOR', 'BLEND_LIFE', 'BLEND_VEL'),
                              self.rendererPointSelectBlendType)
        self.createOptionMenu(rendererPointPage, 'Blend method:',
                              'Interpolation method between colors',
                              ('LINEAR', 'CUBIC'),
                              self.rendererPointSelectBlendMethod)
        # Sparkle #
        sparklePage = self.rendererNotebook.add('Sparkle')
        self.createColorEntry(sparklePage, 'Center color',
                              'Color of sparkle center',
                              command = self.setRendererSparkleCenterColor)
        self.createColorEntry(sparklePage, 'Edge color',
                              'Color of sparkle line endpoints',
                              command = self.setRendererSparkleEdgeColor)
        self.createFloater(sparklePage, 'Birth radius',
                           'Initial sparkle radius',
                           command = self.setRendererSparkleBirthRadius)
        self.createFloater(sparklePage, 'Death radius',
                           'Final sparkle radius',
                           command = self.setRendererSparkleDeathRadius)
        self.createOptionMenu(sparklePage, 'Life scale:',
                              'Does particle scale over its lifetime?',
                              ('NO_SCALE', 'SCALE'),
                              self.setRendererSparkleLifeScale)
        # Sprite #
        spritePage = self.rendererNotebook.add('Sprite')
        f = Frame(spritePage)
        f.pack(fill = X)
        Label(f, width = 12, text = 'Texture:').pack(side = LEFT)
        self.rendererSpriteTexture = StringVar()
        self.rendererSpriteTextureEntry = Entry(
            f, width = 12,
            textvariable = self.rendererSpriteTexture)
        self.rendererSpriteTextureEntry.bind(
            '<Return>', self.setRendererSpriteTexture)
        self.rendererSpriteTextureEntry.pack(side = LEFT, expand = 1, fill = X)

        self.rendererSpriteXScale = self.createCheckbutton(
            spritePage, 'X Scale',
            self.toggleRendererSpriteXScale, 0)
        self.rendererSpriteYScale = self.createCheckbutton(
            spritePage, 'Y Scale',
            self.toggleRendererSpriteYScale, 0)
        self.rendererSpriteAnimAngle = self.createCheckbutton(
            spritePage, 'Anim Angle',
            self.toggleRendererSpriteAnimAngle, 0)
        self.createFloater(spritePage, 'Initial X Scale',
                           'Initial X scaling factor',
                           command = self.setRendererSpriteInitialXScale)
        self.createFloater(spritePage, 'Final X Scale',
                           'Final X scaling factor',
                           command = self.setRendererSpriteFinalXScale)
        self.createFloater(spritePage, 'Initial Y Scale',
                           'Initial Y scaling factor',
                           command = self.setRendererSpriteInitialYScale)
        self.createFloater(spritePage, 'Final Y Scale',
                           'Final Y scaling factor',
                           command = self.setRendererSpriteFinalYScale)
        self.createAngleDial(spritePage, 'Non Animated Theta',
                             'Counter clockwise Z rotation of all sprites',
                             command = self.setRendererSpriteNonAnimatedTheta)
        self.createOptionMenu(spritePage, 'Blend Type',
                              'Interpolation blend type for X and Y scaling',
                              ('LINEAR', 'CUBIC'),
                              self.setRendererSpriteBlendMethod)
        self.rendererSpriteAlphaDisable = self.createCheckbutton(
            spritePage, 'alphaDisable',
            self.toggleRendererSpriteAlphaDisable, 0)
        self.rendererNotebook.pack(fill = X)
        
        self.factoryNotebook.setnaturalsize()
        self.emitterNotebook.setnaturalsize()
        self.rendererNotebook.setnaturalsize()
        notebook.setnaturalsize()
        
        # Make sure input variables processed 
        self.initialiseoptions(ParticlePanel)

    ### WIDGET UTILITY FUNCTIONS ###
    def createCheckbutton(self, parent, text, command, initialState):
        bool = BooleanVar()
        bool.set(initialState)
        cb = Checkbutton(parent, text = text, anchor = W,
                         variable = bool, command = command)
        cb.pack(fill = X)
        return bool
        
    def createFloaters(self, parent, widgetDefinitions):
        widgets = []
        for label, balloonHelp, command, min, resolution in widgetDefinitions:
            widgets.append(
                self.createFloater(parent, label, balloonHelp,
                                   command, min, resolution)
                )
        return widgets

    def createFloater(self, parent, text, balloonHelp, command = None,
                      min = 0.0, resolution = None, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['initialValue'] = min
        kw['resolution'] = resolution
        widget = apply(Floater.Floater, (parent,), kw)
        widget.command = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict['text'] = widget
        return widget

    def createAngleDial(self, parent, text, balloonHelp, **kw):
        kw['text'] = text
        widget = apply(Dial.AngleDial,(parent,), kw)
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        return widget

    def createVector3Entry(self, parent, text, balloonHelp, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(VectorWidgets.Vector3Entry, (parent,), kw)
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        return widget

    def createColorEntry(self, parent, text, balloonHelp, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(VectorWidgets.ColorEntry, (parent,) ,kw)
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        return widget

    def createOptionMenu(self, parent, text, balloonHelp, items, command):
        optionVar = StringVar()
        optionVar.set(items[0])
        widget = Pmw.OptionMenu(parent, labelpos = W, label_text = text,
                                label_width = 12, menu_tearoff = 1,
                                menubutton_textvariable = optionVar,
                                items = items,
                                command = command)
        widget.pack(fill = X)
        self.bind(widget.component('menubutton'), balloonHelp)
        return optionVar

    ### PARTICLE SYSTEM COMMANDS ###
    ## System Page ##
    def setSystemPoolSize(self, value):
	self.particles.setPoolSize(value)
    def setSystemBirthRate(self, value):
	self.particles.setBirthRate(value)
    def setSystemLitterSize(self, value):
	self.particles.setLitterSize(value)
    def setSystemLitterSpread(self, value):
	self.particles.setLitterSpread(value)
    def setSystemLifespan(self, value):
	self.particles.setSystemLifespan(value)
    def toggleSystemLocalVelocity(self):
	self.particles.setLocalVelocityFlag(self.systemLocalVelocity.get())
    def toggleSystemGrowsOlder(self):
	self.particles.setSystemGrowsOlderFlag(self.systemGrowsOlder.get())
    def setSystemPos(self, pos):
	self.particles.getNodePath().setPos(Vec3(pos[0], pos[1], pos[2]))
    def setSystemHpr(self, pos):
	self.particles.getNodePath().setHpr(Vec3(pos[0], pos[1], pos[2]))

    ## Factory Page ##
    def selectFactoryType(self, type):
        self.factoryNotebook.selectpage(type)
	self.particles.setFactory(type)
    def setFactoryLifeSpan(self, value):
	self.particles.factory.setLifeSpanBase(value)
    def setFactoryLifeSpanSpread(self, value):
	self.particles.factory.setLifeSpanSpread(value)
    def setFactoryParticleMass(self, value):
	self.particles.factory.setMassBase(value)
    def setFactoryParticleMassSpread(self, value):
	self.particles.factory.setMassSpread(value)
    def setFactoryTerminalVelocity(self, value):
	self.particles.factory.setTerminalVelocityBase(value)
    def setFactoryTerminalVelocitySpread(self, value):
	self.particles.factory.setTerminalVelocitySpread(value)
    # Point Page #
    # Z Spin Page #
    def setFactoryZSpinInitialAngle(self, angle):
	self.particles.factory.setInitialAngle(angle)
    def setFactoryZSpinFinalAngle(self, angle):
	self.particles.factory.setFinaleAngle(angle)
    def setFactoryZSpinAngleSpread(self, spread):
	self.particles.factory.setInitialAngleSpread(spread)

    ## Emitter page ##
    def selectEmitterType(self, type):
        self.emitterNotebook.selectpage(type)
	self.particles.setEmitter(type)
    # Box #
    def setEmitterBoxPoint1(self, point):
	self.particles.emitter.setMinBound(Vec3(point[0], point[1], point[2]))
    def setEmitterBoxPoint2(self, point):
	self.particles.emitter.setMaxBound(Vec3(point[0], point[1], point[2]))
    def setEmitterBoxVelocityVector(self, vec):
        print 'Emitter box velocity vector:', vec
    # Disc #
    def setEmitterDiscRadius(self, radius):
	self.particles.emitter.setRadius(radius)
    def setEmitterDiscInnerAngle(self, angle):
	self.particles.emitter.setInnerAngle(angle)
    def setEmitterDiscInnerVelocity(self, velocity):
        print 'Emitter disc inner velocity:', velocity
    def setEmitterDiscOuterAngle(self, angle):
	self.particles.emitter.setOuterAngle(angle)
    def setEmitterDiscOuterVelocity(self, velocity):
        print 'Emitter disc outer velocity:', velocity
    def toggleEmitterDiscCubicLerping(self):
	self.particles.emitter.setCubicLerping(self.emitterDiscCubicLerping.get())
    # Line #
    def setEmitterLinePoint1(self, point):
	self.particles.emitter.setEndpoint1(Vec3(point[0], point[1], point[2]))
    def setEmitterLinePoint2(self, point):
	self.particles.emitter.setEndpoint2(Vec3(point[0], point[1], point[2]))
    def setEmitterLineVelocityVector(self, vec):
        print 'Emitter line velocity vector:', vec
    # Point #
    def setEmitterPointPosition(self, pos):
	self.particles.emitter.setLocation(Vec3(pos[0], pos[1], pos[2]))
    def setEmitterPointVelocityVector(self, velocity):
        print 'Emitter point velocity:', velocity
    # Rectangle #
    def setEmitterRectanglePoint1(self, point):
	self.particles.emitter.setMinBound(Vec3(point[0], point[1], point[2]))
    def setEmitterRectanglePoint2(self, point):
	self.particles.emitter.setMaxBound(Vec3(point[0], point[1], point[2]))
    def setEmitterRectangleVelocityVector(self, vec):
        print 'Emitter rectangle velocity vector:', vec
    # Ring #
    def setEmitterRingRadius(self, radius):
	self.particles.emitter.setRadius(radius)
    def setEmitterRingLaunchAngle(self, angle):
	self.particles.emitter.setAngle(angle)
    def setEmitterRingVelocityMultiplier(self, multiplier):
        print 'Emitter ring velocity multiplier:', multiplier
    # Sphere surface #
    def setEmitterSphereSurfaceRadius(self, radius):
	self.particles.emitter.setRadius(radius)
    # Sphere volume #
    def setEmitterSphereVolumeRadius(self, radius):
	self.particles.emitter.setRadius(radius)
    # Tangent ring #
    def setEmitterTangentRingRadius(self, radius):
	self.particles.emitter.setRadius(radius)

    ## RENDERER PAGE ##
    def selectRendererType(self, type):
        self.rendererNotebook.selectpage(type)
	self.particles.setRenderer(type)
    # Line #
    def setRendererLineHeadColor(self, color):
	self.particles.renderer.setHeadColor(Vec4(color[0], color[1], color[2], color[3]))
    def setRendererLineTailColor(self, color):
	self.particles.renderer.setTailColor(Vec4(color[0], color[1], color[2], color[3]))
    # Geom #
    def setRendererGeomNode(self, event):
	self.particles.renderer.setGeomNode(self.rendererGeomNode.get())
    # Point #
    def setRendererPointSize(self, size):
	self.particles.renderer.setPointSize(size)
    def setRendererPointStartColor(self, color):
	self.particles.renderer.setStartColor(Vec4(color[0], color[1], color[2], color[3]))
    def setRendererPointEndColor(self, color):
	self.particles.renderer.setEndColor(Vec4(color[0], color[1], color[2], color[3]))
    def rendererPointSelectBlendType(self, type):
	self.particles.renderer.setBlendType(type)
    def rendererPointSelectBlendMethod(self, method):
	self.particles.renderer.setBlendMethod(method)
    # Sparkle #
    def setRendererSparkleCenterColor(self, color):
	self.particles.renderer.setCenterColor(Vec4(color[0], color[1], color[2], color[3]))
    def setRendererSparkleEdgeColor(self, color):
	self.particles.renderer.setEdgeColor(Vec4(color[0], color[1], color[2], color[3]))
    def setRendererSparkleBirthRadius(self, radius):
	self.particles.renderer.setBirthRadius(radius)
    def setRendererSparkleDeathRadius(self, radius):
	self.particles.renderer.setDeathRadius(radius)
    def setRendererSparkleLifeScale(self, method):
	self.particles.renderer.setLifeScale(method)
    # Sprite #
    def setRendererSpriteTexture(self, event):
	self.particles.renderer.setTexture(self.rendererSpriteTexture.get())
    def toggleRendererSpriteXScale(self):
	self.particles.renderer.setXScaleFlag(self.rendererSpriteXScale.get())
    def toggleRendererSpriteYScale(self):
	self.particles.renderer.setYScaleFlag(self.rendererSpriteYScale.get())
    def toggleRendererSpriteAnimAngle(self):
	self.particles.renderer.setAnimAngleFlag(self.rendererSpriteAnimAngle.get())
    def setRendererSpriteInitialXScale(self, xScale):
	self.particles.renderer.setInitialXScale(xScale)
    def setRendererSpriteFinalXScale(self, xScale):
	self.particles.renderer.setFinalXScale(xScale)
    def setRendererSpriteInitialYScale(self, yScale):
	self.particles.renderer.setInitialYScale(yScale)
    def setRendererSpriteFinalYScale(self, yScale):
	self.particles.renderer.setFinalYScale(yScale)
    def setRendererSpriteNonAnimatedTheta(self, theta):
	self.particles.renderer.setNonanimatedTheta(theta)
    def setRendererSpriteBlendMethod(self, method):
	self.particles.renderer.setAlphaBlendMethod(method)
    def toggleRendererSpriteAlphaDisable(self):
	self.particles.renderer.setAlphaDisable(self.rendererSpriteAlphaDisable.get())
        
    def selectSystemNamed(self, name):
        print name


######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = Pmw.initialise()
    pp = ParticlePanel()
    #ve = VectorEntry(Toplevel(), relief = GROOVE)
    #ve.pack()
