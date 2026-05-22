#!/usr/bin/env python

# Author: Ryan Myers
# Models: Jeff Styers, Reagan Heller
#
# Last Updated: 2015-03-13
#
# This tutorial provides an example of creating a character
# and having it walk around on uneven terrain, as well
# as implementing a fully rotatable camera.

from panda3d.core import loadPrcFileData
loadPrcFileData("", "audio-library-name p3fmod_audio")
loadPrcFileData("", "win-size 1280 800")
loadPrcFileData("", "window-title Roaming Ralph - 3D Audio Demo")

from direct.showbase.ShowBase import ShowBase
from direct.showbase.Audio3DManager import Audio3DManager
from panda3d.core import CollisionTraverser, CollisionNode
from panda3d.core import CollisionHandlerQueue, CollisionRay
from panda3d.core import CollisionHandlerPusher, CollisionSphere
from panda3d.core import Filename, AmbientLight, DirectionalLight
from panda3d.core import PandaNode, NodePath, Camera, TextNode
from panda3d.core import CollideMask, AudioSound, AudioManager, FilterProperties
from direct.gui.OnscreenText import OnscreenText
from direct.actor.Actor import Actor
import random
import sys
import os
import math

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1), scale=.05,
                        shadow=(0, 0, 0, 1), parent=base.a2dTopLeft,
                        pos=(0.08, -pos - 0.04), align=TextNode.ALeft)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(1, 1, 1, 1), scale=.07,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        pos=(-0.1, 0.09), shadow=(0, 0, 0, 1))


class RoamingRalphDemo(ShowBase):
    def __init__(self):
        # Set up the window, camera, etc.
        ShowBase.__init__(self)

        # This is used to store which keys are currently pressed.
        self.keyMap = {
            "left": 0,
            "right": 0,
            "forward": 0,
            "backward": 0,
            "cam-left": 0,
            "cam-right": 0,
        }

        # Post the instructions
        self.title = addTitle(
            "Panda3D Tutorial: Roaming Ralph (Walking on Uneven Terrain)")
        self.inst1 = addInstructions(0.06, "[ESC]: Quit")
        self.inst2 = addInstructions(0.12, "[Left Arrow]: Rotate Ralph Left")
        self.inst3 = addInstructions(0.18, "[Right Arrow]: Rotate Ralph Right")
        self.inst4 = addInstructions(0.24, "[Up Arrow]: Run Ralph Forward")
        self.inst5 = addInstructions(0.30, "[Down Arrow]: Walk Ralph Backward")
        self.inst6 = addInstructions(0.36, "[A]: Rotate Camera Left")
        self.inst7 = addInstructions(0.42, "[S]: Rotate Camera Right")

        # Set up the environment
        #
        # This environment model contains collision meshes.  If you look
        # in the egg file, you will see the following:
        #
        #    <Collide> { Polyset keep descend }
        #
        # This tag causes the following mesh to be converted to a collision
        # mesh -- a mesh which is optimized for collision, not rendering.
        # It also keeps the original mesh, so there are now two copies ---
        # one optimized for rendering, one for collisions.

        self.environ = loader.loadModel("models/world")
        self.environ.reparentTo(render)

        # We do not have a skybox, so we will just use a sky blue background color
        self.setBackgroundColor(0.53, 0.80, 0.92, 1)

        # Create the main character, Ralph

        ralphStartPos = self.environ.find("**/start_point").getPos()
        self.ralph = Actor("models/ralph",
                           {"run": "models/ralph-run",
                            "walk": "models/ralph-walk"})
        self.ralph.reparentTo(render)
        self.ralph.setScale(.2)
        self.ralph.setPos(ralphStartPos + (0, 0, 1.5))

        # Create a floater object, which floats 2 units above ralph.  We
        # use this as a target for the camera to look at.

        self.floater = NodePath(PandaNode("floater"))
        self.floater.reparentTo(self.ralph)
        self.floater.setZ(2.0)

        # Accept the control keys for movement and rotation

        self.accept("escape", sys.exit)
        self.accept("arrow_left", self.setKey, ["left", True])
        self.accept("arrow_right", self.setKey, ["right", True])
        self.accept("arrow_up", self.setKey, ["forward", True])
        self.accept("arrow_down", self.setKey, ["backward", True])
        self.accept("a", self.setKey, ["cam-left", True])
        self.accept("s", self.setKey, ["cam-right", True])
        self.accept("arrow_left-up", self.setKey, ["left", False])
        self.accept("arrow_right-up", self.setKey, ["right", False])
        self.accept("arrow_up-up", self.setKey, ["forward", False])
        self.accept("arrow_down-up", self.setKey, ["backward", False])
        self.accept("a-up", self.setKey, ["cam-left", False])
        self.accept("s-up", self.setKey, ["cam-right", False])

        taskMgr.add(self.move, "moveTask")

        # Set up the camera
        self.disableMouse()
        self.camera.setPos(self.ralph.getX(), self.ralph.getY() + 10, 2)

        self.cTrav = CollisionTraverser()

        # Use a CollisionHandlerPusher to handle collisions between Ralph and
        # the environment. Ralph is added as a "from" object which will be
        # "pushed" out of the environment if he walks into obstacles.
        #
        # Ralph is composed of two spheres, one around the torso and one
        # around the head.  They are slightly oversized since we want Ralph to
        # keep some distance from obstacles.
        self.ralphCol = CollisionNode('ralph')
        self.ralphCol.addSolid(CollisionSphere(center=(0, 0, 2), radius=1.5))
        self.ralphCol.addSolid(CollisionSphere(center=(0, -0.25, 4), radius=1.5))
        self.ralphCol.setFromCollideMask(CollideMask.bit(0))
        self.ralphCol.setIntoCollideMask(CollideMask.allOff())
        self.ralphColNp = self.ralph.attachNewNode(self.ralphCol)
        self.ralphPusher = CollisionHandlerPusher()
        self.ralphPusher.horizontal = True

        # Note that we need to add ralph both to the pusher and to the
        # traverser; the pusher needs to know which node to push back when a
        # collision occurs!
        self.ralphPusher.addCollider(self.ralphColNp, self.ralph)
        self.cTrav.addCollider(self.ralphColNp, self.ralphPusher)

        # We will detect the height of the terrain by creating a collision
        # ray and casting it downward toward the terrain.  One ray will
        # start above ralph's head, and the other will start above the camera.
        # A ray may hit the terrain, or it may hit a rock or a tree.  If it
        # hits the terrain, we can detect the height.
        self.ralphGroundRay = CollisionRay()
        self.ralphGroundRay.setOrigin(0, 0, 9)
        self.ralphGroundRay.setDirection(0, 0, -1)
        self.ralphGroundCol = CollisionNode('ralphRay')
        self.ralphGroundCol.addSolid(self.ralphGroundRay)
        self.ralphGroundCol.setFromCollideMask(CollideMask.bit(0))
        self.ralphGroundCol.setIntoCollideMask(CollideMask.allOff())
        self.ralphGroundColNp = self.ralph.attachNewNode(self.ralphGroundCol)
        self.ralphGroundHandler = CollisionHandlerQueue()
        self.cTrav.addCollider(self.ralphGroundColNp, self.ralphGroundHandler)

        self.camGroundRay = CollisionRay()
        self.camGroundRay.setOrigin(0, 0, 9)
        self.camGroundRay.setDirection(0, 0, -1)
        self.camGroundCol = CollisionNode('camRay')
        self.camGroundCol.addSolid(self.camGroundRay)
        self.camGroundCol.setFromCollideMask(CollideMask.bit(0))
        self.camGroundCol.setIntoCollideMask(CollideMask.allOff())
        self.camGroundColNp = self.camera.attachNewNode(self.camGroundCol)
        self.camGroundHandler = CollisionHandlerQueue()
        self.cTrav.addCollider(self.camGroundColNp, self.camGroundHandler)

        # Uncomment this line to see the collision rays
        #self.ralphColNp.show()
        #self.camGroundColNp.show()

        # Uncomment this line to show a visual representation of the
        # collisions occuring
        #self.cTrav.showCollisions(render)

        # Wire collision events on the pusher so we can play a sound when
        # Ralph bumps into an obstacle. addInPattern fires once on first contact.
        self.ralphPusher.addInPattern("ralph-hit")
        self.accept("ralph-hit", self.onRalphCollision)
        self._hitCooldown = 0.0

        # Each positional sound gets its own FMOD manager so DSP effects can be
        # applied independently. configure_filters() targets a whole manager, not
        # individual sounds, so separate managers are the only way to have
        # different effects on different sounds.
        #
        # sfxManagerList[0] (existing) — forest ambient, no DSP
        # _footstepMgr (new)           — footstep, sfxreverb
        # _hitMgr      (new)           — hit sound, echo

        self._footstepMgr = AudioManager.createAudioManager()
        base.addSfxManager(self._footstepMgr)
        self.footstepAudio3d = Audio3DManager(self._footstepMgr, base.camera)
        self.footstepAudio3d.setDistanceFactor(1.0)
        self.footstepAudio3d.setDropOffFactor(1.0)

        self._hitMgr = AudioManager.createAudioManager()
        base.addSfxManager(self._hitMgr)
        self.hitAudio3d = Audio3DManager(self._hitMgr, base.camera)
        self.hitAudio3d.setDistanceFactor(1.0)
        self.hitAudio3d.setDropOffFactor(1.0)

        # Footstep sound — attached to Ralph so it moves with him.
        # Replace sounds/footstep.mp3 with a short, loopable footstep clip.
        self.footstepSfx = self.footstepAudio3d.loadSfx("sounds/footstep.mp3")
        self.footstepAudio3d.attachSoundToObject(self.footstepSfx, self.ralph)
        self.footstepAudio3d.setSoundMinDistance(self.footstepSfx, 3.0)
        self.footstepAudio3d.setSoundMaxDistance(self.footstepSfx, 40.0)
        self.footstepSfx.setLoop(True)
        self.footstepSfx.setPlayRate(1.5)
        self.footstepSfx.setVolume(0.6)

        # Outdoor reverb on footsteps: short decay, moderate wet level.
        # room → WETLEVEL (dB), decaytime → ms (not seconds).
        foot_step_fp = FilterProperties()
        foot_step_fp.add_sfxreverb(
            drylevel=0,             # DRYLEVEL: 0 dB
            room=-6,                # WETLEVEL: -6 dB — moderate reverb
            roomhf=8000,            # HIGHCUT: 8000 Hz — soften highs outdoors
            decaytime=800,          # DECAYTIME: 800 ms — short outdoor decay
            reflectionslevel=40,    # EARLYLATEMIX: 40% early
            diffusion=80,
            density=80,
        )
        self._footstepMgr.configure_filters(foot_step_fp)

        # Collision/hit sound — emitted from the surface point Ralph collided with,
        # not from Ralph himself. This is the key 3D positional audio demo.
        # Replace sounds/hit.mp3 with a short thud or knock clip.
        self.hitEmitter = render.attachNewNode("hitEmitter")
        self.hitSfx = self.hitAudio3d.loadSfx("sounds/hit.mp3")
        self.hitAudio3d.attachSoundToObject(self.hitSfx, self.hitEmitter)
        self.hitAudio3d.setSoundMinDistance(self.hitSfx, 2.0)
        self.hitAudio3d.setSoundMaxDistance(self.hitSfx, 30.0)
        self.hitSfx.setVolume(2.0)

        # Reverb on the hit sound — simulates the thud bouncing off nearby trees.
        # sfxreverb sounds natural because it diffuses the sound energy rather
        # than repeating it discretely like echo does.
        # Panda3D's add_sfxreverb parameter names come from the old EAX API but
        # map to FMOD Core DSP parameters with different semantics/ranges:
        #   room            → WETLEVEL   (-80 to 20 dB)
        #   roomhf          → HIGHCUT    (20 to 20000 Hz, not a dB value)
        #   decaytime       → DECAYTIME  (100 to 20000 ms — milliseconds!)
        #   reflectionslevel→ EARLYLATEMIX (0 to 100 %)
        hit_fp = FilterProperties()
        hit_fp.add_sfxreverb(
            drylevel=0,             # DRYLEVEL: 0 dB — keep full dry signal
            room=-10,               # WETLEVEL: -10 dB — moderate reverb wet level
            roomhf=10000,           # HIGHCUT: 10000 Hz — slight HF softening
            decaytime=1000,         # DECAYTIME: 1000 ms — 1-second reverb tail
            decayhfratio=0.5,       # HFDECAYRATIO: default
            reflectionslevel=50,    # EARLYLATEMIX: 50% early / 50% late
            diffusion=100,          # DIFFUSION: 100% — smooth reverb
            density=100,            # DENSITY: 100%
        )
        self._hitMgr.configure_filters(hit_fp)

        # Ambient background sound — non-positional, loops for the whole session.
        self.forestSfx = loader.loadSfx("sounds/forest.wav")
        self.forestSfx.setLoop(True)
        self.forestSfx.setVolume(0.2)
        self.forestSfx.play()

        # Create some lighting
        ambientLight = AmbientLight("ambientLight")
        ambientLight.setColor((.3, .3, .3, 1))
        directionalLight = DirectionalLight("directionalLight")
        directionalLight.setDirection((-5, -5, -5))
        directionalLight.setColor((1, 1, 1, 1))
        directionalLight.setSpecularColor((1, 1, 1, 1))
        render.setLight(render.attachNewNode(ambientLight))
        render.setLight(render.attachNewNode(directionalLight))

    # Records the state of the arrow keys
    def setKey(self, key, value):
        self.keyMap[key] = value

    def onRalphCollision(self, entry):
        """Play a hit sound at the collision surface point when Ralph bumps into
        an obstacle. The sound emits from the contact point, not from Ralph,
        demonstrating true 3D positional audio."""
        # Skip ground-ray hits (terrain); we only care about wall/obstacle hits.
        if entry.getIntoNode().getName() == "terrain":
            return
        # Debounce: avoid retriggering while Ralph holds forward into a wall.
        now = base.clock.getRealTime()
        if now - self._hitCooldown < 0.5:
            return
        self._hitCooldown = now

        # Move the hit emitter to the world-space surface contact point.
        surface_pt = entry.getSurfacePoint(render)
        self.hitEmitter.setPos(surface_pt)
        self.hitSfx.play()

    # Accepts arrow keys to move either the player or the menu cursor,
    # Also deals with grid checking and collision detection
    def move(self, task):

        # Get the time that elapsed since last frame.  We multiply this with
        # the desired speed in order to find out with which distance to move
        # in order to achieve that desired speed.
        dt = base.clock.dt

        # If the camera-left key is pressed, move camera left.
        # If the camera-right key is pressed, move camera right.

        if self.keyMap["cam-left"]:
            self.camera.setX(self.camera, -20 * dt)
        if self.keyMap["cam-right"]:
            self.camera.setX(self.camera, +20 * dt)

        # If a move-key is pressed, move ralph in the specified direction.

        if self.keyMap["left"]:
            self.ralph.setH(self.ralph.getH() + 300 * dt)
        if self.keyMap["right"]:
            self.ralph.setH(self.ralph.getH() - 300 * dt)
        if self.keyMap["forward"]:
            self.ralph.setY(self.ralph, -20 * dt)
        if self.keyMap["backward"]:
            self.ralph.setY(self.ralph, +10 * dt)

        # If ralph is moving, loop the run animation.
        # If he is standing still, stop the animation.
        currentAnim = self.ralph.getCurrentAnim()

        if self.keyMap["forward"]:
            if currentAnim != "run":
                self.ralph.loop("run")
        elif self.keyMap["backward"]:
            # Play the walk animation backwards.
            if currentAnim != "walk":
                self.ralph.loop("walk")
            self.ralph.setPlayRate(-1.0, "walk")
        elif self.keyMap["left"] or self.keyMap["right"]:
            if currentAnim != "walk":
                self.ralph.loop("walk")
            self.ralph.setPlayRate(1.0, "walk")
        else:
            if currentAnim is not None:
                self.ralph.stop()
                self.ralph.pose("walk", 5)
                self.isMoving = False

        # Manage footstep sound: play while any movement key is held, stop otherwise.
        is_moving = (self.keyMap["forward"] or self.keyMap["backward"] or
                     self.keyMap["left"] or self.keyMap["right"])
        if is_moving:
            if self.footstepSfx.status() != AudioSound.PLAYING:
                self.footstepSfx.play()
        else:
            if self.footstepSfx.status() == AudioSound.PLAYING:
                self.footstepSfx.stop()

        # If the camera is too far from ralph, move it closer.
        # If the camera is too close to ralph, move it farther.

        camvec = self.ralph.getPos() - self.camera.getPos()
        camvec.setZ(0)
        camdist = camvec.length()
        camvec.normalize()
        if camdist > 10.0:
            self.camera.setPos(self.camera.getPos() + camvec * (camdist - 10))
            camdist = 10.0
        if camdist < 5.0:
            self.camera.setPos(self.camera.getPos() - camvec * (5 - camdist))
            camdist = 5.0

        # Normally, we would have to call traverse() to check for collisions.
        # However, the class ShowBase that we inherit from has a task to do
        # this for us, if we assign a CollisionTraverser to self.cTrav.
        #self.cTrav.traverse(render)

        # Adjust ralph's Z coordinate.  If ralph's ray hit terrain,
        # update his Z

        entries = list(self.ralphGroundHandler.entries)
        entries.sort(key=lambda x: x.getSurfacePoint(render).getZ())

        for entry in entries:
            if entry.getIntoNode().name == "terrain":
                self.ralph.setZ(entry.getSurfacePoint(render).getZ())

        # Keep the camera at one unit above the terrain,
        # or two units above ralph, whichever is greater.

        entries = list(self.camGroundHandler.entries)
        entries.sort(key=lambda x: x.getSurfacePoint(render).getZ())

        for entry in entries:
            if entry.getIntoNode().name == "terrain":
                self.camera.setZ(entry.getSurfacePoint(render).getZ() + 1.5)
        if self.camera.getZ() < self.ralph.getZ() + 2.0:
            self.camera.setZ(self.ralph.getZ() + 2.0)

        # The camera should look in ralph's direction,
        # but it should also try to stay horizontal, so look at
        # a floater which hovers above ralph's head.
        self.camera.lookAt(self.floater)

        return task.cont


demo = RoamingRalphDemo()
demo.run()
