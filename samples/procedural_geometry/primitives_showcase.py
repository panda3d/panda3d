#!/usr/bin/env python

# Author: Epihaius
# Date: 2019-06-21
#
# This demo showcases procedural generation of a variety of 3D primitive model
# types, such as boxes, spheres and cylinders.
# The actual geometry generation code can be found in the src/prim subpackage.
# The models are created using parameter values read from the box.txt, sphere.txt,
# cylinder.txt, cone.txt and torus.txt files located in the main sample directory.
# Edit these files to create different variations of the corresponding primitive
# model types.

from panda3d.core import *
from direct.showbase.ShowBase import ShowBase
from src.gui import *
from src.nav import NavigationManager
from src.prim import *
from src.textures import Textures
from time import strftime


class Demo:

    def __init__(self):

        self.showbase = showbase = ShowBase()
        showbase.disable_mouse()

        gui = GUI(showbase)
        NavigationManager(showbase, gui_mouse_watcher=gui.mouse_watcher)

        # Build the GUI layout

        title = "Panda3D: Showcase - Generating primitives procedurally"
        label = Label(gui.window, 10, title)
        borders = (10, 10, 10, 10)
        gui.add(label, expand=True, borders=borders)

        gui.add((0, 0), proportion=1.)

        sizer = Sizer("vertical")
        borders = (10, 10, 10, 10)
        gui.add(sizer, borders=borders)
        borders = (0, 0, 10, 0)
        get_command = lambda prim_type: lambda: self.__create_primitive(prim_type)

        for prim_type in ("box", "sphere", "cylinder", "cone", "torus"):
            command = get_command(prim_type)
            text = "Create {}".format(prim_type)
            button = Button(gui.window, 10, text, command=command)
            sizer.add(button, expand=True, borders=borders)

        text = 'Edit "<primitive type>.txt"\nfile to change parameters'
        label = Label(gui.window, 6, text, text_alignment="left")
        borders = (10, 10, 10, 10)
        gui.add(label, borders=borders)

        gui.add((0, 0), proportion=1.)

        sizer = Sizer("horizontal")
        borders = (10, 10, 10, 10)
        gui.add(sizer, alignment="right", borders=borders)
        borders = (0, 10, 0, 0)
        command = self.__toggle_wireframe
        button = Button(gui.window, 10, "Toggle wireframe", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = self.__toggle_filled_wireframe
        button = Button(gui.window, 10, "Toggle filled wireframe", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = self.__toggle_texture
        button = Button(gui.window, 10, "Toggle texture", command=command)
        sizer.add(button, expand=True, borders=borders)
        command = self.__save_to_bam
        button = Button(gui.window, 10, "Save to .bam", command=command)
        sizer.add(button, expand=True)

        text = "Orbit camera: left mouse button; zoom: mouse wheel"
        label = Label(gui.window, 6, text, text_alignment="left")
        borders = (10, 10, 10, 10)
        gui.add(label, expand=True, borders=borders)

        gui.finalize()

        # procedurally generate some textures
        Textures.create()

        # set up a light source
        light_node = PointLight("point_light")
        light_node.set_color((1., 1., 1., 1.))
        self.light = showbase.camera.attach_new_node(light_node)
        self.light.set_pos(5., -10., 7.)
        showbase.render.set_light(self.light)

        self.model = None
        self.render_mode = "filled"
        self.render_textured = False
        self.prim_type = ""

        # although not strictly necessary, it's nice to already have a model
        # in the scene at startup
        self.__create_primitive("box")

        showbase.run()

    def __create_primitive(self, prim_type):

        def parse_to_dict(dic, name, value, value_type):

            if value in ("None", "none"):
                dic[name] = None
            elif value in ("False", "false"):
                dic[name] = False
            elif value in ("True", "true"):
                dic[name] = True
            elif "dict" in value_type:
                name, key = name.split()
                val_type = value_type.replace("dict", "").strip()
                d = dic.setdefault(name, {})
                parse_to_dict(d, key, value, val_type)
            elif "tuple" in value_type:
                val_type, _ = value_type.split()
                dic[name] = parse_to_tuple(value, val_type)
            elif value_type == "string":
                dic[name] = value
            elif value_type == "float":
                dic[name] = float(value)
            elif value_type == "int":
                dic[name] = int(value)

        def parse_to_tuple(value, value_type):

            if value_type == "string":
                if not value:
                    return ()
                else:
                    return tuple(value.split())
            elif value_type == "int":
                return tuple(int(val) for val in value.split())
            elif value_type == "float":
                return tuple(float(val) for val in value.split())

        with open("{}.txt".format(prim_type)) as prim_file:

            params = {}

            for line in prim_file:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                else:
                    item, value_type = line.split("#")
                    name, value = item.split("=")
                    name = name.strip()
                    value = value.strip()
                    value_type = value_type.strip()
                    parse_to_dict(params, name, value, value_type)

        if self.model:
            self.model.detach_node()

        if prim_type == "box":
            model_maker = BoxMaker(**params)
        elif prim_type == "sphere":
            model_maker = SphereMaker(**params)
        elif prim_type == "cylinder":
            model_maker = CylinderMaker(**params)
        elif prim_type == "cone":
            model_maker = ConeMaker(**params)
        elif prim_type == "torus":
            model_maker = TorusMaker(**params)

        self.model = self.showbase.render.attach_new_node(model_maker.generate())

        if self.render_mode == "wire":
            self.model.set_render_mode_wireframe()
        elif self.render_mode == "filled_wire":
            self.model.set_render_mode_filled_wireframe((1., 0., 0., 1.))

        if self.render_textured:
            self.model.set_texture(Textures.uv_map)

        self.prim_type = prim_type

    def __toggle_wireframe(self):

        if not self.model:
            return

        if self.render_mode == "wire":
            self.model.set_render_mode_filled()
            self.render_mode = "filled"
        else:
            self.model.set_render_mode_wireframe()
            self.render_mode = "wire"

    def __toggle_filled_wireframe(self):

        if not self.model:
            return

        if self.render_mode == "filled_wire":
            self.model.set_render_mode_filled()
            self.render_mode = "filled"
        else:
            self.model.set_render_mode_filled_wireframe((1., 0., 0., 1.))
            self.render_mode = "filled_wire"

    def __toggle_texture(self):

        if not self.model:
            return

        self.render_textured = not self.render_textured

        if self.render_textured:
            self.model.set_texture(Textures.uv_map)
        else:
            self.model.clear_texture()

    def __save_to_bam(self):

        if not self.model:
            return

        timestamp = strftime("%Y-%m-%d_%H-%M-%S")
        name = "procedural_{}_{}".format(self.prim_type, timestamp)
        root = NodePath(ModelRoot(name))
        model = self.model.copy_to(root)
        model.set_render_mode_filled()
        model.clear_texture()
        root.write_bam_file("{}.bam".format(name))


demo = Demo()
