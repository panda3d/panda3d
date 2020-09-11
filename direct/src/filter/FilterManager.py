"""

The FilterManager is a convenience class that helps with the creation
of render-to-texture buffers for image postprocessing applications.

See :ref:`generalized-image-filters` for information on how to use this class.

Still need to implement:

* Make sure sort-order of buffers is correct.
* Matching buffer size to original region instead of original window.
* Intermediate layer creation.
* Handling of window clears.
* Resizing of windows.
* Do something about window-size roundoff problems.

"""

from panda3d.core import NodePath
from panda3d.core import Texture
from panda3d.core import CardMaker
from panda3d.core import GraphicsPipe, GraphicsOutput
from panda3d.core import WindowProperties, FrameBufferProperties
from panda3d.core import Camera
from panda3d.core import OrthographicLens
from panda3d.core import AuxBitplaneAttrib
from panda3d.core import LightRampAttrib
from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase.DirectObject import DirectObject

__all__ = ["FilterManager"]

class FilterManager(DirectObject):

    notify = None

    def __init__(self, win, cam, forcex=0, forcey=0):

        """ The FilterManager constructor requires you to provide
        a window which is rendering a scene, and the camera which is
        used by that window to render the scene.  These are henceforth
        called the 'original window' and the 'original camera.' """

        # Create the notify category

        if FilterManager.notify is None:
            FilterManager.notify = directNotify.newCategory("FilterManager")

        # Find the appropriate display region.

        region = None
        for dr in win.getDisplayRegions():
            drcam = dr.getCamera()
            if drcam == cam:
                region = dr

        if region is None:
            self.notify.error('Could not find appropriate DisplayRegion to filter')
            return False

        # Instance Variables.

        self.win = win
        self.forcex = forcex
        self.forcey = forcey
        self.engine = win.getGsg().getEngine()
        self.region = region
        self.wclears = self.getClears(self.win)
        self.rclears = self.getClears(self.region)
        self.camera = cam
        self.caminit = cam.node().getInitialState()
        self.camstate = self.caminit
        self.buffers = []
        self.sizes = []
        self.nextsort = self.win.getSort() - 1000
        self.basex = 0
        self.basey = 0
        self.accept("window-event", self.windowEvent)


    def getClears(self,region):
        clears = []
        for i in range(GraphicsOutput.RTPCOUNT):
            clears.append((region.getClearActive(i), region.getClearValue(i)))
        return clears

    def setClears(self,region,clears):
        for i in range(GraphicsOutput.RTPCOUNT):
            (active, value) = clears[i]
            region.setClearActive(i, active)
            region.setClearValue(i, value)

    def setStackedClears(self, region, clears0, clears1):
        clears = []
        for i in range(GraphicsOutput.RTPCOUNT):
            (active, value) = clears0[i]
            if (active == 0):
                (active, value) = clears1[i]
            region.setClearActive(i, active)
            region.setClearValue(i, value)
        return clears

    def isFullscreen(self):
        return ((self.region.getLeft()   == 0.0) and
                (self.region.getRight()  == 1.0) and
                (self.region.getBottom() == 0.0) and
                (self.region.getTop()    == 1.0))

    def getScaledSize(self, mul, div, align):

        """ Calculate the size of the desired window. Not public. """

        winx = self.forcex
        winy = self.forcey
        if winx == 0: winx = self.win.getXSize()
        if winy == 0: winy = self.win.getYSize()

        if div != 1:
            winx = ((winx+align-1) // align) * align
            winy = ((winy+align-1) // align) * align
            winx = winx // div
            winy = winy // div

        if mul != 1:
            winx = winx * mul
            winy = winy * mul

        return winx,winy

    def renderSceneInto(self, depthtex=None, colortex=None, auxtex=None, auxbits=0, textures=None, fbprops=None, clamping=None):

        """ Causes the scene to be rendered into the supplied textures
        instead of into the original window.  Puts a fullscreen quad
        into the original window to show the render-to-texture results.
        Returns the quad.  Normally, the caller would then apply a
        shader to the quad.

        To elaborate on how this all works:

        * An offscreen buffer is created.  It is set up to mimic
          the original display region - it is the same size,
          uses the same clear colors, and contains a DisplayRegion
          that uses the original camera.

        * A fullscreen quad and an orthographic camera to render
          that quad are both created.  The original camera is
          removed from the original window, and in its place, the
          orthographic quad-camera is installed.

        * The fullscreen quad is textured with the data from the
          offscreen buffer.  A shader is applied that tints the
          results pink.

        * Automatic shader generation NOT enabled.
          If you have a filter that depends on a render target from
          the auto-shader, you either need to set an auto-shader
          attrib on the main camera or scene, or, you need to provide
          these outputs in your own shader.

        * All clears are disabled on the original display region.
          If the display region fills the whole window, then clears
          are disabled on the original window as well.  It is
          assumed that rendering the full-screen quad eliminates
          the need to do clears.

        Hence, the original window which used to contain the actual
        scene, now contains a pink-tinted quad with a texture of the
        scene.  It is assumed that the user will replace the shader
        on the quad with a more interesting filter. """

        if (textures):
            colortex = textures.get("color", None)
            depthtex = textures.get("depth", None)
            auxtex = textures.get("aux", None)
            auxtex0 = textures.get("aux0", auxtex)
            auxtex1 = textures.get("aux1", None)
        else:
            auxtex0 = auxtex
            auxtex1 = None

        if (colortex == None):
            colortex = Texture("filter-base-color")
            colortex.setWrapU(Texture.WMClamp)
            colortex.setWrapV(Texture.WMClamp)

        texgroup = (depthtex, colortex, auxtex0, auxtex1)

        # Choose the size of the offscreen buffer.

        (winx, winy) = self.getScaledSize(1,1,1)
        if fbprops is not None:
            buffer = self.createBuffer("filter-base", winx, winy, texgroup, fbprops=fbprops)
        else:
            buffer = self.createBuffer("filter-base", winx, winy, texgroup)

        if (buffer == None):
            return None

        cm = CardMaker("filter-base-quad")
        cm.setFrameFullscreenQuad()
        quad = NodePath(cm.generate())
        quad.setDepthTest(0)
        quad.setDepthWrite(0)
        quad.setTexture(colortex)
        quad.setColor(1, 0.5, 0.5, 1)

        cs = NodePath("dummy")
        cs.setState(self.camstate)
        # Do we really need to turn on the Shader Generator?
        #cs.setShaderAuto()
        if (auxbits):
            cs.setAttrib(AuxBitplaneAttrib.make(auxbits))
        if clamping is False:
            # Disables clamping in the shader generator.
            cs.setAttrib(LightRampAttrib.make_identity())
        self.camera.node().setInitialState(cs.getState())

        quadcamnode = Camera("filter-quad-cam")
        lens = OrthographicLens()
        lens.setFilmSize(2, 2)
        lens.setFilmOffset(0, 0)
        lens.setNearFar(-1000, 1000)
        quadcamnode.setLens(lens)
        quadcam = quad.attachNewNode(quadcamnode)

        self.region.setCamera(quadcam)

        self.setStackedClears(buffer, self.rclears, self.wclears)
        if (auxtex0):
            buffer.setClearActive(GraphicsOutput.RTPAuxRgba0, 1)
            buffer.setClearValue(GraphicsOutput.RTPAuxRgba0, (0.5, 0.5, 1.0, 0.0))
        if (auxtex1):
            buffer.setClearActive(GraphicsOutput.RTPAuxRgba1, 1)
        self.region.disableClears()
        if (self.isFullscreen()):
            self.win.disableClears()

        dr = buffer.makeDisplayRegion()
        dr.disableClears()
        dr.setCamera(self.camera)
        dr.setActive(1)

        self.buffers.append(buffer)
        self.sizes.append((1, 1, 1))

        return quad

    def renderQuadInto(self, name="filter-stage", mul=1, div=1, align=1, depthtex=None, colortex=None, auxtex0=None, auxtex1=None, fbprops=None):

        """ Creates an offscreen buffer for an intermediate
        computation. Installs a quad into the buffer.  Returns
        the fullscreen quad.  The size of the buffer is initially
        equal to the size of the main window.  The parameters 'mul',
        'div', and 'align' can be used to adjust that size. """

        texgroup = (depthtex, colortex, auxtex0, auxtex1)

        winx, winy = self.getScaledSize(mul, div, align)

        depthbits = bool(depthtex != None)

        if fbprops is not None:
            buffer = self.createBuffer(name, winx, winy, texgroup, depthbits, fbprops=fbprops)
        else:
            buffer = self.createBuffer(name, winx, winy, texgroup, depthbits)

        if (buffer == None):
            return None

        cm = CardMaker("filter-stage-quad")
        cm.setFrameFullscreenQuad()
        quad = NodePath(cm.generate())
        quad.setDepthTest(0)
        quad.setDepthWrite(0)
        quad.setColor(1, 0.5, 0.5, 1)

        quadcamnode = Camera("filter-quad-cam")
        lens = OrthographicLens()
        lens.setFilmSize(2, 2)
        lens.setFilmOffset(0, 0)
        lens.setNearFar(-1000, 1000)
        quadcamnode.setLens(lens)
        quadcam = quad.attachNewNode(quadcamnode)

        dr = buffer.makeDisplayRegion((0, 1, 0, 1))
        dr.disableClears()
        dr.setCamera(quadcam)
        dr.setActive(True)
        dr.setScissorEnabled(False)

        # This clear stage is important if the buffer is padded, so that
        # any pixels accidentally sampled in the padded region won't
        # be reading from unititialised memory.
        buffer.setClearColor((0, 0, 0, 1))
        buffer.setClearColorActive(True)

        self.buffers.append(buffer)
        self.sizes.append((mul, div, align))

        return quad

    def createBuffer(self, name, xsize, ysize, texgroup, depthbits=1, fbprops=None):
        """ Low-level buffer creation.  Not intended for public use. """

        winprops = WindowProperties()
        winprops.setSize(xsize, ysize)
        props = FrameBufferProperties(FrameBufferProperties.getDefault())
        props.setBackBuffers(0)
        props.setRgbColor(1)
        props.setDepthBits(depthbits)
        props.setStereo(self.win.isStereo())
        if fbprops is not None:
            props.addProperties(fbprops)

        depthtex, colortex, auxtex0, auxtex1 = texgroup
        if (auxtex0 != None):
            props.setAuxRgba(1)
        if (auxtex1 != None):
            props.setAuxRgba(2)
        buffer=base.graphicsEngine.makeOutput(
            self.win.getPipe(), name, -1,
            props, winprops, GraphicsPipe.BFRefuseWindow | GraphicsPipe.BFResizeable,
            self.win.getGsg(), self.win)
        if (buffer == None):
            return buffer
        if (depthtex):
            buffer.addRenderTexture(depthtex, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPDepth)
        if (colortex):
            buffer.addRenderTexture(colortex, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPColor)
        if (auxtex0):
            buffer.addRenderTexture(auxtex0, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPAuxRgba0)
        if (auxtex1):
            buffer.addRenderTexture(auxtex1, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPAuxRgba1)
        buffer.setSort(self.nextsort)
        buffer.disableClears()
        self.nextsort += 1
        return buffer

    def windowEvent(self, win):
        """ When the window changes size, automatically resize all buffers """
        self.resizeBuffers()

    def resizeBuffers(self):
        """ Resize all buffers to match the size of the window. """
        for i in range(len(self.buffers)):
            (mul, div, align) = self.sizes[i]
            (xsize, ysize) = self.getScaledSize(mul, div, align)
            self.buffers[i].setSize(xsize, ysize)

    def cleanup(self):
        """ Restore everything to its original state, deleting any
        new buffers in the process. """

        for buffer in self.buffers:
            buffer.clearRenderTextures()
            self.engine.removeWindow(buffer)
        self.buffers = []
        self.sizes = []
        self.setClears(self.win, self.wclears)
        self.setClears(self.region, self.rclears)
        self.camstate = self.caminit
        self.camera.node().setInitialState(self.caminit)
        self.region.setCamera(self.camera)
        self.nextsort = self.win.getSort() - 1000
        self.basex = 0
        self.basey = 0

    #snake_case alias:
    is_fullscreen = isFullscreen
    resize_buffers = resizeBuffers
    set_stacked_clears = setStackedClears
    render_scene_into = renderSceneInto
    get_scaled_size = getScaledSize
    render_quad_into = renderQuadInto
    get_clears = getClears
    set_clears = setClears
    create_buffer = createBuffer
    window_event = windowEvent
