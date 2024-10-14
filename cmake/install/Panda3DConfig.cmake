# PANDA 3D SOFTWARE
# Copyright (c) Carnegie Mellon University.  All rights reserved.
#
# All use of this software is subject to the terms of the revised BSD
# license.  You should have received a copy of this license along
# with this source code in a file named "LICENSE."
#
# Author: CFSworks (Dec. 11, 2018)

# This file is installed to CMake's package search path, and is invoked for
# find_package(Panda3D [COMPONENTS ...])
#
# The following components are available for importing:
#
#   Core      - The core Panda3D libraries; this component is always included.
#
#               Panda3D::Core::panda
#               Panda3D::Core::pandaexpress
#               etc.
#
#
#   Python    - Python targets, which can be used for linking against the Python
#               extension modules directly.  Note that this also imports the
#               Python bindings for other requested components that have them.
#
#               Panda3D::Python::panda3d.core
#               Panda3D::Python::panda3d.physics
#               etc.
#
#
#   Tools     - Various tools used in asset manipulation and debugging.
#
#               Panda3D::Tools::egg2bam
#               Panda3D::Tools::egg-optchar
#               Panda3D::Tools::pview
#               etc.
#
#
#   Direct    - Panda's "direct" Python framework; C++ support library.
#
#               Panda3D::Direct::p3direct
#
#
#   Contrib   - Extensions not part of the Panda3D core, but contributed by the
#               community.
#
#               Panda3D::Contrib::p3ai
#               Panda3D::Contrib::p3rplight
#
#
#   Framework - Panda's "p3framework" C++ framework.
#
#               Panda3D::Framework::p3framework
#
#
#   Egg       - Support for the Egg file format.
#
#               Panda3D::Egg::pandaegg
#
#
#   Bullet    - Support for Bullet physics.
#
#               Panda3D::Bullet::p3bullet
#
#
#   ODE       - Support for the ODE physics engine.
#
#               Panda3D::ODE::p3ode
#
#
#   FFmpeg    - Support for FFmpeg media format loading.
#
#               Panda3D::FFmpeg::p3ffmpeg
#
#
#   OpenAL    - Support for OpenAL audio output.
#
#               Panda3D::OpenAL::p3openal_audio
#
#
#   FMOD      - Support for FMOD audio output.
#
#               Panda3D::FMOD::p3fmod_audio
#
#
#   OpenGL    - Support for OpenGL rendering.
#
#               Panda3D::OpenGL::pandagl
#
#
#   DX9       - Support for Direct3D 9 rendering.
#
#               Panda3D::DX9::pandadx9
#
#
#   OpenGLES1 - Support for OpenGL ES 1.x rendering.
#
#               Panda3D::OpenGLES1::pandagles
#
#
#   OpenGLES2 - Support for OpenGL ES 2.x+ rendering.
#
#               Panda3D::OpenGLES2::pandagles2
#
#
#   TinyDisplay - Support for software rendering.
#
#               Panda3D::TinyDisplay::p3tinydisplay
#
#
#   Vision    - Support for vision processing.
#
#               Panda3D::Vision::p3vision
#
#
#   VRPN      - Support for connecting to a VRPN virtual reality server.
#
#               Panda3D::VRPN::p3vrpn

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 3.0)
  message(FATAL_ERROR "CMake >= 3.0.2 required")
endif()

get_filename_component(_panda_config_prefix "${CMAKE_CURRENT_LIST_FILE}" PATH)

include("${_panda_config_prefix}/Panda3DPackages.cmake")

set(_panda_components
  Core Python Tools
  Direct Contrib Framework Egg
  Bullet ODE
  FFmpeg
  OpenAL FMOD
  OpenGL DX9 OpenGLES1 OpenGLES2 TinyDisplay
  Vision VRPN
)

set(Panda3D_FIND_REQUIRED_Core ON)

foreach(_comp Core ${Panda3D_FIND_COMPONENTS})
  if(";${_panda_components};" MATCHES ";${_comp};" AND
     EXISTS "${_panda_config_prefix}/Panda3D${_comp}Targets.cmake")

    include("${_panda_config_prefix}/Panda3D${_comp}Targets.cmake")

    if(";${Panda3D_FIND_COMPONENTS};" MATCHES ";Python;" AND
       EXISTS "${_panda_config_prefix}/Panda3D${_comp}PythonTargets.cmake")

      include("${_panda_config_prefix}/Panda3D${_comp}PythonTargets.cmake")

    endif()

  elseif(Panda3D_FIND_REQUIRED_${_comp})

    message(FATAL_ERROR "Panda3D REQUIRED component ${_comp} not found")

  endif()

endforeach(_comp)
unset(_comp)

unset(_panda_components)
