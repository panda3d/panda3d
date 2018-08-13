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
from seManipulation import *
from seSelection import *
from seGrid import *
from seGeometry import *
from direct.tkpanels import Placer
from direct.tkwidgets import Slider
from direct.gui import OnscreenText
import types
import string
from direct.showbase import Loader

class SeSession(DirectObject):  ### Customized DirectSession

    def __init__(self):
        # Establish a global pointer to the direct object early on
        # so dependant classes can access it in their code
        __builtins__["SEditor"] = self
        # These come early since they are used later on
        self.group = render.attachNewNode('SEditor')
        self.font = TextNode.getDefaultFont()
        self.fEnabled = 0
        self.drList = DisplayRegionList()
        self.iRayList = map(lambda x: x.iRay, self.drList)
        self.dr = self.drList[0]
        self.camera = base.camera
        self.trueCamera = self.camera
        self.iRay = self.dr.iRay
        self.coaMode = COA_ORIGIN

        self.enableAutoCamera = True

        self.cameraControl = DirectCameraControl()
        self.manipulationControl = DirectManipulationControl()
        self.useObjectHandles()
        self.grid = DirectGrid()
        self.grid.disable()

        # Initialize the collection of selected nodePaths
        self.selected = SelectedNodePaths()
        # Ancestry of currently selected object
        self.ancestry = []
        self.ancestryIndex = 0
        self.activeParent = None

        self.selectedNPReadout = OnscreenText.OnscreenText(
            pos = (-1.0, -0.9), bg=Vec4(1,1,1,1),
            scale = 0.05, align = TextNode.ALeft,
            mayChange = 1, font = self.font)
        # Make sure readout is never lit or drawn in wireframe
        useDirectRenderStyle(self.selectedNPReadout)
        self.selectedNPReadout.reparentTo(hidden)

        self.activeParentReadout = OnscreenText.OnscreenText(
            pos = (-1.0, -0.975), bg=Vec4(1,1,1,1),
            scale = 0.05, align = TextNode.ALeft,
            mayChange = 1, font = self.font)
        # Make sure readout is never lit or drawn in wireframe
        useDirectRenderStyle(self.activeParentReadout)
        self.activeParentReadout.reparentTo(hidden)

        self.directMessageReadout = OnscreenText.OnscreenText(
            pos = (-1.0, 0.9), bg=Vec4(1,1,1,1),
            scale = 0.05, align = TextNode.ALeft,
            mayChange = 1, font = self.font)
        # Make sure readout is never lit or drawn in wireframe
        useDirectRenderStyle(self.directMessageReadout)
        self.directMessageReadout.reparentTo(hidden)

        self.fControl = 0
        self.fAlt = 0
        self.fShift = 0

        self.pos = VBase3()
        self.hpr = VBase3()
        self.scale = VBase3()

        self.hitPt = Point3(0.0)

        # Lists for managing undo/redo operations
        self.undoList = []
        self.redoList = []

        # One run through the context task to init everything
        self.drList.updateContext()
        for dr in self.drList:
            dr.camUpdate()



        self.modifierEvents = ['control', 'control-up',
                              'shift', 'shift-up',
                              'alt', 'alt-up',
                              ]
        self.keyEvents = ['escape', 'delete', 'page_up', 'page_down',
                          '[', '{', ']', '}',
                          'shift-a', 'b', 'control-f',
                          'l', 'shift-l', 'o', 'p', 'r',
                          'shift-r', 's', 't', 'v', 'w']
        self.mouseEvents = ['mouse1', 'mouse1-up',
                            'shift-mouse1', 'shift-mouse1-up',
                            'control-mouse1', 'control-mouse1-up',
                            'alt-mouse1', 'alt-mouse1-up',
                            'mouse2', 'mouse2-up',
                            'shift-mouse2', 'shift-mouse2-up',
                            'control-mouse2', 'control-mouse2-up',
                            'alt-mouse2', 'alt-mouse2-up',
                            'mouse3', 'mouse3-up',
                            'shift-mouse3', 'shift-mouse3-up',
                            'control-mouse3', 'control-mouse3-up',
                            'alt-mouse3', 'alt-mouse3-up',
                            ]

    def enable(self):
        if self.fEnabled:
            return
        # Make sure old tasks are shut down
        self.disable()
        # Start all display region context tasks
        self.drList.spawnContextTask()
        # Turn on mouse Flying
        self.cameraControl.enableMouseFly()
        # Turn on object manipulation
        self.manipulationControl.enableManipulation()
        # Make sure list of selected items is reset
        self.selected.reset()
        # Accept appropriate hooks
        self.enableKeyEvents()
        self.enableModifierEvents()
        self.enableMouseEvents()
        # Set flag
        self.fEnabled = 1

        if self.enableAutoCamera:
            self.accept('DH_LoadingComplete', self.autoCameraMove)

    def disable(self):
        # Shut down all display region context tasks
        self.drList.removeContextTask()
        # Turn off camera fly
        self.cameraControl.disableMouseFly()
        # Turn off object manipulation
        self.manipulationControl.disableManipulation()
        self.disableKeyEvents()
        self.disableModifierEvents()
        self.disableMouseEvents()
        self.ignore('DH_LoadingComplete')
        # Kill tasks
        taskMgr.remove('flashNodePath')
        taskMgr.remove('hideDirectMessage')
        taskMgr.remove('hideDirectMessageLater')
        # Set flag
        self.fEnabled = 0

    def minimumConfiguration(self):
        # Remove context task
        self.drList.removeContextTask()
        # Turn off camera fly
        self.cameraControl.disableMouseFly()
        # Ignore keyboard and action events
        self.disableKeyEvents()
        self.disableActionEvents()
        # But let mouse events pass through
        self.enableMouseEvents()
        self.enableModifierEvents()

    def oobe(self):
        # If oobeMode was never set, set it to false and create the
        # structures we need to implement OOBE.
        try:
            self.oobeMode
        except:
            self.oobeMode = 0

            self.oobeCamera = hidden.attachNewNode('oobeCamera')

            self.oobeVis = loader.loadModelOnce('models/misc/camera')
            if self.oobeVis:
                self.oobeVis.node().setFinal(1)

        if self.oobeMode:
            # Position a target point to lerp the oobe camera to
            self.cameraControl.camManipRef.iPosHpr(self.trueCamera)
            t = self.oobeCamera.lerpPosHpr(
                Point3(0), Vec3(0), 2.0,
                other = self.cameraControl.camManipRef,
                task = 'manipulateCamera',
                blendType = 'easeInOut')
            # When move is done, switch to oobe mode
            t.uponDeath = self.endOOBE
        else:
            # Place camera marker at true camera location
            self.oobeVis.reparentTo(self.trueCamera)
            # Remove any transformation on the models arc
            self.oobeVis.clearMat()
            # Make oobeCamera be a sibling of wherever camera is now.
            cameraParent = self.camera.getParent()
            # Prepare oobe camera
            self.oobeCamera.reparentTo(cameraParent)
            self.oobeCamera.iPosHpr(self.trueCamera)
            # Put camera under new oobe camera
            base.cam.reparentTo(self.oobeCamera)
            # Position a target point to lerp the oobe camera to
            self.cameraControl.camManipRef.setPos(
                self.trueCamera, Vec3(-2,-20, 5))
            self.cameraControl.camManipRef.lookAt(self.trueCamera)
            t = self.oobeCamera.lerpPosHpr(
                Point3(0), Vec3(0), 2.0,
                other = self.cameraControl.camManipRef,
                task = 'manipulateCamera',
                blendType = 'easeInOut')
            # When move is done, switch to oobe mode
            t.uponDeath = self.beginOOBE

    def beginOOBE(self, state):
        # Make sure we've reached our final destination
        self.oobeCamera.iPosHpr(self.cameraControl.camManipRef)
        self.camera = self.oobeCamera
        self.oobeMode = 1

    def endOOBE(self, state):
        # Make sure we've reached our final destination
        self.oobeCamera.iPosHpr(self.trueCamera)
        # Disable OOBE mode.
        base.cam.reparentTo(self.trueCamera)
        self.camera = self.trueCamera
        # Get rid of ancillary node paths
        self.oobeVis.reparentTo(hidden)
        self.oobeCamera.reparentTo(hidden)
        self.oobeMode = 0

    def destroy(self):
        self.disable()

    def reset(self):
        self.enable()

    # EVENT FUNCTIONS
    def enableModifierEvents(self):
        for event in self.modifierEvents:
            self.accept(event, self.inputHandler, [event])

    def enableKeyEvents(self):
        for event in self.keyEvents:
            self.accept(event, self.inputHandler, [event])

    def enableMouseEvents(self):
        for event in self.mouseEvents:
            self.accept(event, self.inputHandler, [event])

    def disableModifierEvents(self):
        for event in self.modifierEvents:
            self.ignore(event)

    def disableKeyEvents(self):
        for event in self.keyEvents:
            self.ignore(event)

    def disableMouseEvents(self):
        for event in self.mouseEvents:
            self.ignore(event)

    def inputHandler(self, input):
        # Deal with keyboard and mouse input
        if input == 'mouse1-up':
            messenger.send('DIRECT-mouse1Up')
            if SEditor.widget.fActive:
                messenger.send('shift-f')
        elif input.find('mouse1') != -1:
            modifiers = self.getModifiers(input, 'mouse1')
            messenger.send('DIRECT-mouse1', sentArgs = [modifiers])
        elif input == 'mouse2-up':
            messenger.send('DIRECT-mouse2Up')
            if SEditor.widget.fActive:
                messenger.send('shift-f')
        elif input.find('mouse2') != -1:
            modifiers = self.getModifiers(input, 'mouse2')
            messenger.send('DIRECT-mouse2', sentArgs = [modifiers])
        elif input == 'mouse3-up':
            messenger.send('DIRECT-mouse3Up')
            if SEditor.widget.fActive:
                messenger.send('shift-f')
        elif input.find('mouse3') != -1:
            modifiers = self.getModifiers(input, 'mouse3')
            messenger.send('DIRECT-mouse3', sentArgs = [modifiers])
        elif input == 'shift':
            self.fShift = 1
        elif input == 'shift-up':
            self.fShift = 0
        elif input == 'control':
            self.fControl = 1
        elif input == 'control-up':
            self.fControl = 0
        elif input == 'alt':
            self.fAlt = 1
        elif input == 'alt-up':
            self.fAlt = 0
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
            if SEditor.widget.fActive:
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


    def getModifiers(self, input, base):
        modifiers = DIRECT_NO_MOD
        modifierString = input[: input.find(base)]
        if modifierString.find('shift') != -1:
            modifiers |= DIRECT_SHIFT_MOD
        if modifierString.find('control') != -1:
            modifiers |= DIRECT_CONTROL_MOD
        if modifierString.find('alt') != -1:
            modifiers |= DIRECT_ALT_MOD
        return modifiers

    def gotShift(self, modifiers):
        return modifiers & DIRECT_SHIFT_MOD

    def gotControl(self, modifiers):
        return modifiers & DIRECT_CONTROL_MOD

    def gotAlt(self, modifiers):
        return modifiers & DIRECT_ALT_MOD

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
            mCoa2Camera = dnp.mCoa2Dnp * dnp.getMat(self.camera)
            row = mCoa2Camera.getRow(3)
            coa = Vec3(row[0], row[1], row[2])
            self.cameraControl.updateCoa(coa)
            # Adjust widgets size
            # This uses the additional scaling factor used to grow and
            # shrink the widget
            self.widget.setScalingFactor(dnp.getRadius())
            # Spawn task to have object handles follow the selected object
            taskMgr.remove('followSelectedNodePath')
            t = Task.Task(self.followSelectedNodePathTask)
            t.dnp = dnp
            taskMgr.add(t, 'followSelectedNodePath')
            # Send an message marking the event
            messenger.send('DIRECT_selectedNodePath', [dnp])
            if callback:
                messenger.send('se_selectedNodePath', [dnp, False])
            else:
                messenger.send('se_selectedNodePath', [dnp])

            self.upAncestry()

            if SEditor.widget.fActive:
                messenger.send('shift-f')

    def followSelectedNodePathTask(self, state):
        mCoa2Render = state.dnp.mCoa2Dnp * state.dnp.getMat(render)
        decomposeMatrix(mCoa2Render,
                        self.scale,self.hpr,self.pos,
                        CSDefault)
        self.widget.setPosHpr(self.pos,self.hpr)
        return Task.cont

    def deselect(self, nodePath):
        dnp = self.selected.deselect(nodePath)
        if dnp:
            # Hide the manipulation widget
            self.widget.hideWidget()
            self.selectedNPReadout.reparentTo(hidden)
            self.selectedNPReadout.setText(' ')
            taskMgr.remove('followSelectedNodePath')
            self.ancestry = []
            # Send an message marking the event
            messenger.send('DIRECT_deselectedNodePath', [dnp])

    def deselectAll(self):
        self.selected.deselectAll()
        # Hide the manipulation widget
        self.widget.hideWidget()
        self.selectedNPReadout.reparentTo(hidden)
        self.selectedNPReadout.setText(' ')
        taskMgr.remove('followSelectedNodePath')
        messenger.send('se_deselectedAll')

    def setActiveParent(self, nodePath = None):
        # Record new parent
        self.activeParent = nodePath
        # Update the activeParentReadout
        self.activeParentReadout.reparentTo(aspect2d)
        self.activeParentReadout.setText(
            'Active Reparent Target:' + nodePath.getName())
        # Alert everyone else
        self.activeParentReadout.show()

    def reparent(self, nodePath = None, fWrt = 0):
        if (nodePath and self.activeParent and
            self.isNotCycle(nodePath, self.activeParent)):
            oldParent = nodePath.getParent()
            if fWrt:
                nodePath.wrtReparentTo(self.activeParent)
            else:
                nodePath.reparentTo(self.activeParent)
            # Alert everyone else
            messenger.send('DIRECT_reparent',
                           [nodePath, oldParent, self.activeParent])
            messenger.send('SGE_Update Explorer',[render])
            self.activeParentReadout.hide()


    def isNotCycle(self, nodePath, parent):
        if nodePath.get_key() == parent.get_key():
            print('DIRECT.reparent: Invalid parent')
            return 0
        elif parent.hasParent():
            return self.isNotCycle(nodePath, parent.getParent())
        else:
            return 1

    def fitOnNodePath(self, nodePath = 'None Given'):
        if nodePath == 'None Given':
            # If nothing specified, try selected node path
            nodePath = self.selected.last
        SEditor.select(nodePath)
        def fitTask(state, self = self):
            self.cameraControl.fitOnWidget()
            return Task.done
        taskMgr.doMethodLater(0.1, fitTask, 'manipulateCamera')

    def isolate(self, nodePath = 'None Given'):
        """ Show a node path and hide its siblings """
        # First kill the flashing task to avoid complications
        taskMgr.remove('flashNodePath')
        # Use currently selected node path if node selected
        if nodePath == 'None Given':
            nodePath = self.selected.last
        # Do we have a node path?
        if nodePath:
            # Yes, show everything in level
            self.showAllDescendants(nodePath.getParent())
            # Now hide all of this node path's siblings
            nodePath.hideSiblings()

    def toggleVis(self, nodePath = 'None Given'):
        """ Toggle visibility of node path """
        # First kill the flashing task to avoid complications
        taskMgr.remove('flashNodePath')
        if nodePath == 'None Given':
            # If nothing specified, try selected node path
            nodePath = self.selected.last
        if nodePath:
            # Now toggle node path's visibility state
            if nodePath.is_hidden():
                nodePath.show()
            else:
                nodePath.hide()

    def removeNodePath(self, nodePath = 'None Given'):
        if nodePath == 'None Given':
            # If nothing specified, try selected node path
            nodePath = self.selected.last
        if nodePath:
            nodePath.remove()

    def removeAllSelected(self):
        self.selected.removeAll()

    def showAllDescendants(self, nodePath = render):
        """ Show the level and its descendants """
        nodePath.showAllDescendants()
        nodePath.hideCS()

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


    def getAndSetName(self, nodePath):
        """ Prompt user for new node path name """
        from tkSimpleDialog import askstring
        newName = askstring('Node Path: ' + nodePath.getName(),
                            'Enter new name:')
        if newName:
            nodePath.setName(newName)
            messenger.send('DIRECT_nodePathSetName', [nodePath, newName])

    # UNDO REDO FUNCTIONS
    def pushUndo(self, nodePathList, fResetRedo = 1):
        # Assemble group of changes
        undoGroup = []
        for nodePath in nodePathList:
            t = nodePath.getTransform()
            undoGroup.append([nodePath, t])
        # Now record group
        self.undoList.append(undoGroup)
        # Truncate list
        self.undoList = self.undoList[-25:]
        # Alert anyone who cares
        messenger.send('DIRECT_pushUndo')
        if fResetRedo and (nodePathList != []):
            self.redoList = []
            messenger.send('DIRECT_redoListEmpty')

    def popUndoGroup(self):
        # Get last item
        undoGroup = self.undoList[-1]
        # Strip last item off of undo list
        self.undoList = self.undoList[:-1]
        # Update state of undo button
        if not self.undoList:
            messenger.send('DIRECT_undoListEmpty')
        # Return last item
        return undoGroup

    def pushRedo(self, nodePathList):
        # Assemble group of changes
        redoGroup = []
        for nodePath in nodePathList:
            t = nodePath.getTransform()
            redoGroup.append([nodePath, t])
        # Now record redo group
        self.redoList.append(redoGroup)
        # Truncate list
        self.redoList = self.redoList[-25:]
        # Alert anyone who cares
        messenger.send('DIRECT_pushRedo')

    def popRedoGroup(self):
        # Get last item
        redoGroup = self.redoList[-1]
        # Strip last item off of redo list
        self.redoList = self.redoList[:-1]
        # Update state of redo button
        if not self.redoList:
            messenger.send('DIRECT_redoListEmpty')
        # Return last item
        return redoGroup

    def undo(self):
        if self.undoList:
            # Get last item off of redo list
            undoGroup = self.popUndoGroup()
            # Record redo information
            nodePathList = map(lambda x: x[0], undoGroup)
            self.pushRedo(nodePathList)
            # Now undo xform for group
            for pose in undoGroup:
                # Undo xform
                pose[0].setTransform(pose[1])
            # Alert anyone who cares
            messenger.send('DIRECT_undo')

    def redo(self):
        if self.redoList:
            # Get last item off of redo list
            redoGroup = self.popRedoGroup()
            # Record undo information
            nodePathList = map(lambda x: x[0], redoGroup)
            self.pushUndo(nodePathList, fResetRedo = 0)
            # Redo xform
            for pose in redoGroup:
                pose[0].setTransform(pose[1])
            # Alert anyone who cares
            messenger.send('DIRECT_redo')

    # UTILITY FUNCTIONS
    def message(self, text):
        taskMgr.remove('hideDirectMessage')
        taskMgr.remove('hideDirectMessageLater')
        self.directMessageReadout.reparentTo(aspect2d)
        self.directMessageReadout.setText(text)
        self.hideDirectMessageLater()

    def hideDirectMessageLater(self):
        taskMgr.doMethodLater(3.0, self.hideDirectMessage, 'hideDirectMessage')

    def hideDirectMessage(self, state):
        self.directMessageReadout.reparentTo(hidden)
        return Task.done

    def useObjectHandles(self):
        self.widget = self.manipulationControl.objectHandles
        self.widget.reparentTo(SEditor.group)

    def hideSelectedNPReadout(self):
        self.selectedNPReadout.reparentTo(hidden)

    def hideActiveParentReadout(self):
        self.activeParentReadout.reparentTo(hidden)

    def toggleWidgetVis(self):
        self.widget.toggleWidget()

    def setCOAMode(self, mode):
        self.coaMode = mode

    def isEnabled(self):
        return self.fEnabled

    def addUnpickable(self, item):
        for iRay in self.iRayList:
            iRay.addUnpickable(item)

    def removeUnpickable(self, item):
        for iRay in self.iRayList:
            iRay.removeUnpickable(item)

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


class DisplayRegionContext(DirectObject):
    regionCount = 0
    def __init__(self, cam):
        self.cam = cam
        self.camNode = self.cam.node()
        self.camLens = self.camNode.getLens()
        # set lens change callback
        changeEvent = 'dr%d-change-event' % DisplayRegionContext.regionCount
        DisplayRegionContext.regionCount += 1
        self.camLens.setChangeEvent(changeEvent)
        self.accept(changeEvent, self.camUpdate)
        self.iRay = SelectionRay(self.cam)
        self.nearVec = Vec3(0)
        self.mouseX = 0.0
        self.mouseY = 0.0
        # A Camera node can have more than one display region
        # associated with it.  Here I assume that there is only
        # one display region per camera, since we are defining a
        # display region on a per-camera basis.  See note in
        # DisplayRegionList.__init__()
        try:
            self.dr = self.camNode.getDr(0)
        except:
            self.dr = self.camNode.getDisplayRegion(0)
        left = self.dr.getLeft()
        right = self.dr.getRight()
        bottom = self.dr.getBottom()
        top = self.dr.getTop()
        self.originX = left+right-1
        self.originY = top+bottom-1
        self.scaleX = 1.0/(right-left)
        self.scaleY = 1.0/(top-bottom)
        self.setOrientation()
        self.camUpdate()

    def __getitem__(self,key):
        return self.__dict__[key]

    def setOrientation(self):
        # MRM This assumes orientation is set on transform above cam
        hpr = self.cam.getHpr()
        if hpr[2] < 135 and hpr[2]>45 or hpr[2]>225 and hpr[2]<315:
            self.isSideways = 1
        elif hpr[2] > -135 and hpr[2] < -45 or hpr[2] < -225 and hpr[2] > -315:
            self.isSideways = 1
        else:
            self.isSideways = 0

    # The following take into consideration sideways displays
    def getHfov(self):
        if self.isSideways:
            return self.camLens.getVfov()
        else:
            return self.camLens.getHfov()

    def getVfov(self):
        if self.isSideways:
            return self.camLens.getHfov()
        else:
            return self.camLens.getVfov()

    def setHfov(self,hfov):
        if self.isSideways:
            self.camLens.setFov(self.camLens.getHfov(), hfov)
        else:
            self.camLens.setFov(hfov, self.camLens.getVfov())

    def setVfov(self,vfov):
        if self.isSideways:
            self.camLens.setFov(vfov, self.camLens.getVfov())
        else:
            self.camLens.setFov(self.camLens.getHfov(), vfov)

    def setFov(self,hfov,vfov):
        if self.isSideways:
            self.camLens.setFov(vfov, hfov)
        else:
            self.camLens.setFov(hfov, vfov)

    def getWidth(self):
        prop = base.win.getProperties()
        if prop.hasSize():
            return prop.getXSize()
        else:
            return 640

    def getHeight(self):
        prop = base.win.getProperties()
        if prop.hasSize():
            return prop.getYSize()
        else:
            return 480

    def camUpdate(self, lens = None):
        # Window Data
        self.near = self.camLens.getNear()
        self.far = self.camLens.getFar()
        self.fovH = self.camLens.getHfov()
        self.fovV = self.camLens.getVfov()
        self.nearWidth = math.tan(deg2Rad(self.fovH * 0.5)) * self.near * 2.0
        self.nearHeight = math.tan(deg2Rad(self.fovV * 0.5)) * self.near * 2.0
        self.left = -self.nearWidth * 0.5
        self.right = self.nearWidth * 0.5
        self.top = self.nearHeight * 0.5
        self.bottom = -self.nearHeight * 0.5

    def mouseUpdate(self):
        # Mouse Data
        # Last frame
        self.mouseLastX = self.mouseX
        self.mouseLastY = self.mouseY
        # Values for this frame
        # This ranges from -1 to 1
        if (base.mouseWatcherNode.hasMouse()):
            self.mouseX = base.mouseWatcherNode.getMouseX()
            self.mouseY = base.mouseWatcherNode.getMouseY()
            self.mouseX = (self.mouseX-self.originX)*self.scaleX
            self.mouseY = (self.mouseY-self.originY)*self.scaleY
        # Delta percent of window the mouse moved
        self.mouseDeltaX = self.mouseX - self.mouseLastX
        self.mouseDeltaY = self.mouseY - self.mouseLastY
        self.nearVec.set((self.nearWidth*0.5) * self.mouseX,
                         self.near,
                         (self.nearHeight*0.5) * self.mouseY)

class DisplayRegionList(DirectObject):
    def __init__(self):
        self.displayRegionList = []
        i = 0
        # Things are funky if we are oobe
        if (hasattr(base, 'oobeMode') and base.oobeMode):
            # assume we only have one cam at this point
            drc = DisplayRegionContext(base.cam)
            self.displayRegionList.append(drc)
        else:
            # MRM: Doesn't properly handle multiple camera groups anymore
            # Assumes everything is under main camera

                # This is following the old way of setting up
                # display regions.  A display region is set up for
                # each camera node in the scene graph.  This was done
                # so that only display regions in the scene graph are
                # considered.  The right way to do this is to set up
                # a display region for each real display region, and then
                # keep track of which are currently active (e.g. use a flag)
                # processing only them.
                for camIndex in range(len(base.camList)):
                    cam = base.camList[camIndex]
                    if cam.getName()=='<noname>':
                        cam.setName('Camera%d' % camIndex)
                    drc = DisplayRegionContext(cam)
                    self.displayRegionList.append(drc)

        self.accept("DIRECT-mouse1",self.mouseUpdate)
        self.accept("DIRECT-mouse2",self.mouseUpdate)
        self.accept("DIRECT-mouse3",self.mouseUpdate)
        self.accept("DIRECT-mouse1Up",self.mouseUpdate)
        self.accept("DIRECT-mouse2Up",self.mouseUpdate)
        self.accept("DIRECT-mouse3Up",self.mouseUpdate)

    def __getitem__(self, index):
        return self.displayRegionList[index]

    def __len__(self):
        return len(self.displayRegionList)

    def updateContext(self):
        self.contextTask(None)

    def setNearFar(self, near, far):
        for dr in self.displayRegionList:
            dr.camLens.setNearFar(near, far)

    def setNear(self, near):
        for dr in self.displayRegionList:
            dr.camLens.setNear(near)

    def setFar(self, far):
        for dr in self.displayRegionList:
            dr.camLens.setFar(far)

    def setFov(self, hfov, vfov):
        for dr in self.displayRegionList:
            dr.setFov(hfov, vfov)

    def setHfov(self, fov):
        for dr in self.displayRegionList:
            dr.setHfov(fov)

    def setVfov(self, fov):
        for dr in self.displayRegionList:
            dr.setVfov(fov)

    def mouseUpdate(self, modifiers = DIRECT_NO_MOD):
        for dr in self.displayRegionList:
            dr.mouseUpdate()
        SEditor.dr = self.getCurrentDr()

    def getCurrentDr(self):
        for dr in self.displayRegionList:
            if (dr.mouseX >= -1.0 and dr.mouseX <= 1.0 and
                dr.mouseY >= -1.0 and dr.mouseY <= 1.0):
                return dr
        return self.displayRegionList[0]

    def start(self):
        # First shutdown any existing task
        self.stop()
        # Start a new context task
        self.spawnContextTask()

    def stop(self):
        # Kill the existing context task
        taskMgr.remove('DIRECTContextTask')

    def spawnContextTask(self):
        taskMgr.add(self.contextTask, 'DIRECTContextTask')

    def removeContextTask(self):
        taskMgr.remove('DIRECTContextTask')

    def contextTask(self, state):
        # Window Data
        self.mouseUpdate()
        # hack to test movement
        return Task.cont

# Create one
#__builtins__['direct'] = base.direct = DirectSession()












