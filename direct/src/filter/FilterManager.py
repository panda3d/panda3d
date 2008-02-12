"""

The FilterManager is a convenience class that helps with the creation
of render-to-texture buffers for image postprocessing applications.

Still need to implement:

* Make sure sort-order of buffers is correct.
* Matching buffer size to original region instead of original window.
* Intermediate layer creation.
* Handling of window clears.
* Resizing of windows.
* Do something about window-size roundoff problems.

"""

from pandac.PandaModules import Point3, Vec3, Vec4
from pandac.PandaModules import NodePath, PandaNode
from pandac.PandaModules import RenderState, Texture, Shader
from pandac.PandaModules import CardMaker
from pandac.PandaModules import TextureStage
from pandac.PandaModules import GraphicsPipe, GraphicsOutput
from pandac.PandaModules import WindowProperties, FrameBufferProperties
from pandac.PandaModules import Camera, DisplayRegion
from pandac.PandaModules import OrthographicLens
from pandac.PandaModules import AuxBitplaneAttrib
from direct.directnotify.DirectNotifyGlobal import *

__all__ = ["FilterManager"]

class FilterManager:

    notify = None

    def __init__(self, win, cam):

        """ The FilterManager constructor requires you to provide
        a window which is rendering a scene, and the camera which is
        used by that window to render the scene.  These are henceforth
        called the 'original window' and the 'original camera.' """

        # Create the notify category

        if (FilterManager.notify == None):
            FilterManager.notify = directNotify.newCategory("FilterManager")

        # Find the appropriate display region.
        
        region = None
        for i in range(win.getNumDisplayRegions()):
            dr = win.getDisplayRegion(i)
            drcam = dr.getCamera()
            if (drcam == cam): region=dr

        if (region == None):
            self.notify.error('Could not find appropriate DisplayRegion to filter')
            return False
        
        # Instance Variables.

        self.win = win
        self.engine = win.getGsg().getEngine()
        self.region = region
        self.camera = cam
        self.caminit = cam.node().getInitialState()
        self.scales = []
        self.buffers = []

    def renderSceneInto(self, depthtex=False, colortex=False, normaltex=False, textures=None):

        """ Causes the scene to be rendered into the supplied textures
        instead of into the original window.  Puts a fullscreen quad
        into the original window to show the render-to-texture results.
        Returns the quad.  Normally, the caller would then apply a
        shader to the quad.

        To elaborate on how this all works:

        * An offscreen buffer is created.  It is set up to mimic
          the original window - it is the same size, uses the
          same clear colors, and contains a DisplayRegion that
          uses the original camera.

        * A fullscreen quad and an orthographic camera to render
          that quad are both created.  The original camera is
          removed from the original window, and in its place, the
          orthographic quad-camera is installed.

        * The fullscreen quad is textured with the data from the
          offscreen buffer.  A shader is applied that tints the
          results pink.

        Hence, the original window which used to contain the actual
        scene, now contains a pink-tinted quad with a texture of the
        scene.  It is assumed that the user will replace the shader
        on the quad with a more interesting filter. """

        if (textures):
            colortex = textures.get("color", None)
            depthtex = textures.get("depth", None)
            normaltex = textures.get("normal", None)

        if (colortex == None): colortex = Texture("filter-base-color")

        texgroup = (depthtex, colortex, normaltex, None)

        buffer = self.createBuffer("filter-base", base.win.getXSize(), base.win.getYSize(), texgroup)

        if (buffer == None):
            return None

        cm = CardMaker("filter-base-quad")
        cm.setFrameFullscreenQuad()
        quad = NodePath(cm.generate())
        quad.setDepthTest(0)
        quad.setDepthWrite(0)
        quad.setTexture(colortex)
        quad.setColor(Vec4(1,0.5,0.5,1))

        auxbits = AuxBitplaneAttrib.ABOColor;
        if (normaltex != None):
            auxbits += AuxBitplaneAttrib.ABOCsnormal;
        
        cs = NodePath("dummy")
        cs.setState(self.caminit)
        cs.setShaderAuto();
        cs.setAttrib(AuxBitplaneAttrib.make(auxbits))
        self.camera.node().setInitialState(cs.getState())

        quadcamnode = Camera("filter-quad-cam")
        lens = OrthographicLens()
        lens.setFilmSize(2, 2)
        lens.setFilmOffset(0, 0)
        lens.setNearFar(-1000, 1000)
        quadcamnode.setLens(lens)
        quadcam = quad.attachNewNode(quadcamnode)
        
        self.region.setCamera(quadcam)

        buffer.getDisplayRegion(0).setCamera(self.camera)
        buffer.getDisplayRegion(0).setActive(1)

        self.scales.append(1)
        self.buffers.append(buffer)

        return quad

    def renderQuadInto(self, scale=1, depthtex=None, colortex=None, auxtex0=None, auxtex1=None):

        """ Creates an offscreen buffer for an intermediate
        computation. Installs a quad into the buffer.  Returns
        the fullscreen quad. """

        self.notify.error('renderQuadInto not implemented yet.')
        return None

    def createBuffer(self, name, xsize, ysize, texgroup):
        """ Low-level buffer creation.  Not intended for public use. """
        winprops = WindowProperties()
        props = FrameBufferProperties()
        props.setRgbColor(1)
        props.setDepthBits(1)
        depthtex, colortex, auxtex0, auxtex1 = texgroup
        if (auxtex0 != None):
            props.setAuxRgba(1)
        if (auxtex1 != None):
            props.setAuxRgba(2)
        buffer=base.graphicsEngine.makeOutput(
            self.win.getPipe(), name, -1,
            props, winprops, GraphicsPipe.BFRefuseWindow,
            self.win.getGsg(), self.win)
        if (depthtex):
            buffer.addRenderTexture(depthtex, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPDepth)
        if (colortex):
            buffer.addRenderTexture(colortex, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPColor)
        if (auxtex0):
            buffer.addRenderTexture(auxtex0, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPAuxRgba0)
        if (auxtex1):
            buffer.addRenderTexture(auxtex1, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPAuxRgba1)
        return buffer


    def cleanup(self):
        """ Restore everything to its original state, deleting any
        new buffers in the process. """

        for buffer in self.buffers:
            buffer.clearRenderTextures()
            self.engine.removeWindow(buffer)
        self.scales = []
        self.buffers = []
        self.region.setCamera(self.camera)
        self.camera.node().setInitialState(self.caminit)


