from panda3d.direct import CInterval
from .extension_native_helpers import Dtool_funcToMethod
from direct.directnotify.DirectNotifyGlobal import directNotify

CInterval.DtoolClassDict["notify"] = directNotify.newCategory("Interval")

#####################################################################
def setT(self, t):
    # Overridden from the C++ function to call privPostEvent
    # afterward.  We do this by renaming the C++ function in
    # FFIRename.
    self.setT_Old(t)
    self.privPostEvent()

CInterval.DtoolClassDict["setT_Old"] = CInterval.setT
Dtool_funcToMethod(setT, CInterval)
del setT
#####################################################################

def play(self, t0 = 0.0, duration = None, scale = 1.0):
    self.notify.error("CInterval.play() is deprecated, use start() instead")
    if duration:  # None or 0 implies full length
        self.start(t0, t0 + duration, scale)
    else:
        self.start(t0, -1, scale)

Dtool_funcToMethod(play, CInterval)
del play
#####################################################################

def stop(self):
    self.notify.error("CInterval.stop() is deprecated, use finish() instead")
    self.finish()

Dtool_funcToMethod(stop, CInterval)
del stop
#####################################################################

def setFinalT(self):
    self.notify.error("CInterval.setFinalT() is deprecated, use finish() instead")
    self.finish()

Dtool_funcToMethod(setFinalT, CInterval)
del setFinalT
#####################################################################

def privPostEvent(self):
    # Call after calling any of the priv* methods to do any required
    # Python finishing steps.
    t = self.getT()
    if hasattr(self, "setTHooks"):
        for func in self.setTHooks:
            func(t)

Dtool_funcToMethod(privPostEvent, CInterval)
del privPostEvent
#####################################################################

def popupControls(self, tl = None):
        """
        Popup control panel for interval.
        """
        import math
        # Don't use a regular import, to prevent ModuleFinder from picking
        # it up as a dependency when building a .p3d package.
        import importlib, sys
        EntryScale = importlib.import_module('direct.tkwidgets.EntryScale')
        if sys.version_info >= (3, 0):
            tkinter = importlib.import_module('tkinter')
        else:
            tkinter = importlib.import_module('Tkinter')

        if tl == None:
            tl = tkinter.Toplevel()
            tl.title('Interval Controls')
        outerFrame = tkinter.Frame(tl)
        def entryScaleCommand(t, s=self):
            s.setT(t)
            s.pause()
        self.es = es = EntryScale.EntryScale(
            outerFrame, text = self.getName(),
            min = 0, max = math.floor(self.getDuration() * 100) / 100,
            command = entryScaleCommand)
        es.set(self.getT(), fCommand = 0)
        es.pack(expand = 1, fill = tkinter.X)
        bf = tkinter.Frame(outerFrame)
        # Jump to start and end
        def toStart(s=self, es=es):
            s.setT(0.0)
            s.pause()
        def toEnd(s=self):
            s.setT(s.getDuration())
            s.pause()
        jumpToStart = tkinter.Button(bf, text = '<<', command = toStart)
        # Stop/play buttons
        def doPlay(s=self, es=es):
            s.resume(es.get())

        stop = tkinter.Button(bf, text = 'Stop',
                      command = lambda s=self: s.pause())
        play = tkinter.Button(
            bf, text = 'Play',
            command = doPlay)
        jumpToEnd = tkinter.Button(bf, text = '>>', command = toEnd)
        jumpToStart.pack(side = tkinter.LEFT, expand = 1, fill = tkinter.X)
        play.pack(side = tkinter.LEFT, expand = 1, fill = tkinter.X)
        stop.pack(side = tkinter.LEFT, expand = 1, fill = tkinter.X)
        jumpToEnd.pack(side = tkinter.LEFT, expand = 1, fill = tkinter.X)
        bf.pack(expand = 1, fill = tkinter.X)
        outerFrame.pack(expand = 1, fill = tkinter.X)
        # Add function to update slider during setT calls
        def update(t, es=es):
            es.set(t, fCommand = 0)
        if not hasattr(self, "setTHooks"):
            self.setTHooks = []
        self.setTHooks.append(update)
        self.setWantsTCallback(1)
        # Clear out function on destroy
        def onDestroy(e, s=self, u=update):
            if u in s.setTHooks:
                s.setTHooks.remove(u)
        tl.bind('<Destroy>', onDestroy)

Dtool_funcToMethod(popupControls, CInterval)
del popupControls
#####################################################################
