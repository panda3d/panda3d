# Author: Epihaius
# Date: 2019-06-21
#
# This package contains classes and functions to procedurally create 3D primitives
# such as boxes, spheres and cylinders.

from .box import BoxMaker, build_box
from .cylinder import CylinderMaker, build_cylinder
from .cone import ConeMaker, build_cone
from .sphere import SphereMaker, build_sphere
from .torus import TorusMaker, build_torus
from .base import disintegrate_model
