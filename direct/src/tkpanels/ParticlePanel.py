"""PANDA3D Particle Panel"""

# Import Tkinter, Pmw, and the floater code from this directory tree.
from AppShell import *
from Tkinter import *
import Pmw
import Dial
import Floater
import EntryScale
import VectorWidgets
import Placer
import Particles
import ParticleEffect

class ParticlePanel(AppShell):
    # Override class variables
    appname = 'Particle Panel'
    frameWidth  = 400
    frameHeight = 660
    usecommandarea = 0
    usestatusarea  = 0
    balloonState = 'both'
    
    def __init__(self, particleEffect = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',     self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        # Record particle effect
        if particleEffect != None:
            self.particleEffect = particleEffect
        else:
            # Or create a new one if none given
            self.particleEffect = ParticleEffect.ParticleEffect(name)

        # Initialize application specific info
        AppShell.__init__(self)

        # Initialize panel Pmw options
        self.initialiseoptions(ParticlePanel)

        # Update panel values to reflect particle effect's state
        self.selectEffectNamed(self.effectDict.keys()[0])

    def appInit(self):
        self.widgetDict = {}
        self.variableDict = {}
        self.effectDict = {}
        self.effectDict[self.particleEffect.getName()] = (
            self.particleEffect)

    def createInterface(self):
        # Handle to the toplevels hull
        interior = self.interior()

        self.menuBar.addmenu('Particles', 'Particle Panel Operations')
        self.menuBar.addmenuitem(
            'Particles', 'command',
            'Print Particle System Parameters',
            label = 'Print Params',
            command = lambda s = self: s.particles.printParams())

        # Combo box to switch between particle systems
        self.effectSelector = Pmw.ComboBox(self.menuFrame,
                                     labelpos = W,
                                     label_text = 'Particle System',
                                     entry_width = 16,
                                     selectioncommand = self.selectEffectNamed,
                                     scrolledlist_items = ('system 0',))
        self.effectSelector.selectitem('system 0')
        self.effectSelector.pack(side = 'left', expand = 0)

        self.createCheckbutton(
            self.menuFrame, 'System', 'Active',
            'Turn particle systems on/off',
            self.toggleParticleSystem, 1)

        # Create the notebook pages
        self.mainNotebook = Pmw.NoteBook(interior)
        self.mainNotebook.pack(fill = BOTH, expand = 1)
        systemPage = self.mainNotebook.add('System')
        factoryPage = self.mainNotebook.add('Factory')
        emitterPage = self.mainNotebook.add('Emitter')
        rendererPage = self.mainNotebook.add('Renderer')
        forcePage = self.mainNotebook.add('Force')
        # Put this here so it isn't called right away
        self.mainNotebook['raisecommand'] = self.updateInfo

        ## SYSTEM PAGE ##
        # Create system floaters
        systemFloaterDefs = (
            ('System', 'Pool Size',
             'Max number of simultaneous particles',
             self.setSystemPoolSize,
             1.0, 1.0),
            ('System', 'Birth Rate',
             'Seconds between particle births',
             self.setSystemBirthRate,
             0.0, None),
            ('System', 'Litter Size',
             'Number of particle created at each birth',
             self.setSystemLitterSize,
             1.0, 1.0),
            ('System', 'Litter Spread',
             'Variation in litter size',
             self.setSystemLitterSpread,
             0.0, 1.0),
            ('System', 'Lifespan',
             'Age in seconds at which the system (vs. particles) should die',
             self.setSystemLifespan,
             0.0, None)
            )
        self.createFloaters(systemPage, systemFloaterDefs)
        # Checkboxes
        # Note: Sense is reversed on this one
        self.createCheckbutton(
            systemPage, 'System', 'Render Relative Velocities',
            ('On: velocities are in render space; ' +
             'Off: velocities are in particle local space'),
            self.toggleSystemLocalVelocity, 0)
        self.createCheckbutton(
            systemPage, 'System', 'Grows Older',
            'On: system has a lifespan',
            self.toggleSystemGrowsOlder, 0)
        # Vector widgets
        pos = self.createVector3Entry(systemPage, 'System', 'Pos',
                                      'Particle system position',
                                      command = self.setSystemPos)
        pos.addMenuItem('Popup Placer Panel', Placer.Placer)
        hpr = self.createVector3Entry(systemPage, 'System', 'Hpr',
                                     'Particle system orientation',
                                      fGroup_labels = ('H', 'P', 'R'),
                                      command = self.setSystemHpr)
        hpr.addMenuItem('Popup Placer Panel', Placer.Placer)

        ## FACTORY PAGE ##
        self.createOptionMenu(
            factoryPage,
            'Factory', 'Factory Type',
            'Select type of particle factory',
            ('PointParticleFactory', 'ZSpinParticleFactory',
             'OrientedParticleFactory'),
            self.selectFactoryType)
        factoryWidgets = (
            ('Factory', 'Life Span',
             'Average particle lifespan in seconds',
             self.setFactoryLifeSpan,
             0.0, None),
            ('Factory', 'Life Span Spread',
             'Variation in lifespan',
             self.setFactoryLifeSpanSpread,
             0.0, None),
            ('Factory', 'Mass',
             'Average particle mass',
             self.setFactoryParticleMass,
             0.0, None),
            ('Factory', 'Mass Spread',
             'Variation in particle mass',
             self.setFactoryParticleMassSpread,
             0.0, None),
            ('Factory', 'Terminal Velocity',
             'Cap on average particle velocity',
             self.setFactoryTerminalVelocity,
             0.0, None),
            ('Factory', 'Terminal Vel. Spread',
             'Variation in terminal velocity',
             self.setFactoryTerminalVelocitySpread,
             0.0, None))
        self.createFloaters(factoryPage, factoryWidgets)

        self.factoryNotebook = Pmw.NoteBook(factoryPage, tabpos = None)
        # Point page #
        factoryPointPage = self.factoryNotebook.add('PointParticleFactory')
        # Z spin page #
        zSpinPage = self.factoryNotebook.add('ZSpinParticleFactory')
        self.createAngleDial(zSpinPage, 'Z Spin Factory', 'Initial Angle',
                             'Starting angle in degrees',
                             command = self.setFactoryZSpinInitialAngle)
        self.createAngleDial(
            zSpinPage, 'Z Spin Factory',
            'Initial Angle Spread',
            'Spread of the initial angle',
            command = self.setFactoryZSpinInitialAngleSpread)
        self.createAngleDial(zSpinPage, 'Z Spin Factory', 'Final Angle',
                             'Final angle in degrees',
                             command = self.setFactoryZSpinFinalAngle)
        self.createAngleDial(
            zSpinPage, 'Z Spin Factory',
            'Final Angle Spread',
            'Spread of the final angle',
            command = self.setFactoryZSpinFinalAngleSpread)
        # Oriented page #
        orientedPage = self.factoryNotebook.add('OrientedParticleFactory')
        Label(orientedPage, text = 'Not implemented').pack(expand = 1,
                                                           fill = BOTH)
        self.factoryNotebook.pack(expand = 1, fill = BOTH)

        ## EMITTER PAGE ##
        self.createOptionMenu(
            emitterPage, 'Emitter',
            'Emitter Type',
            'Select type of particle emitter',
            ('BoxEmitter', 'DiscEmitter', 'LineEmitter', 'PointEmitter',
             'RectangleEmitter', 'RingEmitter', 'SphereVolumeEmitter',
             'SphereSurfaceEmitter', 'TangentRingEmitter'),
            self.selectEmitterType)

        # Emitter modes
        self.emissionType = IntVar()
        self.emissionType.set(BaseParticleEmitter.ETRADIATE)
        emissionFrame = Frame(emitterPage)
        self.createRadiobutton(
            emissionFrame, 'left',
            'Emitter', 'Explicit Emission',
            ('particles are all emitted in parallel, direction is based ' +
             'on explicit velocity vector'),
            self.emissionType, BaseParticleEmitter.ETEXPLICIT,
            self.setEmissionType)
        self.createRadiobutton(
            emissionFrame, 'left',
            'Emitter', 'Radiate Emission',
            'particles are emitted away from a specific point',
            self.emissionType, BaseParticleEmitter.ETRADIATE,
            self.setEmissionType)
        self.createRadiobutton(
            emissionFrame, 'left',
            'Emitter', 'Custom Emission',
            ('particles are emitted with a velocity that ' +
             'is determined by the particular emitter'),
            self.emissionType, BaseParticleEmitter.ETCUSTOM,
            self.setEmissionType)
        emissionFrame.pack(fill = 'x', expand = 0)
        
        self.createFloater(
            emitterPage, 'Emitter', 'Velocity Multiplier',
            'launch velocity multiplier (all emission modes)',
            command = self.setEmitterAmplitude,
            min = None)
        
        self.createFloater(
            emitterPage, 'Emitter', 'Velocity Multiplier Spread',
            'spread for launch velocity multiplier (all emission modes)',
            command = self.setEmitterAmplitudeSpread)
        
        self.createVector3Entry(
            emitterPage, 'Emitter', 'Offset Velocity',
            'Velocity vector applied to all particles',
            command = self.setEmitterOffsetForce)
        
        self.createVector3Entry(
            emitterPage, 'Emitter', 'Explicit Velocity',
            'all particles launch with this velocity in Explicit mode',
            command = self.setEmitterExplicitLaunchVector)
        
        self.createVector3Entry(
            emitterPage, 'Emitter', 'Radiate Origin',
            'particles launch away from this point in Radiate mode',
            command = self.setEmitterRadiateOrigin)
        
        self.emitterNotebook = Pmw.NoteBook(emitterPage, tabpos = None)
        # Box page #
        boxPage = self.emitterNotebook.add('BoxEmitter')
        self.createVector3Entry(boxPage, 'Box Emitter', 'Min',
                                'Min point defining emitter box',
                                command = self.setEmitterBoxPoint1)
        self.createVector3Entry(boxPage, 'Box Emitter', 'Max',
                                'Max point defining emitter box',
                                command = self.setEmitterBoxPoint2,
                                initialValue = (1.0, 1.0, 1.0))
        # Disc page #
        discPage = self.emitterNotebook.add('DiscEmitter')
        self.createFloater(discPage, 'Disc Emitter', 'Radius',
                           'Radius of disc',
                           command = self.setEmitterDiscRadius,
                           min = 0.01)
        customPage = self.discCustomFrame = Frame(discPage)
        self.createAngleDial(customPage, 'Disc Emitter', 'Inner Angle',
                             'Particle launch angle at center of disc',
                             command = self.setEmitterDiscInnerAngle)
        self.createFloater(customPage, 'Disc Emitter', 'Inner Velocity',
                           'Launch velocity multiplier at center of disc',
                           command = self.setEmitterDiscInnerVelocity)
        self.createAngleDial(customPage, 'Disc Emitter', 'Outer Angle',
                             'Particle launch angle at outer edge of disc',
                             command = self.setEmitterDiscOuterAngle)
        self.createFloater(customPage, 'Disc Emitter', 'Outer Velocity',
                           'Launch velocity multiplier at edge of disc',
                           command = self.setEmitterDiscOuterVelocity)
        self.createCheckbutton(
            customPage, 'Disc Emitter', 'Cubic Lerping',
            'On: magnitude/angle interpolation from center',
            self.toggleEmitterDiscCubicLerping, 0)
        customPage.pack(fill = BOTH, expand = 1)
        
        # Line page #
        linePage = self.emitterNotebook.add('LineEmitter')
        self.createVector3Entry(linePage, 'Line Emitter', 'Min',
                                'Min point defining emitter line',
                                command = self.setEmitterLinePoint1)
        self.createVector3Entry(linePage, 'Line Emitter', 'Max',
                                'Max point defining emitter line',
                                command = self.setEmitterLinePoint2,
                                initialValue = (1.0, 0.0, 0.0))
        # Point page #
        emitterPointPage = self.emitterNotebook.add('PointEmitter')
        self.createVector3Entry(emitterPointPage, 'Point Emitter', 'Position',
                               'Position of emitter point',
                                command = self.setEmitterPointPosition)
        # Rectangle #
        rectanglePage = self.emitterNotebook.add('RectangleEmitter')
        self.createVector2Entry(rectanglePage,
                                'Rectangle Emitter', 'Min',
                               'Point defining rectangle',
                                command = self.setEmitterRectanglePoint1)
        self.createVector2Entry(rectanglePage,
                                'Rectangle Emitter', 'Max',
                               'Point defining rectangle',
                                command = self.setEmitterRectanglePoint2)
        # Ring #
        ringPage = self.emitterNotebook.add('RingEmitter')
        self.createFloater(ringPage, 'Ring Emitter', 'Radius',
                           'Radius of ring',
                           command = self.setEmitterRingRadius,
                           min = 0.01)
        self.ringCustomFrame = Frame(ringPage)
        self.createAngleDial(self.ringCustomFrame, 'Ring Emitter', 'Angle',
                             'Particle launch angle',
                             command = self.setEmitterRingLaunchAngle)
        self.ringCustomFrame.pack(fill = BOTH, expand = 1)
        
        # Sphere volume #
        sphereVolumePage = self.emitterNotebook.add('SphereVolumeEmitter')
        self.createFloater(sphereVolumePage, 'Sphere Volume Emitter', 'Radius',
                           'Radius of sphere',
                           command = self.setEmitterSphereVolumeRadius,
                           min = 0.01)
        # Sphere surface #
        sphereSurfacePage = self.emitterNotebook.add('SphereSurfaceEmitter')
        self.createFloater(sphereSurfacePage, 'Sphere Surface Emitter',
                           'Radius',
                           'Radius of sphere',
                           command = self.setEmitterSphereSurfaceRadius,
                           min = 0.01)
        # Tangent ring # 
        tangentRingPage = self.emitterNotebook.add('TangentRingEmitter')
        self.createFloater(tangentRingPage, 'Tangent Ring Emitter', 'Radius',
                           'Radius of ring',
                           command = self.setEmitterTangentRingRadius,
                           min = 0.01)
        self.emitterNotebook.pack(fill = X)

        ## RENDERER PAGE ##
        self.createOptionMenu(
            rendererPage, 'Renderer', 'Renderer Type',
            'Select type of particle renderer',
            ('LineParticleRenderer', 'GeomParticleRenderer',
             'PointParticleRenderer', 'SparkleParticleRenderer',
             'SpriteParticleRenderer'),
            self.selectRendererType)

        self.createOptionMenu(rendererPage,
                              'Renderer', 'Alpha Mode',
                              "alpha setting over particles' lifetime",
                              ('NO_ALPHA','ALPHA_OUT','ALPHA_IN','ALPHA_USER'),
                              self.setRendererAlphaMode)
        
        self.createEntryScale(
            rendererPage, 'Renderer', 'User Alpha',
            'alpha value for ALPHA_USER alpha mode',
            command = self.setRendererUserAlpha)
        
        self.rendererNotebook = Pmw.NoteBook(rendererPage, tabpos = None)
	# Line page #
	linePage = self.rendererNotebook.add('LineParticleRenderer')
	self.createColorEntry(linePage, 'Line Renderer', 'Head Color',
				'Head color of line',
				command = self.setRendererLineHeadColor)
	self.createColorEntry(linePage, 'Line Renderer', 'Tail Color',
				'Tail color of line',
				command = self.setRendererLineTailColor)
        # Geom page #
        geomPage = self.rendererNotebook.add('GeomParticleRenderer')
        f = Frame(geomPage)
        f.pack(fill = X)
        Label(f, width = 12, text = 'Geom Node').pack(side = LEFT)
        self.rendererGeomNode = StringVar()
        self.rendererGeomNodeEntry = Entry(
            f, width = 12,
            textvariable = self.rendererGeomNode)
        self.rendererGeomNodeEntry.bind('<Return>', self.setRendererGeomNode)
        self.rendererGeomNodeEntry.pack(side = LEFT, expand = 1, fill = X)
        # Point #
        rendererPointPage = self.rendererNotebook.add('PointParticleRenderer')
        self.createFloater(rendererPointPage, 'Point Renderer', 'Point Size',
                           'Width and height of points in pixels',
                           command = self.setRendererPointSize)
        self.createColorEntry(rendererPointPage, 'Point Renderer',
                              'Start Color',
                               'Starting color of point',
                              command = self.setRendererPointStartColor)
        self.createColorEntry(rendererPointPage, 'Point Renderer',
                              'End Color',
                               'Ending color of point',
                              command = self.setRendererPointEndColor)
        self.createOptionMenu(rendererPointPage, 'Point Renderer',
                              'Blend Type',
                              'Type of color blending used for particle',
                              ('PP_ONE_COLOR', 'PP_BLEND_LIFE',
                               'PP_BLEND_VEL'),
                              self.rendererPointSelectBlendType)
        self.createOptionMenu(rendererPointPage, 'Point Renderer',
                              'Blend Method',
                              'Interpolation method between colors',
                              ('PP_NO_BLEND', 'PP_BLEND_LINEAR',
                               'PP_BLEND_CUBIC'),
                              self.rendererPointSelectBlendMethod)
        # Sparkle #
        sparklePage = self.rendererNotebook.add('SparkleParticleRenderer')
        self.createColorEntry(sparklePage, 'Sparkle Renderer',
                              'Center Color',
                              'Color of sparkle center',
                              command = self.setRendererSparkleCenterColor)
        self.createColorEntry(sparklePage, 'Sparkle Renderer',
                              'Edge Color',
                              'Color of sparkle line endpoints',
                              command = self.setRendererSparkleEdgeColor)
        self.createFloater(sparklePage, 'Sparkle Renderer',
                           'Birth Radius',
                           'Initial sparkle radius',
                           command = self.setRendererSparkleBirthRadius)
        self.createFloater(sparklePage, 'Sparkle Renderer',
                           'Death Radius',
                           'Final sparkle radius',
                           command = self.setRendererSparkleDeathRadius)
        self.createOptionMenu(sparklePage,
                              'Sparkle Renderer', 'Life Scale',
                              'Does particle scale over its lifetime?',
                              ('SP_NO_SCALE', 'SP_SCALE'),
                              self.setRendererSparkleLifeScale)
        # Sprite #
        spritePage = self.rendererNotebook.add('SpriteParticleRenderer')
        f = Frame(spritePage)
        f.pack(fill = X)
        Label(f, width = 12, text = 'Texture').pack(side = LEFT)
        self.rendererSpriteTexture = StringVar()
        self.rendererSpriteTexture.set('phase_3/maps/eyes.jpg')
        self.rendererSpriteTextureEntry = Entry(
            f, width = 12,
            textvariable = self.rendererSpriteTexture)
        self.rendererSpriteTextureEntry.bind(
            '<Return>', self.setRendererSpriteTexture)
        self.rendererSpriteTextureEntry.pack(
            side = LEFT, expand = 1, fill = X)
        self.createCheckbutton(
            spritePage, 'Sprite Renderer', 'X Scale',
            ("On: x scale is interpolated over particle's life; " +
             "Off: stays as start_X_Scale"),
            self.toggleRendererSpriteXScale, 0)
        self.createCheckbutton(
            spritePage, 'Sprite Renderer', 'Y Scale',
            ("On: y scale is interpolated over particle's life; " +
             "Off: stays as start_Y_Scale"),
            self.toggleRendererSpriteYScale, 0)
        self.createCheckbutton(
            spritePage, 'Sprite Renderer', 'Anim Angle',
            ("On: particles that are set to spin on the Z axis will " +
             "spin appropriately"),
            self.toggleRendererSpriteAnimAngle, 0)
        self.createFloater(spritePage, 'Sprite Renderer',
                           'Initial X Scale',
                           'Initial X scaling factor',
                           command = self.setRendererSpriteInitialXScale)
        self.createFloater(spritePage, 'Sprite Renderer',
                           'Final X Scale',
                           'Final X scaling factor, if xScale enabled',
                           command = self.setRendererSpriteFinalXScale)
        self.createFloater(spritePage, 'Sprite Renderer',
                           'Initial Y Scale',
                           'Initial Y scaling factor',
                           command = self.setRendererSpriteInitialYScale)
        self.createFloater(spritePage, 'Sprite Renderer',
                           'Final Y Scale',
                           'Final Y scaling factor, if yScale enabled',
                           command = self.setRendererSpriteFinalYScale)
        self.createAngleDial(spritePage, 'Sprite Renderer',
                             'Non Animated Theta',
                             ('If animAngle is false: counter clockwise ' +
                              'Z rotation of all sprites'),
                             command = self.setRendererSpriteNonAnimatedTheta)
        self.createOptionMenu(spritePage, 'Sprite Renderer',
                              'Blend Type',
                              'Interpolation blend type for X and Y scaling',
                              ('PP_NO_BLEND', 'PP_LINEAR', 'PP_CUBIC'),
                              self.setRendererSpriteBlendMethod)
        self.createCheckbutton(
            spritePage, 'Sprite Renderer', 'Alpha Disable',
            'On: alpha blending is disabled',
            self.toggleRendererSpriteAlphaDisable, 0)
        self.rendererNotebook.pack(fill = X)

        ## FORCE PAGE ##
        # Menu to select between differnt Forces
        self.createOptionMenu(
            forcePage, 'Force',
            'Active Force',
            'Select force component', [],
            self.selectForceType)
        
        forceFrame = Frame(forcePage, borderwidth = 2, relief = 'sunken')
        self.forcesButton = Menubutton(forceFrame, text = 'Forces',
                                       font=('MSSansSerif', 14, 'bold'),
                                       activebackground = '#909090')
        forcesMenu = Menu(self.forcesButton)
        forcesMenu.add_command(label = 'Add Linear Cylinder Vortex Force',
                            command = self.addLinearCylinderVortexForce)
        forcesMenu.add_command(label = 'Add Linear Distance Force',
                            command = self.addLinearDistanceForce)
        forcesMenu.add_command(label = 'Add Linear Friction Force',
                            command = self.addLinearFrictionForce)
        forcesMenu.add_command(label = 'Add Linear Jitter Force',
                            command = self.addLinearJitterForce)
        forcesMenu.add_command(label = 'Add Linear Noise Force',
                            command = self.addLinearNoiseForce)
        forcesMenu.add_command(label = 'Add Linear Random Force',
                            command = self.addLinearRandomForce)
        forcesMenu.add_command(label = 'Add Linear Sink Force',
                            command = self.addLinearSinkForce)
        forcesMenu.add_command(label = 'Add Linear Source Force',
                            command = self.addLinearSourceForce)
        forcesMenu.add_command(label = 'Add Linear User Defined Force',
                            command = self.addLinearUserDefinedForce)
        forcesMenu.add_command(label = 'Add Linear Vector Force',
                            command = self.addLinearVectorForce)
        self.forcesButton.pack(expand = 0)
        self.forcesButton['menu'] = forcesMenu
        
        # Notebook pages for force specific controls
        self.forceNotebook = Pmw.NoteBook(forceFrame, tabpos = None,
                                          borderwidth = 0)
        # Put this here so it isn't called right away
        self.forceNotebook['raisecommand'] = self.updateForceWidgets

        # Widget to select a force to configure
        nameList = map(lambda x: x.getName(), self.particleEffect.getForces())
        forceMenuFrame = Frame(forceFrame)
        self.forceMenu = Pmw.ComboBox(
            forceMenuFrame, labelpos = W, label_text = 'Force:',
            entry_width = 20,
            selectioncommand = self.selectForceNamed,
            scrolledlist_items = nameList)
        self.forceMenu.pack(side = 'left', fill = 'x', expand = 0)
        self.bind(self.forceMenu, 'Select force to configure')
        
        self.forceActive = BooleanVar()
        self.forceActiveButton = Checkbutton(
            forceMenuFrame,
            text = 'On/Off',
            variable = self.forceActive,
            command = self.toggleActiveForce)
        self.forceActiveButton.pack(side = 'left', fill = 'x', expand = 0)

        # Pack force menu
        forceMenuFrame.pack(fill = 'x', expand = 0, padx = 2)

        self.forceNotebook.setnaturalsize()
        self.forceNotebook.pack(expand = 1, fill = BOTH)
        
        forceFrame.pack(expand = 1, fill = BOTH)

        
        self.factoryNotebook.setnaturalsize()
        self.emitterNotebook.setnaturalsize()
        self.rendererNotebook.setnaturalsize()
        self.mainNotebook.setnaturalsize()
        
        # Make sure input variables processed 
        self.initialiseoptions(ParticlePanel)

    ### WIDGET UTILITY FUNCTIONS ###
    def createCheckbutton(self, parent, category, text,
                          balloonHelp, command, initialState):
        bool = BooleanVar()
        bool.set(initialState)
        widget = Checkbutton(parent, text = text, anchor = W,
                         variable = bool)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        self.variableDict[category + '-' + text] = bool
        return widget
        
    def createRadiobutton(self, parent, side, category, text,
                          balloonHelp, variable, value,
                          command):
        widget = Radiobutton(parent, text = text, anchor = W,
                             variable = variable, value = value)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget
        
    def createFloaters(self, parent, widgetDefinitions):
        widgets = []
        for category, label, balloonHelp, command, min, resolution in widgetDefinitions:
            widgets.append(
                self.createFloater(parent, category, label, balloonHelp,
                                   command, min, resolution)
                )
        return widgets

    def createFloater(self, parent, category, text, balloonHelp,
                      command = None, min = 0.0, resolution = None,
                      maxVelocity = 10.0, **kw):
        kw['text'] = text
        kw['min'] = min
        if min != None:
            kw['initialValue'] = min
        else:
            kw['initialValue'] = 0.0
        kw['maxVelocity'] = maxVelocity
        kw['resolution'] = resolution
        widget = apply(Floater.Floater, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createAngleDial(self, parent, category, text, balloonHelp,
                        command = None, **kw):
        kw['text'] = text
        widget = apply(Dial.AngleDial,(parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createEntryScale(self, parent, category, text, balloonHelp,
                         command = None, min = 0.0, max = 1.0,
                         resolution = None, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['max'] = max
        kw['initialValue'] = min
        kw['resolution'] = resolution
        widget = apply(EntryScale.EntryScale, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createVector2Entry(self, parent, category, text, balloonHelp,
                           command = None, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(VectorWidgets.Vector2Entry, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createVector3Entry(self, parent, category, text, balloonHelp,
                           command = None, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(VectorWidgets.Vector3Entry, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createColorEntry(self, parent, category, text, balloonHelp,
                         command = None, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(VectorWidgets.ColorEntry, (parent,) ,kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createOptionMenu(self, parent, category, text, balloonHelp,
                         items, command):
        optionVar = StringVar()
        if len(items) > 0:
            optionVar.set(items[0])
        widget = Pmw.OptionMenu(parent, labelpos = W, label_text = text,
                                label_width = 12, menu_tearoff = 1,
                                menubutton_textvariable = optionVar,
                                items = items)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget.component('menubutton'), balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        self.variableDict[category + '-' + text] = optionVar
        return optionVar

    def createComboBox(self, parent, category, text, balloonHelp,
                         items, command):
        widget = Pmw.ComboBox(parent
                              labelpos = W,
                              label_text = text,
                              entry_width = 16,
                              scrolledlist_items = items)
        if len(items) > 0:
            widget.selectitem(items[0])
        widget['command'] = command
        widget.pack(side = 'left', expand = 0)
        self.bind(widget.component('menubutton'), balloonHelp)
        self.widgetDict[category + '-' + text] = optionVar
        return widget

    def getWidget(self, category, text):
        return self.widgetDict[category + '-' + text]

    def getVariable(self, category, text):
        return self.variableDict[category + '-' + text]

    ### PARTICLE SYSTEM COMMANDS ###
    def updateInfo(self, page = 'System'):
        if page == 'System':
            self.updateSystemWidgets()
        elif page == 'Factory':
            self.selectFactoryPage()
            self.updateFactoryWidgets()
        elif page == 'Emitter':
            self.selectEmitterPage()
            self.updateEmitterWidgets()
        elif page == 'Renderer':
            self.selectRendererPage()
            self.updateRendererWidgets()
        elif page == 'Force':
            self.selectForcePage()
            self.updateForceWidgets()

    def toggleParticleSystem(self):
        if self.getWidget('System', 'Active').get():
            self.particleEffect.enable()
        else:
            self.particleEffect.disable()
	return None
            
    ## SYSTEM PAGE ##
    def updateSystemWidgets(self):
        poolSize = self.particles.getPoolSize()
        self.getWidget('System', 'Pool Size').set(int(poolSize), 0)
        birthRate = self.particles.getBirthRate()
        self.getWidget('System', 'Birth Rate').set(birthRate, 0)
        litterSize = self.particles.getLitterSize()
        self.getWidget('System', 'Litter Size').set(int(litterSize), 0)
        litterSpread = self.particles.getLitterSpread()
        self.getWidget('System', 'Litter Spread').set(litterSpread, 0)
        systemLifespan = self.particles.getSystemLifespan()
        self.getWidget('System', 'Lifespan').set(systemLifespan, 0)
        pos = self.particles.nodePath.getPos()
        self.getWidget('System', 'Pos').set([pos[0], pos[1], pos[2]], 0)
        hpr = self.particles.nodePath.getHpr()
        self.getWidget('System', 'Hpr').set([hpr[0], hpr[1], hpr[2]], 0)
        self.getWidget('System', 'Render Relative Velocities').set(
            self.particles.getLocalVelocityFlag())
        self.getWidget('System', 'Grows Older').set(
            self.particles.getSystemGrowsOlderFlag())
    def setSystemPoolSize(self, value):
	self.particles.setPoolSize(int(value))
    def setSystemBirthRate(self, value):
	self.particles.setBirthRate(value)
    def setSystemLitterSize(self, value):
	self.particles.setLitterSize(int(value))
    def setSystemLitterSpread(self, value):
	self.particles.setLitterSpread(value)
    def setSystemLifespan(self, value):
	self.particles.setSystemLifespan(value)
    def toggleSystemLocalVelocity(self):
	self.particles.setLocalVelocityFlag(
            self.getWidget('System', 'Render Relative Velocities').get())
    def toggleSystemGrowsOlder(self):
	self.particles.setSystemGrowsOlderFlag(
            self.getWidget('System', 'Grows Older').get())
    def setSystemPos(self, pos):
	self.particles.nodePath.setPos(Vec3(pos[0], pos[1], pos[2]))
    def setSystemHpr(self, pos):
	self.particles.nodePath.setHpr(Vec3(pos[0], pos[1], pos[2]))

    ## FACTORY PAGE ##
    def selectFactoryType(self, type):
        self.factoryNotebook.selectpage(type)
	self.particles.setFactory(type)
        self.updateFactoryWidgets()

    def selectFactoryPage(self):
        pass
        
    def updateFactoryWidgets(self):
        factory = self.particles.factory
        lifespan = factory.getLifespanBase()
        self.getWidget('Factory', 'Life Span').set(lifespan, 0)
        lifespanSpread = factory.getLifespanSpread()
        self.getWidget('Factory', 'Life Span Spread').set(lifespanSpread, 0)
        mass = factory.getMassBase()
        self.getWidget('Factory', 'Life Span').set(mass, 0)
        massSpread = factory.getMassSpread()
        self.getWidget('Factory', 'Life Span Spread').set(massSpread, 0)
        terminalVelocity = factory.getTerminalVelocityBase()
        self.getWidget('Factory', 'Terminal Velocity').set(terminalVelocity, 0)
        terminalVelocitySpread = factory.getTerminalVelocitySpread()
        self.getWidget('Factory', 'Terminal Vel. Spread').set(
            terminalVelocitySpread, 0)
        
    def setFactoryLifeSpan(self, value):
	self.particles.factory.setLifespanBase(value)
    def setFactoryLifeSpanSpread(self, value):
	self.particles.factory.setLifespanSpread(value)
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
    def setFactoryZSpinInitialAngleSpread(self, spread):
	self.particles.factory.setInitialAngleSpread(spread)
    def setFactoryZSpinFinalAngle(self, angle):
	self.particles.factory.setFinaleAngle(angle)
    def setFactoryZSpinFinalAngleSpread(self, spread):
	self.particles.factory.setInitialAngleSpread(spread)

    ## EMITTER PAGE ##
    def selectEmitterType(self, type):
        self.emitterNotebook.selectpage(type)
	self.particles.setEmitter(type)
        self.updateEmitterWidgets()

    def selectEmitterPage(self):
        type = self.particles.emitter.__class__.__name__
        self.emitterNotebook.selectpage(type)
        self.getVariable('Emitter', 'Emitter Type').set(type)
        
    def updateEmitterWidgets(self):
        emitter = self.particles.emitter
        self.setEmissionType(self.particles.emitter.getEmissionType())
        amp = emitter.getAmplitude()
        self.getWidget('Emitter', 'Velocity Multiplier').set(amp)
        spread = emitter.getAmplitudeSpread()
        self.getWidget('Emitter', 'Velocity Multiplier Spread').set(spread)
        vec = emitter.getOffsetForce()
        self.getWidget('Emitter', 'Offset Velocity').set(
            [vec[0], vec[1], vec[2]], 0)
        vec = emitter.getRadiateOrigin()
        self.getWidget('Emitter', 'Radiate Origin').set(
            [vec[0], vec[1], vec[2]], 0)
        vec = emitter.getExplicitLaunchVector()
        self.getWidget('Emitter', 'Explicit Velocity').set(
            [vec[0], vec[1], vec[2]], 0)
        if isinstance(emitter, BoxEmitter):
            min = emitter.getMinBound()
            self.getWidget('Box Emitter', 'Min').set(
                [min[0], min[1], min[2]], 0)
            max = emitter.getMaxBound()
            self.getWidget('Box Emitter', 'Max').set(
                [max[0], max[1], max[2]], 0)
        elif isinstance(emitter, DiscEmitter):
            radius = emitter.getRadius()
            self.getWidget('Disc Emitter', 'Radius').set(radius, 0)
            innerAngle = emitter.getInnerAngle()
            self.getWidget('Disc Emitter', 'Inner Angle').set(innerAngle, 0)
            innerMagnitude = emitter.getInnerMagnitude()
            self.getWidget('Disc Emitter', 'Inner Velocity').set(
                innerMagnitude, 0)
            outerAngle = emitter.getOuterAngle()
            self.getWidget('Disc Emitter', 'Outer Angle').set(outerAngle, 0)
            outerMagnitude = emitter.getOuterMagnitude()
            self.getWidget('Disc Emitter', 'Inner Velocity').set(
                outerMagnitude, 0)
            cubicLerping = emitter.getCubicLerping()
            self.getWidget('Disc Emitter', 'Cubic Lerping').set(cubicLerping)
        elif isinstance(emitter, LineEmitter):
            min = emitter.getEndpoint1()
            self.getWidget('Line Emitter', 'Min').set(
                [min[0], min[1], min[2]], 0)
            max = emitter.getEndpoint2()
            self.getWidget('Line Emitter', 'Max').set(
                [max[0], max[1], max[2]], 0)
        elif isinstance(emitter, PointEmitter):
            location = emitter.getLocation()
            self.getWidget('Point Emitter', 'Position').set(
                [location[0], location[1], location[2]], 0)
        elif isinstance(emitter, RectangleEmitter):
            min = emitter.getMinBound()
            self.getWidget('Rectangle Emitter', 'Min').set(
                [min[0], min[1]], 0)
            max = emitter.getMaxBound()
            self.getWidget('Rectangle Emitter', 'Max').set(
                [max[0], max[1]], 0)
        elif isinstance(emitter, RingEmitter):
            radius = emitter.getRadius()
            self.getWidget('Ring Emitter', 'Radius').set(radius, 0)
            angle = emitter.getAngle()
            self.getWidget('Ring Emitter', 'Angle').set(angle, 0)
        elif isinstance(emitter, SphereVolumeEmitter):
            radius = emitter.getRadius()
            self.getWidget('Sphere Volume Emitter', 'Radius').set(radius, 0)
        elif isinstance(emitter, SphereSurfaceEmitter):
            radius = emitter.getRadius()
            self.getWidget('Sphere Surface Emitter', 'Radius').set(radius, 0)
        elif isinstance(emitter, TangentRingEmitter):
            radius = emitter.getRadius()
            self.getWidget('Tangent Ring Emitter', 'Radius').set(radius, 0)
    # All #
    def setEmissionType(self, newType = None):
        if newType:
            type = newType
            self.emissionType.set(type)
        else:
            type = self.emissionType.get()
        self.particles.emitter.setEmissionType(type)
        if type == BaseParticleEmitter.ETEXPLICIT:
            self.getWidget(
                'Emitter', 'Radiate Origin')['state'] = 'disabled'
            self.getWidget(
                'Emitter', 'Explicit Velocity')['state'] = 'normal'
            # Hide custom widgets
            if isinstance(self.particles.emitter, DiscEmitter):
                self.discCustomFrame.pack_forget()
            elif isinstance(self.particles.emitter, RingEmitter):
                self.ringCustomFrame.pack_forget()
        elif type == BaseParticleEmitter.ETRADIATE:
            self.getWidget(
                'Emitter', 'Radiate Origin')['state'] = 'normal'
            self.getWidget(
                'Emitter', 'Explicit Velocity')['state'] = 'disabled'
            # Hide custom widgets
            if isinstance(self.particles.emitter, DiscEmitter):
                self.discCustomFrame.pack_forget()
            elif isinstance(self.particles.emitter, RingEmitter):
                self.ringCustomFrame.pack_forget()
        elif type == BaseParticleEmitter.ETCUSTOM:
            self.getWidget(
                'Emitter', 'Radiate Origin')['state'] = 'disabled'
            self.getWidget(
                'Emitter', 'Explicit Velocity')['state'] = 'disabled'
            # Show custom widgets
            if isinstance(self.particles.emitter, DiscEmitter):
                self.discCustomFrame.pack(fill = BOTH, expand = 1)
            elif isinstance(self.particles.emitter, RingEmitter):
                self.ringCustomFrame.pack(fill = BOTH, expand = 1)

    def setEmitterAmplitude(self, value):
        self.particles.emitter.setAmplitude(value)

    def setEmitterAmplitudeSpread(self, value):
        self.particles.emitter.setAmplitudeSpread(value)

    def setEmitterOffsetForce(self, vec):
        self.particles.emitter.setOffsetForce(
            Vec3(vec[0], vec[1], vec[2]))

    def setEmitterRadiateOrigin(self, origin):
        self.particles.emitter.setRadiateOrigin(
            Point3(origin[0], origin[1], origin[2]))

    def setEmitterExplicitLaunchVector(self, vec):
        self.particles.emitter.setExplicitLaunchVector(
            Vec3(vec[0], vec[1], vec[2]))

    # Box #
    def setEmitterBoxPoint1(self, point):
	self.particles.emitter.setMinBound(Point3(point[0],
                                                  point[1],
                                                  point[2]))
    def setEmitterBoxPoint2(self, point):
	self.particles.emitter.setMaxBound(Point3(point[0],
                                                  point[1],
                                                  point[2]))
    # Disc #
    def setEmitterDiscRadius(self, radius):
	self.particles.emitter.setRadius(radius)
    def setEmitterDiscInnerAngle(self, angle):
	self.particles.emitter.setInnerAngle(angle)
    def setEmitterDiscInnerVelocity(self, velocity):
        self.particles.emitter.setInnerMagnitude(velocity)
    def setEmitterDiscOuterAngle(self, angle):
	self.particles.emitter.setOuterAngle(angle)
    def setEmitterDiscOuterVelocity(self, velocity):
        self.particles.emitter.setOuterMagnitude(velocity)
    def toggleEmitterDiscCubicLerping(self):
	self.particles.emitter.setCubicLerping(
            self.emitterDiscCubicLerping.get())
    # Line #
    def setEmitterLinePoint1(self, point):
	self.particles.emitter.setEndpoint1(Point3(point[0],
                                                   point[1],
                                                   point[2]))
    def setEmitterLinePoint2(self, point):
	self.particles.emitter.setEndpoint2(Point3(point[0],
                                                   point[1],
                                                   point[2]))
    # Point #
    def setEmitterPointPosition(self, pos):
	self.particles.emitter.setLocation(Point3(pos[0], pos[1], pos[2]))
    # Rectangle #
    def setEmitterRectanglePoint1(self, point):
	self.particles.emitter.setMinBound(Point2(point[0], point[1]))
    def setEmitterRectanglePoint2(self, point):
	self.particles.emitter.setMaxBound(Point2(point[0], point[1]))
    # Ring #
    def setEmitterRingRadius(self, radius):
	self.particles.emitter.setRadius(radius)
    def setEmitterRingLaunchAngle(self, angle):
	self.particles.emitter.setAngle(angle)
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
        self.updateRendererWidgets()
        
    def updateRendererWidgets(self):
        renderer = self.particles.renderer
        alphaMode = renderer.getAlphaMode()
        if alphaMode == BaseParticleRenderer.PRALPHANONE:
            aMode = 'NO_ALPHA'
        elif alphaMode == BaseParticleRenderer.PRALPHAOUT:
            aMode = 'ALPHA_OUT'
        elif alphaMode == BaseParticleRenderer.PRALPHAIN:
            aMode = 'ALPHA_IN'
        elif alphaMode == BaseParticleRenderer.PRALPHAUSER:
            aMode = 'ALPHA_USER'
        self.getWidget('Renderer', 'Alpha Mode').set(aMode)
        userAlpha = renderer.getUserAlpha()
        self.getWidget('Renderer', 'User Alpha').set(userAlpha)
        if isinstance(renderer, LineParticleRenderer):
            headColor = renderer.getHeadColor() * 255.0
            self.getWidget('Line Renderer', 'Head Color').set(
                [headColor[0], headColor[1], headColor[2], headColor[3]])
            tailColor = renderer.getTailColor() * 255.0
            self.getWidget('Line Renderer', 'Tail Color').set(
                [tailColor[0], tailColor[1], tailColor[2], tailColor[3]])
        elif isinstance(renderer, GeomParticleRenderer):
            pass
        elif isinstance(renderer, PointParticleRenderer):
            pointSize = renderer.getPointSize()
            self.getWidget('Point Renderer', 'Point Size').set(pointSize)
            startColor = renderer.getStartColor() * 255.0
            self.getWidget('Point Renderer', 'Start Color').set(
                [startColor[0], startColor[1], startColor[2], startColor[3]])
            endColor = renderer.getEndColor() * 255.0
            self.getWidget('Point Renderer', 'End Color').set(
                [endColor[0], endColor[1], endColor[2], endColor[3]])
            blendType = renderer.getBlendType()
	    if (blendType == PointParticleRenderer.PPONECOLOR):
	    	bType = "PP_ONE_COLOR"
	    elif (blendType == PointParticleRenderer.PPBLENDLIFE):
		bType = "PP_BLEND_LIFE"	
	    elif (blendType == PointParticleRenderer.PPBLENDVEL):
		bType = "PP_BLEND_VEL"	
            self.getWidget('Point Renderer', 'Blend Type').set(bType)
            blendMethod = renderer.getBlendMethod()
	    bMethod = "PP_NO_BLEND"
	    if (blendMethod == BaseParticleRenderer.PPNOBLEND):
		bMethod = "PP_NO_BLEND"
	    elif (blendMethod == BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PP_BLEND_LINEAR"
	    elif (blendMethod == BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PP_BLEND_CUBIC"
            self.getWidget('Point Renderer', 'Blend Method').set(bMethod)
        elif isinstance(renderer, SparkleParticleRenderer):
            centerColor = renderer.getCenterColor() * 255.0
            self.getWidget('Sparkle Renderer', 'Center Color').set(
                [centerColor[0], centerColor[1],
                 centerColor[2], centerColor[3]])
            edgeColor = renderer.getEdgeColor() * 255.0
            self.getWidget('Sparkle Renderer', 'Edge Color').set(
                [edgeColor[0], edgeColor[1], edgeColor[2], edgeColor[3]])
            birthRadius = renderer.getBirthRadius()
            self.getWidget('Sparkle Renderer', 'Birth Radius').set(birthRadius)
            deathRadius = renderer.getDeathRadius()
            self.getWidget('Sparkle Renderer', 'Death Radius').set(deathRadius)
            lifeScale = renderer.getLifeScale()
	    lScale = "SP_NO_SCALE"
	    if (lifeScale == SparkleParticleRenderer.SPSCALE):
		lScale = "SP_SCALE"
            self.getWidget('Sparkle Renderer', 'Life Scale').set(lScale)
        elif isinstance(renderer, SpriteParticleRenderer):
            color = renderer.getColor() * 255.0
            texture = renderer.getTexture()
	    if (texture != None):
		self.rendererSpriteTexture = texture.getName()
            self.getWidget('Sprite Renderer', 'X Scale').set(
                renderer.getXScaleFlag())
            self.getWidget('Sprite Renderer', 'Y Scale').set(
                renderer.getYScaleFlag())
            self.getWidget('Sprite Renderer', 'Anim Angle').set(
                renderer.getAnimAngleFlag())
            initialXScale = renderer.getInitialXScale()
            self.getWidget('Sprite Renderer', 'Initial X Scale').set(
                initialXScale)
            initialYScale = renderer.getInitialYScale()
            self.getWidget('Sprite Renderer', 'Initial Y Scale').set(
                initialYScale)
            finalXScale = renderer.getFinalXScale()
            self.getWidget('Sprite Renderer', 'Final X Scale').set(
                finalXScale)
            finalYScale = renderer.getFinalYScale()
            self.getWidget('Sprite Renderer', 'Final Y Scale').set(
                finalYScale)
            nonanimatedTheta = renderer.getNonanimatedTheta()
            self.getWidget('Sprite Renderer', 'Non Animated Theta').set(
                nonanimatedTheta)
            blendMethod = renderer.getAlphaBlendMethod()
	    bMethod = "PP_NO_BLEND"
	    if (blendMethod == BaseParticleRenderer.PPNOBLEND):
		bMethod = "PP_NO_BLEND"
	    elif (blendMethod == BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PP_BLEND_LINEAR"
	    elif (blendMethod == BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PP_BLEND_CUBIC"
            self.getWidget('Sprite Renderer', 'Alpha Disable').set(
                renderer.getAlphaDisable())

    def selectRendererPage(self):
        type = self.particles.renderer.__class__.__name__
        self.rendererNotebook.selectpage(type)
        self.getVariable('Renderer', 'Renderer Type').set(type)

    # All #
    def setRendererAlphaMode(self, alphaMode):
        if alphaMode == 'NO_ALPHA':
            aMode = BaseParticleRenderer.PRALPHANONE
        elif alphaMode == 'ALPHA_OUT':
            aMode = BaseParticleRenderer.PRALPHAOUT
        elif alphaMode == 'ALPHA_IN':
            aMode = BaseParticleRenderer.PRALPHAIN
        elif alphaMode == 'ALPHA_USER':
            aMode = BaseParticleRenderer.PRALPHAUSER
        self.particles.renderer.setAlphaMode(aMode)

    def setRendererUserAlpha(self, alpha):
        self.particles.renderer.setUserAlpha(alpha)

    # Line #
    def setRendererLineHeadColor(self, color):
	self.particles.renderer.setHeadColor(
            Vec4(color[0]/255.0, color[1]/255.0,
                 color[2]/255.0, color[3]/255.0))
    def setRendererLineTailColor(self, color):
	self.particles.renderer.setTailColor(
            Vec4(color[0]/255.0, color[1]/255.0,
                 color[2]/255.0, color[3]/255.0))
    # Geom #
    def setRendererGeomNode(self, event):
	node = loader.loadOnce(self.rendererGeomNode.get())
	if (node == None):
	    return None
	self.particles.renderer.setGeomNode(node)
    # Point #
    def setRendererPointSize(self, size):
	self.particles.renderer.setPointSize(size)
    def setRendererPointStartColor(self, color):
	self.particles.renderer.setStartColor(
            Vec4(color[0]/255.0, color[1]/255.0,
                 color[2]/255.0, color[3]/255.0))
    def setRendererPointEndColor(self, color):
	self.particles.renderer.setEndColor(
            Vec4(color[0]/255.0, color[1]/255.0,
                 color[2]/255.0, color[3]/255.0))
    def rendererPointSelectBlendType(self, blendType):
        if blendType == "PP_ONE_COLOR":
            bType = PointParticleRenderer.PPONECOLOR
        elif blendType == "PP_BLEND_LIFE":
            bType = PointParticleRenderer.PPBLENDLIFE
        elif blendType == "PP_BLEND_VEL":
            bType = PointParticleRenderer.PPBLENDVEL
	self.particles.renderer.setBlendType(bType)
    def rendererPointSelectBlendMethod(self, blendMethod):
        if blendMethod == "PP_NO_BLEND":
            bMethod = BaseParticleRenderer.PPNOBLEND
        elif blendMethod == "PP_BLEND_LINEAR":
            bMethod = BaseParticleRenderer.PPBLENDLINEAR
        elif blendMethod == "PP_BLEND_CUBIC":
            bMethod = BaseParticleRenderer.PPBLENDCUBIC
	self.particles.renderer.setBlendMethod(bMethod)
    # Sparkle #
    def setRendererSparkleCenterColor(self, color):
	self.particles.renderer.setCenterColor(
            Vec4(color[0]/255.0, color[1]/255.0,
                 color[2]/255.0, color[3]/255.0))
    def setRendererSparkleEdgeColor(self, color):
	self.particles.renderer.setEdgeColor(
            Vec4(color[0]/255.0, color[1]/255.0,
                 color[2]/255.0, color[3]/255.0))
    def setRendererSparkleBirthRadius(self, radius):
	self.particles.renderer.setBirthRadius(radius)
    def setRendererSparkleDeathRadius(self, radius):
	self.particles.renderer.setDeathRadius(radius)
    def setRendererSparkleLifeScale(self, lifeScaleMethod):
        if lifeScaleMethod == 'SP_NO_SCALE':
            lScale = SparkleParticleRenderer.SPNOSCALE
        else:
            lScale = SparkleParticleRenderer.SPSCALE
	self.particles.renderer.setLifeScale(lScale)
    # Sprite #
    def setRendererSpriteTexture(self, event):
	t = loader.loadTexture(self.rendererSpriteTexture.get())
	if (t != None):
	    self.particles.renderer.setTexture(t)
	else:
	    print "Couldn't find rendererSpriteTexture"
    def toggleRendererSpriteXScale(self):
	self.particles.renderer.setXScaleFlag(
            self.getWidget('Sprite Renderer', 'X Scale').get())
    def toggleRendererSpriteYScale(self):
	self.particles.renderer.setYScaleFlag(
            self.getWidget('Sprite Renderer', 'Y Scale').get())
    def toggleRendererSpriteAnimAngle(self):
	self.particles.renderer.setAnimAngleFlag(
            self.getWidget('Sprite Renderer', 'Anim Angle').get())
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
    def setRendererSpriteBlendMethod(self, blendMethod):
        print blendMethod
        if blendMethod == 'PP_NO_BLEND':
            bMethod = BaseParticleRenderer.PPNOBLEND
        elif blendMethod == 'PP_BLEND_LINEAR':
            bMethod = BaseParticleRenderer.PPBLENDLINEAR
        elif blendMethod == 'PP_BLEND_CUBIC':
            bMethod = BaseParticleRenderer.PPBLENDCUBIC
        else:
            bMethod = BaseParticleRenderer.PPNOBLEND
	self.particles.renderer.setAlphaBlendMethod(bMethod)
    def toggleRendererSpriteAlphaDisable(self):
	self.particles.renderer.setAlphaDisable(
            self.getWidget('Sprite Renderer', 'Alpha Disable').get())
        
    ## FORCES COMMANDS ##
    def selectForceType(self, type):
        self.forceNotebook.selectpage(type)
        self.updateForceWidgets()

    def selectForcePage(self):
        if self.forces:
            type = self.forces.__class__.__name__
            self.forceNotebook.selectpage(type)
            self.getVariable('Emitter', 'Emitter Type').set(type)
        
    def updateForceWidgets(self):
        pass

    def addLinearCylinderVortexForce(self):
        f = LinearCylinderVortexForce()
        self.activeForce.addLinearForce(f)
    def addLinearDistanceForce(self):
        f = LinearDistanceForce()
        self.activeForce.addLinearForce(f)
    def addLinearFrictionForce(self):
        f = LinearFrictionForce()
        self.activeForce.addLinearForce(f)
    def addLinearJitterForce(self):
        f = LinearJitterForce()
        self.activeForce.addLinearForce(f)
    def addLinearNoiseForce(self):
        f = LinearNoiseForce()
        self.activeForce.addLinearForce(f)
    def addLinearRandomForce(self):
        f = LinearRandomForce()
        self.activeForce.addLinearForce(f)
    def addLinearSinkForce(self):
        f = LinearSingForce()
        self.activeForce.addLinearForce(f)
    def addLinearSourceForce(self):
        f = LinearSourceForce()
        self.activeForce.addLinearForce(f)
    def addLinearUserDefinedForce(self):
        f = LinearUserDefinedForce()
        self.activeForce.addLinearForce(f)
    def addLinearVectorForce(self):
        f = LinearVectorForce()
        self.activeForce.addLinearForce(f)

    ## SYSTEM COMMANDS ##
    def selectEffectNamed(self, name):
        effect = self.effectDict.get(name, None)
        if effect:
            self.particleEffect = effect
            # Default to first particle in particlesDict
            self.particles = self.particleEffect.getParticles()[0]
            # See if particle effect has any forces
            forcesList = self.particleEffect.getForces()
            if len(forcesList) > 0:
                self.forces = forcesList[0]
            else:
                self.forces = None
            self.mainNotebook.selectpage('System')
            self.updateInfo('System')
        else:
            print 'ParticlePanel: No effect named ' + name

    def createEffectNamed(self, name):
        effect = ParticleEffect.ParticleEffect(name)
        self.effectDict[name] = effect
        self.selectEffectNamed(name)
            

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = Pmw.initialise()
    pp = ParticlePanel()
    #ve = VectorEntry(Toplevel(), relief = GROOVE)
    #ve.pack()
