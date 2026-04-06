"""
This package contains functionality for applying post-processing
filters to the result of rendering a 3-D scene.  This is done by
rendering the scene to an off-screen buffer, and then applying this to
a full-screen card that has a shader applied which manipulates the
texture values as desired.

The :class:`.CommonFilters` class contains various filters that are
provided out of the box, whereas the :class:`.FilterManager` class
is a lower-level class that allows you to set up your own filters.

See the :ref:`render-to-texture-and-image-postprocessing` section of the
Programming Guide to learn more about image postprocessing in Panda3D.
"""
