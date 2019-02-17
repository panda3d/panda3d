#!/usr/bin/env python

import sys

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import TransformState
from panda3d.core import BitMask32

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletPlaneShape
from panda3d.bullet import BulletBoxShape
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletVehicle
from panda3d.bullet import ZUp


class Game(ShowBase):
    def __init__(self):
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

        inputState.watchWithModifiers('forward', 'w')
        inputState.watchWithModifiers('reverse', 's')
        inputState.watchWithModifiers('turnLeft', 'a')
        inputState.watchWithModifiers('turnRight', 'd')

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

    def process_input(self, dt):
        engineForce = 0.0
        brakeForce = 0.0

        if inputState.isSet('forward'):
            engineForce = 1000.0
            brakeForce = 0.0

        if inputState.isSet('reverse'):
            engineForce = 0.0
            brakeForce = 100.0

        if inputState.isSet('turnLeft'):
            self.steering += dt * self.steeringIncrement
            self.steering = min(self.steering, self.steeringClamp)

        if inputState.isSet('turnRight'):
            self.steering -= dt * self.steeringIncrement
            self.steering = max(self.steering, -self.steeringClamp)

        # Apply steering to front wheels
        self.vehicle.setSteeringValue(self.steering, 0);
        self.vehicle.setSteeringValue(self.steering, 1);

        # Apply engine and brake to rear wheels
        self.vehicle.applyEngineForce(engineForce, 2);
        self.vehicle.applyEngineForce(engineForce, 3);
        self.vehicle.setBrake(brakeForce, 2);
        self.vehicle.setBrake(brakeForce, 3);

    def update(self, task):
        dt = globalClock.get_dt()
        self.process_input(dt)
        self.world.do_physics(dt, 10, 0.008)
        #print self.vehicle.getWheel(0).getRaycastInfo().isInContact()
        #print self.vehicle.getWheel(0).getRaycastInfo().getContactPointWs()
        #print self.vehicle.getChassis().isKinematic()
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

        # Plane
        shape = BulletPlaneShape((0, 0, 1), 0)

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Ground'))
        np.node().add_shape(shape)
        np.set_pos(0, 0, -1)
        np.set_collide_mask(BitMask32.all_on())

        self.world.attach(np.node())

        # Chassis
        shape = BulletBoxShape((0.6, 1.4, 0.5))
        ts = TransformState.make_pos((0, 0, 0.5))

        np = self.worldNP.attach_new_node(BulletRigidBodyNode('Vehicle'))
        np.node().add_shape(shape, ts)
        np.set_pos(0, 0, 1)
        np.node().set_mass(800.0)
        np.node().set_deactivation_enabled(False)

        self.world.attach(np.node())

        #np.node().set_ccd_swept_sphere_radius(1.0)
        #np.node().set_ccd_motion_threshold(1e-7)

        # Vehicle
        self.vehicle = BulletVehicle(self.world, np.node())
        self.vehicle.set_coordinate_system(ZUp)
        self.world.attach(self.vehicle)

        self.yugoNP = loader.load_model('models/yugo/yugo.egg')
        self.yugoNP.reparent_to(np)

        # Right front wheel
        np = loader.load_model('models/yugo/yugotireR.egg')
        np.reparent_to(self.worldNP)
        self.add_wheel(( 0.70,  1.05, 0.3), True, np)

        # Left front wheel
        np = loader.load_model('models/yugo/yugotireL.egg')
        np.reparent_to(self.worldNP)
        self.add_wheel((-0.70,  1.05, 0.3), True, np)

        # Right rear wheel
        np = loader.load_model('models/yugo/yugotireR.egg')
        np.reparent_to(self.worldNP)
        self.add_wheel(( 0.70, -1.05, 0.3), False, np)

        # Left rear wheel
        np = loader.load_model('models/yugo/yugotireL.egg')
        np.reparent_to(self.worldNP)
        self.add_wheel((-0.70, -1.05, 0.3), False, np)

        # Steering info
        self.steering = 0.0            # degree
        self.steeringClamp = 45.0      # degree
        self.steeringIncrement = 120.0 # degree per second

    def add_wheel(self, pos, front, np):
        wheel = self.vehicle.create_wheel()

        wheel.set_node(np.node())
        wheel.set_chassis_connection_point_cs(pos)
        wheel.set_front_wheel(front)

        wheel.set_wheel_direction_cs((0, 0, -1))
        wheel.set_wheel_axle_cs((1, 0, 0))
        wheel.set_wheel_radius(0.25)
        wheel.set_max_suspension_travel_cm(40.0)

        wheel.set_suspension_stiffness(40.0)
        wheel.set_wheels_damping_relaxation(2.3)
        wheel.set_wheels_damping_compression(4.4)
        wheel.set_friction_slip(100.0);
        wheel.set_roll_influence(0.1)


game = Game()
game.run()
