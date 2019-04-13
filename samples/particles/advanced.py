#!/usr/bin/env python

# This program shows a shader-based particle system. With this approach, you
# can define an inertial particle system with a moving emitter whose position
# can not be pre-determined.

from array import array
from itertools import chain
from random import uniform
from math import pi, sin, cos
from panda3d.core import TextNode
from panda3d.core import AmbientLight, DirectionalLight
from panda3d.core import LVector3
from panda3d.core import NodePath
from panda3d.core import GeomPoints
from panda3d.core import GeomEnums
from panda3d.core import GeomVertexFormat
from panda3d.core import GeomVertexData
from panda3d.core import GeomNode
from panda3d.core import Geom
from panda3d.core import OmniBoundingVolume
from panda3d.core import Texture
from panda3d.core import TextureStage
from panda3d.core import TexGenAttrib
from panda3d.core import Shader
from panda3d.core import ShaderAttrib
from panda3d.core import loadPrcFileData
from direct.showbase.ShowBase import ShowBase
from direct.gui.OnscreenText import OnscreenText
import sys

HELP_TEXT = """
left/right arrow: Rotate teapot
ESC: Quit
"""

# We need to use GLSL 1.50 for these, and some drivers (notably Mesa) require
# us to explicitly ask for an OpenGL 3.2 context in that case.
config = """
gl-version 3 2
"""

vert = """
#version 150
#extension GL_ARB_shader_image_load_store : require

layout(rgba32f) uniform imageBuffer positions;       // current positions
layout(rgba32f) uniform imageBuffer start_vel;       // emission velocities
layout(rgba32f) uniform imageBuffer velocities;      // current velocities
layout(rgba32f) uniform imageBuffer emission_times;  // emission times
uniform mat4 p3d_ModelViewProjectionMatrix;
uniform vec3 emitter_pos;     // emitter's position
uniform vec3 accel;           // the acceleration of the particles
uniform float osg_FrameTime;  // time of the current frame (absolute)
uniform float osg_DeltaFrameTime;// time since last frame
uniform float start_time;     // particle system's start time (absolute)
uniform float part_duration;  // single particle's duration

out float from_emission;      // time from specific particle's emission
out vec4 color;

void main() {
    float emission_time = imageLoad(emission_times, gl_VertexID).x;
    vec4 pos = imageLoad(positions, gl_VertexID);
    vec4 vel = imageLoad(velocities, gl_VertexID);
    float from_start = osg_FrameTime - start_time;  // time from system's start
    from_emission = 0;
    color = vec4(1);
    if (from_start > emission_time) {  // we've to show the particle
        from_emission = from_start - emission_time;
        if (from_emission <= osg_DeltaFrameTime + .01) {
            // it's particle's emission frame: let's set its position at the
            // emitter's position and set the initial velocity
            pos = vec4(emitter_pos, 1);
            vel = imageLoad(start_vel, gl_VertexID);
        }
        pos += vec4((vel * osg_DeltaFrameTime).xyz, 0);
        vel += vec4(accel, 0) * osg_DeltaFrameTime;
    } else color = vec4(0);

    // update the emission time (for particle recycling)
    if (from_start >= emission_time + part_duration) {
        imageStore(emission_times, gl_VertexID, vec4(from_start, 0, 0, 1));
    }
    gl_PointSize = 10;
    gl_Position = p3d_ModelViewProjectionMatrix * pos;
    imageStore(positions, gl_VertexID, pos);
    imageStore(velocities, gl_VertexID, vel);
}
"""

frag = """
#version 150

in float from_emission;       // time elapsed from particle's emission
in vec4 color;
uniform float part_duration;  // single particle's duration
uniform sampler2D image;      // particle's texture
out vec4 p3d_FragData[1];

void main() {
    vec4 col = texture(image, gl_PointCoord) * color;
    // fade the particle considering the time from its emission
    float alpha = clamp(1 - from_emission / part_duration, 0, 1);
    p3d_FragData[0] = vec4(col.rgb, col.a * alpha);
}
"""


class Particle:

    def __init__(
            self,
            emitter,          # the node which is emitting
            texture,          # particle's image
            rate=.001,        # the emission rate
            gravity=-9.81,    # z-component of the gravity force
            vel=1.0,          # length of emission vector
            partDuration=1.0  # single particle's duration
            ):
        self.__emitter = emitter
        self.__texture = texture
        # let's compute the total number of particles
        self.__numPart = int(round(partDuration * 1 / rate))
        self.__rate = rate
        self.__gravity = gravity
        self.__vel = vel
        self.__partDuration = partDuration
        self.__nodepath = render.attachNewNode(self.__node())
        self.__nodepath.setTransparency(True)  # particles have alpha
        self.__nodepath.setBin("fixed", 0)     # render it at the end
        self.__setTextures()
        self.__setShader()
        self.__nodepath.setRenderModeThickness(10)  # we want sprite particles
        self.__nodepath.setTexGen(TextureStage.getDefault(),
                                  TexGenAttrib.MPointSprite)
        self.__nodepath.setDepthWrite(False)   # don't sort the particles
        self.__upd_tsk = taskMgr.add(self.__update, "update")

    def __node(self):
        # this function creates and returns particles' GeomNode
        points = GeomPoints(GeomEnums.UH_static)
        points.addNextVertices(self.__numPart)
        format_ = GeomVertexFormat.getEmpty()
        geom = Geom(GeomVertexData("abc", format_, GeomEnums.UH_static))
        geom.addPrimitive(points)
        geom.setBounds(OmniBoundingVolume())  # always render it
        node = GeomNode("node")
        node.addGeom(geom)
        return node

    def __setTextures(self):
        # initial positions are all zeros (each position is denoted by 4 values)
        # positions are stored in a texture
        positions = [(0, 0, 0, 1) for i in range(self.__numPart)]
        posLst = list(chain.from_iterable(positions))
        self.__texPos = self.__buffTex(posLst)

        # define emission times' texture
        emissionTimes = [(self.__rate * i, 0, 0, 0)
                          for i in range(self.__numPart)]
        timesLst = list(chain.from_iterable(emissionTimes))
        self.__texTimes = self.__buffTex(timesLst)

        # define a list with emission velocities
        velocities = [self.__rndVel() for _ in range(self.__numPart)]
        velLst = list(chain.from_iterable(velocities))
        # we need two textures,
        # the first one contains the emission velocity (we need to keep it for
        # particle recycling)...
        self.__texStartVel = self.__buffTex(velLst)
        # ... and the second one contains the current velocities
        self.__texCurrVel = self.__buffTex(velLst)

    def __buffTex(self, values):
        # this function returns a buffer texture with the received values
        data = array("f", values)
        tex = Texture("tex")
        tex.setupBufferTexture(self.__numPart, Texture.T_float,
                               Texture.F_rgba32, GeomEnums.UH_static)
        tex.setRamImage(data)
        return tex

    def __rndVel(self):
        # this method returns a random vector for emitting the particle
        theta = uniform(0, pi / 12)
        phi = uniform(0, 2 * pi)
        vec = LVector3(
            sin(theta) * cos(phi),
            sin(theta) * sin(phi),
            cos(theta))
        vec *= uniform(self.__vel * .8, self.__vel * 1.2)
        return [vec.x, vec.y, vec.z, 1]

    def __setShader(self):
        shader = Shader.make(Shader.SL_GLSL, vert, frag)

        # Apply the shader to the node, but set a special flag indicating that
        # the point size is controlled bythe shader.
        attrib = ShaderAttrib.make(shader)
        attrib = attrib.setFlag(ShaderAttrib.F_shader_point_size, True)
        self.__nodepath.setAttrib(attrib)

        self.__nodepath.setShaderInputs(
            positions=self.__texPos,
            emitter_pos=self.__emitter.getPos(render),
            start_vel=self.__texStartVel,
            velocities=self.__texCurrVel,
            accel=(0, 0, self.__gravity),
            start_time=globalClock.getFrameTime(),
            emission_times=self.__texTimes,
            part_duration=self.__partDuration,
            image=loader.loadTexture(self.__texture))

    def __update(self, task):
        pos = self.__emitter.getPos(render)
        self.__nodepath.setShaderInput("emitter_pos", pos)
        return task.again


class ParticleDemo(ShowBase):

    def __init__(self):
        loadPrcFileData("config", config)

        ShowBase.__init__(self)

        # Standard title and instruction text
        self.title = OnscreenText(
            text="Panda3D: Tutorial - Shader-based Particles",
            parent=base.a2dBottomCenter,
            style=1, fg=(1, 1, 1, 1), pos=(0, 0.1), scale=.08)
        self.escapeEvent = OnscreenText(
            text=HELP_TEXT, parent=base.a2dTopLeft,
            style=1, fg=(1, 1, 1, 1), pos=(0.06, -0.06),
            align=TextNode.ALeft, scale=.05)

        # More standard initialization
        self.accept('escape', sys.exit)
        self.accept('arrow_left', self.rotate, ['left'])
        self.accept('arrow_right', self.rotate, ['right'])
        base.disableMouse()
        base.camera.setPos(0, -20, 2)
        base.setBackgroundColor(0, 0, 0)

        self.teapot = loader.loadModel("teapot")
        self.teapot.setPos(0, 10, 0)
        self.teapot.reparentTo(render)
        self.setupLights()

        # we define a nodepath as particle's emitter
        self.emitter = NodePath("emitter")
        self.emitter.reparentTo(self.teapot)
        self.emitter.setPos(3.000, 0.000, 2.550)

        # let's create the particle system
        Particle(self.emitter, "smoke.png", gravity=.01, vel=1.2,
                 partDuration=5.0)

    def rotate(self, direction):
        direction_factor = (1 if direction == "left" else -1)
        self.teapot.setH(self.teapot.getH() + 10 * direction_factor)

    # Set up lighting
    def setupLights(self):
        ambientLight = AmbientLight("ambientLight")
        ambientLight.setColor((.4, .4, .35, 1))
        directionalLight = DirectionalLight("directionalLight")
        directionalLight.setDirection(LVector3(0, 8, -2.5))
        directionalLight.setColor((0.9, 0.8, 0.9, 1))

        # Set lighting on teapot so steam doesn't get affected
        self.teapot.setLight(self.teapot.attachNewNode(directionalLight))
        self.teapot.setLight(self.teapot.attachNewNode(ambientLight))


demo = ParticleDemo()
demo.run()
