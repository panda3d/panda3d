"""
Show how to use libRocket in Panda3D.

NOTE: libRocket is only available for Python 2.7, which is itself deprecated.
We have therefore deprecated libRocket support in Panda3D, and it will be
removed in Panda3D 1.11.
"""
import sys
from panda3d.core import loadPrcFile, loadPrcFileData, Point3,Vec4, Mat4, LoaderOptions  # @UnusedImport
from panda3d.core import DirectionalLight, AmbientLight, PointLight
from panda3d.core import Texture, PNMImage
from panda3d.core import PandaSystem
import random
from direct.interval.LerpInterval import LerpHprInterval, LerpPosInterval, LerpFunc
from direct.showbase.ShowBase import ShowBase

# workaround: https://www.panda3d.org/forums/viewtopic.php?t=10062&p=99697#p99054
#from panda3d import rocket
import _rocketcore as rocket

from panda3d.rocket import RocketRegion, RocketInputHandler

loadPrcFileData("", "model-path $MAIN_DIR/assets")

import console

global globalClock

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

        self.loadRocketFonts()

        self.loadingTask = None

        #self.startModelLoadingAsync()
        self.startModelLoading()

        self.inputHandler = RocketInputHandler()
        self.mouseWatcher.attachNewNode(self.inputHandler)

        self.openLoadingDialog()

    def loadRocketFonts(self):
        """ Load fonts referenced from e.g. 'font-family' RCSS directives.

        Note: the name of the font as used in 'font-family'
        is not always the same as the filename;
        open the font in your OS to see its display name.
        """
        rocket.LoadFontFace("modenine.ttf")


    def startModelLoading(self):
        self.monitorNP = None
        self.keyboardNP = None
        self.loadingError = False

        self.taskMgr.doMethodLater(1, self.loadModels, 'loadModels')

    def loadModels(self, task):
        self.monitorNP = self.loader.loadModel("monitor")
        self.keyboardNP = self.loader.loadModel("takeyga_kb")

    def startModelLoadingAsync(self):
        """
        NOTE: this seems to invoke a few bugs (crashes, sporadic model
        reading errors, etc) so is disabled for now...
        """
        self.monitorNP = None
        self.keyboardNP = None
        self.loadingError = False

        # force the "loading" to take some time after the first run...
        options = LoaderOptions()
        options.setFlags(options.getFlags() | LoaderOptions.LFNoCache)

        def gotMonitorModel(model):
            if not model:
                self.loadingError = True
            self.monitorNP = model

        self.loader.loadModel("monitor", loaderOptions=options, callback=gotMonitorModel)

        def gotKeyboardModel(model):
            if not model:
                self.loadingError = True
            self.keyboardNP = model

        self.loader.loadModel("takeyga_kb", loaderOptions=options, callback=gotKeyboardModel)

    def openLoadingDialog(self):
        self.userConfirmed = False

        self.windowRocketRegion = RocketRegion.make('pandaRocket', self.win)
        self.windowRocketRegion.setActive(1)

        self.windowRocketRegion.setInputHandler(self.inputHandler)

        self.windowContext = self.windowRocketRegion.getContext()

        self.loadingDocument = self.windowContext.LoadDocument("loading.rml")
        if not self.loadingDocument:
            raise AssertionError("did not find loading.rml")

        self.loadingDots = 0
        el = self.loadingDocument.GetElementById('loadingLabel')
        self.loadingText = el.first_child
        self.stopLoadingTime = globalClock.getFrameTime() + 3
        self.loadingTask = self.taskMgr.add(self.cycleLoading, 'doc changer')


        # note: you may encounter errors like 'KeyError: 'document'"
        # when invoking events using methods from your own scripts with this
        # obvious code:
        #
        # self.loadingDocument.AddEventListener('aboutToClose',
        #                                       self.onLoadingDialogDismissed, True)
        #
        # A workaround is to define callback methods in standalone Python
        # files with event, self, and document defined to None.
        #
        # see https://www.panda3d.org/forums/viewtopic.php?f=4&t=16412
        #

        # Or, use this indirection technique to work around the problem,
        # by publishing the app into the context, then accessing it through
        # the document's context...

        self.windowContext.app = self
        self.loadingDocument.AddEventListener('aboutToClose',
                                              'document.context.app.handleAboutToClose()', True)

        self.loadingDocument.Show()

    def handleAboutToClose(self):
        self.userConfirmed = True
        if self.monitorNP and self.keyboardNP:
            self.onLoadingDialogDismissed()

    def attachCustomRocketEvent(self, document, rocketEventName, pandaHandler, once=False):
        # handle custom event

        # note: you may encounter errors like 'KeyError: 'document'"
        # when invoking events using methods from your own scripts with this
        # obvious code:
        #
        # self.loadingDocument.AddEventListener('aboutToClose',
        #                                       self.onLoadingDialogDismissed, True)
        #
        # see https://www.panda3d.org/forums/viewtopic.php?f=4&t=16412


        # this technique converts Rocket events to Panda3D events

        pandaEvent = 'panda.' + rocketEventName

        document.AddEventListener(
            rocketEventName,
            "messenger.send('" + pandaEvent + "', [event])")

        if once:
            self.acceptOnce(pandaEvent, pandaHandler)
        else:
            self.accept(pandaEvent, pandaHandler)


    def cycleLoading(self, task):
        """
        Update the "loading" text in the initial window until
        the user presses Space, Enter, or Escape or clicks (see loading.rxml)
        or sufficient time has elapsed (self.stopLoadingTime).
        """
        text = self.loadingText

        now = globalClock.getFrameTime()
        if self.monitorNP and self.keyboardNP:
            text.text = "Ready"
            if now > self.stopLoadingTime or self.userConfirmed:
                self.onLoadingDialogDismissed()
                return task.done
        elif self.loadingError:
            text.text = "Assets not found"
        else:
            count = 5
            intv = int(now * 4) % count  # @UndefinedVariable
            text.text = "Loading" + ("." * (1+intv)) + (" " * (2 - intv))

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
        by modifying the 'color' RCSS attribute to slowly
        change from solid to transparent.

        element: the Rocket element whose style to modify
        time: time in seconds for fadeout
        """

        # get the current color from RCSS effective style
        color = element.style.color
        # convert to RGBA form
        prefix = color[:color.rindex(',')+1].replace('rgb(', 'rgba(')

        def updateAlpha(t):
            # another way of setting style on a specific element
            attr = 'color: ' + prefix + str(int(t)) +');'
            element.SetAttribute('style', attr)

        alphaInterval = LerpFunc(updateAlpha,
                             duration=time,
                             fromData=255,
                             toData=0,
                             blendType='easeIn')

        return alphaInterval

    def showStarting(self):
        """ Models are loaded, so update the dialog,
        fade out, then transition to the console. """
        self.loadingText.text = 'Starting...'

        alphaInterval = self.fadeOut(self.loadingText, 0.5)
        alphaInterval.setDoneEvent('fadeOutFinished')

        def fadeOutFinished():
            if self.loadingDocument:
                self.loadingDocument.Close()
                self.loadingDocument = None
                self.createConsole()

        self.accept('fadeOutFinished', fadeOutFinished)

        alphaInterval.start()

    def createConsole(self):
        """ Create the in-world console, which displays
        a RocketRegion in a GraphicsBuffer, which appears
        in a Texture on the monitor model. """

        self.monitorNP.reparentTo(self.render)
        self.monitorNP.setScale(1.5)

        self.keyboardNP.reparentTo(self.render)
        self.keyboardNP.setHpr(-90, 0, 15)
        self.keyboardNP.setScale(20)

        self.placeItems()

        self.setupRocketConsole()

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


    def setupRocketConsole(self):
        """
        Place a new rocket window onto a texture
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

        self.rocketConsole = RocketRegion.make('console', mybuffer)
        self.rocketConsole.setInputHandler(self.inputHandler)

        self.consoleContext = self.rocketConsole.getContext()
        self.console = console.Console(self, self.consoleContext, 40, 13, self.handleCommand)

        self.console.addLine("Panda DOS")
        self.console.addLine("type 'help'")
        self.console.addLine("")

        self.console.allowEditing(True)

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
            return

        def randchr():
            return chr(int(random.random() < 0.25 and 32 or random.randint(32, 127)))

        line = ''.join([randchr() for _ in range(40) ])

        self.console.addLine(line)
        self.queueSpew()

    def terminateMonitor(self):
        alphaInterval = self.fadeOut(self.console.getTextContainer(), 2)

        alphaInterval.setDoneEvent('fadeOutFinished')

        def fadeOutFinished():
            sys.exit(0)

        self.accept('fadeOutFinished', fadeOutFinished)

        alphaInterval.start()

app = MyApp()
app.run()
