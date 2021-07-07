#!/usr/bin/env python

import sys

from direct.showbase.ShowBase import ShowBase
from direct.showbase.InputStateGlobal import inputState
from direct.interval.MetaInterval import Sequence
from direct.interval.MetaInterval import Parallel
from direct.interval.LerpInterval import LerpFunc
from direct.interval.FunctionInterval import Func
from direct.interval.FunctionInterval import Wait

from panda3d.core import AmbientLight
from panda3d.core import DirectionalLight
from panda3d.core import Material

from panda3d.bullet import BulletWorld
from panda3d.bullet import BulletRigidBodyNode
from panda3d.bullet import BulletGhostNode
from panda3d.bullet import BulletDebugNode
from panda3d.bullet import BulletHelper


class Game(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        base.set_background_color(0.1, 0.1, 0.8, 1)
        base.set_frame_rate_meter(True)

        base.cam.set_pos_hpr(0, 0, 25, 0, -90, 0)
        base.disable_mouse()

        # Input
        self.accept('escape', self.exitGame)
        self.accept('f1', base.toggle_wireframe)
        self.accept('f2', base.toggle_texture)
        self.accept('f3', self.toggle_debug)
        self.accept('f5', self.do_screenshot)

        # Setup scene 1: World
        self.debugNP = render.attach_new_node(BulletDebugNode('Debug'))
        self.debugNP.node().show_wireframe(True)
        self.debugNP.node().show_constraints(True)
        self.debugNP.node().show_bounding_boxes(True)
        self.debugNP.node().show_normals(True)
        self.debugNP.show()

        self.world = BulletWorld()
        self.world.set_gravity((0, 0, -9.81))
        self.world.set_debug_node(self.debugNP.node())

        # Setup scene 2: Ball
        #visNP = loader.load_model('models/ball.egg')
        visNP = loader.load_model('../ball-in-maze/models/ball.egg.pz')
        visNP.clear_model_nodes()

        bodyNPs = BulletHelper.from_collision_solids(visNP, True)
        self.ballNP = bodyNPs[0]
        self.ballNP.reparent_to(render)
        self.ballNP.node().set_mass(1.0)
        self.ballNP.set_pos(4, -4, 1)
        self.ballNP.node().set_deactivation_enabled(False)

        visNP.reparent_to(self.ballNP)

        # Setup scene 3: Maze
        visNP = loader.load_model('models/maze.egg')
        #visNP = loader.load_model('samples/ball-in-maze/models/maze.egg.pz')
        visNP.clear_model_nodes()
        visNP.reparent_to(render)

        self.holes = []
        self.maze = []
        self.mazeNP = visNP

        bodyNPs = BulletHelper.from_collision_solids(visNP, True);
        for bodyNP in bodyNPs:
            bodyNP.reparent_to(render)

            if isinstance(bodyNP.node(), BulletRigidBodyNode):
                bodyNP.node().set_mass(0.0)
                bodyNP.node().set_kinematic(True)
                self.maze.append(bodyNP)

            elif isinstance(bodyNP.node(), BulletGhostNode):
                self.holes.append(bodyNP)

        # Lighting and material for the ball
        ambientLight = AmbientLight('ambientLight')
        ambientLight.set_color((0.55, 0.55, 0.55, 1))
        directionalLight = DirectionalLight('directionalLight')
        directionalLight.set_direction((0, 0, -1))
        directionalLight.set_color((0.375, 0.375, 0.375, 1))
        directionalLight.set_specular_color((1, 1, 1, 1))
        self.ballNP.set_light(render.attach_new_node(ambientLight))
        self.ballNP.set_light(render.attach_new_node(directionalLight))

        m = Material()
        m.set_specular((1,1,1,1))
        m.set_shininess(96)
        self.ballNP.set_material(m, 1)

        # Startup
        self.start_game()

    def exitGame(self):
        sys.exit()

    def toggle_debug(self):
        if self.debugNP.is_hidden():
            self.debugNP.show()
        else:
            self.debugNP.hide()

    def do_screenshot(self):
        base.screenshot('Bullet')

    def start_game(self):
        self.ballNP.set_pos(4, -4, 1)
        self.ballNP.node().set_linear_velocity((0, 0, 0))
        self.ballNP.node().set_angular_velocity((0, 0, 0))

        # Mouse
        p = base.win.get_properties()
        base.win.move_pointer(0, int(p.get_x_size()/2), int(p.get_y_size()/2))

        # Add bodies and ghosts
        self.world.attach(self.ballNP.node())
        for bodyNP in self.maze:
            self.world.attach(bodyNP.node())
        for ghostNP in self.holes:
            self.world.attach(ghostNP.node())

        # Simulation task
        taskMgr.add(self.update_game, 'updateGame')

    def stop_game(self):

        # Remove bodies and ghosts
        self.world.remove(self.ballNP.node())
        for bodyNP in self.maze:
            self.world.remove(bodyNP.node())
        for ghostNP in self.holes:
            self.world.remove(ghostNP.node())

        # Simulation task
        taskMgr.remove('updateGame')

    def update_game(self, task):
        dt = globalClock.get_dt()

        # Get mouse position and tilt maze
        if base.mouseWatcherNode.hasMouse():
            mpos = base.mouseWatcherNode.get_mouse()
            hpr = (0, mpos.y * -10, mpos.x * 10)

            # Maze visual node
            self.mazeNP.set_hpr(hpr)

            # Maze body nodes
            for bodyNP in self.maze:
                bodyNP.set_hpr(hpr)

        # Update simulation
        self.world.do_physics(dt)

        # Check if ball is touching a hole
        for holeNP in self.holes:
            if holeNP.node().get_num_overlapping_nodes() > 2:
                if self.ballNP.node() in holeNP.node().get_overlapping_nodes():
                    self.lose_game(holeNP)

        return task.cont

    def lose_game(self, holeNP):
        toPos = holeNP.node().get_shape_pos(0)
        self.stop_game()

        Sequence(
            Parallel(
                LerpFunc(self.ballNP.set_x, fromData = self.ballNP.get_x(),
                         toData = toPos.get_x(), duration = .1),
                LerpFunc(self.ballNP.set_y, fromData = self.ballNP.get_y(),
                         toData = toPos.get_y(), duration = .1),
                LerpFunc(self.ballNP.set_z, fromData = self.ballNP.get_z(),
                         toData = self.ballNP.get_z() - .9, duration = .2)),
                Wait(1),
                Func(self.start_game)).start()


game = Game()
game.run()
