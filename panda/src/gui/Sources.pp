#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#define NOT_INTEL_BUILDABLE true				   
$[CheckCompilerCompatibility]

#begin lib_target
  #define TARGET gui

  #define LOCAL_LIBS \
    putil display tform device pandabase dgraph sgattrib light gobj text

  #define SOURCES \
    config_gui.h config_gui.cxx \
    guiManager.h guiManager.I guiManager.cxx \
    guiRegion.h guiRegion.I guiRegion.cxx \
    guiLabel.h guiLabel.I guiLabel.cxx \
    guiItem.h guiItem.I guiItem.cxx \
    guiRollover.h guiRollover.I guiRollover.cxx \
    guiButton.h guiButton.I guiButton.cxx \
    guiFrame.h guiFrame.I guiFrame.cxx \
    guiSign.h guiSign.I guiSign.cxx \
    guiListBox.h guiListBox.I guiListBox.cxx \
    guiBackground.h guiBackground.I guiBackground.cxx \
    guiBehavior.h guiBehavior.I guiBehavior.cxx \
    guiChooser.h guiChooser.I guiChooser.cxx \
    guiCollection.h guiCollection.I guiCollection.cxx

  #define INSTALL_HEADERS \
    guiManager.h guiManager.I \
    guiRegion.h guiRegion.I \
    guiLabel.h guiLabel.I \
    guiItem.h guiItem.I \
    guiRollover.h guiRollover.I \
    guiButton.h guiButton.I \
    guiFrame.h guiFrame.I \
    guiSign.h guiSign.I \
    guiListBox.h guiListBox.I \
    guiBackground.h guiBackground.I \
    guiBehavior.h guiBehavior.I \
    guiChooser.h guiChooser.I \
    guiCollection.h guiCollection.I

  #define IGATESCAN \
    guiManager.h guiManager.I \
    guiRegion.h guiRegion.I \
    guiLabel.h guiLabel.I \
    guiItem.h guiItem.I \
    guiRollover.h guiRollover.I \
    guiButton.h guiButton.I \
    guiFrame.h guiFrame.I \
    guiSign.h guiSign.I \
    guiListBox.h guiListBox.I \
    guiBackground.h guiBackground.I \
    guiBehavior.h guiBehavior.I \
    guiChooser.h guiChooser.I \
    guiCollection.h guiCollection.I

#end lib_target
