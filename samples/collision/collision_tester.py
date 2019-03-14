#!/usr/bin/env python
import sys
from math import sin
from math import pi

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


FROM = 1
INTO = 2

class CollisionTester(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        self.disable_mouse()
        self.accept("escape", sys.exit)
        self.cam.set_pos(0, -20, 10)
        self.cam.look_at(0, 0, 0)

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
        self.from_object = Collidable(solids_from, 0, FROM)
        self.into_object = Collidable(solids_into, 0, INTO)
        self.accept("1", self.from_object.switch_solid)
        self.accept("2", self.into_object.switch_solid)

        self.traverser = CollisionTraverser("traverser")
        self.traverser.show_collisions(self.render)
        self.queue = CollisionHandlerQueue()
        self.traverser.add_collider(self.from_object.get_collider(), self.queue)
        self.task_mgr.add(self.collide, "collide")

    def collide(self, task):
        self.traverser.traverse(self.render)
        return task.cont


class Collidable:
    def __init__(self, solids, solid_id, wobble_type):
        self.solids = solids
        self.solid_id = solid_id
        self.wobble_type = wobble_type

        self.np = base.render.attach_new_node("from")
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
        if self.wobble_type == FROM:
            self.np.set_x(sin((task.time*2*pi)/10) * 5)
        elif self.wobble_type == INTO:
            self.np.set_hpr(
                task.time*360/10,
                task.time*360/20,
                task.time*360/30,
            )
        return task.cont


colltester = CollisionTester()
colltester.run()
