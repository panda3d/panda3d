#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET gui

  #define LOCAL_LIBS \
    putil display tform device pandabase dgraph sgattrib light gobj text
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
     config_gui.h guiManager.h guiManager.I guiLabel.h guiLabel.I \
     guiItem.h guiItem.I guiRollover.h guiRollover.I guiButton.h \
     guiButton.I guiFrame.h guiFrame.I guiSign.h guiSign.I \
     guiListBox.h guiListBox.I guiBackground.h guiBackground.I \
     guiBehavior.h guiBehavior.I guiChooser.h guiChooser.I \
     guiCollection.h guiCollection.I 
    
  #define INCLUDED_SOURCES \
     config_gui.cxx guiManager.cxx guiLabel.cxx guiItem.cxx \
     guiRollover.cxx guiButton.cxx guiFrame.cxx guiSign.cxx \
     guiListBox.cxx guiBackground.cxx guiBehavior.cxx \
     guiChooser.cxx guiCollection.cxx 

  #define INSTALL_HEADERS \
    guiManager.h guiManager.I \
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
