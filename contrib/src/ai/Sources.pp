#define LOCAL_LIBS contribbase
#define BUILDING_DLL BUILDING_PANDAAI

#define OTHER_LIBS \
   panda:c \
   express:c putil:c pandabase:c pandaexpress:m \
   interrogatedb:c prc:c dconfig:c dtoolconfig:m \
   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pandaai

  #define COMBINED_SOURCES ai_composite1.cxx

  #define SOURCES \
    aiBehaviors.h \
    aiCharacter.h \
    aiGlobals.h \
    aiNode.h \
    aiPathFinder.h \
    aiWorld.h \
    arrival.h \
    config_ai.h \
    evade.h \
    flee.h \
    flock.h \
    aiGlobals.h \
    meshNode.h \
    obstacleAvoidance.h \
    pathFind.h \
    pathFollow.h \
    pursue.h \
    seek.h \
    wander.h

  #define INCLUDED_SOURCES \
    aiBehaviors.cxx \
    aiCharacter.cxx \
    aiNode.cxx \
    aiPathFinder.cxx \
    aiWorld.cxx \
    ai_composite.cxx \
    ai_composite1.cxx \
    arrival.cxx \
    config_ai.cxx \
    evade.cxx \
    flee.cxx \
    flock.cxx \
    meshNode.cxx \
    obstacleAvoidance.cxx \
    pathFind.cxx \
    pathFollow.cxx \
    pursue.cxx \
    seek.cxx \
    wander.cxx


  #define INSTALL_HEADERS \
    aiBehaviors.h \
    aiCharacter.h \
    aiGlobals.h \
    aiNode.h \
    aiPathFinder.h \
    aiWorld.h \
    arrival.h \
    config_ai.h \
    evade.h \
    flee.h \
    flock.h \
    aiGlobals.h \
    meshNode.h \
    obstacleAvoidance.h \
    pathFind.h \
    pathFollow.h \
    pursue.h \
    seek.h \
    wander.h

  #define IGATESCAN all

#end lib_target

