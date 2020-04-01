if(NOT BUILD_DTOOL)
  message(FATAL_ERROR "Cannot build panda without dtool!  Please enable the BUILD_DTOOL option.")
endif()

# Include panda source directories
add_subdirectory(src/audio)
add_subdirectory(src/audiotraits)
add_subdirectory(src/chan)
add_subdirectory(src/char)
add_subdirectory(src/cocoadisplay)
add_subdirectory(src/collide)
add_subdirectory(src/configfiles)
add_subdirectory(src/cull)
add_subdirectory(src/device)
add_subdirectory(src/dgraph)
add_subdirectory(src/display)
add_subdirectory(src/distort)
add_subdirectory(src/downloader)
add_subdirectory(src/downloadertools)
add_subdirectory(src/dxgsg9)
add_subdirectory(src/dxml)
add_subdirectory(src/egg)
add_subdirectory(src/egg2pg)
add_subdirectory(src/egldisplay)
add_subdirectory(src/event)
add_subdirectory(src/express)
add_subdirectory(src/ffmpeg)
add_subdirectory(src/framework)
add_subdirectory(src/glesgsg)
add_subdirectory(src/gles2gsg)
add_subdirectory(src/glgsg)
add_subdirectory(src/glstuff)
add_subdirectory(src/glxdisplay)
add_subdirectory(src/gobj)
add_subdirectory(src/grutil)
add_subdirectory(src/gsgbase)
add_subdirectory(src/linmath)
add_subdirectory(src/mathutil)
add_subdirectory(src/movies)
add_subdirectory(src/nativenet)
add_subdirectory(src/net)
add_subdirectory(src/pandabase)
add_subdirectory(src/parametrics)
add_subdirectory(src/pgraph)
add_subdirectory(src/pgraphnodes)
add_subdirectory(src/pgui)
add_subdirectory(src/pipeline)
add_subdirectory(src/pnmimage)
add_subdirectory(src/pnmimagetypes)
add_subdirectory(src/pnmtext)
add_subdirectory(src/pstatclient)
add_subdirectory(src/putil)
add_subdirectory(src/recorder)
add_subdirectory(src/testbed)
add_subdirectory(src/text)
add_subdirectory(src/tform)
add_subdirectory(src/vision)
add_subdirectory(src/vrpn)
add_subdirectory(src/wgldisplay)
add_subdirectory(src/windisplay)
add_subdirectory(src/x11display)

# For other components
# bullet
add_subdirectory(src/bullet)
# ode
add_subdirectory(src/ode)
# physics
add_subdirectory(src/particlesystem)
add_subdirectory(src/physics)

# Include panda metalibs
add_subdirectory(metalibs/panda)
add_subdirectory(metalibs/pandadx9)
add_subdirectory(metalibs/pandaegg)
add_subdirectory(metalibs/pandaexpress)
add_subdirectory(metalibs/pandagl)
add_subdirectory(metalibs/pandagles)
add_subdirectory(metalibs/pandagles2)
add_subdirectory(metalibs/pandaphysics)

# Now add the Python modules:
set(CORE_MODULE_COMPONENTS
  p3chan p3char p3collide p3cull p3device p3dgraph p3display
  p3downloader p3dxml p3event p3express p3gobj p3grutil p3gsgbase p3linmath
  p3mathutil p3movies p3parametrics p3pgraph p3pgraphnodes p3pgui
  p3pipeline p3pnmimage p3pstatclient p3putil p3recorder p3text p3tform
  p3prc p3dtoolutil p3dtoolbase
)

if(WANT_NATIVE_NET)
  list(APPEND CORE_MODULE_COMPONENTS p3nativenet)
  if(HAVE_NET)
    list(APPEND CORE_MODULE_COMPONENTS p3net)
  endif()
endif()

if(HAVE_AUDIO)
  list(APPEND CORE_MODULE_COMPONENTS p3audio)
endif()

if(HAVE_FREETYPE)
  list(APPEND CORE_MODULE_COMPONENTS p3pnmtext)
endif()

if(INTERROGATE_PYTHON_INTERFACE)
  add_python_module(panda3d.core ${CORE_MODULE_COMPONENTS} LINK panda)

  # Generate our __init__.py
  if(WIN32)
    file(READ "${PROJECT_SOURCE_DIR}/cmake/templates/win32_python/__init__.py" win32_init)
  else()
    set(win32_init "")
  endif()

  set(init_file "\"Python bindings for the Panda3D libraries\"

__version__ = '${PROJECT_VERSION}'

if __debug__:
    import sys
    if sys.version_info < (3, 0):
        sys.stderr.write('''\\
WARNING: Python 2.7 has reached EOL as of January 1, 2020.
To suppress this warning, upgrade to Python 3.
''')
        sys.stdout.flush()
    del sys

${win32_init}")

  set(dtoolconfig_file "'''Alias of :mod:`panda3d.interrogatedb`.

.. deprecated:: 1.10.0
   This module has been renamed to :mod:`panda3d.interrogatedb`.
'''

if __debug__:
    print(\"Warning: panda3d.dtoolconfig is deprecated, use panda3d.interrogatedb instead.\")
from .interrogatedb import *
")

  if(IS_MULTICONFIG)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      # Generate an __init__.py in the per-configuration directory.
      file(WRITE "${PROJECT_BINARY_DIR}/${config}/panda3d/__init__.py" ${init_file})
      file(WRITE "${PROJECT_BINARY_DIR}/${config}/panda3d/dtoolconfig.py" ${dtoolconfig_file})
    endforeach(config)
  else()
    file(WRITE "${PROJECT_BINARY_DIR}/panda3d/__init__.py" ${init_file})
    file(WRITE "${PROJECT_BINARY_DIR}/panda3d/dtoolconfig.py" ${dtoolconfig_file})
  endif()

  if(HAVE_BULLET)
    add_python_module(panda3d.bullet p3bullet IMPORT panda3d.core COMPONENT BulletPython)

    export_targets(BulletPython NAMESPACE "Panda3D::Python::" COMPONENT BulletDevel)
  endif()

  if(HAVE_EGG)
    add_python_module(panda3d.egg p3egg p3egg2pg LINK pandaegg IMPORT panda3d.core COMPONENT EggPython)

    export_targets(EggPython NAMESPACE "Panda3D::Python::" COMPONENT EggDevel)
  endif()

  add_python_module(panda3d.fx pandafx IMPORT panda3d.core)
  add_python_module(panda3d.physics p3physics p3particlesystem LINK pandaphysics IMPORT panda3d.core)

  if(HAVE_ODE)
    add_python_module(panda3d.ode p3ode IMPORT panda3d.core COMPONENT ODEPython)

    export_targets(ODEPython NAMESPACE "Panda3D::Python::" COMPONENT ODEDevel)
  endif()

  if(HAVE_OPENCV OR HAVE_ARTOOLKIT)
    add_python_module(panda3d.vision p3vision IMPORT panda3d.core COMPONENT VisionPython)

    export_targets(VisionPython NAMESPACE "Panda3D::Python::" COMPONENT VisionDevel)
  endif()

  if(HAVE_VRPN)
    add_python_module(panda3d.vrpn p3vrpn IMPORT panda3d.core COMPONENT VRPNPython)
    set_target_properties(panda3d.vrpn PROPERTIES CXX_EXCEPTIONS ON)

    export_targets(VRPNPython NAMESPACE "Panda3D::Python::" COMPONENT VRPNDevel)
  endif()

  export_targets(Python NAMESPACE "Panda3D::Python::" COMPONENT CoreDevel)
endif()

export_targets(Core COMPONENT CoreDevel)
