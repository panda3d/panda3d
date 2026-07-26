"""
Show how to use RmlUi in Panda3D.
"""
import os
import sys
from panda3d.core import loadPrcFileData, Vec4, Mat4
from panda3d.core import DirectionalLight, AmbientLight, PointLight
from panda3d.core import Texture
from panda3d.core import PandaSystem
import random
from direct.interval.LerpInterval import LerpFunc
from direct.showbase.ShowBase import ShowBase

from panda3d.rmlui import RmlRegion, RmlInputHandler

ASSETS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets")
loadPrcFileData("", "model-path " + ASSETS)
# The RmlUi filter/layer shaders need GLSL 1.50, which macOS only provides in
# a core-profile context.  Stencil bits enable border-radius / transform
# clipping on the main window.
if sys.platform == "darwin":
    loadPrcFileData("", "gl-version 3 2")
loadPrcFileData("", "framebuffer-stencil true")

import console

class MyApp(ShowBase):

    def __init__(self):
        ShowBase.__init__(self)

        self.win.setClearColor(Vec4(0.2, 0.2, 0.2, 1))

        self.disableMouse()

        self.render.setShaderAuto()

        dlight = DirectionalLight('dlight')
        alight = AmbientLight('alight')
        dlnp = self.render.attachNewNode(dlight)
        alnp = self.render.attachNewNode(alight)
        dlight.setColor((0.8, 0.8, 0.5, 1))
        alight.setColor((0.2, 0.2, 0.2, 1))
        dlnp.setHpr(0, -60, 0)
        self.render.setLight(dlnp)
        self.render.setLight(alnp)

        # Put lighting on the main scene
        plight = PointLight('plight')
        plnp = self.render.attachNewNode(plight)
        plnp.setPos(0, 0, 10)
        self.render.setLight(plnp)
        self.render.setLight(alnp)

        self.loadingTask = None

        self.startModelLoading()

        self.inputHandler = RmlInputHandler()
        self.mouseWatcher.attachNewNode(self.inputHandler)

        self.openLoadingDialog()

    def loadRmlUiFonts(self, context):
        """ Load fonts referenced from e.g. 'font-family' RCSS directives.

        Note: the name of the font as used in 'font-family'
        is not always the same as the filename;
        open the font in your OS to see its display name.

        Unlike libRocket, RmlUi requires fonts to be loaded per-context.
        Call this after creating a context and pass it in.
        """
        context.load_font_face(os.path.join(ASSETS, "modenine.ttf"))


    def startModelLoading(self):
        self.monitorNP = None
        self.keyboardNP = None
        self.loadingError = False

        self.taskMgr.doMethodLater(1, self.loadModels, 'loadModels')

    def loadModels(self, task):
        self.monitorNP = self.loader.loadModel("monitor")
        self.keyboardNP = self.loader.loadModel("takeyga_kb")

    def openLoadingDialog(self):
        self.userConfirmed = False

        self.windowRmlRegion = RmlRegion.make('pandaRml', self.win)

        self.windowRmlRegion.set_input_handler(self.inputHandler)

        self.windowContext = self.windowRmlRegion.get_context()

        self.loadRmlUiFonts(self.windowContext)

        self.loadingDocument = self.windowContext.load_document(os.path.join(ASSETS, "loading.rml"))
        if not self.loadingDocument:
            raise AssertionError("did not find loading.rml")

        self.loadingDots = 0
        el = self.loadingDocument.get_element_by_id('loading-label')
        self.loadingText = el
        self.stopLoadingTime = globalClock.getFrameTime() + 3
        self.loadingTask = self.taskMgr.add(self.cycleLoading, 'doc changer')

        body = self.loadingDocument.get_element_by_id('loading-body')
        body.add_event_listener('click', lambda ev: self.handleAboutToClose())

        self.loadingDocument.show()

    def handleAboutToClose(self):
        self.userConfirmed = True
        if self.monitorNP and self.keyboardNP:
            self.onLoadingDialogDismissed()

    def cycleLoading(self, task):
        """
        Update the "loading" text in the initial window until
        the user presses Space, Enter, or Escape or clicks (see loading.rml)
        or sufficient time has elapsed (self.stopLoadingTime).
        """
        text = self.loadingText

        now = globalClock.getFrameTime()
        if self.monitorNP and self.keyboardNP:
            text.set_inner_rml("Ready")
            if now > self.stopLoadingTime or self.userConfirmed:
                self.onLoadingDialogDismissed()
                return task.done
        elif self.loadingError:
            text.set_inner_rml("Assets not found")
        else:
            count = 5
            intv = int(now * 4) % count
            text.set_inner_rml("Loading" + ("." * (1+intv)) + (" " * (2 - intv)))

        return task.cont

    def onLoadingDialogDismissed(self):
        """ Once a models are loaded, stop 'loading' and proceed to 'start' """
        if self.loadingDocument:
            if self.loadingTask:
                self.taskMgr.remove(self.loadingTask)
            self.loadingTask = None

            self.showStarting()

    def fadeOut(self, element, time):
        """ Example updating RCSS attributes from code
        by modifying the 'opacity' RCSS attribute to slowly
        change from solid to transparent.

        element: the RmlUi element whose style to modify
        time: time in seconds for fadeout
        """

        def updateAlpha(t):
            # another way of setting style on a specific element
            element.set_attribute('style', 'opacity: ' + str(t) + ';')

        alphaInterval = LerpFunc(updateAlpha,
                             duration=time,
                             fromData=1.0,
                             toData=0.0,
                             blendType='easeIn')

        return alphaInterval

    def showStarting(self):
        """ Models are loaded, so update the dialog,
        fade out, then transition to the console. """
        self.loadingText.set_inner_rml('Starting...')

        alphaInterval = self.fadeOut(self.loadingText, 0.5)
        alphaInterval.setDoneEvent('fadeOutFinished')

        def fadeOutFinished():
            if self.loadingDocument:
                self.loadingDocument.close()
                self.loadingDocument = None
                self.createConsole()

        self.accept('fadeOutFinished', fadeOutFinished)

        alphaInterval.start()

    def createConsole(self):
        """ Create the in-world console, which displays
        a RmlRegion in a GraphicsBuffer, which appears
        in a Texture on the monitor model. """

        self.monitorNP.reparentTo(self.render)
        self.monitorNP.setScale(1.5)

        self.keyboardNP.reparentTo(self.render)
        self.keyboardNP.setHpr(-90, 0, 15)
        self.keyboardNP.setScale(20)

        self.placeItems()

        self.setupRmlUiConsole()

        # re-enable mouse
        mat=Mat4(self.camera.getMat())
        mat.invertInPlace()
        self.mouseInterfaceNode.setMat(mat)
        self.enableMouse()

    def placeItems(self):
        self.camera.setPos(0, -20, 0)
        self.camera.setHpr(0, 0, 0)
        self.monitorNP.setPos(0, 0, 1)
        self.keyboardNP.setPos(0, -5, -2.5)


    def setupRmlUiConsole(self):
        """
        Place a new RmlUi window onto a texture
        bound to the front of the monitor.
        """
        self.win.setClearColor(Vec4(0.5, 0.5, 0.8, 1))

        faceplate = self.monitorNP.find("**/Faceplate")
        assert faceplate

        mybuffer = self.win.makeTextureBuffer("Console Buffer", 1024, 512)
        tex = mybuffer.getTexture()
        tex.setMagfilter(Texture.FTLinear)
        tex.setMinfilter(Texture.FTLinear)

        faceplate.setTexture(tex, 1)

        self.rmlConsole = RmlRegion.make('console', mybuffer)
        self.rmlConsole.set_input_handler(self.inputHandler)

        self.consoleContext = self.rmlConsole.get_context()
        self.consoleContext.load_font_face(os.path.join(ASSETS, "dos437.ttf"))

        doc = self.consoleContext.load_document(os.path.join(ASSETS, "console.rml"))
        if not doc:
            raise AssertionError("did not find console.rml")
        doc.show()

        self.console = console.Console(self, doc, 40, 13, self.handleCommand)

        self.console.addLine("Panda DOS")
        self.console.addLine("type 'help'")
        self.console.addLine("")

        self.console.allowEditing(True)

        self.setupConsoleInput()

    def setupConsoleInput(self):
        self.accept('escape', sys.exit)

        # Printable characters — forwarded to Console since RmlUi no longer
        # handles textinput/keydown events via JS strings
        chars = "abcdefghijklmnopqrstuvwxyz0123456789 !@#$%^&*()-_=+[]{}\\|;:'\",.<>/?"
        for ch in chars:
            self.accept(ch, self.console.handleChar, [ch])
        for ch in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
            self.accept(ch, self.console.handleChar, [ch])

        self.accept('backspace', self.console.handleBackspace)
        self.accept('enter', self.console.handleEnter)
        self.accept('control-c', self.console.handleBreak)

    def handleCommand(self, command):
        if command is None:
            # hack for Ctrl-Break
            self.spewInProgress = False
            self.console.addLine("*** break ***")
            self.console.allowEditing(True)
            return

        command = command.strip()
        if not command:
            return

        tokens = [x.strip() for x in command.split(' ')]
        command = tokens[0].lower()

        if command == 'help':
            self.console.addLines([
                "Sorry, this is utter fakery.",
                "You won't get much more",
                "out of this simulation unless",
                "you program it yourself. :)"
            ])
        elif command == 'dir':
            self.console.addLines([
                "Directory of C:\\:",
                "HELP     COM    72 05-06-2015 14:07",
                "DIR      COM   121 05-06-2015 14:11",
                "SPEW     COM   666 05-06-2015 15:02",
                "   2 Files(s)  859 Bytes.",
                "   0 Dirs(s)  7333 Bytes free.",
                ""])
        elif command == 'cls':
            self.console.cls()
        elif command == 'echo':
            self.console.addLine(' '.join(tokens[1:]))
        elif command == 'ver':
            self.console.addLine('Panda DOS v0.01 in Panda3D ' + PandaSystem.getVersionString())
        elif command == 'spew':
            self.startSpew()
        elif command == 'exit':
            self.console.setPrompt("System is shutting down NOW!")
            self.terminateMonitor()
        else:
            self.console.addLine("command not found")

    def startSpew(self):
        self.console.allowEditing(False)
        self.console.addLine("LINE NOISE 1.0")
        self.console.addLine("")

        self.spewInProgress = True

        # note: spewage always occurs in 'doMethodLater';
        # time.sleep() would be pointless since the whole
        # UI would be frozen during the wait.
        self.queueSpew(2)

    def queueSpew(self, delay=0.1):
        self.taskMgr.doMethodLater(delay, self.spew, 'spew')

    def spew(self, task):
        # generate random spewage, just like on TV!
        if not self.spewInProgress:
            return task.done

        def randchr():
            return chr(int(random.random() < 0.25 and 32 or random.randint(32, 127)))

        line = ''.join([randchr() for _ in range(40) ])

        self.console.addLine(line)
        self.queueSpew()
        return task.done

    def terminateMonitor(self):
        alphaInterval = self.fadeOut(self.console.getTextContainer(), 2)

        alphaInterval.setDoneEvent('fadeOutFinished')

        def fadeOutFinished():
            sys.exit(0)

        self.accept('fadeOutFinished', fadeOutFinished)

        alphaInterval.start()

app = MyApp()
app.run()
