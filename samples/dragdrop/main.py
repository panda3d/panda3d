'''
Drag & Drop Demo

you can drag the the smaller,darker colored squares in the "correct" bigger colored
squares.

there are default values and "types" that make the small square fall back to the
last valid "anchor"

there are three main functions parts that need to be understood to understand
the program:

drag and drop functions are functions that are executed when the object
is picked up or dropped

update is a task that runs continously and updates the position of the dragged
object

'''

import random

from panda3d.core import *
from direct.gui.DirectGui import *
from direct.showbase import ShowBase

class TargetColoredGrid:
    def __init__(self, pos, offset, colors, rows_collums=(5, 1)):

        # making sure the "targets" are shuffled, so they rely on
        # the type system
        col_l = ["white", "black", "light_red", "light_blue", "light_green"]

        random.shuffle(col_l)

        x, y = rows_collums
        self.d = {}

        x_i = 0
        while x_i < x:
            col_key = col_l[x_i]
            col_i = colors[col_key]
            y_i = 0
            self.d[x_i] = {}
            while y_i < y:

                dd_type = None

                pos_i = (pos[0] + offset[0] * x_i, pos[1] + offset[1] * y_i)

                frame = construct_frame(
                    pos_i,
                    color=col_i,
                    drag_drop_type=col_key)

                self.d[x_i][y_i] = frame

                y_i += 1
            x_i += 1


class Grid:
    """just a utility function to build a grid quickly."""

    def __init__(self, pos, offset, rows_collums=(3, 3)):
        x, y = rows_collums
        self.d = {}

        x_i = 0
        while x_i < x:

            y_i = 0
            self.d[x_i] = {}
            while y_i < y:

                dd_type = None

                if x_i == 2:
                    dd_type = "red"

                pos_i = (pos[0] + offset[0] * x_i, pos[1] + offset[1] * y_i)
                frame = construct_frame(
                    pos_i, drag_drop_type=dd_type)

                self.d[x_i][y_i] = frame

                y_i += 1
            x_i += 1


def construct_frame(pos, frame_kwargs={}, color=(
        1, 1, 1, 1.0), drag_drop_type=None):

    if frame_kwargs == {}:

        kwargs = {"frameColor": color,
                  "frameSize": _rec2d(50, 50),
                  "state": DGG.NORMAL,
                  "parent": pixel2d}

    kwargs["frameColor"] = color

    frame = DirectFrame(**kwargs)
    frame.set_pos(_pos2d(*pos))
    frame.drag_drop_type = drag_drop_type
    return frame

# Helper functions


def _pos2d(x, y):
    return Point3(x, 0, -y)


def _rec2d(width, height):
    return (width, 0, 0, -height)


def get_local_center(ob):

    h = ob.getHeight()
    w = abs(ob.getWidth())

    w = w / 2
    h = h / 2

    return h, w


def get_mid_point(ob):
    h, w = get_local_center(ob)

    r = ob.getPos()

    r = r - LPoint3f(h, w)

    return r
    left, right, bottom, top = ob.frameSize()

    half_r = (right - left) / 2
    half_b = (bottom - top) / 2

    mid_x = left + half_r
    mid_y = top + half_b

    return mid_x, mid_y


class App:
    def __init__(self):

        # for demonstration purposes:
        # get a few colors as "types"

        # red green blue white black
        # only provide red green blue cubes

        colors = {"red": (0.8, 0.2, 0.2, 1.0), "light_red": (1, 0.5, 0.5, 1.0),
                  "green": (0.2, 0.8, 0.2, 1.0), "light_green": (0.5, 1, 0.5, 1.0),
                  "blue": (0.2, 0.2, 0.8, 1.0), "light_blue": (0.5, 0.5, 1, 1.0),
                  "white": (1, 1, 1, 1.0), "black": (0, 0, 0, 1.0)}

        color_l = ["red", "green", "blue"]

        # add a title/instructions
        wp = WindowProperties.getDefault()
        wp.set_title("Drag & Drop the colored squares")
        WindowProperties.setDefault(wp)
        # init showvase
        base = ShowBase.ShowBase()

        self.grid = TargetColoredGrid((32, 32), (64, 64), colors)

        self.bind_grid_events(self.grid)

        # helper attributes

        self.current_dragged = None
        self.last_hover_in = None
        self.current_last_anchor = None
        self.current_drag_drop_type = None
        self.last_hover_drag_drop_type = None

        # the grid containing the starting items
        self.default_grid = Grid((64, 300), (64, 64), (3, 1))
        self.bind_grid_events(self.default_grid)
        self.drag_items = {}

        # make some squares to be drag/dropped
        x = 0
        for col in color_l:
            self.drag_items[x] = {}
            rel_col = colors[col]

            frame = DirectFrame(frameColor=rel_col,
                                frameSize=_rec2d(30, 30),
                                state=DGG.NORMAL,
                                parent=pixel2d)

            # bind the events
            frame.bind(DGG.B1PRESS, self.drag, [frame])
            frame.bind(DGG.B1RELEASE, self.drop)

            frame.set_pos(_pos2d(32 + x * 128, 264))
            # the type in our case is just the color, but it can be
            # anything that you can compare
            frame.drag_drop_type = col

            # anchor here
            frame.last_drag_drop_anchor = self.default_grid.d[x][0]

            snap(frame, self.default_grid.d[x][0])

            self.drag_items[x] = frame  # took out y
            x += 1

        # run a task tracking the mouse cursor
        taskMgr.add(self.update, "update", sort=-50)

    def bind_grid_events(self, grid):

        # bind the events
        for key1 in grid.d:
            for key2 in grid.d[key1]:
                ob = grid.d[key1][key2]
                ob.bind(DGG.WITHIN, self.hover_in, [ob])
                ob.bind(DGG.WITHOUT, self.hover_out)

    def hover_in(self, widget, mouse_pos=None):
        '''Set the widget to be the target to drop objects onto'''

        self.last_hover_in = widget
        self.last_hover_drag_drop_type = widget.drag_drop_type

    def hover_out(self, mouse_pos=None):
        '''Clear the target to drop objects onto'''
        self.last_hover_in = None
        self.last_hover_drag_drop_type = None

    def update(self, task=None):
        '''Track the mouse pos and move self.current_dragged to where the cursor is '''
        if self.current_dragged:

            if base.mouseWatcherNode.has_mouse():
                mpos = base.mouseWatcherNode.get_mouse()
                pos = Point3(mpos.get_x(), 0, mpos.get_y())

                self.current_dragged.set_pos(
                    pixel2d.get_relative_point(render2d, pos))

        if task:
            return task.again

    def drag(self, widget, mouse_pos=None):
        '''Set the widget to be the currently dragged object'''

        self.current_last_drag_drop_anchor = widget.last_drag_drop_anchor

        h, w = get_local_center(widget)

        widget.reparent_to(pixel2d)
        self.current_dragged = widget
        self.current_dragged.h = h
        self.current_dragged.w = w
        self.update()

    def drop(self, mouse_pos=None):
        '''Drop the currently dragged object on the last object the cursor hovered over'''
        if self.current_dragged:

            if self.last_hover_in:
                if self.last_hover_drag_drop_type is not None:
                    if self.current_dragged.drag_drop_type in self.last_hover_drag_drop_type:
                        # everything in order, snap to the one that's currently being
                        # hovered over.
                        snap_target = self.last_hover_in
                        self.current_dragged.last_drag_drop_anchor = snap_target
                    else:
                        # in this case the frame the user hovers over is invalid.
                        # the last one gets picked as default target and is
                        # snapped to.
                        snap_target = self.current_last_drag_drop_anchor

                else:
                    snap_target = self.current_last_drag_drop_anchor

                snap(self.current_dragged, snap_target)

            self.current_dragged = None


def snap(ob, target):

    # this is just some math for nice centering
    lock_pos = get_mid_point(target)
    h, w = get_local_center(ob)
    lock_pos = lock_pos + LPoint3f(h, w)
    lock_pos = LVecBase3f(*lock_pos)

    # this does the actual snapping
    ob.setPos(lock_pos)
    ob.wrt_reparent_to(target)


app = App()
base.run()
