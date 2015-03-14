#!/usr/bin/env python

# Author: Shao Zhang, Phil Saltzman, and Eddie Caanan
# Last Updated: 2015-03-13
#
# This tutorial shows how to play animations on models aka "actors".
# It is based on the popular game of "Rock 'em Sock 'em Robots".

from direct.showbase.ShowBase import ShowBase
from panda3d.core import AmbientLight, DirectionalLight
from panda3d.core import TextNode
from panda3d.core import LVector3
from direct.gui.OnscreenText import OnscreenText
from direct.interval.MetaInterval import Sequence
from direct.interval.FunctionInterval import Func, Wait
from direct.actor import Actor
from random import random
import sys


class BoxingRobotDemo(ShowBase):
    # Macro-like function used to reduce the amount to code needed to create the
    # on screen instructions

    def genLabelText(self, text, i):
        return OnscreenText(text=text, parent=base.a2dTopLeft, scale=.05,
                            pos=(0.1, - 0.1 -.07 * i), fg=(1, 1, 1, 1),
                            align=TextNode.ALeft)

    def __init__(self):
        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)

        # This code puts the standard title and instruction text on screen
        self.title = OnscreenText(text="Panda3D: Tutorial - Actors",
                                  parent=base.a2dBottomRight, style=1,
                                  fg=(0, 0, 0, 1), pos=(-0.2, 0.1),
                                  align=TextNode.ARight, scale=.09)

        self.escapeEventText = self.genLabelText("ESC: Quit", 0)
        self.akeyEventText = self.genLabelText("[A]: Robot 1 Left Punch", 1)
        self.skeyEventText = self.genLabelText("[S]: Robot 1 Right Punch", 2)
        self.kkeyEventText = self.genLabelText("[K]: Robot 2 Left Punch", 3)
        self.lkeyEventText = self.genLabelText("[L]: Robot 2 Right Punch", 4)

        # Set the camera in a fixed position
        self.disableMouse()
        camera.setPosHpr(14.5, -15.4, 14, 45, -14, 0)
        self.setBackgroundColor(0, 0, 0)

        # Add lighting so that the objects are not drawn flat
        self.setupLights()

        # Load the ring
        self.ring = loader.loadModel('models/ring')
        self.ring.reparentTo(render)

        # Models that use skeletal animation are known as Actors instead of models
        # Instead of just one file, the have one file for the main model, and an
        # additional file for each playable animation.
        # They are loaded using Actor.Actor instead of loader.LoadModel.
        # The constructor takes the location of the main object as with a normal model
        # and a dictionary (A fancy python structure that is like a lookup table)
        # that contains names for animations, and paths to the appropriate
        # files
        self.robot1 = Actor.Actor('models/robot',
                                  {'leftPunch': 'models/robot_left_punch',
                                   'rightPunch': 'models/robot_right_punch',
                                   'headUp': 'models/robot_head_up',
                                   'headDown': 'models/robot_head_down'})

        # Actors need to be positioned and parented like normal objects
        self.robot1.setPosHprScale(-1, -2.5, 4, 45, 0, 0, 1.25, 1.25, 1.25)
        self.robot1.reparentTo(render)

        # We'll repeat the process for the second robot. The only thing that changes
        # here is the robot's color and position
        self.robot2 = Actor.Actor('models/robot',
                                  {'leftPunch': 'models/robot_left_punch',
                                   'rightPunch': 'models/robot_right_punch',
                                   'headUp': 'models/robot_head_up',
                                   'headDown': 'models/robot_head_down'})

        # Set the properties of this robot
        self.robot2.setPosHprScale(1, 1.5, 4, 225, 0, 0, 1.25, 1.25, 1.25)
        self.robot2.setColor((.7, 0, 0, 1))
        self.robot2.reparentTo(render)

        # Now we define how the animated models will move. Animations are played
        # through special intervals. In this case we use actor intervals in a
        # sequence to play the part of the punch animation where the arm extends,
        # call a function to check if the punch landed, and then play the part of the
        # animation where the arm retracts

        # Punch sequence for robot 1's left arm
        self.robot1.punchLeft = Sequence(
            # Interval for the outstreched animation
            self.robot1.actorInterval('leftPunch', startFrame=1, endFrame=10),
            # Function to check if the punch was successful
            Func(self.checkPunch, 2),
            # Interval for the retract animation
            self.robot1.actorInterval('leftPunch', startFrame=11, endFrame=32))

        # Punch sequence for robot 1's right arm
        self.robot1.punchRight = Sequence(
            self.robot1.actorInterval('rightPunch', startFrame=1, endFrame=10),
            Func(self.checkPunch, 2),
            self.robot1.actorInterval('rightPunch', startFrame=11, endFrame=32))

        # Punch sequence for robot 2's left arm
        self.robot2.punchLeft = Sequence(
            self.robot2.actorInterval('leftPunch', startFrame=1, endFrame=10),
            Func(self.checkPunch, 1),
            self.robot2.actorInterval('leftPunch', startFrame=11, endFrame=32))

        # Punch sequence for robot 2's right arm
        self.robot2.punchRight = Sequence(
            self.robot2.actorInterval('rightPunch', startFrame=1, endFrame=10),
            Func(self.checkPunch, 1),
            self.robot2.actorInterval('rightPunch', startFrame=11, endFrame=32))

        # We use the same techinique to create a sequence for when a robot is knocked
        # out where the head pops up, waits a while, and then resets

        # Head animation for robot 1
        self.robot1.resetHead = Sequence(
            # Interval for the head going up. Since no start or end frames were given,
            # the entire animation is played.
            self.robot1.actorInterval('headUp'),
            Wait(1.5),
            # The head down animation was animated a little too quickly, so this will
            # play it at 75% of it's normal speed
            self.robot1.actorInterval('headDown', playRate=.75))

        # Head animation for robot 2
        self.robot2.resetHead = Sequence(
            self.robot2.actorInterval('headUp'),
            Wait(1.5),
            self.robot2.actorInterval('headDown', playRate=.75))

        # Now that we have defined the motion, we can define our key input.
        # Each fist is bound to a key. When a key is pressed, self.tryPunch checks to
        # make sure that the both robots have their heads down, and if they do it
        # plays the given interval
        self.accept('escape', sys.exit)
        self.accept('a', self.tryPunch, [self.robot1.punchLeft])
        self.accept('s', self.tryPunch, [self.robot1.punchRight])
        self.accept('k', self.tryPunch, [self.robot2.punchLeft])
        self.accept('l', self.tryPunch, [self.robot2.punchRight])

    # tryPunch will play the interval passed to it only if
    # neither robot has 'resetHead' playing (a head is up) AND
    # the punch interval passed to it is not already playing
    def tryPunch(self, interval):
        if (not self.robot1.resetHead.isPlaying() and
                not self.robot2.resetHead.isPlaying() and
                not interval.isPlaying()):
            interval.start()

    # checkPunch will determine if a successful punch has been thrown
    def checkPunch(self, robot):
        if robot == 1:
            # punch is directed to robot 1
            # if robot 1 is playing'resetHead', do nothing
            if self.robot1.resetHead.isPlaying():
                return
            # if robot 1 is not punching...
            if (not self.robot1.punchLeft.isPlaying() and
                    not self.robot1.punchRight.isPlaying()):
                # ...15% chance of successful hit
                if random() > .85:
                    self.robot1.resetHead.start()
            # Otherwise, only 5% chance of sucessful hit
            elif random() > .95:
                self.robot1.resetHead.start()
        else:
            # punch is directed to robot 2, same as above
            if self.robot2.resetHead.isPlaying():
                return
            if (not self.robot2.punchLeft.isPlaying() and
                    not self.robot2.punchRight.isPlaying()):
                if random() > .85:
                    self.robot2.resetHead.start()
            elif random() > .95:
                self.robot2.resetHead.start()

    # This function sets up the lighting
    def setupLights(self):
        ambientLight = AmbientLight("ambientLight")
        ambientLight.setColor((.8, .8, .75, 1))
        directionalLight = DirectionalLight("directionalLight")
        directionalLight.setDirection(LVector3(0, 0, -2.5))
        directionalLight.setColor((0.9, 0.8, 0.9, 1))
        render.setLight(render.attachNewNode(ambientLight))
        render.setLight(render.attachNewNode(directionalLight))

demo = BoxingRobotDemo()
demo.run()
