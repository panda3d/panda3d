"""PANDA3D Particle Panel"""

# Import Tkinter, Pmw, and the floater code from this directory tree.
from AppShell import *
from Tkinter import *
import Pmw
import Dial
import Floater
import VectorWidgets
import Placer

class ParticlePanel(AppShell):
    # Override class variables
    appname = 'Particle Panel'
    frameWidth  = 300
    frameHeight = 600
    usecommandarea = 0
    usestatusarea  = 0
    
    def __init__(self, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',     self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

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
                                      'Particle system position',
                                      command = self.setSystemPos)
        pos.addMenuItem('Popup Placer Panel', Placer.Placer)
        hpr = self.createVector3Entry(systemPage, 'Hpr',
                                     'Particle system orientation',
                                      fGroup_labels = ('H', 'P', 'R'),
                                      command = self.setSystemHpr)
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
                               'Ring', 'Sphere Surface', 'Sphere Volume',
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
        # Sphere surface #
        sphereSurfacePage = self.emitterNotebook.add('Sphere Surface')
        self.createFloater(sphereSurfacePage, 'Radius',
                           'Radius of sphere',
                           command = self.setEmitterSphereSurfaceRadius)
        # Sphere volume #
        sphereVolumePage = self.emitterNotebook.add('Sphere Volume')
        self.createFloater(sphereVolumePage, 'Radius',
                           'Radius of sphere',
                           command = self.setEmitterSphereVolumeRadius)
        # Tangent ring # 
        tangentRingPage = self.emitterNotebook.add('Tangent Ring')
        self.createFloater(tangentRingPage, 'Radius',
                           'Radius of ring',
                           command = self.setEmitterTangentRingRadius)
        self.emitterNotebook.pack(fill = X)

        ## RENDERER PAGE ##
        self.createOptionMenu(rendererPage, 'Renderer type:',
                              'Select type of particle renderer',
                              ('Geom', 'Point', 'Sparkle', 'Sprite'),
                              self.selectRendererType)
        self.rendererNotebook = Pmw.NoteBook(rendererPage, tabpos = None)
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
                              self.setRendererSparkleBlendMethod)
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
        kw['command'] = command
        widget = apply(Floater.Floater, (parent,), kw)
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
        print 'Pool size:', value
    def setSystemBirthRate(self, value):
        print 'Birth rate:', value
    def setSystemLitterSize(self, value):
        print 'Litter size:', value
    def setSystemLitterSpread(self, value):
        print 'Litter spread:', value
    def setSystemLifespan(self, value):
        print 'System lifespan:', value
    def toggleSystemLocalVelocity(self):
        print 'Local velocity is ',
        if self.systemLocalVelocity.get():
            print 'on'
        else:
            print 'off'
    def toggleSystemGrowsOlder(self):
        print 'System Grows Older is ',
        if self.systemGrowsOlder.get():
            print 'on'
        else:
            print 'off'
    def setSystemPos(self, pos):
        print 'System pos:', pos
    def setSystemHpr(self, pos):
        print 'System hpr:', pos

    ## Factory Page ##
    def selectFactoryType(self, type):
        self.factoryNotebook.selectpage(type)
        print 'Factory type:', type
    def setFactoryLifeSpan(self, value):
        print 'Factory Life span:', value
    def setFactoryLifeSpanSpread(self, value):
        print 'Factory Life span spread:', value
    def setFactoryParticleMass(self, value):
        print 'Factory Particle mass:', value
    def setFactoryParticleMassSpread(self, value):
        print 'Factory Particle mass spread:', value
    def setFactoryTerminalVelocity(self, value):
        print 'Factory Terminal velocity:', value
    def setFactoryTerminalVelocitySpread(self, value):
        print 'Factory Terminal velocity spread:', value
    # Point Page #
    # Z Spin Page #
    def setFactoryZSpinInitialAngle(self, angle):
        print 'Factor Z Spin initial angle:', angle
    def setFactoryZSpinFinalAngle(self, angle):
        print 'Factory Z Spin final angle:', angle
    def setFactoryZSpinAngleSpread(self, spread):
        print 'Factory Z Spin angle spread:', spread

    ## Emitter page ##
    def selectEmitterType(self, type):
        self.emitterNotebook.selectpage(type)
        print 'Emitter type:', type
    # Box #
    def setEmitterBoxPoint1(self, point):
        print 'Emitter box point 1:', point
    def setEmitterBoxPoint2(self, point):
        print 'Emitter box point 2:', point
    def setEmitterBoxVelocityVector(self, vec):
        print 'Emitter box velocity vector:', vec
    # Disc #
    def setEmitterDiscRadius(self, radius):
        print 'Emitter disc radius:', radius
    def setEmitterDiscInnerAngle(self, angle):
        print 'Emitter disc inner angle:', angle
    def setEmitterDiscInnerVelocity(self, velocity):
        print 'Emitter disc inner velocity:', velocity
    def setEmitterDiscOuterAngle(self, angle):
        print 'Emitter disc outer angle:', angle
    def setEmitterDiscOuterVelocity(self, velocity):
        print 'Emitter disc outer velocity:', velocity
    def toggleEmitterDiscCubicLerping(self):
        print 'Emitter disc cubic lerping is ', 
        if self.emitterDiscCubicLerping.get():
            print 'on'
        else:
            print 'off'
    # Line #
    def setEmitterLinePoint1(self, point):
        print 'Emitter line point 1:', point
    def setEmitterLinePoint2(self, point):
        print 'Emitter line point 2:', point
    def setEmitterLineVelocityVector(self, vec):
        print 'Emitter line velocity vector:', vec
    # Point #
    def setEmitterPointPosition(self, pos):
        print 'Emitter point position:', pos
    def setEmitterPointVelocityVector(self, velocity):
        print 'Emitter point velocity:', velocity
    # Rectangle #
    def setEmitterRectanglePoint1(self, point):
        print 'Emitter rectangle point 1:', point
    def setEmitterRectanglePoint2(self, point):
        print 'Emitter rectangle point 2:', point
    def setEmitterRectangleVelocityVector(self, vec):
        print 'Emitter rectangle velocity vector:', vec
    # Ring #
    def setEmitterRingRadius(self, radius):
        print 'Emitter ring radius:', radius
    def setEmitterRingLaunchAngle(self, angle):
        print 'Emitter ring launch angle:', angle
    def setEmitterRingVelocityMultiplier(self, multiplier):
        print 'Emitter ring velocity multiplier:', multiplier
    # Sphere surface #
    def setEmitterSphereSurfaceRadius(self, radius):
        print 'Emitter sphere surface radius:', radius
    # Sphere volume #
    def setEmitterSphereVolumeRadius(self, radius):
        print 'Emitter sphere volume radius:', radius
    # Tangent ring #
    def setEmitterTangentRingRadius(self, radius):
        print 'Emitter tangent ring radius:', radius

    ## RENDERER PAGE ##
    def selectRendererType(self, type):
        self.rendererNotebook.selectpage(type)
    # Geom #
    def setRendererGeomNode(self, event):
        print 'Renderer Geom node:', self.rendererGeomNode.get()
    # Point #
    def setRendererPointSize(self, size):
        print 'Renderer Point size:', size
    def setRendererPointStartColor(self, color):
        print 'Renderer Point start color:', color
    def setRendererPointEndColor(self, color):
        print 'Renderer Point end color:', color
    def rendererPointSelectBlendType(self, type):
        print 'Renderer Point blend type:', type
    def rendererPointSelectBlendMethod(self, method):
        print 'Renderer Point blend method:', method
    # Sparkle #
    def setRendererSparkleCenterColor(self, color):
        print 'Renderer Sparkle center color:', color
    def setRendererSparkleEdgeColor(self, color):
        print 'Renderer Sparkle edge color:', color
    def setRendererSparkleBirthRadius(self, radius):
        print 'Renderer Sparkle birth radius:', radius
    def setRendererSparkleDeathRadius(self, radius):
        print 'Renderer Sparkle death radius:', radius
    def setRendererSparkleBlendMethod(self, method):
        print 'Renderer Sparkle blend method:', method
    # Sprite #
    def setRendererSpriteTexture(self, event):
        print 'Sprite texture:', self.rendererSpriteTexture.get()
    def toggleRendererSpriteXScale(self):
        print 'Renderer Sprite x scale is ', 
        if self.rendererSpriteXScale.get():
            print 'on'
        else:
            print 'off'
    def toggleRendererSpriteYScale(self):
        print 'Renderer Sprite y scale is ', 
        if self.rendererSpriteYScale.get():
            print 'on'
        else:
            print 'off'
    def toggleRendererSpriteAnimAngle(self):
        print 'Renderer Sprite anim angle is ', 
        if self.rendererSpriteAnimAngle.get():
            print 'on'
        else:
            print 'off'
    def setRendererSpriteInitialXScale(self, xScale):
        print 'Renderer Sprite initial x scale:', xScale
    def setRendererSpriteFinalXScale(self, xScale):
        print 'Renderer Sprite final x scale:', xScale
    def setRendererSpriteInitialYScale(self, yScale):
        print 'Renderer Sprite initial y scale:', yScale
    def setRendererSpriteFinalYScale(self, yScale):
        print 'Renderer Sprite final y scale:', yScale
    def setRendererSpriteNonAnimatedTheta(self, theta):
        print 'Renderer Sprite non animated theta:', theta
    def setRendererSpriteBlendMethod(self, method):
        print 'Renderer Sprite blend method:', method
    def toggleRendererSpriteAlphaDisable(self):
        print 'Renderer Sprite alpha disable is ', 
        if self.rendererSpriteAlphaDisable.get():
            print 'on'
        else:
            print 'off'
        
    def selectSystemNamed(self, name):
        print name


######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = Pmw.initialise()
    pp = ParticlePanel()
    #ve = VectorEntry(Toplevel(), relief = GROOVE)
    #ve.pack()
