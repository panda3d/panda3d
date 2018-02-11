p3cull: Panda3D's cull traverser
================================

Overview
--------

### What is "culling"?

In the 3D graphics world, "culling" refers to the process of determining
objects that are hidden from view or are otherwise guaranteed not to contribute
meaningfully to the scene. These objects are then skipped in the rendering step
to save on valuable rendering time.

Panda3D uses the term "cull" in a slightly different sense. In particular, it
refers to the whole process of preparing a complex, non-linear and arbitrarily
deep scenegraph for rendering by the graphics backend, which needs a list of
objects to render one at a time. This involves, roughly:

1. Determining all *visible* objects, given a viewpoint, a scenegraph, and a
   bitmask indicating which objects should be shown.
2. *Composing* (combining) the state of each object, as inherited from its
   ancestors in the scenegraph.
3. Deciding the order in which the objects are to be sent for rendering.

Note that this processs should not be confused with "back-face culling," which
is where triangles that face away from the screen are assumed to be on the
opposite side of of an object and therefore not visible. Back-face culling is
performed at render-time and therefore does not reduce the total number of
objects being sent to the backend.

### Determining visible objects

Panda3D's primary culling mode is called "viewing frustum culling," where a
given viewpoint (Camera and Lens, in Panda3D terms) is represented as a
bounding volume, typically a frustum - a chopped-off pyramid. As the theory
goes, whenever a volume represents completely and precisely the visible portion
of a space, any object that does not intersect with the volume must not be
visible and can therefore be ignored during that frame.

Every object also has a bounding volume, which is a geometric approximation of
its actual shape. This approximation is computed to be as small as possible,
while staying computationally simple and guaranteeing that the volume
encompasses the entire object and its children. In other words, it's better to
err on the side of being too large.

When Panda3D performs cull traversals, it walks the scenegraph in a depth-first
search order, visiting each node, and testing for intersections between the
view volume and the node's bounding volume. These intersections usually fall
into one of three categories:
1. No intersection: The node is culled, and none of its children are
   considered.
2. Some: A partial intersection means the node is considered visible, but
   further tests against children are necessary. (This can be overridden by the
   `set_final` flag; see PandaNode.)
3. All: If the node's bounding volume is completely enclosed within the viewing
   volume, then no part of it is *not* visible. All children are assumed to
   pass the intersection test as well.

Panda3D supports the "portal culling" and "occlusion culling" paradigms as well.
In portal culling, visual chokepoints in the scene (e.g. narrow doorways) are
represented by a rectangular "portal" into a different area of the scene. If
the portal is culled, every object on the opposite side of the portal is also
culled. If the portal is not culled, the portal clips the frustum and culling
continues across the portal. In occlusion culling, rectangular "occluders"
(sometimes called "antiportals") are placed in the scene; however, objects
using the occluder are _only_ culled if they are completely enclosed within the
occluder's clipped view frustrum (since the view frustrum, as clipped by the
occluder, represents the region blocked by the occluder -
that is, the "shadow" of the camera - which cannot be seen)

Normally, this would be enough, but Panda3D also offers the option of hiding
nodes, either from every camera or selectively on a camera-by-camera basis
(this is useful in implementing certain graphical effects such as shadows). To
do this, Panda3D gives every node and camera a special bitmask called a
DrawMask, with the rule being that an object can only be seen by a camera if
they share bits in common. If not, the node simply fails the cull test and
isn't sent for rendering.

### Composing state

Each node can have state - either in the form of a transform, and/or rendering
effects/attributes - applied to it. These attributes apply both to the node
and to all children of that node. (See the [pgraph](../pgraph) component for
more details.)

For every object that passes the visibility test, its render state is fully
composed, starting with the initial state set on the camera (see
`Camera::set_initial_state`), then the root of the scenegraph, and then every
ancestor of the object in top-down order, with lower nodes taking precedence
over higher nodes.

The transform state is composed similarly, but it begins with the *inverse* of
the camera's transform.

The result is a series of CullableObjects, which typically represent a Geom
(see [gobj](../gobj)) each. These are the atomic unit of rendering during
Panda3D's render pass, and as such are independent of one another.

These CullableObjects are sent to a CullHandler, which is responsible for
collecting CullableObjects for rendering.

### Deciding rendering order

Before each CullableObject can be submitted for rendering on the graphics
backend, Panda3D needs to determine the best order in which to draw them.
Panda3D typically takes into account:

1. Specific requests from the user. Some graphical effects, for example, may
   require that objects are rendered in a particular order.
2. Specific orderings based on the kind of the object. Transparent objects must
   be rendered in back-to-front order after the rest of the scene for alpha
   blending to work properly, for instance.
3. Grouping objects by similarity of render state to minimize the number of
   times the graphics hardware must change state. Each change of state involves
   some performance cost, sometimes even a full stall of the graphics pipeline,
   and so grouping similar objects together helps keep framerates high.

To do this, Panda3D's BinCullHandler groups all CullableObjects into CullBins,
which are responsible for determining the coarse and fine rendering order of
objects. For more on how CullBins work, see the Panda3D manual page
[How to Control Render Order](https://www.panda3d.org/manual/index.php/How_to_Control_Render_Order).

The BinCullHandler produces a CullResult, which is a series of CullableObjects
(again, typically Geoms) ready to be submitted to the GraphicsStateGuardian for
rendering.

Classes
-------

* **SceneSetup** describes one rendering pass, including the camera, scene,
  lens, DisplayRegion, and GraphicsStateGuardian involved.
* **CullTraverser** takes the parameters from SceneSetup and traverses the
  scene in a depth-first order, searching for visible geometry, which is passed
  to a CullHandler object.
* **CullableObject** is a container for any visible geometry that has passed
  the cull test and its associated (composed) state. Typically, this is a Geom,
  a RenderState, and a TransformState.
* **CullHandler** is an abstract interface for accepting CullableObjects which
  have passed the cull test.
* **DrawCullHandler** is a kind of CullHandler that immediately passes the
  objects on to the GraphicsStateGuardian. See also
  `GraphicsThreadingModel::set_cull_sorting`
* **BinCullHandler** is a kind of CullHandler that "bins" the
  CullableObjects - sorting them into an optimal rendering order.
* **CullBin** is a base class for any class that sorts CullableObjects in some
  order.
* **CullBinStateSorted** is a CullBin that sorts the CullableObjects in such a
  way as to minimize state transitions, which are expensive on pipelined
  graphics hardware.
* **CullBinFixed** is a CullBin that sorts the CullableObjects in an
  application-specified order, specified in the CullBinAttrib.
* **CullBinBackToFront** is a CullBin that sorts the CullableObjects in
  back-to-front order, which is useful for transparent operation.
* **CullBinFrontToBack** is a CullBin that sorts front-to-back.
* **CullBinUnsorted** is a CullBin that doesn't try to sort geometry in any
  meaningful way.
* **CullBinManager** is a global object that maintains a list of the named
  CullBins in the application, and keeps track of their global ordering.
* **CullBinAttrib** is a RenderAttrib that allows the application to specify a
  CullBin for its geometry to be categorized into.

Configuration
-------------

* `cull-bin bin_name sort type` defines a new CullBin by name.

Build-time flags
----------------

None affect this component.
