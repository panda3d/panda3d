#!/usr/bin/env python

import sys

from direct.actor.Actor import Actor
from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import load_prc_file_data
from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import LVector3
from panda3d.core import TransformState
from panda3d.core import BitMask32
from panda3d.core import Filename
from panda3d.core import PNMImage

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletCapsuleShape
from panda3d.bullet import BulletCharacterControllerNode
from panda3d.bullet import BulletHeightfieldShape
from panda3d.bullet import BulletTriangleMesh
from panda3d.bullet import BulletTriangleMeshShape
from panda3d.bullet import ZUp


class Game(ShowBase):
    def __init__(self):
        """
        Load some configuration variables, it's important for this to happen
        before the ShowBase is initialized
        """
        load_prc_file_data("", """
            sync-video #t
            ### add entries below if you are not running from an installation.
            #model-path /path/to/panda3d
            """)
        ShowBase.__init__(self)
        base.set_background_color(0.1, 0.1, 0.8, 1)
        base.set_frame_rate_meter(True)

        base.cam.set_pos(0, -20, 4)
        base.cam.look_at(0, 0, 0)

        # Light
        alight = AmbientLight('ambientLight')
        alight.set_color((0.5, 0.5, 0.5, 1))
        alightNP = render.attach_new_node(alight)

        dlight = DirectionalLight('directionalLight')
        dlight.set_direction((1, 1, -1))
        dlight.set_color((0.7, 0.7, 0.7, 1))
        dlightNP = render.attach_new_node(dlight)

        render.clear_light()
        render.set_light(alightNP)
        render.set_light(dlightNP)

        # Input
        self.accept('escape', self.do_exit)
        self.accept('r', self.do_reset)
        self.accept('f1', base.toggle_wireframe)
        self.accept('f2', base.toggle_texture)
        self.accept('f3', self.toggle_debug)
        self.accept('f5', self.do_screenshot)

        self.accept('space', self.do_jump)
        self.accept('c', self.do_crouch)

        inputState.watchWithModifiers('forward', 'w')
        inputState.watchWithModifiers('left', 'a')
        inputState.watchWithModifiers('reverse', 's')
        inputState.watchWithModifiers('right', 'd')
        inputState.watchWithModifiers('turnLeft', 'q')
        inputState.watchWithModifiers('turnRight', 'e')

        # Task
        taskMgr.add(self.update, 'updateWorld')

        # Physics
        self.setup()

    def do_exit(self):
        self.cleanup()
        sys.exit(1)

    def do_reset(self):
        self.cleanup()
        self.setup()

    def toggle_debug(self):
        if self.debugNP.is_hidden():
            self.debugNP.show()
        else:
            self.debugNP.hide()

    def do_screenshot(self):
        base.screenshot('Bullet')

    def do_jump(self):
        self.character.set_max_jump_height(5.0)
        self.character.set_jump_speed(8.0)
        self.character.do_jump()
        self.actorNP.play("jump")

    def do_crouch(self):
        self.crouching = not self.crouching
        sz = self.crouching and 0.6 or 1.0
        self.characterNP.set_scale((1, 1, sz))
        #self.character.get_shape().set_local_scale(LVector3(1, 1, sz))
        #self.characterNP.set_scale(LVector3(1, 1, sz) * 0.3048)
        #self.characterNP.set_pos(0, 0, -1 * sz)

    def process_input(self, dt):
        speed = LVector3(0, 0, 0)
        omega = 0.0

        if inputState.isSet('forward'): speed.y = 2.0
        if inputState.isSet('reverse'): speed.y = -2.0
        if inputState.isSet('left'): speed.x = -2.0
        if inputState.isSet('right'): speed.x = 2.0
        if inputState.isSet('turnLeft'): omega = 120.0
        if inputState.isSet('turnRight'): omega = -120.0

        self.character.set_angular_movement(omega)
        self.character.set_linear_movement(speed, True)

    def update(self, task):
        dt = globalClock.get_dt()
        self.process_input(dt)
        self.world.do_physics(dt, 4, 1./240.)
        return task.cont

    def cleanup(self):
        self.world = None
        self.worldNP.remove_node()

    def setup(self):
        self.worldNP = render.attach_new_node('World')

        # World
        self.debugNP = self.worldNP.attach_new_node(BulletDebugNode('Debug'))
        self.debugNP.show()

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Ground
        shape = BulletPlaneShape((0, 0, 1), 0)

        #img = PNMImage(Filename('models/elevation.png'))
        #shape = BulletHeightfieldShape(img, 1.0, ZUp)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Ground'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, -1)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Box
        shape = BulletBoxShape((1.0, 3.0, 0.3))

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Box'))
        np.node().set_mass(10.0)
        np.node().add_shape(shape)
        np.set_pos(3, 0, 4)
        np.setH(20.0)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Character
        self.crouching = False

        h = 1.75
        w = 0.4
        shape = BulletCapsuleShape(w, h - 2 * w, ZUp)
        self.character = BulletCharacterControllerNode(shape, 0.4, 'Player')
        self.characterNP = self.worldNP.attach_new_node(self.character)
        self.characterNP.set_pos(-2, 0, 14)
        self.characterNP.set_h(45)
        self.characterNP.set_collide_mask(BitMask32.all_on())
        self.world.attach(self.character)

        self.actorNP = Actor('../roaming-ralph/models/ralph.egg.pz',
            {'run' : '../roaming-ralph/models/ralph-run.egg.pz',
             'walk' : '../roaming-ralph/models/ralph-walk.egg.pz'})
        self.actorNP.reparent_to(self.characterNP)
        self.actorNP.set_scale(0.3048) # 1ft = 0.3048m
        self.actorNP.setH(180)
        self.actorNP.set_pos(0, 0, -1)


game = Game()
game.run()
