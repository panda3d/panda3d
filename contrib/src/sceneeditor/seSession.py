#################################################################
# seSession.py
# Originally from DirectSession.py
# Altered by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#
# We took out a lot of stuff we don't need in the sceneeditor.
# This is also the main reason we didn't just inherite the original directSession.
# Also, the way of selecting, renaming and some hot-key controls are changed.
#
#################################################################
from direct.showbase.DirectObject import *
from direct.directtools.DirectGlobals import *
from direct.directtools.DirectUtil import*
from direct.interval.IntervalGlobal import *
from seCameraControl import *
from direct.directtools.DirectManipulation import *
from direct.directtools.DirectSelection import *
from direct.directtools.DirectGrid import *
from seGeometry import *
from direct.tkpanels import Placer
from direct.tkwidgets import Slider
from direct.gui import OnscreenText
import types
import string
from direct.showbase import Loader

from direct.directtools.DirectSession import DirectSession

class SeSession(DirectSession):  ### Customized DirectSession

    def __init__(self):
        DirectSession.__init__(self)
        __builtins__["SEditor"] = __builtins__["direct"] = base.direct = self

        self.enableAutoCamera = True

    def enable(self):
        if self.fEnabled:
            return
        DirectSession.enable(self)

        if self.enableAutoCamera:
            self.accept('DH_LoadingComplete', self.autoCameraMove)

    def disable(self):
        self.ignore('DH_LoadingComplete')
        DirectSession.disable(self)

    def inputHandler(self, input):
        DirectSession.inputHandler(self, input)

        # Scene Editor custom deal with keyboard and mouse input
        if input == 'mouse1-up':
            messenger.send('DIRECT-mouse1Up')
            if base.direct.widget.fActive:
                messenger.send('shift-f')
        elif input == 'mouse2-up':
            messenger.send('DIRECT-mouse2Up')
            if base.direct.widget.fActive:
                messenger.send('shift-f')

        elif input == 'mouse3-up':
            messenger.send('DIRECT-mouse3Up')
            if base.direct.widget.fActive:
                messenger.send('shift-f')

        elif input == 'page_up':
            self.upAncestry()
        elif input == 'page_down':
            self.downAncestry()
        elif input == 'escape':
            self.deselectAll()
        elif input == 'delete':
            taskMgr.remove('followSelectedNodePath')
            #self.removeAllSelected()
            messenger.send('SGE_Remove',[None])
            self.deselectAll()
        elif input == 'v':
            messenger.send('SEditor-ToggleWidgetVis')
            self.toggleWidgetVis()
            if base.direct.widget.fActive:
                messenger.send('shift-f')
        elif input == 'b':
            messenger.send('SEditor-ToggleBackface')
            base.toggleBackface()
        #elif input == 'control-f':
        #    self.flash(last)
        elif input == 'shift-l':
            self.cameraControl.toggleCOALock()
        elif input == 'o':
            self.oobe()
        elif input == 'p':
            if self.selected.last:
                self.setActiveParent(self.selected.last)
        elif input == 'r':
            # Do wrt reparent
            if self.selected.last:
                self.reparent(self.selected.last, fWrt = 1)
        elif input == 'shift-r':
            # Do regular reparent
            if self.selected.last:
                self.reparent(self.selected.last)
        elif input == 's':
            if self.selected.last:
                self.select(self.selected.last)
        elif input == 't':
            messenger.send('SEditor-ToggleTexture')
            base.toggleTexture()
        elif input == 'shift-a':
            self.selected.toggleVisAll()
        elif input == 'w':
            messenger.send('SEditor-ToggleWireframe')
            base.toggleWireframe()
        elif (input == '[') or (input == '{'):
            self.undo()
        elif (input == ']') or (input == '}'):
            self.redo()

    def select(self, nodePath, fMultiSelect = 0, fResetAncestry = 1, callback=False):
        dnp = self.selected.select(nodePath, fMultiSelect)
        if dnp:
            messenger.send('DIRECT_preSelectNodePath', [dnp])
            if fResetAncestry:
                # Update ancestry
                self.ancestry = list(dnp.getAncestors())
                self.ancestry.reverse()
                self.ancestryIndex = 0
            # Update the selectedNPReadout
            self.selectedNPReadout.reparentTo(aspect2d)
            self.selectedNPReadout.setText(
                'Selected:' + dnp.getName())
            # Show the manipulation widget
            self.widget.showWidget()
            # Update camera controls coa to this point
            # Coa2Camera = Coa2Dnp * Dnp2Camera
            mCoa2Camera = dnp.mCoa2Dnp * dnp.getMat(base.direct.camera)
            row = mCoa2Camera.getRow(3)
            coa = Vec3(row[0], row[1], row[2])
            self.cameraControl.updateCoa(coa)
            # Adjust widgets size
            # This uses the additional scaling factor used to grow and
            # shrink the widget
            self.widget.setScalingFactor(dnp.getRadius())
            # Spawn task to have object handles follow the selected object
            taskMgr.remove('followSelectedNodePath')
            t = Task(self.followSelectedNodePathTask)
            t.dnp = dnp
            taskMgr.add(t, 'followSelectedNodePath')
            # Send an message marking the event
            messenger.send('DIRECT_selectedNodePath', [dnp])
            if callback:
                messenger.send('se_selectedNodePath', [dnp, False])
            else:
                messenger.send('se_selectedNodePath', [dnp])

            self.upAncestry()

            if base.direct.widget.fActive:
                messenger.send('shift-f')

    def deselectAll(self):
        DirectSession.deselectAll(self)
        messenger.send('se_deselectedAll')

    def setActiveParent(self, nodePath = None):
        DirectSession.setActiveParent(self, nodePath)
        self.activeParentReadout.show()

    def reparent(self, nodePath = None, fWrt = 0):
        if (nodePath and self.activeParent and
            self.isNotCycle(nodePath, self.activeParent)):
            DirectSession.reparent(self, nodePath, fWrt)
            messenger.send('SGE_Update Explorer',[render])
            self.activeParentReadout.hide()

    def upAncestry(self):
        if self.ancestry:
            l = len(self.ancestry)
            i = self.ancestryIndex + 1
            if i < l:
                np = self.ancestry[i]
                name = np.getName()
                if i>0:
                    type = self.ancestry[i-1].node().getType().getName()
                else:
                    type = self.ancestry[0].node().getType().getName()

                ntype = np.node().getType().getName()
                if (name != 'render') and (name != 'renderTop')and(self.checkTypeNameForAncestry(type, ntype)):
                    self.ancestryIndex = i
                    self.select(np, 0, 0, True)
                    self.flash(np)

    def checkTypeNameForAncestry(self, type, nextType ):
        if (type=='ModelRoot'):
            if (nextType=='AmbientLight')or(nextType=='PointLight')or(nextType=='DirectionalLight')or(nextType=='Spotlight'):
                return True
            return False
        elif (type=='ModelNode'):
            if (nextType=='ModelNode'):
                return True
            return False
        elif (type=='CollisionNode'):
            return False
        elif (type=='ActorNode'):
            return False
        elif (type=='AmbientLight')or(type=='PointLight')or(type=='DirectionalLight')or(type=='Spotlight'):
            return False
        else:
            return True

    def downAncestry(self):
        if self.ancestry:
            l = len(self.ancestry)
            i = self.ancestryIndex - 1
            if i >= 0:
                np = self.ancestry[i]
                name = np.getName()
                if (name != 'render') and (name != 'renderTop'):
                    self.ancestryIndex = i
                    self.select(np, 0, 0, True)
                    self.flash(np)

    # CUSTOM UTILITY FUNCTIONS
    def toggleAutoCamera(self):
        self.enableAutoCamera = (self.enableAutoCamera+1)%2
        if self.enableAutoCamera==1:
            self.accept('DH_LoadingComplete', self.autoCameraMove)
        else:
            self.ignore('DH_LoadingComplete')
        return

    def autoCameraMove(self, nodePath):
        time = 1
        node = DirectNodePath(nodePath)
        radius = node.getRadius()
        center = node.getCenter()
        node.dehighlight()
        posB = base.camera.getPos()
        hprB = base.camera.getHpr()
        posE = Point3((radius*-1.41)+center.getX(), (radius*-1.41)+center.getY(), (radius*1.41)+center.getZ())
        hprE = Point3(-45, -38, 0)
        print(posB, hprB)
        print(posE, hprE)
        posInterval1 = base.camera.posInterval(time, posE, bakeInStart = 1)
        posInterval2 = base.camera.posInterval(time, posB, bakeInStart = 1)

        hprInterval1 = base.camera.hprInterval(time, hprE, bakeInStart = 1)
        hprInterval2 = base.camera.hprInterval(time, hprB, bakeInStart = 1)

        parallel1 = Parallel(posInterval1, hprInterval1)
        parallel2 = Parallel(posInterval2, hprInterval2)

        Sequence(Wait(7), parallel1, Wait(1), parallel2).start()

        return

