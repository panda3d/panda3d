"PANDA3D Particle Panel"

# Import Tkinter, Pmw, and the floater code from this directory tree.
from Tkinter import *
import Pmw
import Dial
import Floater
import VectorWidgets

class ParticlePanel(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):

        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       'Particle Panel',       None),
            )
        self.defineoptions(kw, optiondefs)

        Pmw.MegaToplevel.__init__(self, parent, title = self['title'])

        # Handle to the toplevels hull
        hull = self.component('hull')

        balloon = self.balloon = Pmw.Balloon(hull)
        # Start with balloon help disabled
        self.balloon.configure(state = 'none')
        
        menuFrame = Frame(hull, relief = GROOVE, bd = 2)
        menuFrame.pack(fill = X, expand = 1)

        menuBar = Pmw.MenuBar(menuFrame, hotkeys = 1, balloon = balloon)
        menuBar.pack(side = LEFT, expand = 1, fill = X)
        menuBar.addmenu('Particles', 'Particle Panel Operations')
        menuBar.addmenuitem('Particles', 'command',
                            'Exit Particles Panel',
                            label = 'Exit',
                            command = self.destroy)

        menuBar.addmenu('Help', 'Particle Panel Help Operations')
        self.toggleBalloonVar = IntVar()
        self.toggleBalloonVar.set(0)
        menuBar.addmenuitem('Help', 'checkbutton',
                            'Toggle balloon help',
                            label = 'Balloon Help',
                            variable = self.toggleBalloonVar,
                            command = self.toggleBalloon)

        self.systemSelector = Pmw.ComboBox(menuFrame,
                                     labelpos = W,
                                     label_text = 'Particle System:',
                                     entry_width = 16,
                                     selectioncommand = self.selectSystemNamed,
                                     scrolledlist_items = ('system 0',))
        self.systemSelector.selectitem('system 0')
        self.systemSelector.pack(side = 'left', expand = 0)

        # Create the notebook pages
        notebook = Pmw.NoteBook(hull)
        notebook.pack(fill = BOTH, expand = 1)
        systemPage = notebook.add('System')
        factoryPage = notebook.add('Factory')
        emitterPage = notebook.add('Emitter')
        rendererPage = notebook.add('Renderer')

        # System page
        systemWidgets = (
            ('Pool size', 'Size of particle pool', 0.0, 1.0),
            ('Birth rate', 'Seconds between particle births', 0.0, None),
            ('Litter size', 'Number of particle created at each birth', 1.0, 1.0),
            ('Litter spread', 'Variation in litter size', 0.0, 1.0),
            ('System lifespan', 'Age in seconds at which system should die', 0.0, None)
            )
        self.createFloaters(systemPage, systemWidgets)
        Checkbutton(systemPage, text = 'Local velocity',anchor = W).pack(
            fill = X)
        Checkbutton(systemPage, text = 'System grows older',anchor = W).pack(
            fill = X)
        pos = self.createVector3Entry(systemPage, 'Pos',
                                      'Particle system position')
        pos.addMenuItem('Popup 3DoF Panel')
        hpr = self.createVector3Entry(systemPage, 'Hpr',
                                     'Particle system orientation',
                                      floaterGroup_labels = ('H', 'P', 'R'))
        hpr.addMenuItem('Popup 3DoF Panel')

        # FACTORY PAGE
        self.createOptionMenu(factoryPage, 'Factory type:',
                               'Select type of particle factory',                              
                              ('Point', 'Z Spin', 'Oriented'),
                              self.selectFactoryType)
        factoryWidgets = (
            ('Life span', 'Average lifespan in seconds', 0.0, None),
            ('Life span spread', 'Variation in lifespan', 0.0, None),
            ('Mass', 'Average particle mass', 0.0, None),
            ('Mass spread', 'Variation in particle mass', 0.0, None),
            ('Terminal velocity', 'Average particle terminal velocity', 0.0, None),
            ('Terminal vel. spread', 'Variation in terminal velocity', 0.0, None))
        self.createFloaters(factoryPage, factoryWidgets)

        self.factoryNotebook = Pmw.NoteBook(factoryPage, tabpos = None)
        pointPage = self.factoryNotebook.add('Point')
        zSpinPage = self.factoryNotebook.add('Z Spin')
        self.createAngleDial(zSpinPage, 'Initial angle',
                             'Starting angle in degrees')
        self.createAngleDial(zSpinPage, 'Final angle',
                             'Final angle in degrees')
        self.createAngleDial(zSpinPage, 'Angle spread',
                             'Spread of the final angle')
        orientedPage = self.factoryNotebook.add('Oriented')
        Label(orientedPage, text = 'Not implemented').pack(expand = 1,
                                                           fill = BOTH)
        self.factoryNotebook.pack(expand = 1, fill = BOTH)

        # EMITTER PAGE
        self.createOptionMenu(emitterPage, 'Emitter type:',
                              'Select type of particle emitter',
                              ('Box', 'Disc', 'Line', 'Point', 'Rectangle',
                               'Ring', 'Sphere Surface', 'Sphere Volume',
                               'Tangent Ring'),
                              self.selectEmitterType)
        self.emitterNotebook = Pmw.NoteBook(emitterPage, tabpos = None)
        pointPage = self.emitterNotebook.add('Box')
        self.createVector3Entry(pointPage, 'Point 1',
                               'Point defining emitter box')
        self.createVector3Entry(pointPage, 'Point 2',
                               'Point defining emitter box',
                                initialValue = (1.0, 1.0, 1.0))
        self.createVector3Entry(pointPage, 'Launch vector',
                               'Initial particle velocity vector')

        discPage = self.emitterNotebook.add('Disc')
        self.createFloater(discPage, 'Radius', 'Radius of disc')
        self.createAngleDial(discPage, 'Inner angle',
                             'Particle launch angle at center of disc')
        self.createFloater(discPage, 'Inner magnitude',
                           'Launch velocity multiplier at center of disc')
        self.createAngleDial(discPage, 'Outer angle',
                             'Particle launch angle at outer edge of disc')
        self.createFloater(discPage, 'Outer magnitude',
                           'Launch velocity multiplier at edge of disc')

        Checkbutton(discPage, text = 'Cubic Lerping').pack(
            side = LEFT, expand = 1, fill = X)

        linePage = self.emitterNotebook.add('Line')
        self.createVector3Entry(linePage, 'Point 1',
                               'Point defining emitter line')
        self.createVector3Entry(linePage, 'Point 2',
                               'Point defining emitter line',
                                initialValue = (1.0, 0.0, 0.0))
        self.createVector3Entry(linePage, 'Launch Vector',
                               'Initial particle velocity vector',
                                initialValue = (0.0, 0.0, 1.0))

        pointPage = self.emitterNotebook.add('Point')
        self.createVector3Entry(pointPage, 'Location',
                               'Location of emitter point')
        self.createVector3Entry(pointPage, 'Launch vector',
                               'Initial particle velocity vector',
                                initialValue = (0.0, 0.0, 1.0))
        
        rectanglePage = self.emitterNotebook.add('Rectangle')
        self.createVector3Entry(rectanglePage, 'Point 1',
                               'Point defining rectangle')
        self.createVector3Entry(rectanglePage, 'Point 2',
                               'Point defining rectangle')
        self.createVector3Entry(rectanglePage, 'Launch vector',
                               'Initial particle velocity vector',
                                initialValue = (0.0, 0.0, 1.0))

        ringPage = self.emitterNotebook.add('Ring')
        self.createFloater(ringPage, 'Radius', 'Radius of ring')
        self.createAngleDial(ringPage, 'Angle', 'Particle launch angle')
        self.createFloater(ringPage, 'Magnitude',
                           'Launch velocity multiplier at outer edge of ring')

        sphereSurfacePage = self.emitterNotebook.add('Sphere Surface')
        self.createFloater(sphereSurfacePage, 'Radius',
                           'Radius of sphere')

        sphereVolumePage = self.emitterNotebook.add('Sphere Volume')
        self.createFloater(sphereVolumePage, 'Radius',
                           'Radius of sphere')

        tangentRingPage = self.emitterNotebook.add('Tangent Ring')
        self.createFloater(tangentRingPage, 'Radius',
                           'Radius of ring')
                
        self.emitterNotebook.pack(fill = X)

        # RENDERER PAGE
        self.createOptionMenu(rendererPage, 'Renderer type:',
                              'Select type of particle renderer',
                              ('Geom', 'Point', 'Sparkle', 'Sprite'),
                              self.selectRendererType)
        self.rendererNotebook = Pmw.NoteBook(rendererPage, tabpos = None)
        geomPage = self.rendererNotebook.add('Geom')
        f = Frame(geomPage)
        f.pack(fill = X)
        Label(f, width = 12, text = 'Geom node:').pack(side = LEFT)
        Entry(f, width = 12).pack(side = LEFT, expand = 1, fill = X)

        pointPage = self.rendererNotebook.add('Point')
        self.createFloater(pointPage, 'Point size',
                           'Width and height of points in pixels')
        self.createColorEntry(pointPage, 'Start color',
                               'Starting color of point')
        self.createColorEntry(pointPage, 'End color',
                               'Ending color of point')
        self.createOptionMenu(pointPage, 'Blend type:',
                              'Type of color blending used for particle',
                              ('ONE_COLOR', 'BLEND_LIFE', 'BLEND_VEL'),
                              self.selectBlendType)
        self.createOptionMenu(pointPage, 'Blend method:',
                              'Interpolation method between colors',
                              ('LINEAR', 'CUBIC'),
                              self.selectBlendMethod)

        sparklePage = self.rendererNotebook.add('Sparkle')
        self.createColorEntry(sparklePage, 'Center color',
                               'Color of sparkle center')
        self.createColorEntry(sparklePage, 'Edge color',
                               'Color of sparkle line endpoints')
        self.createFloater(sparklePage, 'Birth radius',
                           'Initial sparkle radius')
        self.createFloater(sparklePage, 'Death radius',
                           'Final sparkle radius')
        self.createOptionMenu(pointPage, 'Life scale:',
                              'Does particle scale over its lifetime?',
                              ('NO_SCALE', 'SCALE'),
                              self.selectBlendMethod)

        spritePage = self.rendererNotebook.add('Sprite')
        f = Frame(spritePage)
        f.pack(fill = X)
        Label(f, width = 12, text = 'Texture:').pack(side = LEFT)
        Entry(f, width = 12).pack(side = LEFT, expand = 1, fill = X)

        Checkbutton(spritePage, text = 'xScale',anchor = W).pack(fill = X)
        Checkbutton(spritePage, text = 'yScale',anchor = W).pack(fill = X)
        Checkbutton(spritePage, text = 'animAngle',anchor = W).pack(fill = X)
        self.createFloater(spritePage, 'Initial X Scale',
                           'Initial X scaling factor')
        self.createFloater(spritePage, 'Final X Scale',
                           'Final X scaling factor')
        self.createFloater(spritePage, 'Initial Y Scale',
                           'Initial Y scaling factor')
        self.createFloater(spritePage, 'Final Y Scale',
                           'Final Y scaling factor')
        self.createAngleDial(spritePage, 'Non Animated Theta',
                             'Counter clockwise Z rotation of all sprites')
        self.createOptionMenu(spritePage, 'Blend Type',
                              'Interpolation blend type for X and Y scaling',
                              ('LINEAR', 'CUBIC'),
                              self.selectBlendMethod)
        Checkbutton(spritePage, text = 'alphaDisable',anchor = W).pack(fill = X)
        
                
        self.rendererNotebook.pack(fill = X)
        
        self.factoryNotebook.setnaturalsize()
        self.emitterNotebook.setnaturalsize()
        self.rendererNotebook.setnaturalsize()
        notebook.setnaturalsize()
        
        # Make sure input variables processed 
        self.initialiseoptions(ParticlePanel)

    def createFloaters(self, parent, widgetDefinitions):
        for label, balloonHelp, min, resolution in widgetDefinitions:
            self.createFloater(parent, label, balloonHelp, min, resolution)

    def createFloater(self, parent, text, balloonHelp,
                      min = 0.0, resolution = None, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['initialValue'] = min
        kw['resolution'] = resolution
        widget = apply(floater.Floater, (parent,), kw)
        widget.pack(fill = X)
        self.balloon.bind(widget, balloonHelp)
        return widget

    def createAngleDial(self, parent, text, balloonHelp, **kw):
        kw['text'] = text
        widget = apply(dial.AngleDial,(parent,), kw)
        widget.pack(fill = X)
        self.balloon.bind(widget, balloonHelp)
        return widget

    def createVector3Entry(self, parent, text, balloonHelp, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(vectorWidgets.Vector3Entry, (parent,), kw)
        widget.pack(fill = X)
        self.balloon.bind(widget, balloonHelp)
        return widget

    def createColorEntry(self, parent, text, balloonHelp, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(vectorWidgets.ColorEntry, (parent,) ,kw)
        widget.pack(fill = X)
        self.balloon.bind(widget, balloonHelp)
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
        self.balloon.bind(widget.component('menubutton'), balloonHelp)
        return optionVar
        
    def selectFactoryType(self, type):
        self.factoryNotebook.selectpage(type)

    def selectEmitterType(self, type):
        self.emitterNotebook.selectpage(type)

    def selectRendererType(self, type):
        self.rendererNotebook.selectpage(type)

    def selectBlendType(self, type):
        print type

    def selectBlendMethod(self, method):
        print method

    def selectSystemNamed(self, name):
        print name

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = Pmw.initialise()
    pp = ParticlePanel()
    #ve = VectorEntry(Toplevel(), relief = GROOVE)
    #ve.pack()
