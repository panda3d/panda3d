"""Undocumented Module"""

__all__ = ['setupMirror', 'showFrustum']

"""This file demonstrates one way to create a mirror effect in Panda.
Call setupMirror() to create a mirror in the world that reflects
everything in front of it.

The approach taken here is to create an offscreen buffer with its own
camera that renders its view into a texture, which is then applied to
the mirror geometry.  The mirror's camera is repositioned each frame
with a task to keep it always on the opposite side of the mirror from
the main camera.

This demonstrates the basic interface for offscreen
render-to-a-texture in Panda.  Similar approaches can be used for
related effects, such as a remote spy camera presenting its view onto
a closed-circuit television screen.

In this example the mirror itself is always perfectly flat--it's just
a single polygon, after all--but small distortions of the mirror
surface are possible, like a funhouse mirror.  However, the reflection
itself is always basically planar; for more accurate convex
reflections, you will need to use a sphere map or a cube map."""

from pandac.PandaModules import *
from direct.task import Task

def setupMirror(name, width, height):
    # The return value is a NodePath that contains a rectangle that
    # reflects render.  You can reparent, reposition, and rotate it
    # anywhere you like.

    root = render.attachNewNode(name)

    # Create a polygon to be the visible representation of the mirror.
    cm = CardMaker('mirror')
    cm.setFrame(width / 2.0, -width / 2.0, -height / 2.0, height / 2.0)
    cm.setHasUvs(1)
    card = root.attachNewNode(cm.generate())
    
    # Create a PlaneNode to represent the mirror's position, for
    # computing where the mirror's camera belongs each frame.
    plane = Plane(Vec3(0, 1, 0), Point3(0, 0, 0))
    planeNode = PlaneNode('mirrorPlane')
    planeNode.setPlane(plane)
    planeNP = root.attachNewNode(planeNode)

    # Now create an offscreen buffer for rendering the mirror's point
    # of view.  The parameters here control the resolution of the
    # texture.
    buffer = base.win.makeTextureBuffer(name, 256, 256)
    #buffer.setClearColor(base.win.getClearColor())
    buffer.setClearColor(VBase4(0, 0, 1, 1))

    # Set up a display region on this buffer, and create a camera.
    dr = buffer.makeDisplayRegion()
    camera = Camera('mirrorCamera')
    lens = PerspectiveLens()
    lens.setFilmSize(width, height)
    camera.setLens(lens)
    cameraNP = planeNP.attachNewNode(camera)
    dr.setCamera(cameraNP)

    # Since the reflection matrix will reverse the vertex-winding
    # order of all the polygons in the world, we have to tell the
    # camera to reverse the direction of its face culling.  We also
    # tell it not to draw (that is, to clip) anything behind the
    # mirror plane.
    camera.setInitialState(RenderState.make(
        CullFaceAttrib.makeReverse(),
        ClipPlaneAttrib.make(ClipPlaneAttrib.OAdd, planeNode)))

    # Create a visible representation of the camera so we can see it.
    #cameraVis = loader.loadModel('camera.egg')
    #if not cameraVis.isEmpty():
    #    cameraVis.reparentTo(cameraNP)

    # Spawn a task to keep that camera on the opposite side of the
    # mirror.
    def moveCamera(task, cameraNP = cameraNP, plane = plane,
                   planeNP = planeNP, card = card, lens = lens,
                   width = width, height = height):
        # Set the camera to the mirror-image position of the main camera.
        cameraNP.setMat(base.camera.getMat(planeNP) * plane.getReflectionMat())

        # And reset the frustum to exactly frame the mirror's corners.
        # This is a minor detail, but it helps to provide a realistic
        # reflection and keep the subject centered.
        ul = cameraNP.getRelativePoint(card, Point3(-width / 2.0, 0, height / 2.0))
        ur = cameraNP.getRelativePoint(card, Point3(width / 2.0, 0, height / 2.0))
        ll = cameraNP.getRelativePoint(card, Point3(-width / 2.0, 0, -height / 2.0))
        lr = cameraNP.getRelativePoint(card, Point3(width / 2.0, 0, -height / 2.0))
        lens.setFrustumFromCorners(ul, ur, ll, lr, Lens.FCCameraPlane | Lens.FCOffAxis | Lens.FCAspectRatio)
        
        return Task.cont

    # Add it with a fairly high priority to make it happen late in the
    # frame, after the avatar controls (or whatever) have been applied
    # but before we render.
    taskMgr.add(moveCamera, name, priority = 40)

    # Now apply the output of this camera as a texture on the mirror's
    # visible representation.
    card.setTexture(buffer.getTexture())

    return root
    
def showFrustum(np):
    # Utility function to reveal the frustum for a particular camera.
    cameraNP = np.find('**/+Camera')
    camera = cameraNP.node()
    lens = camera.getLens()
    geomNode = GeomNode('frustum')
    geomNode.addGeom(lens.makeGeometry())
    cameraNP.attachNewNode(geomNode)
    
