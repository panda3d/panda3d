from PandaObject import *
from DirectGrid import DirectGrid

class DirectCameraManipulation(PandaObject):
    def __init__():
        # Create the grid
	self.grid = DirectGrid(render)
	self.grid.hide()
	self.hitPt = Point3(0)
	self.iPoint = Point3(0)
	self.centerOfRotation = render.attachNewNode( NamedNode() )
	self.centerOfRotation.node().setName( 'centerOfRotation' )
	self.centerOfRotation.setPosHpr(Vec3(0), Vec3(0))
	self.orthoViewRoll = 0.0
	self.lastView = 0
	self.targetNodePath = render.attachNewNode(NamedNode('targetNode'))
        self.zeroBaseVec = VBase3(0)
        self.zeroVector = Vec3(0)
        self.centerVec = Vec3(0., 1., 0.)
        self.zeroPoint = Point3(0)

    def centerCam(chan):
        # Chan is a display region context
	self.centerCamIn(chan, 1.0)
        
    def centerCamNow(chan):
        self.centerCamIn(chan, 0.)

    def centerCamIn(chan,t):
        # Chan is a display region context
	taskMgr.removeTasksNamed('manipulateCamera')
	widgetToCam = direct.widget.getPos( chan.camera )
	dist = Vec3(widgetToCam - zeroPoint).length()
	scaledCenterVec = centerVec * dist.
	delta = widgetToCam - scaledCenterVec.
	relNodePath = render.attachNewNode(Node())
	relNodePath.setPosHpr(chan.camera, Point3(0), Point3(0))
        ###
	[ chan camera setPos: relNodePath pos: delta t: t. 
		] spawnTaskNamed: 'manipulateCamera'
	uponDeath: [ relNodePath removeNode. ]
        ###

    def homeCam(chan):
        chan.camera.setMat(Mat4.identMat())


    def mouseFlyStart(chan):
	# Record starting mouse positions
	initMouseX = chan.mouseX
	initMouseY = chan.mouseY

	# Where are we in the channel?
        if ((initMouseX abs < 0.9) & (initMouseY abs < 0.9)):
            # Mouse is in central region
            # spawn task to determine mouse fly mode"
            self.determineMouseFlyMode()
        else:
            #Mouse is in outer frame, spawn mouseRotateTask
            self.spawnMouseRotateTask()

    def mouseFlyStop():
	taskMgr.removeTasksNamed('determineMouseFlyMode')
	taskMgr.removeTasksNamed('manipulateCamera')

    def removeManipulateCameraTask():
        taskMgr.removeTasksNamed('manipulateCamera')

    def enableMouseFly():
	self.enableMouseInteraction()
	self.enableHotKeys()

    def enableMouseInteraction():
	# disable C++ fly interface
	base.disableMouse()
	# Accept middle mouse events
	self.accept('mouse2', self.mouseFlyStart, [chanCenter])
	self.accept('mouse2-up' self.mouseFlyStop)

    def disableMouseFly():
	# Accept middle mouse events
	self.ignore('mouse2')
	self.ignore:'mouse2-up')
	self.ignore('u')
	self.ignore('c')
	self.ignore('h')
        for i in range(0,10):
            self.ignore(`i`)
	self.ignore('=')
	self.ignore('+')
	self.ignore('-')
	self.ignore('=')

    def enableHotKeys():
	self.accept('u', self.uprightCam, [chanCenter])
	self.accept('c', self.centerCamIn, [chanCenter, 0.5])
	self.accept('h', self.homeCam, [chanCenter])
        for i in range(1,8):
            self.accept(`i`, self.moveToView, [chanCenter, i])
	self.accept('9', self.swingCamAboutWidget, [chanCenter, -90.0, 1.0])
	self.accept('0', self.swingCamAboutWidget, [chanCenter,  90.0, 1.0])
	self.accept('8', self.removeManipulateCameraTask)
	self.accept('=', self.zoomCam, [chanCenter, 0.5, 1.0])
	self.accept('+', self.zoomCam, [chanCenter, 0.5, 1.0])
	self.accept('-', self.zoomCam, [chanCenter, -2.0, 1.0])
	self.accept('=', self.zoomCam, [chanCenter, -2.0, 1.0])

    def SpawnMoveToView(chan, view):
        # Kill any existing tasks
	taskMgr.removeTasksNamed('manipulateCamera')
        # Calc hprOffset
	hprOffset = VBase3()
        if view = 1:
            hprOffset.set(180., 0., 0.)
        elif view = 2:
            hprOffset set(0., 0., 0.)
        elif view = 3:
            hprOffset set(90., 0., 0.)
        elif view = 4:
            hprOffset set(-90., 0., 0.)
        elif view = 5:
            hprOffset set(0., -90., 0.)
        elif view = 6:
            hprOffset set(0., 90., 0.)
        elif view = 7:
            hprOffset set(135., -35.264, 0.)
        # Position target
	targetNodePath.setPosHpr(direct.widget,
                                 self.zeroBaseVec,
                                 hprOffset)
	# Scale center vec by current distance to target
	offsetDistance = Vec3(chan.camera.getPos(targetNodePath) - \
                              zeroPoint).length()
	scaledCenterVec = centerVec * (-1.0 * offsetDistance).

   	# Now put the targetNodePath at that point
	targetNodePath.setPosHpr(targetNodePath,
                                 scaledCenterVec,
                                 zeroBaseVec)

	# Start off with best view if change is to new view
        if (view != lastView):
            orthoViewRoll = 0.0
        lastView = view.

	[ chan camera setPosHpr: targetNodePath pos: zeroBaseVec 
    			       hpr: (VBase3 new: 0.0 y: 0.0 z: orthoViewRoll)
			       t: 1.0.
	  "Try another roll next time"
	  orthoViewRoll = (orthoViewRoll + 90.0) rem: 360.0.]
		spawnTaskNamed: #manipulateCamera.
        

    def determineMouseFlyMode():
        # Get mouse intersection point
        # TBS

        # Find this point to camera space
	gridToCamera = grid getMat: chanCenter camera.
	hitPt operatorAssign: (gridToCamera xformPoint: iPoint).
	"Make sure hitPt is in front of the camera"
	hitPt setY: (hitPt at: 1) abs.

	"Handle case of bad hit point (too close or too far)"
	hitDistance = (hitPt - zeroPoint) length.
	((hitDistance < (1.1 * chanCenter near)) | (hitDistance > chanCenter far)) ifTrue: [ 
		"Just use grid origin"
		"hitPt operatorAssign: centerVec * (0.5 * (chanCenter far + chanCenter near))"
		hitPt operatorAssign: (grid getPos: chanCenter camera).
		].

	(direct fShift) ifTrue: [ self.spawnHPPan. ]
	ifFalse: [
		[[deltaX = chanCenter mouseX - initMouseX.
			deltaY = chanCenter mouseY - initMouseY.
			((deltaX abs < 0.1) & (deltaY abs < 0.1))] taskWhileTrue: [ nil ]
			] spawnTaskNamed: #determineMouseFlyMode
			uponDeath: [
				(deltaY abs > 0.1) ifTrue: [
					self.spawnHPanYZoom]
	 			ifFalse: [
					self.spawnXZTranslate ].
		].
			].
! !



!DirectCameraManipulation methodsFor: 'event handling' stamp: 'panda 00/00/0000 00:00'!
spawnHPPan
	[[true] taskWhileTrue:
		[ chanCenter camera setHpr: chanCenter camera 
			h: (0.5 * chanCenter mouseDeltaX * chanCenter fovH)
			p: (-0.5 * chanCenter mouseDeltaY * chanCenter fovV)
			r: 0.0 ]]
	spawnTaskNamed: #manipulateCamera.
! !

!DirectCameraManipulation methodsFor: 'event handling' stamp: 'panda 00/00/0000 00:00'!
spawnHPanYZoom
	| targetVector distToMove |
	targetVector = Vec3 new: (hitPt at: 0) y: (hitPt at: 1) z: (hitPt at: 2).
	[[true] taskWhileTrue:
		[ distToMove = targetVector * (-1.0 * chanCenter mouseDeltaY).
		  chanCenter camera setPosHpr: chanCenter camera 
			x: (distToMove at: 0)
			y: (distToMove at: 1)
			z: (distToMove at: 2)
			h: (0.5 * chanCenter mouseDeltaX * chanCenter fovH)
			p: 0.0 r: 0.0. ]]
	spawnTaskNamed: #manipulateCamera.
! !

!DirectCameraManipulation methodsFor: 'event handling' stamp: 'panda 00/00/0000 00:00'!
spawnMouseRotateTask
	| wrtMat |
	centerOfRotation setPos: grid pos: iPoint.
	centerOfRotation setHpr: chanCenter camera h: 0.0 p: 0.0 r: 0.0.
	
	wrtMat = chanCenter camera getMat: centerOfRotation.

	[[true] taskWhileTrue:
		[ centerOfRotation setHpr: centerOfRotation
			h: (-0.5 * chanCenter mouseDeltaX * 180.0)
			p: (0.5 * chanCenter mouseDeltaY * 180.0)
			r: 0.0.
		   chanCenter camera setMat: centerOfRotation mat: wrtMat. ]]
	spawnTaskNamed: #manipulateCamera.
! !

!DirectCameraManipulation methodsFor: 'event handling' stamp: 'panda 00/00/0000 00:00'!
spawnXZTranslate
	| scaleFactor | 
	scaleFactor = ((hitPt at: 1) / chanCenter near).
	[[true] taskWhileTrue:
		[ chanCenter camera setPos: chanCenter camera 
			x: (-0.5 * chanCenter mouseDeltaX * chanCenter nearWidth * scaleFactor)
			y: 0.0
			z: (-0.5 * chanCenter mouseDeltaY * chanCenter nearHeight * scaleFactor) ] ]
	spawnTaskNamed: #manipulateCamera. 
! !

!DirectCameraManipulation methodsFor: 'event handling' stamp: 'panda 00/00/0000 00:00'!
spawnXZTranslateOrHPPan
	| scaleFactor | 
	scaleFactor = ((hitPt at: 1) / chanCenter near).
	[[true] taskWhileTrue:
		[ direct fShift 
			ifTrue: [ chanCenter camera setHpr: chanCenter camera 
						h: (0.5 * chanCenter mouseDeltaX * chanCenter fovH)
						p: (-0.5 * chanCenter mouseDeltaY * chanCenter fovV)
						r: 0.0 ]
			ifFalse: [ chanCenter camera setPos: chanCenter camera 
						x: (-0.5 * chanCenter mouseDeltaX * chanCenter nearWidth * scaleFactor)
						y: 0.0
						z: (-0.5 * chanCenter mouseDeltaY * chanCenter nearHeight * scaleFactor)]
		]]
	spawnTaskNamed: #manipulateCamera. 
! !

!DirectCameraManipulation methodsFor: 'event handling' stamp: 'panda 00/00/0000 00:00'!
swingCamAboutWidget: chan deg: degrees in: t
	| relNodePath parent |

	Task removeTasksNamed: #manipulateCamera.
	
	relNodePath = showBase render attachNewNode: (Node new).
	"Coincident with widget"
	relNodePath setPos: direct widget pos: (Point3 zero).
	"But aligned with render space"
	relNodePath setHpr: (Point3 zero).

	parent = chanCenter camera getParent.
	chanCenter camera wrtReparentTo: relNodePath.

	[ relNodePath setHpr: (VBase3 new: degrees y: 0.0 z: 0.0) t: t. ] 
		spawnTaskNamed: #manipulateCamera
		uponDeath: [ 
			chanCenter camera wrtReparentTo: parent.
			relNodePath reparentTo: showBase hidden.
			relNodePath removeNode.
			].! !

!DirectCameraManipulation methodsFor: 'event handling' stamp: 'panda 00/00/0000 00:00'!
uprightCam: chan
	| currH |
	Task removeTasksNamed: #manipulateCamera.
	currH = chan camera getH.
	[ chan camera setHpr: showBase render h: currH p: 0.0 r: 0.0 t: 1.0.
		] spawnTaskNamed: #manipulateCamera.! !

!DirectCameraManipulation methodsFor: 'event handling' stamp: 'panda 00/00/0000 00:00'!
zoomCam: chan zoom: zoom in: t
	| relNodePath zoomPtToCam |
	Task removeTasksNamed: #manipulateCamera.
	"Find a point zoom factor times the current separation of the widget and cam"
	zoomPtToCam = (direct widget getPos: chan camera) * zoom.

	"Put a target nodePath there"
	relNodePath = showBase render attachNewNode: (Node new).
	relNodePath setPos: chanCenter camera pos: zoomPtToCam.
	
	"Move to that point"
	[ chan camera setPos: relNodePath pos: (Point3 zero) t: t. ] 
		spawnTaskNamed: #manipulateCamera
		uponDeath: [ relNodePath removeNode. ].! !

