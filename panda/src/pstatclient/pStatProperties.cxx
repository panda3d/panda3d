// Filename: pStatProperties.cxx
// Created by:  drose (17May01)
// 
////////////////////////////////////////////////////////////////////

#include "pStatProperties.h"
#include "pStatCollectorDef.h"
#include "pStatClient.h"

#ifdef DO_PSTATS

////////////////////////////////////////////////////////////////////
//
// This file defines the predefined properties (color, sort, etc.) for
// the various PStatCollectors that may be defined within Panda or
// even elsewhere.
//
// It is a little strange to defined these properties here instead of
// where the collectors are actually declared, but it's handy to have
// them all in one place, so we can easily see which colors are
// available, etc.  It also makes the declarations a lot simpler,
// since there are quite a few esoteric parameters to specify.
//
// We could define these in some external data file that is read in at
// runtime, so that you could extend this list without having to
// relink panda, but then there are the usual problems with ensuring
// that the file is available to you at runtime.  The heck with it.
//
// At least, no other file depends on this file, so it may be modified
// without forcing anything else to be recompiled.
//
////////////////////////////////////////////////////////////////////

struct ColorDef {
  float r, g, b;
};

struct TimeCollectorProperties {
  const char *name;
  ColorDef color;
  float suggested_scale;
};

struct LevelCollectorProperties {
  const char *name;
  ColorDef color;
  const char *units;
  float suggested_scale;
};

static TimeCollectorProperties time_properties[] = {
  { "App",                              { 0.0, 1.0, 1.0 },  1.0 / 30.0 },
  { "App:Animation",                    { 1.0, 0.0, 1.0 } },
  { "App:Collisions",                   { 1.0, 0.5, 0.0 } },
  { "App:Data graph",                   { 0.5, 0.8, 0.4 } },
  { "App:Show code",                    { 0.8, 0.2, 1.0 } },
  { "Cull",                             { 0.0, 1.0, 0.0 },  1.0 / 30.0 },
  { "Draw",                             { 1.0, 0.0, 0.0 },  1.0 / 30.0 },
  { "Draw:Swap buffers",                { 0.5, 1.0, 0.8 } },
  { "Draw:Clear",                       { 0.5, 0.7, 0.7 } },
  { "Draw:Show fps",                    { 0.5, 0.8, 1.0 } },
  { "Draw:Make current",                { 1.0, 0.6, 0.3 } },
  { NULL }
};

static LevelCollectorProperties level_properties[] = {
  { "Texture usage",                    { 1.0, 0.0, 0.0 },  "MB", 12.0 },
  { "Texture usage:Active",             { 1.0, 1.0, 0.0 } },
  { "Texture memory",                   { 0.0, 0.0, 1.0 },  "MB", 12.0 },
  { "Texture memory:In use",            { 0.0, 1.0, 1.0 } },
  { "Vertices",                         { 0.5, 0.2, 0.0 },  "", 10000.0 },
  { "Vertices:Other",                   { 0.2, 0.2, 0.2 } },
  { "Vertices:Triangles",               { 0.8, 0.8, 0.8 } },
  { "Vertices:Triangle fans",           { 0.8, 0.5, 0.2 } },
  { "Vertices:Triangle strips",         { 0.2, 0.5, 0.8 } },
  { NULL }
};

    
////////////////////////////////////////////////////////////////////
//     Function: initialize_collector_def
//  Description: This is the only accessor function into this table.
//               The PStatCollectorDef constructor calls it when a new
//               PStatCollectorDef is created.  It should look up in
//               the table and find a matching definition for this def
//               by name; if one is found, the properties are applied.
////////////////////////////////////////////////////////////////////
void
initialize_collector_def(PStatClient *client, PStatCollectorDef *def) {
  int i;
  string fullname;

  if (def->_index == 0) {
    fullname = def->_name;
  } else {
    fullname = client->get_collector_fullname(def->_index);
  }

  for (i = 0;
       time_properties[i].name != (const char *)NULL; 
       i++) {
    const TimeCollectorProperties &tp = time_properties[i];
    if (fullname == tp.name) {
      def->_sort = i;
      def->_suggested_color.set(tp.color.r, tp.color.g, tp.color.b);
      if (tp.suggested_scale != 0.0) {
        def->_suggested_scale = tp.suggested_scale;
      }
      return;
    }
  }

  for (i = 0;
       level_properties[i].name != (const char *)NULL; 
       i++) {
    const LevelCollectorProperties &lp = level_properties[i];
    if (fullname == lp.name) {
      def->_sort = i;
      def->_suggested_color.set(lp.color.r, lp.color.g, lp.color.b);
      if (lp.suggested_scale != 0.0) {
        def->_suggested_scale = lp.suggested_scale;
      }
      if (lp.units != (const char *)NULL) {
        def->_level_units = lp.units;
      }
      return;
    }
  }
}


#endif // DO_PSTATS
