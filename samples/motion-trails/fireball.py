#!/usr/bin/env python

from random import choice

from panda3d.core import Point3, Vec4
from direct.showbase.ShowBase import ShowBase
from direct.motiontrail.MotionTrail import MotionTrail
from direct.interval.LerpInterval import LerpPosInterval, LerpHprInterval
from direct.interval.LerpInterval import LerpScaleInterval
from direct.interval.LerpInterval import LerpTexOffsetInterval
from direct.interval.IntervalGlobal import Sequence


base = ShowBase()
base.set_background_color(0.1, 0.1, 0.1, 1)

base.cam.set_pos(0, -128, 32)
base.cam.look_at(render)

flame_colors = (
    Vec4(1.0, 0.0, 0.0, 1),
    Vec4(1.0, 0.2, 0.0, 1),
    Vec4(1.0, 0.7, 0.0, 1),
    Vec4(0.0, 0.0, 0.2, 1),
)

# A NodePath, rotating in empty space.
pivot = render.attach_new_node("pivot")
pivot.hprInterval(3, (360, 0, 0)).loop()
Sequence( # Bobs up and down
    LerpPosInterval(pivot, 0.3, (0, 0,-2), (0, 0, 1), blendType="easeInOut"),
    LerpPosInterval(pivot, 0.5, (0, 0, 1), (0, 0,-2), blendType="easeInOut")
).loop()

# A little chunk of charcoal that rotates along the pivot with an offset.
charcoal = loader.load_model("models/smiley").copy_to(pivot)
charcoal.set_texture(loader.load_texture("models/plasma.png"), 1)
charcoal.set_color(flame_colors[0] * 1.5)
charcoal.set_x(-32)

# It leaves a trail of flames.
fire_trail = MotionTrail("fire trail", charcoal)
fire_trail.register_motion_trail()
fire_trail.geom_node_path.reparent_to(render)
fire_trail.set_texture(loader.load_texture("models/plasma.png"))
fire_trail.time_window = 3 # Length of trail

# A circle as the trail's shape, by plotting a NodePath in a circle.
center = render.attach_new_node("center")
around = center.attach_new_node("around")
around.set_z(1)
res = 8 # Amount of angles in "circle". Higher is smoother.
for i in range(res + 1):
    center.set_r((360 / res) * i)
    vertex_pos = around.get_pos(render)
    fire_trail.add_vertex(vertex_pos)

    start_color = flame_colors[i % len(flame_colors)] * 1.7
    end_color = Vec4(1, 1, 0, 1)
    fire_trail.set_vertex_color(i, start_color, end_color)

'''
# A simple flat line, tron lightcycle-style, would be like so:

fire_trail.add_vertex(Point3(0, 0, 1))
fire_trail.add_vertex(Point3(0, 0,-1))
fire_trail.set_vertex_color(0, flame_colors[0], flame_colors[0])
fire_trail.set_vertex_color(1, flame_colors[1], flame_colors[1])
'''

fire_trail.update_vertices()

# Adding intervals to the trail to give it swoops and bends.
LerpHprInterval(fire_trail, 2, (0, 0, -360)).loop()
LerpTexOffsetInterval(fire_trail.geom_node_path, 4, (1, 1), (1, 0)).loop()
Sequence( # Grow and shrink
    LerpScaleInterval(fire_trail, 0.3, 1.4, 0.4, blendType="easeInOut"),
    LerpScaleInterval(fire_trail, 0.5, 0.4, 1.4, blendType="easeInOut")
).loop()

base.run()
