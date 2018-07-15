/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatProperties.cxx
 * @author drose
 * @date 2001-05-17
 */

#include "pStatProperties.h"
#include "pStatCollectorDef.h"
#include "pStatClient.h"
#include "config_pstatclient.h"
#include "configVariableBool.h"
#include "configVariableColor.h"
#include "configVariableDouble.h"
#include "configVariableInt.h"
#include "configVariableString.h"

#include <ctype.h>

using std::string;

static const int current_pstat_major_version = 3;
static const int current_pstat_minor_version = 0;
// Initialized at 2.0 on 51801, when version numbers were first added.
// Incremented to 2.1 on 52101 to add support for TCP frame data.  Incremented
// to 3.0 on 42805 to bump TCP headers to 32 bits.

/**
 * Returns the current major version number of the PStats protocol.  This is
 * the version number that will be reported by clients running this code, and
 * that will be expected by servers running this code.
 *
 * The major version numbers must match exactly in order for a communication
 * to be successful.
 */
int
get_current_pstat_major_version() {
  return current_pstat_major_version;
}

/**
 * Returns the current minor version number of the PStats protocol.  This is
 * the version number that will be reported by clients running this code, and
 * that will be expected by servers running this code.
 *
 * The minor version numbers need not match exactly, but the server must be >=
 * the client.
 */
int
get_current_pstat_minor_version() {
  return current_pstat_minor_version;
}


#ifdef DO_PSTATS

/*
 * The rest of this file defines the predefined properties (color, sort, etc.)
 * for the various PStatCollectors that may be defined within Panda or even
 * elsewhere.  It is a little strange to defined these properties here instead
 * of where the collectors are actually declared, but it's handy to have them
 * all in one place, so we can easily see which colors are available, etc.  It
 * also makes the declarations a lot simpler, since there are quite a few
 * esoteric parameters to specify.  We could define these in some external
 * data file that is read in at runtime, so that you could extend this list
 * without having to relink panda, but then there are the usual problems with
 * ensuring that the file is available to you at runtime.  The heck with it.
 * At least, no other file depends on this file, so it may be modified without
 * forcing anything else to be recompiled.
 */

typedef PStatCollectorDef::ColorDef ColorDef;

struct TimeCollectorProperties {
  bool is_active;
  const char *name;
  ColorDef color;
  double suggested_scale;
};

struct LevelCollectorProperties {
  bool is_active;
  const char *name;
  ColorDef color;
  const char *units;
  double suggested_scale;
  double inv_factor;
};

static TimeCollectorProperties time_properties[] = {
  { 1, "Frame",                            { 0.95, 1.0, 0.35 } },
  { 1, "Wait",                             { 0.6, 0.6, 0.6 } },
  { 0, "Wait:Mutex block",                 { 0.5, 0.0, 1.0 } },
  { 1, "Wait:Thread sync",                 { 0.0, 1.0, 0.5 } },
  { 1, "Wait:Clock Wait",                  { 0.2, 0.8, 0.2 } },
  { 1, "Wait:Clock Wait:Sleep",            { 0.9, 0.4, 0.8 } },
  { 1, "Wait:Clock Wait:Spin",             { 0.2, 0.8, 1.0 } },
  { 1, "Wait:Flip",                        { 1.0, 0.6, 0.3 } },
  { 1, "Wait:Flip:Begin",                  { 0.3, 0.3, 0.9 } },
  { 1, "Wait:Flip:End",                    { 0.9, 0.3, 0.6 } },
  { 1, "App",                              { 0.0, 0.4, 0.8 },  1.0 / 30.0 },
  { 1, "App:Collisions",                   { 1.0, 0.5, 0.0 } },
  { 1, "App:Collisions:Reset",             { 0.0, 0.0, 0.5 } },
  { 0, "App:Data graph",                   { 0.5, 0.8, 0.4 } },
  { 1, "App:Show code",                    { 0.8, 0.2, 1.0 } },
  { 0, "App:Show code:Nametags",           { 0.8, 0.8, 1.0 } },
  { 0, "App:Show code:Nametags:2d",        { 0.0, 0.0, 0.5 } },
  { 0, "App:Show code:Nametags:2d:Contents", { 0.0, 0.5, 0.0 } },
  { 0, "App:Show code:Nametags:2d:Adjust",   { 0.5, 0.0, 0.5 } },
  { 0, "App:Show code:Nametags:3d",        { 1.0, 0.0, 0.0 } },
  { 0, "App:Show code:Nametags:3d:Contents", { 0.0, 0.5, 0.0 } },
  { 0, "App:Show code:Nametags:3d:Adjust",   { 0.5, 0.0, 0.5 } },
  { 1, "Cull",                             { 0.21, 0.68, 0.37 },  1.0 / 30.0 },
  { 1, "Cull:Setup",                       { 0.7, 0.4, 0.5 } },
  { 1, "Cull:Sort",                        { 0.3, 0.3, 0.6 } },
  { 1, "*",                                { 0.1, 0.1, 0.5 } },
  { 1, "*:Show fps",                       { 0.5, 0.8, 1.0 } },
  { 1, "*:Munge",                          { 0.3, 0.3, 0.9 } },
  { 1, "*:Munge:Geom",                     { 0.4, 0.2, 0.8 } },
  { 1, "*:Munge:Sprites",                  { 0.2, 0.8, 0.4 } },
  { 0, "*:Munge:Data",                     { 0.7, 0.5, 0.2 } },
  { 0, "*:Munge:Rotate",                   { 0.9, 0.8, 0.5 } },
  { 0, "*:Munge:Decompose",                { 0.1, 0.3, 0.1 } },
  { 1, "*:PStats",                         { 0.4, 0.8, 1.0 } },
  { 1, "*:Animation",                      { 1.0, 0.0, 1.0 } },
  { 0, "*:Flatten",                        { 0.0, 0.7, 0.4 } },
  { 0, "*:State Cache",                    { 0.4, 0.7, 0.7 } },
  { 0, "*:NodePath",                       { 0.1, 0.6, 0.8 } },
  { 1, "Draw",                             { 0.83, 0.02, 0.01 },  1.0 / 30.0 },
  { 1, "Draw:Make current",                { 0.4, 0.2, 0.6 } },
  { 1, "Draw:Copy texture",                { 0.2, 0.6, 0.4 } },
  { 1, "Draw:Transfer data",               { 0.8, 0.0, 0.6 } },
  { 1, "Draw:Transfer data:Vertex buffer", { 0.0, 0.1, 0.9 } },
  { 1, "Draw:Transfer data:Index buffer",  { 0.1, 0.9, 0.0 } },
  { 1, "Draw:Transfer data:Texture",       { 0.9, 0.0, 0.1 } },
  { 1, "Draw:Transfer data:Display lists", { 0.5, 0.0, 0.9 } },
  { 1, "Draw:Clear",                       { 0.0, 0.8, 0.6 } },
  { 1, "Draw:Flush",                       { 0.9, 0.2, 0.7 } },
  { 1, "Draw:Sync",                        { 0.5, 0.7, 0.7 } },
  { 0, "Draw:Transform",                   { 0.0, 0.5, 0.0 } },
  { 1, "Draw:Primitive",                   { 0.0, 0.0, 0.5 } },
  { 1, "Draw:Set State",                   { 0.2, 0.6, 0.8 } },
  { 1, "Draw:Wait occlusion",              { 1.0, 0.5, 0.0 } },
  { 1, "Draw:Bind FBO",                    { 0.0, 0.8, 0.8 } },
  { 0, nullptr }
};

static LevelCollectorProperties level_properties[] = {
  { 1, "Graphics memory",                  { 0.0, 0.0, 1.0 },  "MB", 64, 1048576 },
  { 1, "Buffer switch",                    { 0.0, 0.6, 0.8 },  "", 500 },
  { 1, "Buffer switch:Vertex",             { 0.8, 0.0, 0.6 } },
  { 1, "Buffer switch:Index",              { 0.8, 0.6, 0.3 } },
  { 1, "Geom cache size",                  { 0.6, 0.8, 0.6 },  "", 500 },
  { 1, "Geom cache size:Active",           { 0.9, 1.0, 0.3 },  "", 500 },
  { 1, "Geom cache operations",            { 1.0, 0.6, 0.6 },  "", 500 },
  { 1, "Geom cache operations:record",     { 0.2, 0.4, 0.8 } },
  { 1, "Geom cache operations:erase",      { 0.4, 0.8, 0.2 } },
  { 1, "Geom cache operations:evict",      { 0.8, 0.2, 0.4 } },
  { 1, "Data transferred",                 { 0.0, 0.2, 0.4 },  "MB", 12, 1048576 },
  { 1, "Primitive batches",                { 0.2, 0.5, 0.9 },  "", 500 },
  { 1, "Primitive batches:Other",          { 0.2, 0.2, 0.2 } },
  { 1, "Primitive batches:Triangles",      { 0.8, 0.8, 0.8 } },
  { 1, "Primitive batches:Triangle fans",  { 0.8, 0.5, 0.2 } },
  { 1, "Primitive batches:Triangle strips",{ 0.2, 0.5, 0.8 } },
  { 1, "Primitive batches:Display lists",  { 0.8, 0.5, 1.0 } },
  { 1, "SW Sprites",                       { 0.2, 0.7, 0.3 },  "K", 10, 1000 },
  { 1, "Vertices",                         { 0.5, 0.2, 0.0 },  "K", 10, 1000 },
  { 1, "Vertices:Other",                   { 0.2, 0.2, 0.2 } },
  { 1, "Vertices:Triangles",               { 0.8, 0.8, 0.8 } },
  { 1, "Vertices:Triangle fans",           { 0.8, 0.5, 0.2 } },
  { 1, "Vertices:Triangle strips",         { 0.2, 0.5, 0.8 } },
  { 1, "Vertices:Indexed triangle strips", { 0.5, 0.2, 0.8 } },
  { 1, "Vertices:Display lists",           { 0.8, 0.5, 1.0 } },
  { 1, "Vertices:Immediate mode",          { 1.0, 0.5, 0.0 } },
  { 1, "Pixels",                           { 0.8, 0.3, 0.7 },  "M", 5, 1000000 },
  { 1, "Nodes",                            { 0.4, 0.2, 0.8 },  "", 500.0 },
  { 1, "Nodes:GeomNodes",                  { 0.8, 0.2, 0.0 } },
  { 1, "Geoms",                            { 0.4, 0.8, 0.3 },  "", 500.0 },
  { 1, "Cull volumes",                     { 0.7, 0.6, 0.9 },  "", 500.0 },
  { 1, "Cull volumes:Transforms",          { 0.9, 0.6, 0.0 } },
  { 1, "State changes",                    { 1.0, 0.5, 0.2 },  "", 500.0 },
  { 1, "State changes:Other",              { 0.2, 0.2, 0.2 } },
  { 1, "State changes:Transforms",         { 0.2, 0.2, 0.8 } },
  { 1, "State changes:Textures",           { 0.8, 0.2, 0.2 } },
  { 1, "Occlusion tests",                  { 0.9, 0.8, 0.3 },  "", 500.0 },
  { 1, "Occlusion results",                { 0.3, 0.9, 0.8 },  "", 500.0 },
  { 1, "System memory",                    { 0.5, 1.0, 0.5 },  "MB", 64, 1048576 },
  { 1, "System memory:Heap",               { 0.2, 0.2, 1.0 } },
  { 1, "System memory:Heap:Overhead",      { 0.3, 0.4, 0.6 } },
  { 1, "System memory:Heap:Single",        { 0.8, 0.3, 0.3 } },
  { 1, "System memory:Heap:Array",         { 0.1, 0.3, 1.0 } },
  { 1, "System memory:Heap:Overhead",      { 0.9, 0.7, 0.8 } },
  { 1, "System memory:Heap:External",      { 0.2, 0.2, 0.5 } },
  { 1, "System memory:MMap",               { 0.9, 0.4, 0.7 } },
  { 1, "Vertex Data",                      { 1.0, 0.4, 0.0 },  "MB", 64, 1048576 },
  { 1, "Vertex Data:Independent",          { 0.9, 0.1, 0.9 } },
  { 1, "Vertex Data:Small",                { 0.2, 0.3, 0.4 } },
  { 1, "Vertex Data:Pending",              { 0.6, 0.8, 1.0 } },
  { 1, "Vertex Data:Resident",             { 0.9, 1.0, 0.7 } },
  { 1, "Vertex Data:Compressed",           { 0.5, 0.1, 0.4 } },
  { 1, "Vertex Data:Disk",                 { 0.6, 0.9, 0.1 } },
  { 1, "Vertex Data:Disk:Unused",          { 0.8, 0.4, 0.5 } },
  { 1, "Vertex Data:Disk:Used",            { 0.2, 0.1, 0.6 } },
  { 1, "TransformStates",                  { 1.0, 0.5, 0.5 },  "", 5000 },
  { 1, "TransformStates:On nodes",         { 0.2, 0.8, 1.0 } },
  { 1, "TransformStates:Cached",           { 1.0, 0.0, 0.2 } },
  { 1, "TransformStates:Unused",           { 0.2, 0.2, 0.2 } },
  { 1, "RenderStates",                     { 0.5, 0.5, 1.0 },  "", 1000 },
  { 1, "RenderStates:On nodes",            { 0.2, 0.8, 1.0 } },
  { 1, "RenderStates:Cached",              { 1.0, 0.0, 0.2 } },
  { 1, "RenderStates:Unused",              { 0.2, 0.2, 0.2 } },
  { 1, "PipelineCyclers",                  { 0.5, 0.5, 1.0 },  "", 50000 },
  { 1, "Dirty PipelineCyclers",            { 0.2, 0.2, 0.2 },  "", 5000 },
  { 1, "Collision Volumes",                { 1.0, 0.8, 0.5 },  "", 500 },
  { 1, "Collision Tests",                  { 0.5, 0.8, 1.0 },  "", 100 },
  { 1, "Command latency",                  { 0.8, 0.2, 0.0 },  "ms", 10, 1.0 / 1000.0 },
  { 0, nullptr }
};


/**
 * Looks up the collector in the compiled-in table defined above, and sets its
 * properties appropriately if it is found.
 */
static void
initialize_collector_def_from_table(const string &fullname, PStatCollectorDef *def) {
  int i;

  for (i = 0;
       time_properties[i].name != nullptr;
       i++) {
    const TimeCollectorProperties &tp = time_properties[i];
    if (fullname == tp.name) {
      def->_sort = i;
      if (!def->_active_explicitly_set) {
        def->_is_active = tp.is_active;
      }
      def->_suggested_color = tp.color;
      if (tp.suggested_scale != 0.0) {
        def->_suggested_scale = tp.suggested_scale;
      }
      return;
    }
  }

  for (i = 0;
       level_properties[i].name != nullptr;
       i++) {
    const LevelCollectorProperties &lp = level_properties[i];
    if (fullname == lp.name) {
      def->_sort = i;
      if (!def->_active_explicitly_set) {
        def->_is_active = lp.is_active;
      }
      def->_suggested_color = lp.color;
      if (lp.suggested_scale != 0.0) {
        def->_suggested_scale = lp.suggested_scale;
      }
      if (lp.units != nullptr) {
        def->_level_units = lp.units;
      }
      if (lp.inv_factor != 0.0) {
        def->_factor = 1.0 / lp.inv_factor;
      }
      return;
    }
  }
}


/**
 * This is the only accessor function into this table.  The PStatCollectorDef
 * constructor calls it when a new PStatCollectorDef is created.  It should
 * look up in the table and find a matching definition for this def by name;
 * if one is found, the properties are applied.
 */
void
initialize_collector_def(const PStatClient *client, PStatCollectorDef *def) {
  string fullname;

  if (def->_index == 0) {
    fullname = def->_name;
  } else {
    fullname = client->get_collector_fullname(def->_index);
  }

  // First, check the compiled-in defaults.
  initialize_collector_def_from_table(fullname, def);

  // Then, look to Config for more advice.  To do this, we first change the
  // name to something more like a Config variable name.  We replace colons
  // and spaces with hyphens, eliminate other punctuation, and make all
  // letters lowercase.

  string config_name;
  string::const_iterator ni;
  for (ni = fullname.begin(); ni != fullname.end(); ++ni) {
    switch (*ni) {
    case ':':
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      config_name += '-';
      break;

    default:
      if (isalnum(*ni)) {
        config_name += tolower(*ni);
      }
    }
  }

  ConfigVariableBool pstats_active
    ("pstats-active-" + config_name, true, "", ConfigVariable::F_dynamic);
  ConfigVariableInt pstats_sort
    ("pstats-sort-" + config_name, def->_sort, "", ConfigVariable::F_dynamic);
  ConfigVariableDouble pstats_scale
    ("pstats-scale-" + config_name, def->_suggested_scale, "", ConfigVariable::F_dynamic);
  ConfigVariableString pstats_units
    ("pstats-units-" + config_name, def->_level_units, "", ConfigVariable::F_dynamic);
  ConfigVariableDouble pstats_factor
    ("pstats-factor-" + config_name, 1.0, "", ConfigVariable::F_dynamic);
  ConfigVariableColor pstats_color
    ("pstats-color-" + config_name, LColor::zero(), "", ConfigVariable::F_dynamic);

  if (pstats_active.has_value()) {
    def->_is_active = pstats_active;
    def->_active_explicitly_set = true;
  }

  def->_sort = pstats_sort;
  def->_suggested_scale = pstats_scale;
  def->_level_units = pstats_units;
  if (pstats_factor.has_value()) {
    def->_factor = pstats_factor;
  }

  if (pstats_color.has_value()) {
    def->_suggested_color.r = pstats_color[0];
    def->_suggested_color.g = pstats_color[1];
    def->_suggested_color.b = pstats_color[2];
  }
}

#endif // DO_PSTATS
