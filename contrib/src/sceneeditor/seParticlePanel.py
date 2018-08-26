"""PANDA3D Particle Panel"""

# Import Tkinter, Pmw, and the floater code from this directory tree.
from direct.tkwidgets.AppShell import AppShell
import os, Pmw, sys
from direct.tkwidgets.Dial import AngleDial
from direct.tkwidgets.Floater import Floater
from direct.tkwidgets.Slider import Slider
from direct.tkwidgets.VectorWidgets import Vector2Entry, Vector3Entry
from direct.tkwidgets.VectorWidgets import ColorEntry
import sePlacer
import seForceGroup
import seParticles
import seParticleEffect


if sys.version_info >= (3, 0):
    from tkinter.filedialog import *
    from tkinter.simpledialog import askstring
else:
    from tkFileDialog import *
    from tkSimpleDialog import askstring


class ParticlePanel(AppShell):
    # Override class variables
    appname = 'Particle Panel'
    frameWidth  = 375
    frameHeight = 575
    usecommandarea = 0
    usestatusarea  = 0
    balloonState = 'both'
    effectsDict={}
    def __init__(self, particleEffect = None, effectsDict={}, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',     self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        # Record particle effect
        if particleEffect != None:
            self.particleEffect = particleEffect
            self.effectsDict = effectsDict
        else:
            # Or create a new one if none given
            particles = seParticles.Particles()
            particles.setBirthRate(0.02)
            particles.setLitterSize(10)
            particles.setLitterSpread(0)
            particles.setFactory("PointParticleFactory")
            particles.setRenderer("PointParticleRenderer")
            particles.setEmitter("SphereVolumeEmitter")
            particles.enable()
            pe = seParticleEffect.ParticleEffect('effect1', particles)
            self.particleEffect = pe
            self.emitter=loader.loadModel("sphere")
            pe.reparentTo(self.emitter)
            self.emitter.setName("effect1")
            self.emitter.reparentTo(render)
            pe.enable()
            messenger.send('ParticlePanel_Added_Effect',['effect1',pe,self.emitter])
            self.effectsDict[self.particleEffect.getName()]=self.particleEffect


        messenger.send('SGE_Update Explorer',[render])

        # Initialize application specific info
        AppShell.__init__(self)

        # Initialize panel Pmw options
        self.initialiseoptions(ParticlePanel)
        # Update panel values to reflect particle effect's state
        self.selectEffectNamed(self.effectsDict.keys()[0])
        # Make sure labels/menus reflect current state
        self.updateMenusAndLabels()
        # Make sure there is a page for each forceGroup objects
        for forceGroup in self.particleEffect.getForceGroupList():
            self.addForceGroupNotebookPage(self.particleEffect, forceGroup)

    def appInit(self):
        # Create dictionaries to keep track of panel objects
        self.widgetDict = {}
        self.variableDict = {}
        self.forcePagesDict = {}
        # Make sure particles are enabled
        base.enableParticles()

    def onDestroy(self, event):
        messenger.send('ParticlePanle_close')
        return

    def createInterface(self):
        # Handle to the toplevels hull
        interior = self.interior()

        # Create particle panel menu items

        ## MENUBAR ENTRIES ##
        # FILE MENU
        # Get a handle on the file menu so commands can be inserted
        # before quit item
        fileMenu = self.menuBar.component('File-menu')
        # MRM: Need to add load and save effects methods
        fileMenu.insert_command(
            fileMenu.index('Quit'),
            label = 'Load Params',
            command = self.loadParticleEffectFromFile)
        fileMenu.insert_command(
            fileMenu.index('Quit'),
            label = 'Save Params',
            command = self.saveParticleEffectToFile)
        fileMenu.insert_command(
            fileMenu.index('Quit'),
            label = 'Print Params',
            command = lambda s = self: s.particles.printParams())

        # PARTICLE MANAGER MENU
        self.menuBar.addmenu('ParticleMgr', 'ParticleMgr Operations')
        self.particleMgrActive = IntVar()
        self.particleMgrActive.set(base.isParticleMgrEnabled())
        self.menuBar.addmenuitem(
            'ParticleMgr', 'checkbutton',
            'Enable/Disable ParticleMgr',
            label = 'Active',
            variable = self.particleMgrActive,
            command = self.toggleParticleMgr)

        ## MENUBUTTON LABELS ##
        # Menubutton/label to identify the current objects being configured
        labelFrame = Frame(interior)
        # Current effect
        self.effectsLabel = Menubutton(labelFrame, width = 10,
                                       relief = RAISED,
                                       borderwidth = 2,
                                       font=('MSSansSerif', 12, 'bold'),
                                       activebackground = '#909090')
        self.effectsLabelMenu = Menu(self.effectsLabel, tearoff = 0)
        self.effectsLabel['menu'] = self.effectsLabelMenu
        self.effectsLabel.pack(side = LEFT, fill = 'x', expand = 1)
        self.bind(self.effectsLabel,
                  'Select effect to configure or create new effect')
        self.effectsLabelMenu.add_command(label = 'Create New Effect',
                                          command = self.createNewEffect)
        self.effectsLabelMenu.add_command(
            label = 'Select Particle Effect',
            command = lambda s = self: SEditor.select(s.particleEffect))
        self.effectsLabelMenu.add_command(
            label = 'Place Particle Effect',
            command = lambda s = self: sePlacer.place(s.particleEffect))
        def togglePEVis(s = self):
            if s.particleEffect.isHidden():
                s.particleEffect.show()
            else:
                s.particleEffect.hide()
        self.effectsLabelMenu.add_command(
            label = 'Toggle Effect Vis',
            command = togglePEVis)
        self.effectsEnableMenu = Menu(self.effectsLabelMenu, tearoff = 0)
        self.effectsLabelMenu.add_cascade(label = 'Enable/Disable',
                                          menu = self.effectsEnableMenu)
        self.effectsLabelMenu.add_separator()
        # Current particles
        self.particlesLabel = Menubutton(labelFrame, width = 10,
                                         relief = RAISED,
                                         borderwidth = 2,
                                         font=('MSSansSerif', 12, 'bold'),
                                         activebackground = '#909090')
        self.particlesLabelMenu = Menu(self.particlesLabel, tearoff = 0)
        self.particlesLabel['menu'] = self.particlesLabelMenu
        self.particlesLabel.pack(side = LEFT, fill = 'x', expand = 1)
        self.bind(self.particlesLabel,
                  ('Select particles object to configure ' +
                   'or add new particles object to current effect' ))
        self.particlesLabelMenu.add_command(label = 'Create New Particles',
                                            command = self.createNewParticles)
        self.particlesEnableMenu = Menu(self.particlesLabelMenu, tearoff = 0)
        self.particlesLabelMenu.add_cascade(label = 'Enable/Disable',
                                            menu = self.particlesEnableMenu)
        self.particlesLabelMenu.add_separator()
        # Current force
        self.forceGroupLabel = Menubutton(labelFrame, width = 10,
                                      relief = RAISED,
                                      borderwidth = 2,
                                      font=('MSSansSerif', 12, 'bold'),
                                      activebackground = '#909090')
        self.forceGroupLabelMenu = Menu(self.forceGroupLabel, tearoff = 0)
        self.forceGroupLabel['menu'] = self.forceGroupLabelMenu
        self.forceGroupLabel.pack(side = LEFT, fill = 'x', expand = 1)
        self.bind(self.forceGroupLabel,
                  ('Select force group to configure ' +
                   'or add a new force group to current effect'))
        self.forceGroupLabelMenu.add_command(
            label = 'Create New ForceGroup',
            command = self.createNewForceGroup)
        self.forceGroupEnableMenu = Menu(self.forceGroupLabelMenu, tearoff = 0)
        self.forceGroupLabelMenu.add_cascade(label = 'Enable/Disable',
                                             menu = self.forceGroupEnableMenu)
        self.forceGroupLabelMenu.add_separator()
        # Pack labels
        labelFrame.pack(fill = 'x', expand = 0)

        # Create the toplevel notebook pages
        self.mainNotebook = Pmw.NoteBook(interior)
        self.mainNotebook.pack(fill = BOTH, expand = 1)
        systemPage = self.mainNotebook.add('System')
        factoryPage = self.mainNotebook.add('Factory')
        emitterPage = self.mainNotebook.add('Emitter')
        rendererPage = self.mainNotebook.add('Renderer')
        forcePage = self.mainNotebook.add('Force')
        # Put this here so it isn't called right away
        self.mainNotebook['raisecommand'] = self.updateInfo

        ## SYSTEM PAGE WIDGETS ##
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
        self.createCheckbutton(
            systemPage, 'System', 'Render Space Velocities',
            ('On: velocities are in render space; ' +
             'Off: velocities are in particle local space'),
            self.toggleSystemLocalVelocity, 0)
        self.createCheckbutton(
            systemPage, 'System', 'System Grows Older',
            'On: system has a lifespan',
            self.toggleSystemGrowsOlder, 0)

        # Vector widgets
        pos = self.createVector3Entry(systemPage, 'System', 'Pos',
                                      'Particle system position',
                                      command = self.setSystemPos)
        pos.addMenuItem('Popup Placer Panel', sePlacer.Placer)
        hpr = self.createVector3Entry(systemPage, 'System', 'Hpr',
                                     'Particle system orientation',
                                      fGroup_labels = ('H', 'P', 'R'),
                                      command = self.setSystemHpr)
        hpr.addMenuItem('Popup Placer Panel', sePlacer.Placer)

        ## FACTORY PAGE WIDGETS ##
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
             0.001, None),
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
             0.0, None),
        )
        self.createFloaters(factoryPage, factoryWidgets)

        self.factoryNotebook = Pmw.NoteBook(factoryPage, tabpos = None)
        # Point page #
        factoryPointPage = self.factoryNotebook.add('PointParticleFactory')
        # Z spin page #
        zSpinPage = self.factoryNotebook.add('ZSpinParticleFactory')

        self.createCheckbutton(
            zSpinPage, 'Z Spin Factory', 'Enable Angular Velocity',
            ("On: angular velocity is used; " +
             "Off: final angle is used"),
            self.toggleAngularVelocity, 0, side = TOP),

        self.createFloater(
            zSpinPage, 'Z Spin Factory', 'Angular Velocity',
             'How fast sprites rotate',
             command = self.setFactoryZSpinAngularVelocity)

        self.createFloater(
            zSpinPage, 'Z Spin Factory', 'Angular Velocity Spread',
             'Variation in how fast sprites rotate',
             command = self.setFactoryZSpinAngularVelocitySpread)

        self.createAngleDial(zSpinPage, 'Z Spin Factory', 'Initial Angle',
                             'Starting angle in degrees',
                             fRollover = 1,
                             command = self.setFactoryZSpinInitialAngle)
        self.createAngleDial(
            zSpinPage, 'Z Spin Factory',
            'Initial Angle Spread',
            'Spread of the initial angle',
            fRollover = 1,
            command = self.setFactoryZSpinInitialAngleSpread)
        self.createAngleDial(zSpinPage, 'Z Spin Factory', 'Final Angle',
                             'Final angle in degrees',
                             fRollover = 1,
                             command = self.setFactoryZSpinFinalAngle)
        self.createAngleDial(
            zSpinPage, 'Z Spin Factory',
            'Final Angle Spread',
            'Spread of the final angle',
            fRollover = 1,
            command = self.setFactoryZSpinFinalAngleSpread)
        # Oriented page #
        orientedPage = self.factoryNotebook.add('OrientedParticleFactory')
        Label(orientedPage, text = 'Not implemented').pack(expand = 1,
                                                           fill = BOTH)
        self.factoryNotebook.pack(expand = 1, fill = BOTH)

        ## EMITTER PAGE WIDGETS ##
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
                                value = (1.0, 1.0, 1.0))
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
                                value = (1.0, 0.0, 0.0))
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

        ## RENDERER PAGE WIDGETS ##
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

        self.createSlider(
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
        Label(f, width = 12, text = 'Texture Type:').pack(side = LEFT)
        self.rendererSpriteSourceType = IntVar()
        self.rendererSpriteSourceType.set(SpriteParticleRenderer.STTexture)
        self.rendererSpriteSTTexture = self.createRadiobutton(
            f, 'left',
            'Sprite Renderer', 'Texture Type',
            'Sprite particle renderer created from texture file',
            self.rendererSpriteSourceType, SpriteParticleRenderer.STTexture,
            self.setSpriteSourceType)
        self.rendererSpriteSTTexture = self.createRadiobutton(
            f, 'left',
            'Sprite Renderer', 'NodePath Type',
            'Sprite particle renderer created from node path',
            self.rendererSpriteSourceType, SpriteParticleRenderer.STFromNode,
            self.setSpriteSourceType)
        f.pack(fill = X)
        f = Frame(spritePage)
        Label(f, width = 6, text = 'Texture:').pack(side = LEFT)
        self.rendererSpriteTexture = StringVar()
        self.rendererSpriteTexture.set(SpriteParticleRenderer.sourceTextureName)
        self.rendererSpriteTextureEntry = Entry(
            f, width = 12,
            textvariable = self.rendererSpriteTexture)
        self.rendererSpriteTextureEntry.pack(side = LEFT, expand = 1, fill = X)
        f.pack(fill = X)
        f = Frame(spritePage)
        Label(f, width = 6, text = 'File:').pack(side = LEFT)
        self.rendererSpriteFile = StringVar()
        self.rendererSpriteFile.set(SpriteParticleRenderer.sourceFileName)
        self.rendererSpriteFileEntry = Entry(
            f, width = 12,
            textvariable = self.rendererSpriteFile)
        self.rendererSpriteFileEntry.pack(side = LEFT, expand = 1, fill = X)
        Label(f, width = 6, text = 'Node:').pack(side = LEFT)
        self.rendererSpriteNode = StringVar()
        self.rendererSpriteNode.set(SpriteParticleRenderer.sourceNodeName)
        self.rendererSpriteNodeEntry = Entry(
            f, width = 6,
            textvariable = self.rendererSpriteNode)
        self.rendererSpriteNodeEntry.pack(side = LEFT, expand = 1, fill = X)
        f.pack(fill = X)
        # Init entries
        self.setSpriteSourceType()
        self.setTextureButton = Button(spritePage, text = 'Set Texture',
                                       command = self.setRendererSpriteTexture)
        self.setTextureButton.pack(fill = X)
        f = Frame(spritePage)
        self.createCheckbutton(
            f, 'Sprite Renderer', 'X Scale',
            ("On: x scale is interpolated over particle's life; " +
             "Off: stays as start_X_Scale"),
            self.toggleRendererSpriteXScale, 0, side = LEFT)
        self.createCheckbutton(
            f, 'Sprite Renderer', 'Y Scale',
            ("On: y scale is interpolated over particle's life; " +
             "Off: stays as start_Y_Scale"),
            self.toggleRendererSpriteYScale, 0, side = LEFT)
        self.createCheckbutton(
            f, 'Sprite Renderer', 'Anim Angle',
            ("On: particles that are set to spin on the Z axis will " +
             "spin appropriately"),
            self.toggleRendererSpriteAnimAngle, 0, side = LEFT)
        f.pack(fill = X)
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

        ## FORCE PAGE WIDGETS ##
        self.addForceButton = Menubutton(forcePage, text = 'Add Force',
                                          relief = RAISED,
                                          borderwidth = 2,
                                          font=('MSSansSerif', 14, 'bold'),
                                          activebackground = '#909090')
        forceMenu = Menu(self.addForceButton)
        self.addForceButton['menu'] = forceMenu
        # DERIVED FROM LINEAR FORCE
        # This also has: setVector
        forceMenu.add_command(label = 'Add Linear Vector Force',
                            command = self.addLinearVectorForce)
        # Parameters: setAmplitude, setMassDependent, setVectorMasks
        forceMenu.add_command(label = 'Add Linear Noise Force',
                            command = self.addLinearNoiseForce)
        forceMenu.add_command(label = 'Add Linear Jitter Force',
                            command = self.addLinearJitterForce)
        # This also has setCoef
        forceMenu.add_command(label = 'Add Linear Friction Force',
                            command = self.addLinearFrictionForce)
        # This also has: setCoef, setLength, setRadius,
        forceMenu.add_command(label = 'Add Linear Cylinder Vortex Force',
                            command = self.addLinearCylinderVortexForce)

        # DERIVED FROM LINEAR DISTANCE FORCE
        # Parameters: setFalloffType, setForceCenter, setRadius
        forceMenu.add_command(label = 'Add Linear Sink Force',
                            command = self.addLinearSinkForce)
        forceMenu.add_command(label = 'Add Linear Source Force',
                            command = self.addLinearSourceForce)
        """
        # Avoid for now
        forceMenu.add_command(label = 'Add Linear User Defined Force',
                            command = self.addLinearUserDefinedForce)
        """

        self.addForceButton.pack(expand = 0)

        # Scrolled frame to hold force widgets
        self.sf = Pmw.ScrolledFrame(forcePage, horizflex = 'elastic')
        self.sf.pack(fill = 'both', expand = 1)
        self.forceFrame = self.sf.interior()
        # Notebook to hold force widgets as the are added
        self.forceGroupNotebook = Pmw.NoteBook(self.forceFrame, tabpos = None)
        self.forceGroupNotebook.pack(fill = X)

        self.factoryNotebook.setnaturalsize()
        self.emitterNotebook.setnaturalsize()
        self.rendererNotebook.setnaturalsize()
        self.forceGroupNotebook.setnaturalsize()
        self.mainNotebook.setnaturalsize()

        # Make sure input variables processed
        self.initialiseoptions(ParticlePanel)

    ### WIDGET UTILITY FUNCTIONS ###
    def createCheckbutton(self, parent, category, text,
                          balloonHelp, command, initialState, side = 'top'):
        bool = BooleanVar()
        bool.set(initialState)
        widget = Checkbutton(parent, text = text, anchor = W,
                         variable = bool)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X, side = side)
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
                      numDigits = 3, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['resolution'] = resolution
        kw['numDigits'] = numDigits
        widget = Floater(parent, **kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createAngleDial(self, parent, category, text, balloonHelp,
                        command = None, **kw):
        kw['text'] = text
        kw['style'] = 'mini'
        widget = AngleDial(parent, **kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createSlider(self, parent, category, text, balloonHelp,
                     command = None, min = 0.0, max = 1.0,
                     resolution = 0.001, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['max'] = max
        kw['resolution'] = resolution
        widget = Slider(parent, **kw)
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
        widget = Vector2Entry(parent, **kw)
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
        widget = Vector3Entry(parent, **kw)
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
        widget = ColorEntry(parent, **kw)
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
                         items, command, history = 0):
        widget = Pmw.ComboBox(parent,
                              labelpos = W,
                              label_text = text,
                              label_anchor = 'w',
                              label_width = 12,
                              entry_width = 16,
                              history = history,
                              scrolledlist_items = items)
        # Don't allow user to edit entryfield
        widget.configure(entryfield_entry_state = 'disabled')
        # Select first item if it exists
        if len(items) > 0:
            widget.selectitem(items[0])
        # Bind selection command
        widget['selectioncommand'] = command
        widget.pack(side = 'left', expand = 0)
        # Bind help
        self.bind(widget, balloonHelp)
        # Record widget
        self.widgetDict[category + '-' + text] = widget
        return widget

    def updateMenusAndLabels(self):
        self.updateMenus()
        self.updateLabels()

    def updateLabels(self):
        self.effectsLabel['text'] = self.particleEffect.getName()
        self.particlesLabel['text'] = self.particles.getName()
        if self.forceGroup != None:
            self.forceGroupLabel['text'] = self.forceGroup.getName()
        else:
            self.forceGroupLabel['text'] = 'Force Group'

    def updateMenus(self):
        self.updateEffectsMenus()
        self.updateParticlesMenus()
        self.updateForceGroupMenus()

    def updateEffectsMenus(self):
        # Get rid of old effects entries if any
        self.effectsEnableMenu.delete(0, 'end')
        self.effectsLabelMenu.delete(5, 'end')
        self.effectsLabelMenu.add_separator()
        # Add in a checkbutton for each effect (to toggle on/off)
        keys = self.effectsDict.keys()
        keys.sort()
        for name in keys:
            effect = self.effectsDict[name]
            self.effectsLabelMenu.add_command(
                label = effect.getName(),
                command = (lambda s = self,
                           e = effect: s.selectEffectNamed(e.getName()))
                )
            effectActive = IntVar()
            effectActive.set(effect.isEnabled())
            self.effectsEnableMenu.add_checkbutton(
                label = effect.getName(),
                variable = effectActive,
                command = (lambda s = self,
                           e = effect,
                           v = effectActive: s.toggleEffect(e, v)))

    def updateParticlesMenus(self):
        # Get rid of old particles entries if any
        self.particlesEnableMenu.delete(0, 'end')
        self.particlesLabelMenu.delete(2, 'end')
        self.particlesLabelMenu.add_separator()
        # Add in a checkbutton for each effect (to toggle on/off)
        particles = self.particleEffect.getParticlesList()
        names = map(lambda x: x.getName(), particles)
        names.sort()
        for name in names:
            particle = self.particleEffect.getParticlesNamed(name)
            self.particlesLabelMenu.add_command(
                label = name,
                command = (lambda s = self,
                           n = name: s.selectParticlesNamed(n))
                )
            particleActive = IntVar()
            particleActive.set(particle.isEnabled())
            self.particlesEnableMenu.add_checkbutton(
                label = name,
                variable = particleActive,
                command = (lambda s = self,
                           p = particle,
                           v = particleActive: s.toggleParticles(p, v)))

    def updateForceGroupMenus(self):
        # Get rid of old forceGroup entries if any
        self.forceGroupEnableMenu.delete(0, 'end')
        self.forceGroupLabelMenu.delete(2, 'end')
        self.forceGroupLabelMenu.add_separator()
        # Add in a checkbutton for each effect (to toggle on/off)
        forceGroupList = self.particleEffect.getForceGroupList()
        names = map(lambda x: x.getName(), forceGroupList)
        names.sort()
        for name in names:
            force = self.particleEffect.getForceGroupNamed(name)
            self.forceGroupLabelMenu.add_command(
                label = name,
                command = (lambda s = self,
                           n = name: s.selectForceGroupNamed(n))
                )
            forceActive = IntVar()
            forceActive.set(force.isEnabled())
            self.forceGroupEnableMenu.add_checkbutton(
                label = name,
                variable = forceActive,
                command = (lambda s = self,
                           f = force,
                           v = forceActive: s.toggleForceGroup(f, v)))

    def selectEffectNamed(self, name):
        effect = self.effectsDict.get(name, None)
        if effect != None:
            self.particleEffect = effect
            # Default to first particle in particlesDict
            self.particles = self.particleEffect.getParticlesList()[0]
            # See if particle effect has any forceGroup
            forceGroupList = self.particleEffect.getForceGroupList()
            if len(forceGroupList) > 0:
                self.forceGroup = forceGroupList[0]
            else:
                self.forceGroup = None
            self.mainNotebook.selectpage('System')
            self.updateInfo('System')
        else:
            print('ParticlePanel: No effect named ' + name)

    def toggleEffect(self, effect, var):
        if var.get():
            effect.enable()
        else:
            effect.disable()

    def selectParticlesNamed(self, name):
        particles = self.particleEffect.getParticlesNamed(name)
        if particles != None:
            self.particles = particles
            self.updateInfo()

    def toggleParticles(self, particles, var):
        if var.get():
            particles.enable()
        else:
            particles.disable()

    def selectForceGroupNamed(self, name):
        forceGroup = self.particleEffect.getForceGroupNamed(name)
        if forceGroup != None:
            self.forceGroup = forceGroup
            self.updateInfo('Force')

    def toggleForceGroup(self, forceGroup, var):
        if var.get():
            forceGroup.enable()
        else:
            forceGroup.disable()

    def toggleForce(self, force, pageName, variableName):
        v = self.getVariable(pageName, variableName)
        if v.get():
            force.setActive(1)
        else:
            force.setActive(0)

    def getWidget(self, category, text):
        return self.widgetDict[category + '-' + text]

    def getVariable(self, category, text):
        return self.variableDict[category + '-' + text]

    def loadParticleEffectFromFile(self):
        # Find path to particle directory
        pPath = getParticlePath()
        if pPath.getNumDirectories() > 0:
            if repr(pPath.getDirectory(0)) == '.':
                path = '.'
            else:
                path = pPath.getDirectory(0).toOsSpecific()
        else:
            path = '.'
        if not os.path.isdir(path):
            print('ParticlePanel Warning: Invalid default DNA directory!')
            print('Using current directory')
            path = '.'
        particleFilename = askopenfilename(
            defaultextension = '.ptf',
            filetypes = (('Particle Files', '*.ptf'),('All files', '*')),
            initialdir = path,
            title = 'Load Particle Effect',
            parent = self.parent)
        if particleFilename:
            # Delete existing particles and forces
            self.particleEffect.loadConfig(
                Filename.fromOsSpecific(particleFilename))
            self.selectEffectNamed(self.particleEffect.getName())
            # Enable effect
            self.particleEffect.enable()
        messenger.send('SGE_Update Explorer',[render])

    def saveParticleEffectToFile(self):
        # Find path to particle directory
        pPath = getParticlePath()
        if pPath.getNumDirectories() > 0:
            if repr(pPath.getDirectory(0)) == '.':
                path = '.'
            else:
                path = pPath.getDirectory(0).toOsSpecific()
        else:
            path = '.'
        if not os.path.isdir(path):
            print('ParticlePanel Warning: Invalid default DNA directory!')
            print('Using current directory')
            path = '.'
        particleFilename = asksaveasfilename(
            defaultextension = '.ptf',
            filetypes = (('Particle Files', '*.ptf'),('All files', '*')),
            initialdir = path,
            title = 'Save Particle Effect as',
            parent = self.parent)
        if particleFilename:
            self.particleEffect.saveConfig(Filename(particleFilename))

    ### PARTICLE EFFECTS COMMANDS ###
    def toggleParticleMgr(self):
        if self.particleMgrActive.get():
            base.enableParticles()
        else:
            base.disableParticles()

    ### PARTICLE SYSTEM COMMANDS ###
    def updateInfo(self, page = 'System'):
        self.updateMenusAndLabels()
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
            self.updateForceWidgets()

    def toggleParticleEffect(self):
        if self.getVariable('Effect', 'Active').get():
            self.particleEffect.enable()
        else:
            self.particleEffect.disable()

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
        self.getVariable('System', 'Render Space Velocities').set(
            self.particles.getLocalVelocityFlag())
        self.getVariable('System', 'System Grows Older').set(
            self.particles.getSystemGrowsOlderFlag())
    def setSystemPoolSize(self, value):
        self.particles.setPoolSize(int(value))
    def setSystemBirthRate(self, value):
        self.particles.setBirthRate(value)
    def setSystemLitterSize(self, value):
        self.particles.setLitterSize(int(value))
    def setSystemLitterSpread(self, value):
        self.particles.setLitterSpread(int(value))
    def setSystemLifespan(self, value):
        self.particles.setSystemLifespan(value)
    def toggleSystemLocalVelocity(self):
        self.particles.setLocalVelocityFlag(
            self.getVariable('System', 'Render Space Velocities').get())
    def toggleSystemGrowsOlder(self):
        self.particles.setSystemGrowsOlderFlag(
            self.getVariable('System', 'System Grows Older').get())
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
        self.getWidget('Factory', 'Mass').set(mass, 0)
        massSpread = factory.getMassSpread()
        self.getWidget('Factory', 'Mass Spread').set(massSpread, 0)
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
        self.particles.factory.setFinalAngle(angle)
    def setFactoryZSpinFinalAngleSpread(self, spread):
        self.particles.factory.setFinalAngleSpread(spread)
    def setFactoryZSpinAngularVelocity(self, vel):
        self.particles.factory.setAngularVelocity(vel)
    def setFactoryZSpinAngularVelocitySpread(self, spread):
        self.particles.factory.setAngularVelocitySpread(spread)

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
            self.getVariable('Disc Emitter', 'Cubic Lerping').set(cubicLerping)
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
            self.getVariable('Disc Emitter', 'Cubic Lerping').get())
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
        self.getVariable('Renderer', 'Alpha Mode').set(aMode)
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
            self.getVariable('Point Renderer', 'Blend Type').set(bType)
            blendMethod = renderer.getBlendMethod()
            bMethod = "PP_NO_BLEND"
            if (blendMethod == BaseParticleRenderer.PPNOBLEND):
                bMethod = "PP_NO_BLEND"
            elif (blendMethod == BaseParticleRenderer.PPBLENDLINEAR):
                bMethod = "PP_BLEND_LINEAR"
            elif (blendMethod == BaseParticleRenderer.PPBLENDCUBIC):
                bMethod = "PP_BLEND_CUBIC"
            self.getVariable('Point Renderer', 'Blend Method').set(bMethod)
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
            self.getVariable('Sparkle Renderer', 'Life Scale').set(lScale)
        elif isinstance(renderer, SpriteParticleRenderer):
            color = renderer.getColor() * 255.0
            # Update widgets to reflect current default values
            # Texture
            textureName = renderer.getSourceTextureName()
            if textureName != None:
                self.rendererSpriteTexture.set(textureName)
            # File
            fileName = renderer.getSourceFileName()
            if fileName != None:
                self.rendererSpriteFile.set(fileName)
            # Node
            nodeName = renderer.getSourceNodeName()
            if nodeName != None:
                self.rendererSpriteNode.set(nodeName)
            self.getVariable('Sprite Renderer', 'X Scale').set(
                renderer.getXScaleFlag())
            self.getVariable('Sprite Renderer', 'Y Scale').set(
                renderer.getYScaleFlag())
            self.getVariable('Sprite Renderer', 'Anim Angle').set(
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
            self.getVariable('Sprite Renderer', 'Alpha Disable').set(
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
        node = None
        nodePath = loader.loadModel(self.rendererGeomNode.get())
        if nodePath != None:
            node = nodePath.node()
        if (node != None):
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
    def setSpriteSourceType(self):
        if self.rendererSpriteSourceType.get() == SpriteParticleRenderer.STTexture:
            self.rendererSpriteTextureEntry['state'] = 'normal'
            self.rendererSpriteFileEntry['state'] = 'disabled'
            self.rendererSpriteNodeEntry['state'] = 'disabled'
            self.rendererSpriteTextureEntry['background'] = '#FFFFFF'
            self.rendererSpriteFileEntry['background'] = '#C0C0C0'
            self.rendererSpriteNodeEntry['background'] = '#C0C0C0'
        else:
            self.rendererSpriteTextureEntry['state'] = 'disabled'
            self.rendererSpriteFileEntry['state'] = 'normal'
            self.rendererSpriteNodeEntry['state'] = 'normal'
            self.rendererSpriteTextureEntry['background'] = '#C0C0C0'
            self.rendererSpriteFileEntry['background'] = '#FFFFFF'
            self.rendererSpriteNodeEntry['background'] = '#FFFFFF'
    def setRendererSpriteTexture(self):
        if self.rendererSpriteSourceType.get() == SpriteParticleRenderer.STTexture:
            self.particles.renderer.setTextureFromFile(self.rendererSpriteTexture.get())
        else:
            self.particles.renderer.setTextureFromNode(
                self.rendererSpriteFile.get(), self.rendererSpriteNode.get())
    def toggleRendererSpriteXScale(self):
        self.particles.renderer.setXScaleFlag(
            self.getVariable('Sprite Renderer', 'X Scale').get())
    def toggleRendererSpriteYScale(self):
        self.particles.renderer.setYScaleFlag(
            self.getVariable('Sprite Renderer', 'Y Scale').get())
    def toggleRendererSpriteAnimAngle(self):
        self.particles.renderer.setAnimAngleFlag(
            self.getVariable('Sprite Renderer', 'Anim Angle').get())

    def toggleAngularVelocity(self):
        self.particles.factory.enableAngularVelocity(
            self.getVariable('Z Spin Factory', 'Enable Angular Velocity').get())

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
        print(blendMethod)
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
            self.getVariable('Sprite Renderer', 'Alpha Disable').get())

    ## FORCEGROUP COMMANDS ##
    def updateForceWidgets(self):
        # Select appropriate notebook page
        if self.forceGroup != None:
            self.forceGroupNotebook.pack(fill = X)
            self.forcePageName = (self.particleEffect.getName() + '-' +
                                  self.forceGroup.getName())
            self.forcePage = self.forcePagesDict.get(
                self.forcePageName, None)
            # Page doesn't exist, add it
            if self.forcePage == None:
                self.addForceGroupNotebookPage(
                    self.particleEffect, self.forceGroup)
            self.forceGroupNotebook.selectpage(self.forcePageName)
        else:
            self.forceGroupNotebook.pack_forget()

    def addLinearVectorForce(self):
        self.addForce(LinearVectorForce())
    def addLinearFrictionForce(self):
        self.addForce(LinearFrictionForce())
    def addLinearJitterForce(self):
        self.addForce(LinearJitterForce())
    def addLinearNoiseForce(self):
        self.addForce(LinearNoiseForce())
    def addLinearSinkForce(self):
        self.addForce(LinearSinkForce())
    def addLinearSourceForce(self):
        self.addForce(LinearSourceForce())
    def addLinearCylinderVortexForce(self):
        self.addForce(LinearCylinderVortexForce())
    def addLinearUserDefinedForce(self):
        self.addForce(LinearUserDefinedForce())

    def addForce(self, f):
        if self.forceGroup == None:
            self.createNewForceGroup()
        self.forceGroup.addForce(f)
        self.addForceWidget(self.forceGroup,f)

    ## SYSTEM COMMANDS ##
    def createNewEffect(self):
        name = askstring('Particle Panel', 'Effect Name:',
                         parent = self.parent)
        if name:
            particles = seParticles.Particles()
            particles.setBirthRate(0.02)
            particles.setLitterSize(10)
            particles.setLitterSpread(0)
            particles.setFactory("PointParticleFactory")
            particles.setRenderer("PointParticleRenderer")
            particles.setEmitter("SphereVolumeEmitter")
            particles.enable()
            effect = seParticleEffect.ParticleEffect(name,particles)
            self.effectsDict[name] = effect
            self.updateMenusAndLabels()
            self.selectEffectNamed(name)
            self.emitter=loader.loadModel("sphere")
            self.emitter.setName(name)
            effect.reparentTo(self.emitter)
            self.emitter.reparentTo(render)
            effect.enable()
            messenger.send('ParticlePanel_Added_Effect',[name,effect,self.emitter])
            messenger.send('SGE_Update Explorer',[render])

    def createNewParticles(self):
        name = askstring('Particle Panel', 'Particles Name:',
                         parent = self.parent)
        if name:
            p = seParticles.Particles(name)
            p.setBirthRate(0.02)
            p.setLitterSize(10)
            p.setLitterSpread(0)
            p.setFactory("PointParticleFactory")
            p.setRenderer("PointParticleRenderer")
            p.setEmitter("SphereVolumeEmitter")
            self.particleEffect.addParticles(p)
            self.updateParticlesMenus()
            self.selectParticlesNamed(name)
            p.enable()

    def createNewForceGroup(self):
        name = askstring('Particle Panel', 'ForceGroup Name:',
                         parent = self.parent)
        if name:
            forceGroup = seForceGroup.ForceGroup(name)
            self.particleEffect.addForceGroup(forceGroup)
            self.updateForceGroupMenus()
            self.addForceGroupNotebookPage(self.particleEffect, forceGroup)
            self.selectForceGroupNamed(name)
            forceGroup.enable()

    def addForceGroupNotebookPage(self, particleEffect, forceGroup):
        self.forcePageName = (particleEffect.getName() + '-' +
                              forceGroup.getName())
        self.forcePage = self.forceGroupNotebook.add(self.forcePageName)
        self.forcePagesDict[self.forcePageName] = self.forcePage
        for force in forceGroup:
            self.addForceWidget(forceGroup, force)

    def addForceWidget(self, forceGroup, force):
        forcePage = self.forcePage
        pageName = self.forcePageName
        # How many forces of the same type in the force group object
        count = 0
        for f in forceGroup:
            if f.getClassType().eq(force.getClassType()):
                count += 1
        if isinstance(force, LinearVectorForce):
            self.createLinearVectorForceWidget(
                forcePage, pageName, count, force)
        elif isinstance(force, LinearNoiseForce):
            self.createLinearRandomForceWidget(
                forcePage, pageName, count, force, 'Noise')
        elif isinstance(force, LinearJitterForce):
            self.createLinearRandomForceWidget(
                forcePage, pageName, count, force, 'Jitter')
        elif isinstance(force, LinearFrictionForce):
            self.createLinearFrictionForceWidget(
                forcePage, pageName, count, force)
        elif isinstance(force, LinearCylinderVortexForce):
            self.createLinearCylinderVortexForceWidget(
                forcePage, pageName, count, force)
        elif isinstance(force, LinearSinkForce):
            self.createLinearDistanceForceWidget(
                forcePage, pageName, count, force, 'Sink')
        elif isinstance(force, LinearSourceForce):
            self.createLinearDistanceForceWidget(
                forcePage, pageName, count, force, 'Source')
        elif isinstance(force, LinearUserDefinedForce):
            # Nothing
            pass
        self.forceGroupNotebook.setnaturalsize()

    def createForceFrame(self, forcePage, forceName, force):
        frame = Frame(forcePage, relief = RAISED, borderwidth = 2)
        lFrame = Frame(frame, relief = FLAT)
        def removeForce(s = self, f = force, fr = frame):
            s.forceGroup.removeForce(f)
            fr.pack_forget()
        b = Button(lFrame, text = 'X',
                   command = removeForce)
        b.pack(side = 'right', expand = 0)
        Label(lFrame, text = forceName,
              foreground = 'Blue',
              font=('MSSansSerif', 12, 'bold'),
              ).pack(expand = 1, fill = 'x')
        lFrame.pack(fill = 'x', expand =1)
        frame.pack(pady = 3, fill = 'x', expand =0)
        return frame

    def createLinearForceWidgets(self, frame, pageName, forceName, force):
        def setAmplitude(amp, f = force):
            f.setAmplitude(amp)
        def toggleMassDependent(s=self, f=force, p=pageName, n=forceName):
            v = s.getVariable(p, n+' Mass Dependent')
            f.setMassDependent(v.get())
        def setVectorMasks(s=self, f=force, p=pageName, n=forceName):
            xMask = s.getVariable(p, n+' Mask X').get()
            yMask = s.getVariable(p, n+' Mask Y').get()
            zMask = s.getVariable(p, n+' Mask Z').get()
            f.setVectorMasks(xMask, yMask, zMask)
        self.createFloater(frame, pageName, forceName + ' Amplitude',
                           'Force amplitude multiplier',
                           command = setAmplitude,
                           value = force.getAmplitude())
        cbf = Frame(frame, relief = FLAT)
        self.createCheckbutton(cbf, pageName, forceName + ' Mass Dependent',
                               ('On: force depends on mass; ' +
                                'Off: force does not depend on mass'),
                               toggleMassDependent,
                               force.getMassDependent())
        self.createCheckbutton(cbf, pageName, forceName + ' Mask X',
                               'On: enable force along X axis',
                               setVectorMasks, 1)
        self.createCheckbutton(cbf, pageName, forceName + ' Mask Y',
                               'On: enable force along X axis',
                               setVectorMasks, 1)
        self.createCheckbutton(cbf, pageName, forceName + ' Mask Z',
                               'On: enable force along X axis',
                               setVectorMasks, 1)
        cbf.pack(fill = 'x', expand = 0)

    def createForceActiveWidget(self, frame, pageName, forceName, force):
        cbName = forceName + ' Active'
        def toggle(s = self, f = force, p = pageName, n = cbName):
            s.toggleForce(f, p, n)
        self.createCheckbutton(frame, pageName, cbName,
                               'On: force is enabled; Off: force is disabled',
                               toggle, 1)

    def createLinearVectorForceWidget(self, forcePage, pageName,
                                      count, force):
        def setVec(vec, f = force):
            f.setVector(vec[0], vec[1], vec[2])
        forceName = 'Vector Force-' + repr(count)
        frame = self.createForceFrame(forcePage, forceName, force)
        self.createLinearForceWidgets(frame, pageName, forceName, force)
        vec = force.getLocalVector()
        self.createVector3Entry(frame, pageName, forceName,
                                'Set force direction and magnitude',
                                command = setVec,
                                value = [vec[0], vec[1], vec[2]])
        self.createForceActiveWidget(frame, pageName, forceName, force)

    def createLinearRandomForceWidget(self, forcePage, pageName, count,
                                force, type):
        forceName = type + ' Force-' + repr(count)
        frame = self.createForceFrame(forcePage, forceName, force)
        self.createLinearForceWidgets(frame, pageName, forceName, force)
        self.createForceActiveWidget(frame, pageName, forceName, force)

    def createLinearFrictionForceWidget(self, forcePage, pageName,
                                        count, force):
        def setCoef(coef, f = force):
            f.setCoef(coef)
        forceName = 'Friction Force-' + repr(count)
        frame = self.createForceFrame(forcePage, forceName, force)
        self.createLinearForceWidgets(frame, pageName, forceName, force)
        self.createFloater(frame, pageName, forceName + ' Coef',
                           'Set linear friction force',
                           command = setCoef, min = None,
                           value = force.getCoef())
        self.createForceActiveWidget(frame, pageName, forceName, force)

    def createLinearCylinderVortexForceWidget(self, forcePage, pageName,
                                              count, force):
        forceName = 'Vortex Force-' + repr(count)
        def setCoef(coef, f = force):
            f.setCoef(coef)
        def setLength(length, f = force):
            f.setLength(length)
        def setRadius(radius, f = force):
            f.setRadius(radius)
        frame = self.createForceFrame(forcePage, forceName, force)
        self.createLinearForceWidgets(frame, pageName, forceName, force)
        self.createFloater(frame, pageName, forceName + ' Coef',
                           'Set linear cylinder vortex coefficient',
                           command = setCoef,
                           value = force.getCoef())
        self.createFloater(frame, pageName, forceName + ' Length',
                           'Set linear cylinder vortex length',
                           command = setLength,
                           value = force.getLength())
        self.createFloater(frame, pageName, forceName + ' Radius',
                           'Set linear cylinder vortex radius',
                           command = setRadius,
                           value = force.getRadius())
        self.createForceActiveWidget(frame, pageName, forceName, force)

    def createLinearDistanceForceWidget(self, forcePage, pageName,
                                        count, force, type):
        def setFalloffType(type, f=force):
            if type == 'FT_ONE_OVER_R':
                #f.setFalloffType(LinearDistanceForce.FTONEOVERR)
                f.setFalloffType(0)
            if type == 'FT_ONE_OVER_R_SQUARED':
                #f.setFalloffType(LinearDistanceForce.FTONEOVERRSQUARED)
                f.setFalloffType(1)
            if type == 'FT_ONE_OVER_R_CUBED':
                #f.setFalloffType(LinearDistanceForce.FTONEOVERRCUBED)
                f.setFalloffType(2)
        def setForceCenter(vec, f = force):
            f.setForceCenter(Point3(vec[0], vec[1], vec[2]))
        def setRadius(radius, f = force):
            f.setRadius(radius)
        forceName = type + ' Force-' + repr(count)
        frame = self.createForceFrame(forcePage, forceName, force)
        self.createLinearForceWidgets(frame, pageName, forceName, force)
        var = self.createOptionMenu(
            frame, pageName, forceName + ' Falloff',
            'Set force falloff type',
            ('FT_ONE_OVER_R',
             'FT_ONE_OVER_R_SQUARED',
             'FT_ONE_OVER_R_CUBED'),
            command = setFalloffType)
        self.getWidget(pageName, forceName + ' Falloff').configure(
            label_width = 16)
        falloff = force.getFalloffType()
        if falloff == LinearDistanceForce.FTONEOVERR:
            var.set('FT_ONE_OVER_R')
        elif falloff == LinearDistanceForce.FTONEOVERRSQUARED:
            var.set('FT_ONE_OVER_R_SQUARED')
        elif falloff == LinearDistanceForce.FTONEOVERRCUBED:
            var.set('FT_ONE_OVER_R_CUBED')
        vec = force.getForceCenter()
        self.createVector3Entry(frame, pageName, forceName + ' Center',
                                'Set center of force',
                                command = setForceCenter,
                                label_width = 16,
                                value = [vec[0], vec[1], vec[2]])
        self.createFloater(frame, pageName, forceName + ' Radius',
                           'Set falloff radius',
                           command = setRadius,
                           min = 0.01,
                           value = force.getRadius())
        self.createForceActiveWidget(frame, pageName, forceName, force)

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = Pmw.initialise()
    pp = ParticlePanel()
    #ve = VectorEntry(Toplevel(), relief = GROOVE)
    #ve.pack()
