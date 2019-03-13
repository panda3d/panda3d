#!/usr/bin/env python
import sys
from math import sin
from math import pi

from panda3d.core import CollisionNode
from panda3d.core import CollisionTraverser
from panda3d.core import CollisionHandlerQueue

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


class CollisionTester(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        self.disable_mouse()
        self.accept("escape", sys.exit)

        self.from_np = self.render.attach_new_node("from")
        self.from_np.set_pos(5, 0, 0)
        self.from_model = self.loader.load_model("models/zup-axis")
        self.from_model.reparent_to(self.from_np)
        self.from_model.set_scale(0.1)
        self.from_coll = CollisionNode("from_coll")
        self.from_coll.add_solid(CollisionSphere((0, 0, 0), 1))
        self.from_coll_np = self.from_np.attach_new_node(self.from_coll)
        self.from_coll_np.show()
        self.task_mgr.add(self.wobble_from, "wobble_from")
        self.solid_from = [("", )]

        self.into_np = self.render.attach_new_node("from")
        self.into_np.set_pos(0, 0, 0)
        self.into_model = self.loader.load_model("models/zup-axis")
        self.into_model.reparent_to(self.into_np)
        self.into_model.set_scale(0.1)
        self.into_coll = CollisionNode("into_coll")
        self.into_coll.add_solid(CollisionSphere((0, 0, 0), 1))
        self.into_coll_np = self.into_np.attach_new_node(self.into_coll)
        self.into_coll_np.show()
        self.task_mgr.add(self.wobble_into, "wobble_into")
        self.solid_into = [
            ("Sphere", CollisionSphere((0, 0, 0), 1)),
            ("Box", CollisionBox((-0.5, -0.5, -0.5), (0.5, 0.5, 0.5))),
            ]
        #self.

        self.traverser = CollisionTraverser("traverser")
        self.traverser.show_collisions(self.render)
        self.queue = CollisionHandlerQueue()
        self.traverser.add_collider(self.from_coll_np, self.queue)
        self.task_mgr.add(self.collide, "collide")

        self.cam.set_pos(0, -20, 10)
        self.cam.look_at(0, 0, 0)

    def wobble_from(self, task):
        self.from_np.set_x(sin((task.time*2*pi)/5) * 5)
        return task.cont

    def wobble_into(self, task):
        self.into_np.set_hpr(
            task.time*360/10,
            task.time*360/20,
            task.time*360/30,
        )
        return task.cont

    def collide(self, task):
        self.traverser.traverse(self.render)
        return task.cont

colltester = CollisionTester()
colltester.run()
