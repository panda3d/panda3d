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

configure_file(panda.prc.in "${CMAKE_BINARY_DIR}/etc/20_panda.prc")
if(IS_MULTICONFIG)
  file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/$<CONFIG>/etc/20_panda.prc"
                INPUT "${PROJECT_BINARY_DIR}/etc/20_panda.prc")
endif()

if(WIN32)
  set(_confdir "etc")

else()
  # On Unices, ask the GNUInstallDirs module where /etc is
  set(_confdir "${CMAKE_INSTALL_FULL_SYSCONFDIR}")

endif()

install(FILES "${CMAKE_BINARY_DIR}/etc/20_panda.prc" COMPONENT Core DESTINATION ${_confdir})
