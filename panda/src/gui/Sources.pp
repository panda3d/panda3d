#define OTHER_LIBS dtool

#begin lib_target
  #define TARGET gui

  #define LOCAL_LIBS \
    putil display tform device pandabase dgraph sgattrib light gobj text

  #define SOURCES \
    config_gui.h config_gui.cxx \
    guiManager.h guiManager.I guiManager.cxx \
    guiRegion.h guiRegion.I guiRegion.cxx \
    guiLabel.h guiLabel.I guiLabel.cxx \
    guiRollover.h guiRollover.I guiRollover.cxx \
    guiButton.h guiButton.I guiButton.cxx

  #define INSTALL_HEADERS \
    guiManager.h guiManager.I \
    guiRegion.h guiRegion.I \
    guiLabel.h guiLabel.I \
    guiRollover.h guiRollover.I \
    guiButton.h guiButton.I

  #define IGATESCAN \
    guiManager.h guiManager.I \
    guiRegion.h guiRegion.I \
    guiLabel.h guiLabel.I \
    guiRollover.h guiRollover.I \
    guiButton.h guiButton.I

#end lib_target
