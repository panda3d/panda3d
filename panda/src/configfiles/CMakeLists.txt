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
  set(_confdir "${CMAKE_INSTALL_FULL_SYSCONFDIR}/panda3d")
  set(_out_path "etc/panda3d")
  set(ETC_PARENT_PATH "../..")
else()
  set(_confdir "etc")
  set(_out_path "etc")
  set(ETC_PARENT_PATH "..")
endif()

if(IS_MULTICONFIG)
  set(_out_path "${PROJECT_BINARY_DIR}/$<CONFIG>/${_out_path}")
else()
  set(_out_path "${PROJECT_BINARY_DIR}/${_out_path}")
endif()

# Path from the directory containing *.prc to the *parent of* models/
# In the build tree, reckoning this is pretty easy
set(MODELS_PARENT_PATH "..")
configure_file(panda.prc.in "${CMAKE_CURRENT_BINARY_DIR}/20_panda.prc")

# For the install tree, we need to introspect our paths
if(_confdir MATCHES "^/")
  set(_abs_confdir "${_confdir}")
else()
  set(_abs_confdir "${CMAKE_INSTALL_PREFIX}/${_confdir}")
endif()
if(CMAKE_INSTALL_DATADIR MATCHES "^/")
  set(_abs_datadir "${CMAKE_INSTALL_DATADIR}")
else()
  set(_abs_datadir "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}")
endif()
file(RELATIVE_PATH MODELS_PARENT_PATH
  "${_abs_confdir}" "${_abs_datadir}/panda3d")
configure_file(panda.prc.in "${CMAKE_CURRENT_BINARY_DIR}/20_panda.prc.install")

file(GENERATE OUTPUT "${_out_path}/20_panda.prc"
              INPUT "${CMAKE_CURRENT_BINARY_DIR}/20_panda.prc")
file(GENERATE OUTPUT "${_out_path}/20_panda.prc.install"
              INPUT "${CMAKE_CURRENT_BINARY_DIR}/20_panda.prc.install")

install(FILES "${_out_path}/20_panda.prc.install" RENAME "20_panda.prc"
  COMPONENT Core DESTINATION ${_confdir})
