// Filename: showBase.cxx
// Created by:  shochet (02Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "showBase.h"

#include "throw_event.h"
#include "graphicsWindow.h"
#include "renderBuffer.h"
#include "camera.h"
#include "graphicsPipeSelection.h"


ConfigureDef(config_showbase);
ConfigureFn(config_showbase) {
}

ConfigVariableSearchPath particle_path
("particle-path", 
 PRC_DESC("The directories to search for particle files to be loaded."));

ConfigVariableSearchPath &
get_particle_path() {
  return particle_path;
}

// Throw the "NewFrame" event in the C++ world.  Some of the lerp code
// depends on receiving this.
void 
throw_new_frame() {
  throw_event("NewFrame");
}

// Returns the configure object for accessing config variables from a
// scripting language.
ConfigShowbase &
get_config_showbase() {
  return config_showbase;
}

// klunky interface since we cant pass array from python->C++ to use verify_window_sizes directly
static int num_fullscreen_testsizes = 0;
#define MAX_FULLSCREEN_TESTS 10
static int fullscreen_testsizes[MAX_FULLSCREEN_TESTS * 2];

void
add_fullscreen_testsize(int xsize, int ysize) {
  if ((xsize == 0) && (ysize == 0)) {
    num_fullscreen_testsizes = 0;
    return;
  }

  // silently fail if maxtests exceeded
  if (num_fullscreen_testsizes < MAX_FULLSCREEN_TESTS) {
    fullscreen_testsizes[num_fullscreen_testsizes * 2] = xsize;
    fullscreen_testsizes[num_fullscreen_testsizes * 2 + 1] = ysize;
    num_fullscreen_testsizes++;
  }
}

void
runtest_fullscreen_sizes(GraphicsWindow *win) {
  win->verify_window_sizes(num_fullscreen_testsizes, fullscreen_testsizes);
}

bool
query_fullscreen_testresult(int xsize, int ysize) {
  // stupid linear search that works ok as long as total tests are small
  int i;
  for (i=0; i < num_fullscreen_testsizes; i++) {
    if((fullscreen_testsizes[i * 2] == xsize) &&
       (fullscreen_testsizes[i * 2 + 1] == ysize))
      return true;
  }
  return false;
}

#if 0
int TempGridZoneManager::
add_grid_zone(unsigned int x, 
              unsigned int y, 
              unsigned int width, 
              unsigned int height, 
              unsigned int zoneBase, 
              unsigned int xZoneResolution,
              unsigned int yZoneResolution) {
  // zoneBase is the first zone in the grid (e.g. the upper left)
  // zoneResolution is the number of cells on each axsis.
  // returns the next available zoneBase (i.e. zoneBase+xZoneResolution*yZoneResolution)
  cerr<<"adding grid zone with a zoneBase of "<<zoneBase<<" and a zoneResolution of "<<zoneResolution;
  _grids.append(TempGridZoneManager::GridZone(x, y, width, height, zoneBase, xZoneResolution, yZoneResolution));
  return zoneBase+xZoneResolution*yZoneResolution;
}

void TempGridZoneManager::GridZone
GridZone(unsigned int x, 
         unsigned int y, 
         unsigned int width, 
         unsigned int height, 
         unsigned int zoneBase, 
         unsigned int xZoneResolution,
         unsigned int yZoneResolution) {
  _x=x;
  _y=y;
  _width=width;
  _height=heigth; 
  _zoneBase=zoneBase;
  _xZoneResolution=xZoneResolution;
  _yZoneResolution=yZoneResolution;
  
  // The cellVis is the number of cells radius that can
  // be seen, including the center cell.  So, for a 5 x 5
  // visible area, the cellVis is 3.
  const float cellVis=3.0;
  unsigned int xMargine=(unsigned int)((float)width/(float)xZoneResolution*cellVis+0.5);
  unsigned int yMargine=(unsigned int)((float)height/(float)yZoneResolution*cellVis+0.5);
  _xMinVis=x-xMargine;
  _yMinVis=y-yMargine;
  _xMaxVis=x+width+xMargine;
  _yMaxVis=y+height+yMargine;
}

void TempGridZoneManager::
get_grids(int x, int y) {
  TempGridZoneManager::ZoneSet canSee;
  TempGridZoneManager::GridSet::const_iterator i=_grids.begin();
  for (; i!=_grids.end(); ++i) {
    if (x >= i._xMinVis && x < i._xMaxVis && y >= i._yMinVis && y < i._yMaxVis) {
      add_to_zone_list(i, x, y, canSee);
    }
  }
}

void TempGridZoneManager::
add_to_zone_list(const TempGridZoneManager::GridZone &gridZone, 
    unsigned int x,
    unsigned int y,
    TempGridZoneManager::ZoneSet &zoneSet) {
  unsigned int xRes=gridZone._xZoneResolution;
  unsigned int yRes=gridZone._yZoneResolution;
  float xP=((float)(x-gridZone._x))/gridZone._width;
  float yP=((float)(y-gridZone._y))/gridZone._height;
  int xCell=(int)(xP*xRes);
  int yCell=(int)(yP*yRes);

  // range is how many cells can be seen in each direction:
  const int range=2;
  int yBegin=max(0, yCell-range);
  int yEnd=min(yRes, yCell+range);
  int xBegin=max(0, xCell-range);
  int xEnd=min(xRes, xCell+range);
  unsigned int zone=gridZone._zoneBase+yBegin*xRes+xBegin;

  for (yCell=yBegin; yCell < yEnd; ++yCell) {
    for (xCell=xBegin; xCell < xEnd; ++xCell) {
      zoneSet.append(zone+xCell);
    }
    zone+=xRes;
  }
}

int TempGridZoneManager::
get_zone_list(int x, int y, int resolution) {
  // x is a float in the range 0.0 to 1.0
  // y is a float in the range 0.0 to 1.0
  // resolution is the number of cells on each axsis.
  // returns a list of zone ids.
  // 
  // Create a box of cell numbers, while clipping
  // to the edges of the set of cells.
  if (x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0) {
    return 0;
  }
  cerr<<"resolution="<<resolution;
  xCell=min(int(x*resolution), resolution-1)
  yCell=min(int(y*resolution), resolution-1)
  cell=yCell*resolution+xCell
  print "cell", cell,
  zone=zoneBase+cell
  print "zone", zone

  zone=zone-2*resolution
  endZone=zone+5*resolution
  yCell=yCell-2
  while zone < endZone:
      if yCell >= 0 and yCell < resolution:
          if xCell > 1:
              zoneList.append(zone-2)
              zoneList.append(zone-1)
          elif xCell > 0:
              zoneList.append(zone-1)
          r.append(zone)
          if xCell < resolution-2:
              zoneList.append(zone+1)
              zoneList.append(zone+2)
          elif xCell < resolution-1:
              zoneList.append(zone+1)
      yCell+=1
      zone+=resolution
  return zoneList
  return 5;
}
#endif

