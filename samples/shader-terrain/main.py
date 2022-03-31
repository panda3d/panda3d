#!/usr/bin/env python

# Author: tobspr
#
# Last Updated: 2016-04-30
#
# This tutorial provides an example of using the ShaderTerrainMesh class

from direct.showbase.ShowBase import ShowBase
from panda3d.core import ShaderTerrainMesh, Shader, load_prc_file_data
from panda3d.core import SamplerState


class ShaderTerrainDemo(ShowBase):
    def __init__(self):

        # Load some configuration variables, its important for this to happen
        # before the ShowBase is initialized
        load_prc_file_data("", """
            textures-power-2 none
            gl-coordinate-system default
            window-title Panda3D ShaderTerrainMesh Demo
            filled-wireframe-apply-shader true

            # As an optimization, set this to the maximum number of cameras
            # or lights that will be rendering the terrain at any given time.
            stm-max-views 8

            # Further optimize the performance by reducing this to the max
            # number of chunks that will be visible at any given time.
            stm-max-chunk-count 2048
        """)

        # Initialize the showbase
        ShowBase.__init__(self)

        # Increase camera FOV as well as the far plane
        self.camLens.set_fov(90)
        self.camLens.set_near_far(0.1, 50000)

        # Construct the terrain
        self.terrain_node = ShaderTerrainMesh()

        # Set a heightfield, the heightfield should be a 16-bit png and
        # have a quadratic size of a power of two.
        heightfield = self.loader.loadTexture("heightfield.png")
        heightfield.wrap_u = SamplerState.WM_clamp
        heightfield.wrap_v = SamplerState.WM_clamp
        self.terrain_node.heightfield = heightfield

        # Set the target triangle width. For a value of 10.0 for example,
        # the terrain will attempt to make every triangle 10 pixels wide on screen.
        self.terrain_node.target_triangle_width = 10.0

        # Generate the terrain
        self.terrain_node.generate()

        # Attach the terrain to the main scene and set its scale. With no scale
        # set, the terrain ranges from (0, 0, 0) to (1, 1, 1)
        self.terrain = self.render.attach_new_node(self.terrain_node)
        self.terrain.set_scale(1024, 1024, 100)
        self.terrain.set_pos(-512, -512, -70.0)

        # Set a shader on the terrain. The ShaderTerrainMesh only works with
        # an applied shader. You can use the shaders used here in your own application
        terrain_shader = Shader.load(Shader.SL_GLSL, "terrain.vert.glsl", "terrain.frag.glsl")
        self.terrain.set_shader(terrain_shader)
        self.terrain.set_shader_input("camera", self.camera)

        # Shortcut to view the wireframe mesh
        self.accept("f3", self.toggleWireframe)

        # Set some texture on the terrain
        grass_tex = self.loader.loadTexture("textures/grass.png")
        grass_tex.set_minfilter(SamplerState.FT_linear_mipmap_linear)
        grass_tex.set_anisotropic_degree(16)
        self.terrain.set_texture(grass_tex)

        # Load a skybox - you can safely ignore this code
        skybox = self.loader.loadModel("models/skybox.bam")
        skybox.reparent_to(self.render)
        skybox.set_scale(20000)

        skybox_texture = self.loader.loadTexture("textures/skybox.jpg")
        skybox_texture.set_minfilter(SamplerState.FT_linear)
        skybox_texture.set_magfilter(SamplerState.FT_linear)
        skybox_texture.set_wrap_u(SamplerState.WM_repeat)
        skybox_texture.set_wrap_v(SamplerState.WM_mirror)
        skybox_texture.set_anisotropic_degree(16)
        skybox.set_texture(skybox_texture)

        skybox_shader = Shader.load(Shader.SL_GLSL, "skybox.vert.glsl", "skybox.frag.glsl")
        skybox.set_shader(skybox_shader)

ShaderTerrainDemo().run()
