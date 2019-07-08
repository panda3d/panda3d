# Set some variables that are used in panda.prc.in.
set(AUX_DISPLAYS)

if(HAVE_GL)
  set(AUX_DISPLAYS "${AUX_DISPLAYS}\naux-display pandagl")
endif()

if(HAVE_DX9)
  set(AUX_DISPLAYS "${AUX_DISPLAYS}\naux-display pandadx9")
endif()

if(HAVE_GLES)
  set(AUX_DISPLAYS "${AUX_DISPLAYS}\naux-display pandagles")
endif()

if(HAVE_GLES2)
  set(AUX_DISPLAYS "${AUX_DISPLAYS}\naux-display pandagles2")
endif()

if(HAVE_TINYDISPLAY)
  set(AUX_DISPLAYS "${AUX_DISPLAYS}\naux-display p3tinydisplay")
endif()

if(HAVE_RAD_MSS)
  set(AUDIO_LIBRARY_NAME p3miles_audio)
elseif(HAVE_OPENAL)
  set(AUDIO_LIBRARY_NAME p3openal_audio)
elseif(HAVE_FMODEX)
  set(AUDIO_LIBRARY_NAME p3fmod_audio)
else()
  set(AUDIO_LIBRARY_NAME none)
endif()

# We'll want the default window size to be whatever the size of the containing
# view is.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(WIN_SIZE "")
else()
  set(WIN_SIZE "# Let's set up a default window size of 800x600.\n  The user can
# override this in his or her personal prc file.\nwin-origin -2 -2\nwin-size 800 600\n")
endif()

configure_file(panda.prc.in "${CMAKE_BINARY_DIR}/etc/20_panda.prc")
if(IS_MULTICONFIG)
  file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/$<CONFIG>/etc/20_panda.prc"
                INPUT "${PROJECT_BINARY_DIR}/etc/20_panda.prc")
endif()

install(FILES "${CMAKE_BINARY_DIR}/${PANDA_CFG_INTDIR_GEN}/etc/20_panda.prc" COMPONENT Core DESTINATION etc)
