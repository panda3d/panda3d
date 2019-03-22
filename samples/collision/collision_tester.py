#!/usr/bin/env python
import sys
from math import sin
from math import pi

from panda3d.core import NodePath
from panda3d.core import Point3
from panda3d.core import CollisionNode
from panda3d.core import CollisionTraverser
from panda3d.core import CollisionHandlerQueue

from panda3d.core import Plane
#from panda3d.core import Parabolaf
from panda3d.core import CollisionBox
from panda3d.core import CollisionCapsule
from panda3d.core import CollisionFloorMesh
from panda3d.core import CollisionInvSphere
from panda3d.core import CollisionLine
from panda3d.core import CollisionParabola
from panda3d.core import CollisionPlane
from panda3d.core import CollisionPolygon
from panda3d.core import CollisionRay
from panda3d.core import CollisionSegment
from panda3d.core import CollisionSphere
from panda3d.core import CollisionTube

from direct.showbase.ShowBase import ShowBase


# Wobble modes
STILL = 1
ROTATE = 2
LEFTRIGHT = 3
CAMERA = 4


solids_from = [
    ("Sphere", CollisionSphere((0, 0, 0), 1.5)),
    ("Capsule", CollisionCapsule((-1, 0, 0), (1, 0, 0), 1)),
    ("Box", CollisionBox((-1, -1, -1), (1, 1, 1))),
    ("Segment", CollisionSegment((-1, 0, 0), (1, 0, 0))),
    ("Ray", CollisionRay((0, 0, 0), (-1, 0, 0))),
    ("Line", CollisionSegment((-1, 0, 0), (1, 0, 0))),
    #("Parabola", CollisionParabola(
    #    Parabolaf((0, 0, -1), (1, 0, 5), (0, 0, 0)),
    #    0, 5,
    #)),
]
solids_into = [
    ("Sphere", CollisionSphere((0, 0, 0), 1.5)),
    ("Capsule", CollisionCapsule((-1, 0, 0), (1, 0, 0), 1)),
    ("Box", CollisionBox((-1, -1, -1), (1, 1, 1))),
    ("Plane", CollisionPlane(Plane((1, 0, 0), (0, 0, 0)))),
    ("InvSphere", CollisionInvSphere((0, 0, 0), 2)),
]


solids_camera = [
    ("Segment", CollisionSegment((0, 0, 0), (0, 50, 0))),
    ("Ray", CollisionRay((0, 0, 0), (0, 1, 0))),
    ("Line", CollisionSegment((0, 0, 0), (0, 1, 0))),
]


class CollisionTester(ShowBase):
    def __init__(self, cam_mode=False):
        ShowBase.__init__(self)
        self.disable_mouse()
        self.accept("escape", sys.exit)
        self.cam.set_pos(0, -20, 10)
        self.cam.look_at(0, 0, 0)

        self.from_object = Collidable(solids_from, 0, self.render, LEFTRIGHT)
        self.into_object = Collidable(solids_into, 0, self.render, ROTATE)
        self.camera_object = Collidable(
            solids_camera, 0, self.cam, CAMERA, origin=False,
        )

        self.from_object.get_collider().node().set_into_collide_mask(0)
        self.camera_object.get_collider().node().set_into_collide_mask(0)

        self.traverser = CollisionTraverser("traverser")
        self.traverser.show_collisions(self.render)
        self.queue = CollisionHandlerQueue()
        self.task_mgr.add(self.collide, "collide")

        self.accept("1", self.into_object.switch_solid)
        self.cam_mode = cam_mode
        self.traverser.add_collider(
            NodePath(CollisionNode("dummy")), self.queue,
        )
        self.toggle_cam_mode(force_mode=self.cam_mode)
        self.accept("m", self.toggle_cam_mode)

    def toggle_cam_mode(self, force_mode=None):
        if force_mode is None:
            self.cam_mode = not self.cam_mode
        else:
            self.cam_mode = force_mode
        if self.cam_mode:
            self.traverser.remove_collider(self.traverser.get_collider(0))
            self.traverser.add_collider(
                self.camera_object.get_collider(), self.queue,
            )
            self.ignore("2")
            self.from_object.off()
            self.camera_object.on()
            self.accept("2", self.camera_object.switch_solid)
        else:
            self.traverser.remove_collider(self.traverser.get_collider(0))
            self.traverser.add_collider(
                self.from_object.get_collider(), self.queue,
            )
            self.ignore("2")
            self.from_object.on()
            self.camera_object.off()
            self.accept("2", self.from_object.switch_solid)

    def collide(self, task):
        self.traverser.traverse(self.render)
        return task.cont


class Collidable:
    def __init__(self, solids, solid_id, root_node, wobble_type, origin=True):
        self.solids = solids
        self.solid_id = solid_id
        self.wobble_type = wobble_type

        self.np = root_node.attach_new_node("from")
        if origin:
            self.model = base.loader.load_model("models/zup-axis")
            self.model.reparent_to(self.np)
            self.model.set_scale(0.1)
        self.coll = CollisionNode("coll")
        self.coll.add_solid(self.solids[self.solid_id][1])
        self.coll_np = self.np.attach_new_node(self.coll)
        self.coll_np.show()
        base.task_mgr.add(self.wobble, "wobble")

    def get_collider(self):
        return self.coll_np

    def switch_solid(self):
        self.coll.remove_solid(0)
        self.solid_id = (self.solid_id + 1) % len(self.solids)
        self.coll.add_solid(self.solids[self.solid_id][1])

    def wobble(self, task):
        if self.wobble_type == STILL:
            pass
        elif self.wobble_type == LEFTRIGHT:
            self.np.set_x(sin((task.time*2*pi)/10) * 5)
        elif self.wobble_type == ROTATE:
            self.np.set_hpr(
                task.time*360/10,
                task.time*360/20,
                task.time*360/30,
            )
        elif self.wobble_type == CAMERA:
            if base.mouseWatcherNode.hasMouse():
                mpos = base.mouseWatcherNode.getMouse()
                mpos = Point3(mpos.x, mpos.y, 0)
                cam_space_vec = Point3()
                base.cam.node().get_lens().extrude_depth(mpos, cam_space_vec)
                cam_space_vec /= cam_space_vec.length()
                self.np.look_at(cam_space_vec)
        return task.cont

    def off(self):
        self.coll_np.hide()

    def on(self):
        self.coll_np.show()


colltester = CollisionTester()
colltester.run()
