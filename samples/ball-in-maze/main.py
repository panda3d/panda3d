#!/usr/bin/env python

# Author: Shao Zhang, Phil Saltzman
# Last Updated: 2015-03-13
#
# This tutorial shows how to detect and respond to collisions. It uses solids
# create in code and the egg files, how to set up collision masks, a traverser,
# and a handler, how to detect collisions, and how to dispatch function based
# on the collisions. All of this is put together to simulate a labyrinth-style
# game

from direct.showbase.ShowBase import ShowBase
from panda3d.core import CollisionTraverser, CollisionNode
from panda3d.core import CollisionHandlerQueue, CollisionRay
from panda3d.core import Material, LRotationf, NodePath
from panda3d.core import AmbientLight, DirectionalLight
from panda3d.core import TextNode
from panda3d.core import LVector3, BitMask32
from direct.gui.OnscreenText import OnscreenText
from direct.interval.MetaInterval import Sequence, Parallel
from direct.interval.LerpInterval import LerpFunc
from direct.interval.FunctionInterval import Func, Wait
from direct.task.Task import Task
import sys

# Some constants for the program
ACCEL = 70         # Acceleration in ft/sec/sec
MAX_SPEED = 5      # Max speed in ft/sec
MAX_SPEED_SQ = MAX_SPEED ** 2  # Squared to make it easier to use lengthSquared
# Instead of length


class BallInMazeDemo(ShowBase):

    def __init__(self):
        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)

        # This code puts the standard title and instruction text on screen
        self.title = \
            OnscreenText(text="Panda3D: Tutorial - Collision Detection",
                         parent=base.a2dBottomRight, align=TextNode.ARight,
                         fg=(1, 1, 1, 1), pos=(-0.1, 0.1), scale=.08,
                         shadow=(0, 0, 0, 0.5))
        self.instructions = \
            OnscreenText(text="Mouse pointer tilts the board",
                         parent=base.a2dTopLeft, align=TextNode.ALeft,
                         pos=(0.05, -0.08), fg=(1, 1, 1, 1), scale=.06,
                         shadow=(0, 0, 0, 0.5))

        self.accept("escape", sys.exit)  # Escape quits

        # Disable default mouse-based camera control.  This is a method on the
        # ShowBase class from which we inherit.
        self.disableMouse()
        camera.setPosHpr(0, 0, 25, 0, -90, 0)  # Place the camera

        # Load the maze and place it in the scene
        self.maze = loader.loadModel("models/maze")
        self.maze.reparentTo(render)

        # Most times, you want collisions to be tested against invisible geometry
        # rather than every polygon. This is because testing against every polygon
        # in the scene is usually too slow. You can have simplified or approximate
        # geometry for the solids and still get good results.
        #
        # Sometimes you'll want to create and position your own collision solids in
        # code, but it's often easier to have them built automatically. This can be
        # done by adding special tags into an egg file. Check maze.egg and ball.egg
        # and look for lines starting with <Collide>. The part is brackets tells
        # Panda exactly what to do. Polyset means to use the polygons in that group
        # as solids, while Sphere tells panda to make a collision sphere around them
        # Keep means to keep the polygons in the group as visable geometry (good
        # for the ball, not for the triggers), and descend means to make sure that
        # the settings are applied to any subgroups.
        #
        # Once we have the collision tags in the models, we can get to them using
        # NodePath's find command

        # Find the collision node named wall_collide
        self.walls = self.maze.find("**/wall_collide")

        # Collision objects are sorted using BitMasks. BitMasks are ordinary numbers
        # with extra methods for working with them as binary bits. Every collision
        # solid has both a from mask and an into mask. Before Panda tests two
        # objects, it checks to make sure that the from and into collision masks
        # have at least one bit in common. That way things that shouldn't interact
        # won't. Normal model nodes have collision masks as well. By default they
        # are set to bit 20. If you want to collide against actual visable polygons,
        # set a from collide mask to include bit 20
        #
        # For this example, we will make everything we want the ball to collide with
        # include bit 0
        self.walls.node().setIntoCollideMask(BitMask32.bit(0))
        # CollisionNodes are usually invisible but can be shown. Uncomment the next
        # line to see the collision walls
        #self.walls.show()

        # We will now find the triggers for the holes and set their masks to 0 as
        # well. We also set their names to make them easier to identify during
        # collisions
        self.loseTriggers = []
        for i in range(6):
            trigger = self.maze.find("**/hole_collide" + str(i))
            trigger.node().setIntoCollideMask(BitMask32.bit(0))
            trigger.node().setName("loseTrigger")
            self.loseTriggers.append(trigger)
            # Uncomment this line to see the triggers
            # trigger.show()

        # Ground_collide is a single polygon on the same plane as the ground in the
        # maze. We will use a ray to collide with it so that we will know exactly
        # what height to put the ball at every frame. Since this is not something
        # that we want the ball itself to collide with, it has a different
        # bitmask.
        self.mazeGround = self.maze.find("**/ground_collide")
        self.mazeGround.node().setIntoCollideMask(BitMask32.bit(1))

        # Load the ball and attach it to the scene
        # It is on a root dummy node so that we can rotate the ball itself without
        # rotating the ray that will be attached to it
        self.ballRoot = render.attachNewNode("ballRoot")
        self.ball = loader.loadModel("models/ball")
        self.ball.reparentTo(self.ballRoot)

        # Find the collison sphere for the ball which was created in the egg file
        # Notice that it has a from collision mask of bit 0, and an into collison
        # mask of no bits. This means that the ball can only cause collisions, not
        # be collided into
        self.ballSphere = self.ball.find("**/ball")
        self.ballSphere.node().setFromCollideMask(BitMask32.bit(0))
        self.ballSphere.node().setIntoCollideMask(BitMask32.allOff())

        # No we create a ray to start above the ball and cast down. This is to
        # Determine the height the ball should be at and the angle the floor is
        # tilting. We could have used the sphere around the ball itself, but it
        # would not be as reliable
        self.ballGroundRay = CollisionRay()     # Create the ray
        self.ballGroundRay.setOrigin(0, 0, 10)    # Set its origin
        self.ballGroundRay.setDirection(0, 0, -1)  # And its direction
        # Collision solids go in CollisionNode
        # Create and name the node
        self.ballGroundCol = CollisionNode('groundRay')
        self.ballGroundCol.addSolid(self.ballGroundRay)  # Add the ray
        self.ballGroundCol.setFromCollideMask(
            BitMask32.bit(1))  # Set its bitmasks
        self.ballGroundCol.setIntoCollideMask(BitMask32.allOff())
        # Attach the node to the ballRoot so that the ray is relative to the ball
        # (it will always be 10 feet over the ball and point down)
        self.ballGroundColNp = self.ballRoot.attachNewNode(self.ballGroundCol)
        # Uncomment this line to see the ray
        #self.ballGroundColNp.show()

        # Finally, we create a CollisionTraverser. CollisionTraversers are what
        # do the job of walking the scene graph and calculating collisions.
        # For a traverser to actually do collisions, you need to call
        # traverser.traverse() on a part of the scene. Fortunately, ShowBase
        # has a task that does this for the entire scene once a frame.  By
        # assigning it to self.cTrav, we designate that this is the one that
        # it should call traverse() on each frame.
        self.cTrav = CollisionTraverser()

        # Collision traversers tell collision handlers about collisions, and then
        # the handler decides what to do with the information. We are using a
        # CollisionHandlerQueue, which simply creates a list of all of the
        # collisions in a given pass. There are more sophisticated handlers like
        # one that sends events and another that tries to keep collided objects
        # apart, but the results are often better with a simple queue
        self.cHandler = CollisionHandlerQueue()
        # Now we add the collision nodes that can create a collision to the
        # traverser. The traverser will compare these to all others nodes in the
        # scene. There is a limit of 32 CollisionNodes per traverser
        # We add the collider, and the handler to use as a pair
        self.cTrav.addCollider(self.ballSphere, self.cHandler)
        self.cTrav.addCollider(self.ballGroundColNp, self.cHandler)

        # Collision traversers have a built in tool to help visualize collisions.
        # Uncomment the next line to see it.
        #self.cTrav.showCollisions(render)

        # This section deals with lighting for the ball. Only the ball was lit
        # because the maze has static lighting pregenerated by the modeler
        ambientLight = AmbientLight("ambientLight")
        ambientLight.setColor((.55, .55, .55, 1))
        directionalLight = DirectionalLight("directionalLight")
        directionalLight.setDirection(LVector3(0, 0, -1))
        directionalLight.setColor((0.375, 0.375, 0.375, 1))
        directionalLight.setSpecularColor((1, 1, 1, 1))
        self.ballRoot.setLight(render.attachNewNode(ambientLight))
        self.ballRoot.setLight(render.attachNewNode(directionalLight))

        # This section deals with adding a specular highlight to the ball to make
        # it look shiny.  Normally, this is specified in the .egg file.
        m = Material()
        m.setSpecular((1, 1, 1, 1))
        m.setShininess(96)
        self.ball.setMaterial(m, 1)

        # Finally, we call start for more initialization
        self.start()

    def start(self):
        # The maze model also has a locator in it for where to start the ball
        # To access it we use the find command
        startPos = self.maze.find("**/start").getPos()
        # Set the ball in the starting position
        self.ballRoot.setPos(startPos)
        self.ballV = LVector3(0, 0, 0)         # Initial velocity is 0
        self.accelV = LVector3(0, 0, 0)        # Initial acceleration is 0

        # Create the movement task, but first make sure it is not already
        # running
        taskMgr.remove("rollTask")
        self.mainLoop = taskMgr.add(self.rollTask, "rollTask")

    # This function handles the collision between the ray and the ground
    # Information about the interaction is passed in colEntry
    def groundCollideHandler(self, colEntry):
        # Set the ball to the appropriate Z value for it to be exactly on the
        # ground
        newZ = colEntry.getSurfacePoint(render).getZ()
        self.ballRoot.setZ(newZ + .4)

        # Find the acceleration direction. First the surface normal is crossed with
        # the up vector to get a vector perpendicular to the slope
        norm = colEntry.getSurfaceNormal(render)
        accelSide = norm.cross(LVector3.up())
        # Then that vector is crossed with the surface normal to get a vector that
        # points down the slope. By getting the acceleration in 3D like this rather
        # than in 2D, we reduce the amount of error per-frame, reducing jitter
        self.accelV = norm.cross(accelSide)

    # This function handles the collision between the ball and a wall
    def wallCollideHandler(self, colEntry):
        # First we calculate some numbers we need to do a reflection
        norm = colEntry.getSurfaceNormal(render) * -1  # The normal of the wall
        curSpeed = self.ballV.length()                # The current speed
        inVec = self.ballV / curSpeed                 # The direction of travel
        velAngle = norm.dot(inVec)                    # Angle of incidance
        hitDir = colEntry.getSurfacePoint(render) - self.ballRoot.getPos()
        hitDir.normalize()
        # The angle between the ball and the normal
        hitAngle = norm.dot(hitDir)

        # Ignore the collision if the ball is either moving away from the wall
        # already (so that we don't accidentally send it back into the wall)
        # and ignore it if the collision isn't dead-on (to avoid getting caught on
        # corners)
        if velAngle > 0 and hitAngle > .995:
            # Standard reflection equation
            reflectVec = (norm * norm.dot(inVec * -1) * 2) + inVec

            # This makes the velocity half of what it was if the hit was dead-on
            # and nearly exactly what it was if this is a glancing blow
            self.ballV = reflectVec * (curSpeed * (((1 - velAngle) * .5) + .5))
            # Since we have a collision, the ball is already a little bit buried in
            # the wall. This calculates a vector needed to move it so that it is
            # exactly touching the wall
            disp = (colEntry.getSurfacePoint(render) -
                    colEntry.getInteriorPoint(render))
            newPos = self.ballRoot.getPos() + disp
            self.ballRoot.setPos(newPos)

    # This is the task that deals with making everything interactive
    def rollTask(self, task):
        # Standard technique for finding the amount of time since the last
        # frame
        dt = globalClock.getDt()

        # If dt is large, then there has been a # hiccup that could cause the ball
        # to leave the field if this functions runs, so ignore the frame
        if dt > .2:
            return Task.cont

        # The collision handler collects the collisions. We dispatch which function
        # to handle the collision based on the name of what was collided into
        for i in range(self.cHandler.getNumEntries()):
            entry = self.cHandler.getEntry(i)
            name = entry.getIntoNode().getName()
            if name == "wall_collide":
                self.wallCollideHandler(entry)
            elif name == "ground_collide":
                self.groundCollideHandler(entry)
            elif name == "loseTrigger":
                self.loseGame(entry)

        # Read the mouse position and tilt the maze accordingly
        if base.mouseWatcherNode.hasMouse():
            mpos = base.mouseWatcherNode.getMouse()  # get the mouse position
            self.maze.setP(mpos.getY() * -10)
            self.maze.setR(mpos.getX() * 10)

        # Finally, we move the ball
        # Update the velocity based on acceleration
        self.ballV += self.accelV * dt * ACCEL
        # Clamp the velocity to the maximum speed
        if self.ballV.lengthSquared() > MAX_SPEED_SQ:
            self.ballV.normalize()
            self.ballV *= MAX_SPEED
        # Update the position based on the velocity
        self.ballRoot.setPos(self.ballRoot.getPos() + (self.ballV * dt))

        # This block of code rotates the ball. It uses something called a quaternion
        # to rotate the ball around an arbitrary axis. That axis perpendicular to
        # the balls rotation, and the amount has to do with the size of the ball
        # This is multiplied on the previous rotation to incrimentally turn it.
        prevRot = LRotationf(self.ball.getQuat())
        axis = LVector3.up().cross(self.ballV)
        newRot = LRotationf(axis, 45.5 * dt * self.ballV.length())
        self.ball.setQuat(prevRot * newRot)

        return Task.cont       # Continue the task indefinitely

    # If the ball hits a hole trigger, then it should fall in the hole.
    # This is faked rather than dealing with the actual physics of it.
    def loseGame(self, entry):
        # The triggers are set up so that the center of the ball should move to the
        # collision point to be in the hole
        toPos = entry.getInteriorPoint(render)
        taskMgr.remove('rollTask')  # Stop the maze task

        # Move the ball into the hole over a short sequence of time. Then wait a
        # second and call start to reset the game
        Sequence(
            Parallel(
                LerpFunc(self.ballRoot.setX, fromData=self.ballRoot.getX(),
                         toData=toPos.getX(), duration=.1),
                LerpFunc(self.ballRoot.setY, fromData=self.ballRoot.getY(),
                         toData=toPos.getY(), duration=.1),
                LerpFunc(self.ballRoot.setZ, fromData=self.ballRoot.getZ(),
                         toData=self.ballRoot.getZ() - .9, duration=.2)),
            Wait(1),
            Func(self.start)).start()

# Finally, create an instance of our class and start 3d rendering
demo = BallInMazeDemo()
demo.run()
