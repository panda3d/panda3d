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
add_subdirectory(src/osxdisplay)
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
    p3chan p3char p3collide p3cull p3device p3dgraph p3display p3downloader p3dxml
    p3event p3express p3gobj p3grutil p3gsgbase p3linmath p3mathutil p3nativenet
    p3net p3movies p3parametrics p3pgraph p3pgraphnodes p3pgui p3pipeline
    p3pnmimage p3pstatclient p3putil p3recorder p3text p3tform
    p3prc p3dtoolutil p3dtoolbase)

if(HAVE_AUDIO)
  list(APPEND CORE_MODULE_COMPONENTS p3audio)
endif()
if(HAVE_FREETYPE)
  list(APPEND CORE_MODULE_COMPONENTS p3pnmtext)
endif()

if(HAVE_PYTHON)
  add_python_module(core ${CORE_MODULE_COMPONENTS} LINK panda)

  if(HAVE_BULLET)
    add_python_module(bullet p3bullet IMPORT panda3d.core COMPONENT BulletPython)
  endif()

  if(HAVE_EGG)
    add_python_module(egg p3egg p3egg2pg LINK pandaegg IMPORT panda3d.core COMPONENT EggPython)
  endif()

  add_python_module(physics p3physics p3particlesystem LINK pandaphysics IMPORT panda3d.core)

  if(HAVE_ODE)
    add_python_module(ode p3ode IMPORT panda3d.core COMPONENT ODEPython)
  endif()

  if(HAVE_OPENCV OR HAVE_ARTOOLKIT)
    add_python_module(vision p3vision IMPORT panda3d.core COMPONENT VisionPython)
  endif()

  if(HAVE_VRPN)
    add_python_module(vrpn p3vrpn IMPORT panda3d.core COMPONENT VRPNPython)
  endif()
endif()
