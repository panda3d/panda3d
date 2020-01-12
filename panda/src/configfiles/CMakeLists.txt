# Set some variables that are used in panda.prc.in.
set(AUX_DISPLAYS)

if(HAVE_GL)
  set(AUX_DISPLAYS "${AUX_DISPLAYS}\naux-display pandagl")
endif()

if(HAVE_DX9)
  set(AUX_DISPLAYS "${AUX_DISPLAYS}\naux-display pandadx9")
endif()

if(HAVE_TINYDISPLAY)
  set(AUX_DISPLAYS "${AUX_DISPLAYS}\naux-display p3tinydisplay")
endif()

if(HAVE_OPENAL)
  set(AUDIO_LIBRARY_NAME p3openal_audio)
elseif(HAVE_FMODEX)
  set(AUDIO_LIBRARY_NAME p3fmod_audio)
else()
  set(AUDIO_LIBRARY_NAME none)
endif()

if(WIN32)
  set(BAM_CACHE_DIR
    "$USER_APPDATA/Panda3D-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
elseif(APPLE)
  set(BAM_CACHE_DIR
    "$HOME/Library/Caches/Panda3D-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
else()
  set(BAM_CACHE_DIR
    "$XDG_CACHE_HOME/panda3d")
endif()

if(UNIX)
  # On Unices, ask the GNUInstallDirs module where /etc is -- and then install
  # ourselves into /etc/panda3d
  set(prc_install_path "${CMAKE_INSTALL_FULL_SYSCONFDIR}/panda3d")
  set(prc_build_path "${PANDA_OUTPUT_DIR}/etc/panda3d")
else()
  set(prc_install_path "etc")
  set(prc_build_path "${PANDA_OUTPUT_DIR}/etc")
endif()

# Path from the directory containing *.prc to the directory containing plugins
file(RELATIVE_PATH PLUGINS_PATH
  "${prc_build_path}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

# Path from the directory containing *.prc to the *parent of* models/
# In the build tree, reckoning this is pretty easy
file(RELATIVE_PATH MODELS_PARENT_PATH "${prc_build_path}" "${PANDA_OUTPUT_DIR}")
configure_file(panda.prc.in "${CMAKE_CURRENT_BINARY_DIR}/20_panda.prc")

# For the install tree, we need to introspect our paths
set(abs_prc_install_path "${prc_install_path}")
set(abs_datadir "${CMAKE_INSTALL_DATADIR}")
set(abs_plugindir "${MODULE_DESTINATION}")
if(NOT IS_ABSOLUTE "${abs_prc_install_path}")
  set(abs_prc_install_path "${CMAKE_INSTALL_PREFIX}/${abs_prc_install_path}")
endif()
if(NOT IS_ABSOLUTE "${abs_datadir}")
  set(abs_datadir "${CMAKE_INSTALL_PREFIX}/${abs_datadir}")
endif()
if(NOT IS_ABSOLUTE "${abs_plugindir}")
  set(abs_plugindir "${CMAKE_INSTALL_PREFIX}/${abs_plugindir}")
endif()
file(RELATIVE_PATH PLUGINS_PATH
  "${abs_prc_install_path}" "${abs_plugindir}")
file(RELATIVE_PATH MODELS_PARENT_PATH
  "${abs_prc_install_path}" "${abs_datadir}/panda3d")
configure_file(panda.prc.in "${CMAKE_CURRENT_BINARY_DIR}/20_panda.prc.install")

file(GENERATE OUTPUT "${prc_build_path}/20_panda.prc"
              INPUT "${CMAKE_CURRENT_BINARY_DIR}/20_panda.prc")
file(GENERATE OUTPUT "${prc_build_path}/20_panda.prc.install"
              INPUT "${CMAKE_CURRENT_BINARY_DIR}/20_panda.prc.install")

install(FILES "${prc_build_path}/20_panda.prc.install" RENAME "20_panda.prc"
  COMPONENT Core DESTINATION ${prc_install_path})
