from PandaObject import *
from DirectUtil import *
from DirectCameraControl import *
from DirectManipulation import *
from DirectSelection import *
from DirectGrid import *
from DirectGeometry import *
from DirectLights import *
from DirectSessionPanel import *
from tkSimpleDialog import askstring
import Placer
import EntryScale
import SceneGraphExplorer
import OnscreenText
import types
import __builtin__

DIRECT_FLASH_DURATION = 1.5

class DirectSession(PandaObject):

    def __init__(self):
        # Establish a global pointer to the direct object early on
        # so dependant classes can access it in their code
        __builtin__.direct = self
        self.fEnabled = 0
        self.drList = DisplayRegionList()
        self.iRayList = map(lambda x: x.iRay, self.drList)
        self.dr = self.drList[0]
        self.camera = self.dr.camera
        self.iRay = self.dr.iRay

        self.group = render.attachNewNode('DIRECT')
        self.cameraControl = DirectCameraControl()
        self.manipulationControl = DirectManipulationControl()
        self.useObjectHandles()
        self.grid = DirectGrid()
        self.grid.disable()
        self.lights = DirectLights(direct.group)
        # Create some default lights
        self.lights.createDefaultLights()
        # But turn them off
        self.lights.allOff()

        # Initialize the collection of selected nodePaths
        self.selected = SelectedNodePaths()
        # Ancestry of currently selected object
        self.ancestry = []
        self.ancestryIndex = 0
        self.activeParent = None

        self.selectedNPReadout = OnscreenText.OnscreenText(
            pos = (-1.0, -0.9), bg=Vec4(1,1,1,1),
            scale = 0.05, align = TMALIGNLEFT)
        # Make sure readout is never lit or drawn in wireframe
        useDirectRenderStyle(self.selectedNPReadout)
        self.selectedNPReadout.reparentTo( hidden )

        self.activeParentReadout = OnscreenText.OnscreenText(
            pos = (-1.0, -0.975), bg=Vec4(1,1,1,1),
            scale = 0.05, align = TMALIGNLEFT)
        # Make sure readout is never lit or drawn in wireframe
        useDirectRenderStyle(self.activeParentReadout)
        self.activeParentReadout.reparentTo( hidden )

        # Create a vrpn client vrpn-server or default
        if base.config.GetBool('want-vrpn', 0):
            from DirectDeviceManager import *
            self.deviceManager = DirectDeviceManager()
            # Automatically create any devices specified in config file
            joybox = base.config.GetString('vrpn-joybox-device', '')
            if joybox:
                from DirectJoybox import *
                self.joybox = DirectJoybox(joybox)
            else:
                self.joybox = None
        else:
            self.deviceManager = None
            self.joybox = None

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

        self.actionEvents = [
            ['select', self.select],
            ['deselect', self.deselect],
            ['deselectAll', self.deselectAll],
            ['highlightAll', self.selected.highlightAll],
            ['preRemoveNodePath', self.deselect],
            # Scene graph explorer functions
            ['SGE_Select', self.select],
            ['SGE_Deselect', self.deselect],
            ['SGE_Set Parent', self.setActiveParent],
            ['SGE_Reparent', self.reparent],
            ['SGE_WRT Reparent', lambda np, s=self: s.reparent(np, fWrt = 1)],
            ['SGE_Flash', self.flash],
            ['SGE_Isolate', self.isolate],
            ['SGE_Toggle Vis', self.toggleVis],
            ['SGE_Show All', self.showAllDescendants],
            ['SGE_Fit', self.fitOnNodePath],
            ['SGE_Place', Placer.place],
            ['SGE_Set Color', EntryScale.rgbPanel],
            ['SGE_Explore', SceneGraphExplorer.explore],
            ['SGE_Delete', self.removeNodePath],
            ['SGE_Set Name', self.getAndSetName],
            ]
        self.keyEvents = ['escape', 'delete', 'control', 'control-up',
                          'shift', 'shift-up', 'alt', 'alt-up',
                          'page_up', 'page_down', 
                          '[', '{', ']', '}',
                          'A', 'b', 'l', 'p', 'r', 'R', 's', 't', 'v', 'w']
        self.mouseEvents = ['mouse1', 'mouse1-up',
                            'mouse2', 'mouse2-up',
                            'mouse3', 'mouse3-up']

        if base.wantTk:
            from TkGlobal import *
            self.panel = DirectSessionPanel(parent = tkroot)

    def enable(self):
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
        self.enableMouseEvents()
        self.enableActionEvents()
        # Set flag
        self.fEnabled = 1

    def disable(self):
        # Shut down all display region context tasks
        self.drList.removeContextTask()
        # Turn off camera fly
        self.cameraControl.disableMouseFly()
        # Turn off object manipulation
        self.manipulationControl.disableManipulation()
        self.disableKeyEvents()
        self.disableMouseEvents()
        self.disableActionEvents()
        # Set flag
        self.fEnabled = 0

    def toggleDirect(self):
        if self.fEnabled:
            self.disable()
        else:
            self.enable()

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

    def destroy(self):
        self.disable()

    def reset(self):
        self.enable()

    # EVENT FUNCTIONS
    def enableActionEvents(self):
        for event in self.actionEvents:
            self.accept(event[0], event[1], extraArgs = event[2:])

    def enableKeyEvents(self):
        for event in self.keyEvents:
            self.accept(event, self.inputHandler, [event])

    def enableMouseEvents(self):
        for event in self.mouseEvents:
            self.accept(event, self.inputHandler, [event])

    def disableActionEvents(self):
        for event, method in self.actionEvents:
            self.ignore(event)

    def disableKeyEvents(self):
        for event in self.keyEvents:
            self.ignore(event)

    def disableMouseEvents(self):
        for event in self.mouseEvents:
            self.ignore(event)

    def inputHandler(self, input):
        # Deal with keyboard and mouse input
        if input == 'mouse1':
            messenger.send('DIRECT_mouse1')
        elif input == 'mouse1-up':
            messenger.send('DIRECT_mouse1Up')
        elif input == 'mouse2': 
            messenger.send('DIRECT_mouse2')
        elif input == 'mouse2-up':
            messenger.send('DIRECT_mouse2Up')
        elif input == 'mouse3': 
            messenger.send('DIRECT_mouse3')
        elif input == 'mouse3-up':
            messenger.send('DIRECT_mouse3Up')
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
            self.removeAllSelected()
        elif input == 'v':
            self.toggleWidgetVis()
        elif input == 'b':
            base.toggleBackface()
        elif input == 'l':
            self.lights.toggle()
        elif input == 'p':
            if self.selected.last:
                self.setActiveParent(self.selected.last)
        elif input == 'r':
            # Do wrt reparent
            if self.selected.last:
                self.reparent(self.selected.last, fWrt = 1)
        elif input == 'R':
            # Do regular reparent
            if self.selected.last:
                self.reparent(self.selected.last)
        elif input == 's':
            if self.selected.last:
                self.select(self.selected.last)
        elif input == 't':
            base.toggleTexture()
        elif input == 'A':
            self.selected.toggleVisAll()
        elif input == 'w':
            base.toggleWireframe()
        elif (input == '[') or (input == '{'):
            self.undo()
        elif (input == ']') or (input == '}'):
            self.redo()
        
    def select(self, nodePath, fMultiSelect = 0, fResetAncestry = 1):
        dnp = self.selected.select(nodePath, fMultiSelect)
        if dnp:
            messenger.send('DIRECT_preSelectNodePath', [dnp])
            if fResetAncestry:
                # Update ancestry
                self.ancestry = dnp.getAncestry()
                self.ancestry.reverse()
                self.ancestryIndex = 0
            # Update the selectedNPReadout
            self.selectedNPReadout.reparentTo(aspect2d)
            self.selectedNPReadout.setText(
                'Selected:' + dnp.name)
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
            taskMgr.removeTasksNamed('followSelectedNodePath')
            t = Task.Task(self.followSelectedNodePathTask)
            t.dnp = dnp
            taskMgr.spawnTaskNamed(t, 'followSelectedNodePath')
            # Send an message marking the event
            messenger.send('DIRECT_selectedNodePath', [dnp])

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
            taskMgr.removeTasksNamed('followSelectedNodePath')
            self.ancestry = []
            # Send an message marking the event
            messenger.send('DIRECT_deselectedNodePath', [dnp])

    def deselectAll(self):
        self.selected.deselectAll()
        # Hide the manipulation widget
        self.widget.hideWidget()
        self.selectedNPReadout.reparentTo(hidden)
        self.selectedNPReadout.setText(' ')
        taskMgr.removeTasksNamed('followSelectedNodePath')

    def setActiveParent(self, nodePath = None):
        # Record new parent
        self.activeParent = nodePath
        # Update the activeParentReadout
        self.activeParentReadout.reparentTo(aspect2d)
        self.activeParentReadout.setText(
            'Active Parent:' + nodePath.getName())
        # Alert everyone else
        messenger.send('DIRECT_activeParent', [self.activeParent])
        
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

    def isNotCycle(self, nodePath, parent):
        if nodePath.id() == parent.id():
            print 'DIRECT.reparent: Invalid parent'
            return 0
        elif parent.hasParent():
            return self.isNotCycle(nodePath, parent.getParent())
        else:
            return 1
        
    def flash(self, nodePath = 'None Given'):
        """ Highlight an object by setting it red for a few seconds """
        # Clean up any existing task
        taskMgr.removeTasksNamed('flashNodePath')
        # Spawn new task if appropriate
        if nodePath == 'None Given':
            # If nothing specified, try selected node path
            nodePath = self.selected.last
        if nodePath:
            if nodePath.hasColor():
                doneColor = nodePath.getColor()
                flashColor = VBase4(1) - doneColor
                flashColor.setW(1)
            else:
                doneColor = None
                flashColor = VBase4(1,0,0,1)
            # Temporarily set node path color
            nodePath.setColor(flashColor)
            # Clean up color in a few seconds
            t = taskMgr.spawnTaskNamed(
                Task.doLater(DIRECT_FLASH_DURATION,
                             # This is just a dummy task
                             Task.Task(self.flashDummy),
                             'flashNodePath'),
                'flashNodePath')
            t.nodePath = nodePath
            t.doneColor = doneColor
            # This really does all the work
            t.uponDeath = self.flashDone

    def flashDummy(self, state):
        # Real work is done in upon death function
        return Task.done
        
    def flashDone(self,state):
        # Return node Path to original state
        if state.doneColor:
            state.nodePath.setColor(state.doneColor)
        else:
            state.nodePath.clearColor()

    def fitOnNodePath(self, nodePath = 'None Given'):
        if nodePath == 'None Given':
            # If nothing specified, try selected node path
            nodePath = self.selected.last
        direct.select(nodePath)
        def fitTask(state, self = self):
            self.cameraControl.fitOnWidget()
            return Task.done
        taskMgr.doMethodLater(0.1, fitTask, 'manipulateCamera')

    def isolate(self, nodePath = 'None Given'):
        """ Show a node path and hide its siblings """
        # First kill the flashing task to avoid complications
        taskMgr.removeTasksNamed('flashNodePath')
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
        taskMgr.removeTasksNamed('flashNodePath')
        if nodePath == 'None Given':
            # If nothing specified, try selected node path
            nodePath = self.selected.last
        if nodePath:
            # Now toggle node path's visibility state
            nodePath.toggleVis()

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
        nodePath.hideCollisionSolids()

    def upAncestry(self):
        if self.ancestry:
            l = len(self.ancestry)
            i = self.ancestryIndex + 1
            if i < l:
                np = self.ancestry[i]
                name = np.getName()
                if (name != 'render') and (name != 'renderTop'):
                    self.ancestryIndex = i
                    self.select(np, 0, 0)
                    self.flash(np)

    def downAncestry(self):
        if self.ancestry:
            l = len(self.ancestry)
            i = self.ancestryIndex - 1
            if i >= 0:
                np = self.ancestry[i]
                name = np.getName()
                if (name != 'render') and (name != 'renderTop'):
                    self.ancestryIndex = i
                    self.select(np, 0, 0)
                    self.flash(np)

    def getAndSetName(self, nodePath):
        """ Prompt user for new node path name """
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
            pos = nodePath.getPos()
            hpr = nodePath.getHpr()
            scale = nodePath.getScale()
            undoGroup.append([nodePath, pos,hpr,scale])
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
            pos = nodePath.getPos()
            hpr = nodePath.getHpr()
            scale = nodePath.getScale()
            redoGroup.append([nodePath, pos,hpr,scale])
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
                pose[0].setPosHprScale(pose[1], pose[2], pose[3])
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
                pose[0].setPosHprScale(pose[1], pose[2], pose[3])
            # Alert anyone who cares
            messenger.send('DIRECT_redo')

    # UTILITY FUNCTIONS
    def useObjectHandles(self):
        self.widget = self.manipulationControl.objectHandles
        self.widget.reparentTo(direct.group)

    def hideSelectedNPReadout(self):
        self.selectedNPReadout.reparentTo(hidden)

    def hideActiveParentReadout(self):
        self.activeParentReadout.reparentTo(hidden)

    def toggleWidgetVis(self):
        self.widget.toggleWidget()

    def isEnabled(self):
        return self.fEnabled

    def addUnpickable(self, item):
        for iRay in self.iRayList:
            iRay.addUnpickable(item)

    def removeUnpickable(self, item):
        for iRay in self.iRayList:
            iRay.removeUnpickable(item)

class DisplayRegionList:
    def __init__(self):
        self.displayRegionList = []
        for camera in base.cameraList:
            self.displayRegionList.append(
                DisplayRegionContext(base.win, camera))

    def __getitem__(self, index):
        return self.displayRegionList[index]

    def __len__(self):
        return len(self.displayRegionList)

    def updateContext(self):
        for dr in self.displayRegionList:
            dr.contextTask(None)
        
    def spawnContextTask(self):
        for dr in self.displayRegionList:
            dr.start()

    def removeContextTask(self):
        for dr in self.displayRegionList:
            dr.stop()

    def setNearFar(self, near, far):
        for dr in self.displayRegionList:
            dr.camNode.setNearFar(near, far)
    
    def setNear(self, near):
        for dr in self.displayRegionList:
            dr.camNode.setNear(near)
    
    def setFar(self, far):
        for dr in self.displayRegionList:
            dr.camNode.setFar(far)

    def setFov(self, hfov, vfov):
        for dr in self.displayRegionList:
            dr.camNode.setFov(hfov, vfov)

    def setHfov(self, fov):
        for dr in self.displayRegionList:
            dr.camNode.setHfov(fov)

    def setVfov(self, fov):
        for dr in self.displayRegionList:
            dr.camNode.setVfov(fov)

class DisplayRegionContext:
    def __init__(self, win, camera):
        self.win = win
        self.camera = camera
        self.cam = self.camera.find('**/+Camera')
        self.camNode = self.cam.getNode(0)
        self.iRay = SelectionRay(self.camera)
        self.nearVec = Vec3(0)
        self.mouseX = 0.0
        self.mouseY = 0.0

    def __getitem__(self,key):
        return self.__dict__[key]

    def start(self):
        # First shutdown any existing task
        self.stop()
        # Start a new context task
        self.spawnContextTask()

    def stop(self):
        # Kill the existing context task
        taskMgr.removeTasksNamed('DIRECTContextTask')

    def spawnContextTask(self):
        taskMgr.spawnTaskNamed(Task.Task(self.contextTask),
                               'DIRECTContextTask')

    def removeContextTask(self):
        taskMgr.removeTasksNamed('DIRECTContextTask')

    def contextTask(self, state):
        # Window Data
        self.width = self.win.getWidth()
        self.height = self.win.getHeight()
        self.near = self.camNode.getNear()
        self.far = self.camNode.getFar()
        self.fovH = self.camNode.getHfov()
        self.fovV = self.camNode.getVfov()
        self.nearWidth = math.tan(deg2Rad(self.fovH / 2.0)) * self.near * 2.0
        self.nearHeight = math.tan(deg2Rad(self.fovV / 2.0)) * self.near * 2.0
        self.left = -self.nearWidth/2.0
        self.right = self.nearWidth/2.0
        self.top = self.nearHeight/2.0
        self.bottom = -self.nearHeight/2.0
        # Mouse Data
        # Last frame
        self.mouseLastX = self.mouseX
        self.mouseLastY = self.mouseY
        # Values for this frame
        # This ranges from -1 to 1
        if (base.mouseWatcher.node().hasMouse()):
            self.mouseX = base.mouseWatcher.node().getMouseX()
            self.mouseY = base.mouseWatcher.node().getMouseY()
        # Delta percent of window the mouse moved
        self.mouseDeltaX = self.mouseX - self.mouseLastX
        self.mouseDeltaY = self.mouseY - self.mouseLastY
        self.nearVec.set((self.nearWidth/2.0) * self.mouseX,
                         self.near,
                         (self.nearHeight/2.0) * self.mouseY)
        # Continue the task
        return Task.cont

