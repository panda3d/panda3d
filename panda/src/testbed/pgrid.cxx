// Filename: pgrid.cxx
// Created by:  drose (03Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pandaFramework.h"
#include "pandaNode.h"
#include "transformState.h"
#include "clockObject.h"
#include "string_utils.h"
#include "pvector.h"

#ifdef HAVE_GETOPT
#include <getopt.h>
#else
#include "gnu_getopt.h"
#endif


#define RANDFRAC (rand()/(float)(RAND_MAX))

class GriddedFilename {
public:
  Filename _filename;
  int _count;
  NodePath _model;
};
typedef pvector<GriddedFilename> GriddedFilenames;

typedef struct {
  // for rot moving
  float xcenter,ycenter;
  float xoffset,yoffset;
  float ang1,ang1_vel;
  float ang2,ang2_vel;
  
  float radius;
  
  // for moving
  float xstart,ystart;
  float xend,yend;
  float xdel,ydel,timedel;
  double starttime,endtime;
  double vel;
  LMatrix4f rotmat;

  PandaNode *node;
} gridded_file_info;

typedef pvector<gridded_file_info> GriddedInfoArray;

typedef enum {None,Rotation,LinearMotion} GriddedMotionType;

#define GRIDCELLSIZE 5.0
static int gridwidth;  // cells/side

#define MIN_WANDERAREA_DIMENSION 120.0f

static float grid_pos_offset;  // origin of grid
static float wander_area_pos_offset;

static GriddedMotionType gridmotiontype = None;


// making these fns to get around ridiculous VC++ matrix inlining bugs at Opt2
static void
move_gridded_stuff(GriddedMotionType gridmotiontype,
                   gridded_file_info *InfoArr, int size) {
  double now = ClockObject::get_global_clock()->get_frame_time();

  LMatrix4f tmat1,tmat2,xfm_mat;

  for(int i = 0; i < size; i++) {
    double time_delta = (now-InfoArr[i].starttime);
#define DO_FP_MODULUS(VAL,MAXVAL)  \
    {if(VAL > MAXVAL) {int idivresult = (int)(VAL / (float)MAXVAL);  VAL=VAL-idivresult*MAXVAL;} else  \
    if(VAL < -MAXVAL) {int idivresult = (int)(VAL / (float)MAXVAL);  VAL=VAL+idivresult*MAXVAL;}}
  
    // probably should use panda lerps for this stuff, but I dont understand how

    if(gridmotiontype==Rotation) {

      InfoArr[i].ang1=time_delta*InfoArr[i].ang1_vel;
      DO_FP_MODULUS(InfoArr[i].ang1,360.0);
      InfoArr[i].ang2=time_delta*InfoArr[i].ang2_vel;
      DO_FP_MODULUS(InfoArr[i].ang2,360.0);

      // xforms happen left to right
      LVector2f new_center = LVector2f(InfoArr[i].radius,0.0) *
        LMatrix3f::rotate_mat(InfoArr[i].ang1);

      LVector3f translate_vec(InfoArr[i].xcenter+new_center._v.v._0,
                              InfoArr[i].ycenter+new_center._v.v._1,
                              0.0);

      const LVector3f rotation_axis(0.0, 0.0, 1.0);

      tmat1 = LMatrix4f::rotate_mat_normaxis(InfoArr[i].ang2,rotation_axis);
      tmat2 = LMatrix4f::translate_mat(translate_vec);
      xfm_mat = tmat1 * tmat2;
    } else {

      float xpos,ypos;

      if(now>InfoArr[i].endtime) {
        InfoArr[i].starttime = now;

        xpos = InfoArr[i].xstart = InfoArr[i].xend;
        ypos = InfoArr[i].ystart = InfoArr[i].yend;

        InfoArr[i].xend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;
        InfoArr[i].yend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;

        float xdel = InfoArr[i].xdel = InfoArr[i].xend-InfoArr[i].xstart;
        float ydel = InfoArr[i].ydel = InfoArr[i].yend-InfoArr[i].ystart;

        InfoArr[i].endtime = now + csqrt(xdel*xdel+ydel*ydel)/InfoArr[i].vel;
        InfoArr[i].timedel = InfoArr[i].endtime - InfoArr[i].starttime;

        const LVector3f rotate_axis(0.0, 0.0, 1.0);

        float ang = rad_2_deg(atan2(-xdel,ydel));

        InfoArr[i].rotmat= LMatrix4f::rotate_mat_normaxis(ang,rotate_axis);
      } else {
        float timefrac= time_delta/InfoArr[i].timedel;

        xpos = InfoArr[i].xdel*timefrac+InfoArr[i].xstart;
        ypos = InfoArr[i].ydel*timefrac+InfoArr[i].ystart;
      }

      LVector3f translate_vec(xpos, ypos, 0.0);
      LMatrix4f tmat2 = LMatrix4f::translate_mat(translate_vec);

      xfm_mat = InfoArr[i].rotmat * tmat2;
    }
    InfoArr[i].node->set_transform(TransformState::make_mat(xfm_mat));
  }
}

bool
get_command_line_opts(int &argc, char **&argv) {
  // Use getopt() to decode the optional command-line parameters.
  //  extern char *optarg;
  extern int optind;
  const char *options = "rm";
  int flag = getopt(argc, argv, options);
  while (flag != EOF) {
    switch (flag) {
    case 'r':
      gridmotiontype = Rotation;
      break;

    case 'm':
      gridmotiontype = LinearMotion;
      break;

    case '?':
      nout << "Invalid parameter.\n";
      return false;
    }

    flag = getopt(argc, argv, options);
  }

  argv += (optind - 1);
  argc -= (optind - 1);

  return true;
}

void
get_command_line_filenames(int argc, char *argv[],
                           pvector<Filename> &static_filenames,
                           GriddedFilenames &gridded_filenames) {
  for (int i = 1; i < argc && argv[i] != (char *)NULL; i++) {
    const string &arg = argv[i];
    size_t comma = arg.find(',');
    if (comma == string::npos) {
      // No comma in the filename, so it must be an ordinary static file.
      static_filenames.push_back(Filename::from_os_specific(arg));

    } else {
      // A comma in the filename indicates a gridded file.  The syntax
      // is filename,count where count represents the number of times
      // the file is repeated.
      string name = arg.substr(0, comma);
      string count_str = arg.substr(comma + 1);
      int count;
      if (!string_to_int(count_str, count)) {
        nout << "Ignoring invalid number: " << count_str << "\n";
        count = 1;
      } else if (count <= 0) {
        nout << "Ignoring inappropriate number: " << count << "\n";
        count = 1;
      }

      GriddedFilename gf;
      gf._filename = Filename::from_os_specific(name);
      gf._count = count;
      gridded_filenames.push_back(gf);
    }
  }
}

void
load_gridded_models(WindowFramework *window, 
                    GriddedFilenames &filenames,
                    GriddedInfoArray &info_arr) {
  // Load up all the files indicated in the list of gridded filenames
  // and store them in the given vector.

  // First, load up each model from disk once, and store them all
  // separate from the scene graph.  Also count up the total number of
  // models we'll be putting in the grid.
  int grid_count = 0;
  NodePath models("models");
  GriddedFilenames::iterator fi;
  for (fi = filenames.begin(); fi != filenames.end(); ++fi) {
    GriddedFilename &gf = (*fi);
    gf._model = window->load_model(models, gf._filename);
    if (!gf._model.is_empty()) {
      grid_count += gf._count;
    }
  }

  info_arr.clear();
  info_arr.reserve(grid_count);

  // Compute the integer square root of grid_count, so that we put our
  // models in a nice square grid.
      
  gridwidth=1;
  while(gridwidth*gridwidth < grid_count) {
    gridwidth++;
  }
  
  grid_pos_offset = -gridwidth*GRIDCELLSIZE/2.0;
  wander_area_pos_offset = -max(fabs(grid_pos_offset), MIN_WANDERAREA_DIMENSION/2.0f);

  // Now walk through the list again, copying models into the scene
  // graph as we go.
  
  float xpos = grid_pos_offset;
  float ypos = grid_pos_offset;
      
  srand( (unsigned)time( NULL ) );
  double now = ClockObject::get_global_clock()->get_frame_time();

  int model_count = 0;
  int passnum = 0;
  bool loaded_any;

  NodePath render = window->get_render();
  do {
    loaded_any = false;

    for (fi = filenames.begin(); fi != filenames.end(); ++fi) {
      const GriddedFilename &gf = (*fi);
      if (!gf._model.is_empty() && gf._count > passnum) {
        loaded_any = true;
        // Copy this model into the scene graph, and assign it a
        // position on the grid.

        string model_name = format_string(++model_count);
        NodePath model = render.attach_new_node(model_name);
        gf._model.copy_to(model);

        gridded_file_info info;
        info.node = model.node();

        LMatrix4f xfm_mat,tmat1,tmat2;

        if(gridmotiontype==Rotation) {

#define MIN_REVOLUTION_ANGVEL 30
#define MAX_REVOLUTION_ANGVEL 60

#define MIN_ROTATION_ANGVEL 30
#define MAX_ROTATION_ANGVEL 600

#define MAX_RADIUS 4.0*GRIDCELLSIZE
#define MIN_RADIUS 0.1*GRIDCELLSIZE

          info.starttime = now;

          info.xcenter=xpos;
          info.ycenter=ypos;
          info.ang1=RANDFRAC * 360.0;
          info.ang1_vel=((MAX_REVOLUTION_ANGVEL-MIN_REVOLUTION_ANGVEL) * RANDFRAC) + MIN_REVOLUTION_ANGVEL;

          info.ang2=RANDFRAC * 360.0;
          info.ang2_vel=((MAX_ROTATION_ANGVEL-MIN_ROTATION_ANGVEL) * RANDFRAC) + MIN_ROTATION_ANGVEL;

          info.radius = (RANDFRAC * (MAX_RADIUS-MIN_RADIUS)) + MIN_RADIUS;

          if(RANDFRAC>0.5) {
            info.ang1_vel=-info.ang1_vel;
          }

          if(RANDFRAC>0.5) {
            info.ang2_vel=-info.ang2_vel;
          }

          // xforms happen left to right
          LVector2f new_center = LVector2f(info.radius,0.0) *
            LMatrix3f::rotate_mat(info.ang1);

          const LVector3f rotate_axis(0.0, 0.0, 1.0);

          LVector3f translate_vec(xpos+new_center._v.v._0,
                                  ypos+new_center._v.v._1,
                                  0.0);

          LMatrix4f::rotate_mat_normaxis(info.ang2,rotate_axis,tmat1);
          tmat2 = LMatrix4f::translate_mat(translate_vec);
          xfm_mat = tmat1 * tmat2;
        } else if(gridmotiontype==LinearMotion) {

#define MIN_VEL 2.0
#define MAX_VEL (fabs(wander_area_pos_offset))

          info.vel=((MAX_VEL-MIN_VEL) * RANDFRAC) + MIN_VEL;

          info.xstart=xpos;
          info.ystart=ypos;

          info.xend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;
          info.yend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;

          info.starttime = now;

          float xdel = info.xdel = info.xend-info.xstart;
          float ydel = info.ydel = info.yend-info.ystart;

          info.endtime = csqrt(xdel*xdel+ydel*ydel)/info.vel;

          info.timedel = info.endtime - info.starttime;

          const LVector3f rotate_axis(0.0, 0.0, 1.0);
          float ang = rad_2_deg(atan2(-xdel,ydel));

          LMatrix4f::rotate_mat_normaxis(ang,rotate_axis,info.rotmat);

          LVector3f translate_vec(xpos, ypos, 0.0);
          LMatrix4f tmat2 = LMatrix4f::translate_mat(translate_vec);

          xfm_mat = info.rotmat * tmat2;
        } else {
          LVector3f translate_vec(xpos, ypos, 0.0);
          xfm_mat = LMatrix4f::translate_mat(translate_vec);
        }

        info.node->set_transform(TransformState::make_mat(xfm_mat));

        info_arr.push_back(info);

        if((model_count % gridwidth) == 0) {
          xpos= -gridwidth*GRIDCELLSIZE/2.0;
          ypos+=GRIDCELLSIZE;
        } else {
          xpos+=GRIDCELLSIZE;
        }
      }
    }

    passnum++;
  } while (loaded_any);

  // Finally, remove the source models we loaded up.  Not a real big deal.
  for (fi = filenames.begin(); fi != filenames.end(); ++fi) {
    GriddedFilename &gf = (*fi);
    if (!gf._model.is_empty()) {
      gf._model.remove_node();
    }
  }
}

int
main(int argc, char *argv[]) {
  PandaFramework framework;
  vector_string args;
  framework.open_framework(argc, argv);
  framework.set_window_title("Gridded Object Viewer");

  if (!get_command_line_opts(argc, argv)) {
    return (1);
  }

  // Extract the remaining arguments into two lists of files: those
  // with a grid parameter, and those without.
  pvector<Filename> static_filenames;
  GriddedFilenames gridded_filenames;
  get_command_line_filenames(argc, argv, static_filenames, gridded_filenames);

  WindowFramework *window = framework.open_window();
  if (window != (WindowFramework *)NULL) {
    // We've successfully opened a window.

    window->enable_keyboard();
    window->setup_trackball();
    window->load_models(window->get_render(), static_filenames);

    GriddedInfoArray info_arr;
    load_gridded_models(window, gridded_filenames, info_arr);

    window->loop_animations();

    framework.enable_default_keys();

    while (framework.do_frame()) {
      if (!info_arr.empty() && gridmotiontype) {
        move_gridded_stuff(gridmotiontype, &info_arr[0], info_arr.size());
      }
    }
  }

  framework.report_frame_rate(nout);
  return (0);
}
