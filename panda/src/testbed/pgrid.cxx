/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgrid.cxx
 * @author drose
 * @date 2002-04-03
 */

#include "pandaFramework.h"
#include "pandaNode.h"
#include "transformState.h"
#include "clockObject.h"
#include "string_utils.h"
#include "pvector.h"
#include "panda_getopt.h"
#include "preprocess_argv.h"

#define RANDFRAC (rand()/(PN_stdfloat)(RAND_MAX))

using std::string;

class GriddedFilename {
public:
  Filename _filename;
  int _count;
  NodePath _model;
};
typedef pvector<GriddedFilename> GriddedFilenames;

typedef struct {
  // for rot moving
  PN_stdfloat xcenter,ycenter;
  PN_stdfloat xoffset,yoffset;
  PN_stdfloat ang1,ang1_vel;
  PN_stdfloat ang2,ang2_vel;

  PN_stdfloat radius;

  // for moving
  PN_stdfloat xstart,ystart;
  PN_stdfloat xend,yend;
  PN_stdfloat xdel,ydel,timedel;
  double starttime,endtime;
  double vel;
  LMatrix4 rotmat;

  PandaNode *node;
} gridded_file_info;

typedef pvector<gridded_file_info> GriddedInfoArray;

typedef enum {None,Rotation,LinearMotion} GriddedMotionType;

#define GRIDCELLSIZE 5.0
static int gridwidth;  // cells/side

#define MIN_WANDERAREA_DIMENSION 120.0f

static PN_stdfloat grid_pos_offset;  // origin of grid
static PN_stdfloat wander_area_pos_offset;

static GriddedMotionType gridmotiontype = None;


// making these fns to get around ridiculous VC++ matrix inlining bugs at Opt2
static void
move_gridded_stuff(GriddedMotionType gridmotiontype,
                   gridded_file_info *InfoArr, int size) {
  double now = ClockObject::get_global_clock()->get_frame_time();

  LMatrix4 tmat1,tmat2,xfm_mat;

  for(int i = 0; i < size; i++) {
    double time_delta = (now-InfoArr[i].starttime);
#define DO_FP_MODULUS(VAL,MAXVAL)  \
    {if(VAL > MAXVAL) {int idivresult = (int)(VAL / (PN_stdfloat)MAXVAL);  VAL=VAL-idivresult*MAXVAL;} else  \
    if(VAL < -MAXVAL) {int idivresult = (int)(VAL / (PN_stdfloat)MAXVAL);  VAL=VAL+idivresult*MAXVAL;}}

    // probably should use panda lerps for this stuff, but I don't understand
    // how

    if(gridmotiontype==Rotation) {

      InfoArr[i].ang1=time_delta*InfoArr[i].ang1_vel;
      DO_FP_MODULUS(InfoArr[i].ang1,360.0);
      InfoArr[i].ang2=time_delta*InfoArr[i].ang2_vel;
      DO_FP_MODULUS(InfoArr[i].ang2,360.0);

      // xforms happen left to right
      LVector2 new_center = LVector2(InfoArr[i].radius,0.0) *
        LMatrix3::rotate_mat(InfoArr[i].ang1);

      LVector3 translate_vec(InfoArr[i].xcenter+new_center._v.v._0,
                              InfoArr[i].ycenter+new_center._v.v._1,
                              0.0);

      const LVector3 rotation_axis(0.0, 0.0, 1.0);

      tmat1 = LMatrix4::rotate_mat_normaxis(InfoArr[i].ang2,rotation_axis);
      tmat2 = LMatrix4::translate_mat(translate_vec);
      xfm_mat = tmat1 * tmat2;
    } else {

      PN_stdfloat xpos,ypos;

      if(now>InfoArr[i].endtime) {
        InfoArr[i].starttime = now;

        xpos = InfoArr[i].xstart = InfoArr[i].xend;
        ypos = InfoArr[i].ystart = InfoArr[i].yend;

        InfoArr[i].xend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;
        InfoArr[i].yend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;

        PN_stdfloat xdel = InfoArr[i].xdel = InfoArr[i].xend-InfoArr[i].xstart;
        PN_stdfloat ydel = InfoArr[i].ydel = InfoArr[i].yend-InfoArr[i].ystart;

        InfoArr[i].endtime = now + csqrt(xdel*xdel+ydel*ydel)/InfoArr[i].vel;
        InfoArr[i].timedel = InfoArr[i].endtime - InfoArr[i].starttime;

        const LVector3 rotate_axis(0.0, 0.0, 1.0);

        PN_stdfloat ang = rad_2_deg(atan2(-xdel,ydel));

        InfoArr[i].rotmat= LMatrix4::rotate_mat_normaxis(ang,rotate_axis);
      } else {
        PN_stdfloat timefrac= time_delta/InfoArr[i].timedel;

        xpos = InfoArr[i].xdel*timefrac+InfoArr[i].xstart;
        ypos = InfoArr[i].ydel*timefrac+InfoArr[i].ystart;
      }

      LVector3 translate_vec(xpos, ypos, 0.0);
      LMatrix4 tmat2 = LMatrix4::translate_mat(translate_vec);

      xfm_mat = InfoArr[i].rotmat * tmat2;
    }
    InfoArr[i].node->set_transform(TransformState::make_mat(xfm_mat));
  }
}

bool
get_command_line_opts(int &argc, char **&argv) {
  // Use getopt() to decode the optional command-line parameters.  extern char
  // *optarg;
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
  for (int i = 1; i < argc && argv[i] != nullptr; i++) {
    const string &arg = argv[i];
    size_t comma = arg.find(',');
    if (comma == string::npos) {
      // No comma in the filename, so it must be an ordinary static file.
      static_filenames.push_back(Filename::from_os_specific(arg));

    } else {
      // A comma in the filename indicates a gridded file.  The syntax is
      // filename,count where count represents the number of times the file is
      // repeated.
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
  // Load up all the files indicated in the list of gridded filenames and
  // store them in the given vector.

  Loader loader;
  LoaderOptions options;
  // options.set_flags(options.get_flags() | LoaderOptions::LF_no_ram_cache);

  // First, load up each model from disk once, and store them all separate
  // from the scene graph.  Also count up the total number of models we'll be
  // putting in the grid.
  int grid_count = 0;
  GriddedFilenames::iterator fi;
  for (fi = filenames.begin(); fi != filenames.end(); ++fi) {
    GriddedFilename &gf = (*fi);
    PT(PandaNode) node = loader.load_sync(gf._filename, options);
    if (node != nullptr) {
      gf._model = NodePath(node);
      grid_count += gf._count;
    }
  }

  info_arr.clear();
  info_arr.reserve(grid_count);

  // Compute the integer square root of grid_count, so that we put our models
  // in a nice square grid.

  gridwidth=1;
  while(gridwidth*gridwidth < grid_count) {
    gridwidth++;
  }

  grid_pos_offset = -gridwidth*GRIDCELLSIZE/2.0;
  wander_area_pos_offset = -std::max((PN_stdfloat)fabs(grid_pos_offset), MIN_WANDERAREA_DIMENSION/2.0f);

  // Now walk through the list again, copying models into the scene graph as
  // we go.

  PN_stdfloat xpos = grid_pos_offset;
  PN_stdfloat ypos = grid_pos_offset;

  srand( (unsigned)time( nullptr ) );
  double now = ClockObject::get_global_clock()->get_frame_time();

  int model_count = 0;
  int passnum = 0;
  bool loaded_any;

  NodePath root = window->get_panda_framework()->get_models();
  do {
    loaded_any = false;

    for (fi = filenames.begin(); fi != filenames.end(); ++fi) {
      const GriddedFilename &gf = (*fi);
      if (!gf._model.is_empty() && gf._count > passnum) {
        loaded_any = true;
        // Copy this model into the scene graph, and assign it a position on
        // the grid.

        ++model_count;
        PT(PandaNode) node = loader.load_sync(gf._filename, options);
        NodePath model;
        if (node == nullptr) {
          model = gf._model.copy_to(NodePath());
        } else {
          model = NodePath(node);
        }
        model.reparent_to(root);

        gridded_file_info info;
        info.node = model.node();

        LMatrix4 xfm_mat,tmat1,tmat2;

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
          LVector2 new_center = LVector2(info.radius,0.0) *
            LMatrix3::rotate_mat(info.ang1);

          const LVector3 rotate_axis(0.0, 0.0, 1.0);

          LVector3 translate_vec(xpos+new_center._v.v._0,
                                  ypos+new_center._v.v._1,
                                  0.0);

          tmat1.set_rotate_mat_normaxis(info.ang2,rotate_axis);
          tmat2 = LMatrix4::translate_mat(translate_vec);
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

          PN_stdfloat xdel = info.xdel = info.xend-info.xstart;
          PN_stdfloat ydel = info.ydel = info.yend-info.ystart;

          info.endtime = csqrt(xdel*xdel+ydel*ydel)/info.vel;

          info.timedel = info.endtime - info.starttime;

          const LVector3 rotate_axis(0.0, 0.0, 1.0);
          PN_stdfloat ang = rad_2_deg(atan2(-xdel,ydel));

          info.rotmat.set_rotate_mat_normaxis(ang,rotate_axis);

          LVector3 translate_vec(xpos, ypos, 0.0);
          LMatrix4 tmat2 = LMatrix4::translate_mat(translate_vec);

          xfm_mat = info.rotmat * tmat2;
        } else {
          LVector3 translate_vec(xpos, ypos, 0.0);
          xfm_mat = LMatrix4::translate_mat(translate_vec);
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
}

int
main(int argc, char **argv) {
  preprocess_argv(argc, argv);
  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("Gridded Object Viewer");

  if (!get_command_line_opts(argc, argv)) {
    return (1);
  }

  // Extract the remaining arguments into two lists of files: those with a
  // grid parameter, and those without.
  pvector<Filename> static_filenames;
  GriddedFilenames gridded_filenames;
  get_command_line_filenames(argc, argv, static_filenames, gridded_filenames);

  WindowFramework *window = framework.open_window();
  if (window != nullptr) {
    // We've successfully opened a window.

    window->enable_keyboard();
    window->setup_trackball();
    window->load_models(framework.get_models(), static_filenames);
    framework.get_models().instance_to(window->get_render());

    GriddedInfoArray info_arr;
    load_gridded_models(window, gridded_filenames, info_arr);

    window->loop_animations();
    window->stagger_animations();
    window->center_trackball(framework.get_models());

    framework.enable_default_keys();

    Thread *current_thread = Thread::get_current_thread();
    while (framework.do_frame(current_thread)) {
      if (!info_arr.empty() && gridmotiontype) {
        move_gridded_stuff(gridmotiontype, &info_arr[0], info_arr.size());
      }
    }
  }

  framework.report_frame_rate(nout);
  return (0);
}
