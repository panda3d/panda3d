// Filename: softToEggConverter.cxx
// Created by:  masad (25Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2003, Disney Enterprises, Inc.  All rights reserved
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


#include "softToEggConverter.h"
#include "config_softegg.h"
#include "softEggGroupUserData.h"

#include "eggData.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggNurbsSurface.h"
#include "eggNurbsCurve.h"
#include "eggPolygon.h"
#include "eggPrimitive.h"
#include "eggTexture.h"
#include "eggTextureCollection.h"
#include "eggXfmSAnim.h"
#include "string_utils.h"
#include "dcast.h"

SoftToEggConverter stec;

static const int    TEX_PER_MAT = 1;

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftToEggConverter::
SoftToEggConverter(const string &program_name) :
  _program_name(program_name)
{
  _from_selection = false;
  _polygon_output = false;
  _polygon_tolerance = 0.01;
  /*
  _respect_maya_double_sided = maya_default_double_sided;
  _always_show_vertex_color = maya_default_vertex_color;
  */
  _transform_type = TT_model;

  database_name = NULL;
  scene_name = NULL;
  model_name = NULL;
  animFileName = NULL;
  eggFileName = NULL;
  tex_path = NULL;
  eggGroupName = NULL;
  tex_filename = NULL;
  search_prefix = NULL;
  result = SI_SUCCESS;
  
  // skeleton = new EggGroup();
  foundRoot = FALSE;
  //  animRoot = NULL;
  //  morphRoot = NULL;
  geom_as_joint = 0;
  make_anim = 0;
  make_nurbs = 0;
  make_poly = 0;
  make_soft = 0;
  make_morph = 1;
  make_duv = 1;
  make_dart = TRUE;
  has_morph = 0;
  make_pose = 0;
  //  animData.is_z_up = FALSE;
  nurbs_step = 1;
  anim_start = -1000;
  anim_end = -1000;
  anim_rate = 24;
  pose_frame = -1;
  verbose = 0;
  flatten = 0;
  shift_textures = 0;
  ignore_tex_offsets = 0;
  use_prefix = 0;
}

////////////////////////////////////////////////////////////////////

//     Function: SoftToEggConverter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftToEggConverter::
SoftToEggConverter(const SoftToEggConverter &copy) :
  _from_selection(copy._from_selection),
  /*
  _maya(copy._maya),
  */
  _polygon_output(copy._polygon_output),
  _polygon_tolerance(copy._polygon_tolerance),
  /*
  _respect_maya_double_sided(copy._respect_maya_double_sided),
  _always_show_vertex_color(copy._always_show_vertex_color),
  */
  _transform_type(copy._transform_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SoftToEggConverter::
~SoftToEggConverter() {
  /*
  close_api();
  */
}
////////////////////////////////////////////////////////////////////
//     Function: Help
//       Access: Public
//  Description: Displays the "what is this program" message, along
//               with the usage message.  Should be overridden in base
//               classes to describe the current program.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
Help() 
{
    softegg_cat.info() <<
      "soft2egg takes a SoftImage scene or model\n"
      "and outputs its contents as an egg file\n";
    
    Usage();
}

////////////////////////////////////////////////////////////////////
//     Function: Usage
//       Access: Public
//  Description: Displays the usage message.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
Usage() {
  softegg_cat.info()
    << "\nUsage:\n"
    //    << _commandName << " [opts] (must specify -m or -s)\n\n"
    << "soft" << " [opts] (must specify -m or -s)\n\n"
    << "Options:\n";
  
  ShowOpts();
  softegg_cat.info() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ShowOpts
//       Access: Public
//  Description: Displays the valid options.  Should be extended in
//               base classes to show additional options relevant to
//               the current program.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
ShowOpts() 
{
  softegg_cat.info() <<
    "  -r <path>  - Used to provide soft with the resource\n" 
    "               Defaults to '/ful/ufs/soft371_mips2/3D/rsrc'.\n"
    "  -d <path>  - Database path.\n"
    "  -s <scene> - Indicates that a scene will be converted.\n"
    "  -m <model> - Indicates that a model will be converted.\n"
    "  -t <path>  - Specify path to place converted textures.\n"
    "  -T <name>  - Specify filename for texture map listing.\n"
    "  -S <step>  - Specify step for nurbs surface triangulation.\n"
    "  -M <name>  - Specify model output filename. Defaults to scene name.\n"
    "  -A <name>  - Specify anim output filename. Defaults to scene name.\n"
    "  -N <name>  - Specify egg group name.\n"
    "  -k         - Enable soft assignment for geometry.\n"
    "  -n         - Specify egg NURBS representation instead of poly's.\n"
    "  -p         - Specify egg polygon output for geometry.\n"
    "  -P <frame> - Specify frame number for static pose.\n"
    "  -b <frame> - Specify starting frame for animation (default = first).\n"
    "  -e <frame> - Specify ending frame for animation (default = last).\n"
    "  -f <fps>   - Specify frame rate for animation playback.\n"
    "  -a         - Compile animation tables if animation present.\n"
    "  -F         - Ignore hierarchy and build a completely flat skeleton.\n"
    "  -v <level> - Set debug level.\n"
    "  -x         - Shift NURBS parameters to preserve Alias textures.\n"
    "  -i         - Ignore Soft texture uv offsets.\n"
    "  -u         - Use Soft prefix in model names.\n"
    "  -c         - Cancel morph conversion.\n"
    "  -C         - Cancel duv conversion.\n"
    "  -D         - Don't make the output model a character.\n"
    "  -o <prefix>- Convert only models with given prefix.\n";

  //  EggBase::ShowOpts();
}

////////////////////////////////////////////////////////////////////
//     Function: DoGetopts
//       Access: Public
//  Description: Calls getopt() to parse the command-line switches.
//               Calls HandleGetopts() to interpret each switch.
//               Returns true if the parsing was successful; false if
//               there was an error.  Adjusts argc and argv to remove
//               the switches from the parameter list.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
DoGetopts(int &argc, char **&argv) {
  bool okflag = true;
  int i = 1;
  softegg_cat.info() << "argc " << argc << "\n";
  if (argc <2) {
    Usage();
    okflag = false;
  }
  while ((i < argc-1) && (argv[i][0] == '-') && okflag) {
    softegg_cat.info() << "arg " << i << " is " << argv[i] << "\n";
    //    softegg_cat.info() << argv[i] << ", " << argv[i+1];
    okflag = HandleGetopts(i, argc, argv);
  }
  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: HandleGetopts
//       Access: Public
//  Description: increment idx based on what kind of option parsed
//               Supported options are as follows:
//               r:d:s:m:t:P:b:e:f:T:S:M:A:N:v:o:FhknpaxiucCD
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
HandleGetopts(int &idx, int argc, char **argv) 
{
  bool okflag = true;

  char flag = argv[idx][1];    // skip the '-' from option
  
  switch (flag) 
    {
    case 'r':       // Set the resource path for soft.
      if ( strcmp( argv[idx+1], "" ) ) {
        // Get the path.
        rsrc_path = argv[idx+1];
        softegg_cat.info() << "using rsrc path " << rsrc_path << "\n";
      }
      ++idx;
      break;

    case 'd':       // Set the database path.
      if ( strcmp( argv[idx+1], "" ) ) {
        // Get the path.
        database_name = argv[idx+1];
        softegg_cat.info() << "using database " << database_name << "\n";
      }
      ++idx;
      break;
      
    case 's':     // Check if its a scene.
      if ( strcmp( argv[idx+1], "" ) ) {
        // Get scene name.
        scene_name = argv[idx+1];
        softegg_cat.info() << "loading scene " << scene_name << "\n";
      }
      ++idx;
      break;
      
    case 'm':     // Check if its a model.
      if ( strcmp( argv[idx+1], "" ) ) {
        // Get model name.
        model_name = argv[idx+1];
        softegg_cat.info() << "loading model %s\n" <<  model_name;
      }
      ++idx;
      break;
      
    case 't':     // Get converted texture path.
      if ( strcmp( argv[idx+1], "" ) ) {
        // Get tex path name.
        tex_path = argv[idx+1];
        softegg_cat.info() << "texture path:  %s\n" << tex_path;
      }
      ++idx;
      break;
      
    case 'T':      // Specify texture list filename. 
      if ( strcmp( argv[idx+1], "") ) {
        // Get the name.
        tex_filename = argv[idx+1];
        softegg_cat.info() << "creating texture list file: %s\n" << tex_filename;
      }
      ++idx;
      break;

    case 'S':     // Set NURBS step.
      if ( strcmp( argv[idx+1], "" ) ) {
        nurbs_step = atoi(argv[idx+1]);
        softegg_cat.info() << "NURBS step:  %d\n" << nurbs_step;
      }
      ++idx;
      break;
      
    case 'M':     // Set model output file name.
      if ( strcmp( argv[idx+1], "" ) ) {
        eggFileName = argv[idx+1];
        softegg_cat.info() << "Model output filename:  %s\n" << eggFileName;
      }
      ++idx;
      break;
      
    case 'A':     // Set anim output file name.
      if ( strcmp( argv[idx+1], "" ) ) {
        animFileName = argv[idx+1];
        softegg_cat.info() << "Anim output filename:  %s\n" << animFileName;
      }
      ++idx;
      break;
      
    case 'N':     // Set egg model name.
      if ( strcmp( argv[idx+1], "" ) ) {
        eggGroupName = argv[idx+1];
        softegg_cat.info() << "Egg group name:  %s\n" << eggGroupName;
      }
      ++idx;
      break;
      
    case 'o':     // Set search_prefix.
      if ( strcmp( argv[idx+1], "" ) ) {
        search_prefix = argv[idx+1]; 
        softegg_cat.info() << "Only converting models with prefix:  %s\n" << search_prefix;
      }
      ++idx;
      break;
      
    case 'h':    // print help message
      Help();
      exit(1);
      break;
      
    case 'c':    // Cancel morph animation conversion
      make_morph = FALSE;
      softegg_cat.info() << "canceling morph conversion\n";
      break;

    case 'C':    // Cancel uv animation conversion
      make_duv = FALSE;
      softegg_cat.info() << "canceling uv animation conversion\n";
      break;
      
    case 'D':    // Omit the Dart flag
      make_dart = FALSE;
      softegg_cat.info() << "making a non-character model\n";
      break;
      
    case 'k':    // Enable soft skinning
      //make_soft = TRUE;
      //fprintf( outStream, "enabling soft skinning\n" );
      softegg_cat.info() << "-k flag no longer necessary\n";
      break;
      
    case 'n':    // Generate egg NURBS output
      make_nurbs = TRUE;
      softegg_cat.info() << "outputting egg NURBS info\n";
      break;
      
    case 'p':    // Generate egg polygon output
      make_poly = TRUE;
      softegg_cat.info() << "outputting egg polygon info\n";
      break;
      
    case 'P':    // Generate static pose from given frame
      if ( strcmp( argv[idx+1], "" ) ) {
        make_pose = TRUE;
        pose_frame = atoi(argv[idx+1]);
        softegg_cat.info() << "generating static pose from frame %d\n" << pose_frame;
      }
      ++idx;
      break;
      
    case 'a':     // Compile animation tables.
      make_anim = TRUE;
      softegg_cat.info() << "attempting to compile anim tables\n";
      break;
      
    case 'F':     // Build a flat skeleton.
      flatten = TRUE;
      softegg_cat.info() << "building a flat skeleton!!!\n";
      break;
      
    case 'x':     // Shift NURBS parameters to preserve Alias textures.
      shift_textures = TRUE;
      softegg_cat.info() << "shifting NURBS parameters...\n";
      break;
      
    case 'i':     // Ignore Soft uv texture offsets 
      ignore_tex_offsets = TRUE;
      softegg_cat.info() << "ignoring texture offsets...\n";
      break;
      
    case 'u':     // Use Soft prefix in model names 
      use_prefix = TRUE;
      softegg_cat.info() << "using prefix in model names...\n";
      break;
      
      
    case 'v':     // print debug messages.
      if ( strcmp( argv[idx+1], "" ) ) {
        verbose = atoi(argv[idx+1]);
        softegg_cat.info() << "using debug level %d\n" << verbose;
      }
      ++idx;
      break;
      
    case 'b':     // Set animation start frame.
      if ( strcmp( argv[idx+1], "" ) ) {
        anim_start = atoi(argv[idx+1]);
        softegg_cat.info() << "animation starting at frame:  %d\n" << anim_start;
      }
      break;
      
    case 'e':     /// Set animation end frame.
      if ( strcmp( argv[idx+1], "" ) ) {
        anim_end = atoi(argv[idx+1]);
        softegg_cat.info() << "animation ending at frame:  %d\n" << anim_end;
      }
      ++idx;
      break;
      
    case 'f':     /// Set animation frame rate.
      if ( strcmp( argv[idx+1], "" ) ) {
        anim_rate = atoi(argv[idx+1]);
        softegg_cat.info() << "animation frame rate:  %d\n" << anim_rate;
      }
      ++idx;
      break;

    default:
      softegg_cat.info() << flag << " flag not supported\n";
      okflag = false;
    }
  idx++;
  return (okflag);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the converter.
////////////////////////////////////////////////////////////////////
SomethingToEggConverter *SoftToEggConverter::
make_copy() {
  return new SoftToEggConverter(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::get_name
//       Access: Public, Virtual
//  Description: Returns the English name of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string SoftToEggConverter::
get_name() const {
  return "Soft";
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::get_extension
//       Access: Public, Virtual
//  Description: Returns the common extension of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string SoftToEggConverter::
get_extension() const {
  return "mb";
}

////////////////////////////////////////////////////////////////////
//     Function: GetTextureName
//       Access: Public
//  Description: Given a texture element, return texture name 
//               with given tex_path
////////////////////////////////////////////////////////////////////
char *SoftToEggConverter::
GetTextureName( SAA_Scene *scene, SAA_Elem *texture ) {
  char *fileName = new char[_MAX_PATH];
  char tempName[_MAX_PATH];
  SAA_texture2DGetPicName( scene, texture, _MAX_PATH, tempName );

  if (tex_path) {
    //  cout << "tempName :" << tempName << endl;
    strcpy(fileName, tex_path);

    // do some processing on the name string
    char *tmpName = NULL;
    tmpName = strrchr(tempName, '/');
    if (tmpName)
      tmpName++;
    else
      tmpName = tempName;

    //    cout << "tmpName : " << tmpName << endl;
    strcat(fileName, "/");
    strcat(fileName, tmpName);
  }
  else {
    strcpy(fileName, tempName);
  }

  strcat(fileName, ".pic");
  //  cout << "fileName : " << fileName << endl;

  return fileName;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::convert_file
//       Access: Public, Virtual
//  Description: Handles the reading of the input file and converting
//               it to egg.  Returns true if successful, false
//               otherwise.
//
//               This is designed to be as generic as possible,
//               generally in support of run-time loading.
//               Also see convert_soft().
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
convert_file(const Filename &filename) {
  if (!open_api()) {
    softegg_cat.error()
      << "Soft is not available.\n";
    return false;
  }
  if (_character_name.empty()) {
    _character_name = filename.get_basename_wo_extension();
  }
  return convert_soft(false);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::convert_soft
//       Access: Public
//  Description: Fills up the egg_data structure according to the
//               global soft model data.  Returns true if successful,
//               false if there is an error.  If from_selection is
//               true, the converted geometry is based on that which
//               is selected; otherwise, it is the entire Soft scene.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
convert_soft(bool from_selection) {
  bool all_ok = true;

  _from_selection = from_selection;
  _textures.clear();

  EggData egg_data;
  set_egg_data(&egg_data, false);
  cout << "eggData " << &get_egg_data() << "\n";

  if (_egg_data->get_coordinate_system() != CS_default) {
    cout << "coordinate system is not default\n";
    exit(1);
  }

  _tree._use_prefix = use_prefix;
  _tree._search_prefix = search_prefix;
  all_ok = _tree.build_complete_hierarchy(scene, database);
  //  exit(1);

  char *root_name = _tree.GetRootName( eggFileName );
  cout << "main group name: " << root_name << endl;
  if (root_name)
    _character_name = root_name;
  
  if (make_poly) {
    if (!convert_char_model()) {
      all_ok = false;
    }

    //  reparent_decals(&get_egg_data());
    cout << softegg_cat.info() << "Converted Softimage file\n";

    // write out the egg model file
    _egg_data->write_egg(Filename(eggFileName));
    cout << softegg_cat.info() << "Wrote Egg file " << eggFileName << endl;
  }
  if (make_anim) {
    if (!convert_char_chan()) {
      all_ok = false;
    }

    //  reparent_decals(&get_egg_data());
    cout << softegg_cat.info() << "Converted Softimage file\n";
    
    // write out the egg model file
    _egg_data->write_egg(Filename(animFileName));
    cout << softegg_cat.info() << "Wrote Anim file " << animFileName << endl;
  }
  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::open_api
//       Access: Public
//  Description: Attempts to open the Soft API if it was not already
//               open, and returns true if successful, or false if
//               there is an error.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
open_api() {
  if ((scene_name == NULL && model_name == NULL) || database_name == NULL) {
    Usage();
    exit( 1 );
  }
  if ((result = SAA_Init(rsrc_path, FALSE)) != SI_SUCCESS) {
    softegg_cat.info() << "Error: Couldn't get resource path!\n";
    exit( 1 );
  }
  if ((result = SAA_databaseLoad(database_name, &database)) != SI_SUCCESS) {
    softegg_cat.info() << "Error: Couldn't load database!\n";
    exit( 1 );
  }
  if ((result = SAA_sceneGetCurrent(&scene)) != SI_SUCCESS) {
    softegg_cat.info() << "Error: Couldn't get current scene!\n";
    exit( 1 );
  }
  if ((result = SAA_sceneLoad( &database, scene_name, &scene )) != SI_SUCCESS) {
    softegg_cat.info() << "Error: Couldn't load scene " << scene_name << "!\n";
    exit( 1 );
  }
  if ( SAA_updatelistGet( &scene ) == SI_SUCCESS ) {
    float time;
    
    softegg_cat.info() << "setting Scene to frame " << pose_frame << "...\n";
    //SAA_sceneSetPlayCtrlCurrentFrame( &scene, pose_frame );
    SAA_frame2Seconds( &scene, pose_frame, &time );
    SAA_updatelistEvalScene( &scene, time );
    if ( make_pose )
      SAA_sceneFreeze(&scene);
  } 
  
  // if no egg filename specified, make up a name
  if ( eggFileName == NULL ) {
    string madeName;
    string tempName(scene_name);
    string::size_type end = tempName.find(".dsc");
    if (end != string::npos) {
      madeName.assign(tempName.substr(0,end));
      if ( make_nurbs )
        madeName.insert(madeName.size(), "-nurb");
      madeName.insert(madeName.size(), ".egg" );
    }
    eggFileName = new char[madeName.size()+1];
    strcpy(eggFileName, madeName.c_str());

    // if no anim filename specified, make up a name
    if ( animFileName == NULL ) {
      madeName.assign(tempName.substr(0,end));
      madeName.insert(madeName.size(), "-chan.egg");
      animFileName = new char[strlen(scene_name)+ 10];
      strcpy(animFileName, madeName.c_str());
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::close_api
//       Access: Public
//  Description: Closes the Soft API, if it was previously opened.
//               Caution!  Soft appears to call exit() when its API is
//               closed.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
close_api() {
  // don't know yet
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::convert_char_model
//       Access: Private
//  Description: Converts the file as an animatable character
//               model, with joints and vertex membership.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
convert_char_model() {
#if 0
  if (has_neutral_frame()) {
    MTime frame(get_neutral_frame(), MTime::uiUnit());
    softegg_cat.info(false)
      << "neutral frame " << frame.value() << "\n";
    MGlobal::viewFrame(frame);
  }
#endif
  cout << "character name " << _character_name << "\n";
  EggGroup *char_node = new EggGroup(_character_name);
  get_egg_data().add_child(char_node);
  char_node->set_dart_type(EggGroup::DT_default);

  return convert_hierarchy(char_node);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::convert_char_chan
//       Access: Private
//  Description: Converts the animation as a series of tables to apply
//               to the character model, as retrieved earlier via
//               AC_model.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
convert_char_chan() {
  int start_frame = -1;
  int end_frame = -1;
  int frame_inc, frame;
  double output_frame_rate = anim_rate;
  
  float time;

  EggTable *root_table_node = new EggTable();
  get_egg_data().add_child(root_table_node);
  EggTable *bundle_node = new EggTable(_character_name);
  bundle_node->set_table_type(EggTable::TT_bundle);
  root_table_node->add_child(bundle_node);
  EggTable *skeleton_node = new EggTable("<skeleton>");
  bundle_node->add_child(skeleton_node);

  // Set the frame rate before we start asking for anim tables to be
  // created.
  SAA_sceneGetPlayCtrlStartFrame(&scene, &start_frame);
  SAA_sceneGetPlayCtrlEndFrame(&scene, &end_frame);
  SAA_sceneGetPlayCtrlFrameStep( &scene, &frame_inc );
  
  cout << "animation start frame: " << start_frame << " end frame: " << end_frame << endl;
  cout << "animation frame inc: " << frame_inc << endl;
  
  _tree._fps = output_frame_rate / frame_inc;
  _tree.clear_egg(&get_egg_data(), NULL, skeleton_node);

  // Now we can get the animation data by walking through all of the
  // frames, one at a time, and getting the joint angles at each
  // frame.

  // This is just a temporary EggGroup to receive the transform for
  // each joint each frame.
  PT(EggGroup) tgroup = new EggGroup;

  int num_nodes = _tree.get_num_nodes();
  int i;

  //  MTime frame(start_frame, MTime::uiUnit());
  //  MTime frame_stop(end_frame, MTime::uiUnit());
  // start at first frame and go to last
  for ( frame = start_frame; frame <= end_frame; frame += frame_inc) {
    SAA_frame2Seconds( &scene, frame, &time );
    SAA_updatelistEvalScene( &scene, time );
    cout << "\n> animating frame " << frame << endl;

    //    if (softegg_cat.is_debug()) {
    //      softegg_cat.debug(false)
    softegg_cat.info() << "frame " << time << "\n";
    //} else {
      // We have to write to cerr instead of softegg_cat to allow
      // flushing without writing a newline.
    //      cerr << "." << flush;
    //    }
    //    MGlobal::viewFrame(frame);

    for (i = 0; i < num_nodes; i++) {
      SoftNodeDesc *node_desc = _tree.get_node(i);
      if (node_desc->is_joint()) {
        if (softegg_cat.is_spam()) {
          softegg_cat.spam()
            << "joint " << node_desc->get_name() << "\n";
        }
        EggXfmSAnim *anim = _tree.get_egg_anim(node_desc);
        // following function fills in the anim structure
        node_desc->get_joint_transform(&scene, tgroup, anim);
      }
    }

    //    frame += frame_inc;
  }

  // Now optimize all of the tables we just filled up, for no real
  // good reason, except that it makes the resulting egg file a little
  // easier to read.
  for (i = 0; i < num_nodes; i++) {
    SoftNodeDesc *node_desc = _tree.get_node(i);
    if (node_desc->is_joint()) {
      _tree.get_egg_anim(node_desc)->optimize();
    }
  }

  softegg_cat.info(false)
    << "\n";

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::convert_hierarchy
//       Access: Private
//  Description: Generates egg structures for each node in the Soft
//               hierarchy.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
convert_hierarchy(EggGroupNode *egg_root) {
  int num_nodes = _tree.get_num_nodes();

  _tree.clear_egg(&get_egg_data(), egg_root, NULL);
  cout << "num_nodes = " << num_nodes << endl;
  for (int i = 0; i < num_nodes; i++) {
    if (!process_model_node(_tree.get_node(i))) {
      return false;
    }
    softegg_cat.info() << i << endl;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::process_model_node
//       Access: Private
//  Description: Converts the indicated Soft node (given a MDagPath,
//               similar in concept to Panda's NodePath) to the
//               corresponding Egg structure.  Returns true if
//               successful, false if an error was encountered.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
process_model_node(SoftNodeDesc *node_desc) {
  EggGroup *egg_group = NULL;
  const char *name = NULL;
  char *fullname = NULL;
  SAA_ModelType type;

#if 0
  // Get the name of the model
  if ( use_prefix ) {
    // Get the FULL name of the model
    name = fullname = _tree.GetFullName( &scene, node_desc->get_model() );
  }
  else {
    // Get the name of the trim curve
    name = _tree.GetName( &scene, node_desc->get_model() );
  }
#endif
  name = node_desc->get_name().c_str();
  cout << "element name <" << name << ">\n";

  // find out what type of node we're dealing with
  result = SAA_modelGetType( &scene, node_desc->get_model(), &type );
  egg_group = _tree.get_egg_group(node_desc);
  cout << "encountered ";
  switch(type){
  case SAA_MNILL:
    cout << "null\n";
    node_desc->get_transform(&scene, egg_group);
    handle_null(node_desc->get_model(), egg_group, type, name);
    break;
  case SAA_MPTCH:
    cout << "patch\n";
    break;
  case SAA_MFACE:
    cout << "face\n";
    break;
  case SAA_MSMSH:
    cout << "mesh\n";
    node_desc->get_transform(&scene, egg_group);
    make_polyset(node_desc, egg_group, type);
    break;
  case SAA_MJNT:
    cout << "joint\n";
    break;
  case SAA_MSPLN:
    cout << "spline\n";
    break;
  case SAA_MMETA:
    cout << "meta element\n";
    break;
  case SAA_MBALL:
    cout << "meta ball\n";
    break;
  case SAA_MNCRV:
    cout << "nurbs curve\n";
    break;
  case SAA_MNSRF:
    cout << "nurbs surf\n";
    break;
  default:
    cout << "unknown type: " << type << "\n";
  }

  if (node_desc->is_joint())
    node_desc->get_transform(&scene, egg_group, TRUE);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::make_polyset
//       Access: Private
//  Description: Converts the indicated Soft polyset to a bunch of
//               EggPolygons and parents them to the indicated egg
//               group.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
make_polyset(SoftNodeDesc *node_desc, EggGroup *egg_group, SAA_ModelType type) {
  string name = node_desc->get_name();
  int id = 0;

  float *uCoords = NULL;
  float *vCoords = NULL;


  SAA_Boolean valid;
  SAA_Boolean visible;

  int i, idx;
  
  SAA_modelGetNodeVisibility( &scene, node_desc->get_model(), &visible ); 
  cout << "model visibility: " << visible << endl; 
  
  ///////////////////////////////////////////////////////////////////////
  // Only create egg polygon data if: the node is visible, and its not
  // a NULL or a Joint, and we're outputing polys (or if we are outputing 
  // NURBS and the model is a poly mesh or a face) 
  ///////////////////////////////////////////////////////////////////////
  if ( visible &&  
       (type != SAA_MNILL) &&
       (type != SAA_MJNT) && 
       ((make_poly || 
         (make_nurbs && ((type == SAA_MSMSH) || (type == SAA_MFACE )) )) ||
        (!make_poly && !make_nurbs && make_duv && 
         ((type == SAA_MSMSH) || (type == SAA_MFACE )) ))
       )
    {
      // load all node data from soft for this node_desc
      node_desc->load_model(&scene, type);

      string vpool_name = name + ".verts";
      EggVertexPool *vpool = new EggVertexPool(vpool_name);
      egg_group->add_child(vpool);
      
      // little detour...bear with me for now...TODO: move these to a new function
      
      // We will need to transform all vertices from world coordinate
      // space into the vertex space appropriate to this node.  Usually,
      // this is the same thing as world coordinate space, and this matrix
      // will be identity; but if the node is under an instance
      // (particularly, for instance, a billboard) then the vertex space
      // will be different from world space.
      LMatrix4d vertex_frame_inv = egg_group->get_vertex_frame_inv();
      
      // Asad: change from soft2egg.c. Here I am trying to get one triangles vertices not all
      for (idx=0; idx<node_desc->numTri; ++idx) {
        EggPolygon *egg_poly = new EggPolygon;
        egg_group->add_child(egg_poly);
        
        // Is this a double sided polygon? meaning check for back face flag
        char *modelNoteStr = _tree.GetModelNoteInfo( &scene, node_desc->get_model() );
        if ( modelNoteStr != NULL ) {
          if ( strstr( modelNoteStr, "bface" ) != NULL )
            egg_poly->set_bface_flag(TRUE);
        }
        
        // read each triangle's control vertices into array
        SAA_SubElem cvertices[3];
        SAA_triangleGetCtrlVertices( &scene, node_desc->get_model(), node_desc->gtype, id, 1, node_desc->triangles+idx, cvertices );
        
        // read control vertices in this triangle
        SAA_DVector cvertPos[3];
        SAA_ctrlVertexGetPositions( &scene, node_desc->get_model(), 3, cvertices, cvertPos);
        
        // read indices of each vertices in this triangle
        int indices[3];
        indices[0] = indices[1] = indices[2] = 0;
        SAA_ctrlVertexGetIndices( &scene, node_desc->get_model(), 3, cvertices, indices );
        
        // read each control vertex's normals into an array
        SAA_DVector normals[3];
        SAA_ctrlVertexGetNormals( &scene, node_desc->get_model(), 3, cvertices, normals );
        for (i=0; i<3; ++i)
          cout << "normals[" << i <<"] = " << normals[i].x << " " <<  normals[i].y
               << " " << normals[i].z << " " <<  normals[i].w << "\n";
        
        // allocate arrays for u & v coords
        if (node_desc->textures) {
          if (node_desc->numTexLoc) {
            // allocate arrays for u & v coords
            // I think there are one texture per triangle hence we need only 3 corrdinates
            uCoords = new float[3];
            vCoords = new float[3];
            
            // read the u & v coords into the arrays
            if ( uCoords != NULL && vCoords != NULL) {
              for ( i = 0; i < 3; i++ )
                uCoords[i] = vCoords[i] = 0.0f;
              
              // TODO: investigate the coord_cnt parameter...
              SAA_ctrlVertexGetUVTxtCoords( &scene, node_desc->get_model(), 3, cvertices,
                                            3, uCoords, vCoords );
            }
            else
              cout << "Not enough Memory for texture coords...\n";
            
#if 1
            for ( i=0; i<3; i++ )
              cout << "texcoords[" << i << "] = ( " << uCoords[i] << " , " << vCoords[i] <<" )\n";
#endif
          }
          else if (node_desc->numTexGlb) {
            // allocate arrays for u & v coords
            uCoords = new float[node_desc->numTexGlb*3];
            vCoords = new float[node_desc->numTexGlb*3];
            
            for ( i = 0; i < node_desc->numTexGlb*3; i++ ) {
              uCoords[i] = vCoords[i] = 0.0f;
            }                
            
            // read the u & v coords into the arrays
            if ( uCoords != NULL && vCoords != NULL) {
              SAA_triCtrlVertexGetGlobalUVTxtCoords( &scene, node_desc->get_model(), 3, cvertices, 
                                                     node_desc->numTexGlb, node_desc->textures, uCoords, vCoords );
            }
            else
              cout << "Not enough Memory for texture coords...\n";
          }
        }
        
        for ( i=0; i < 3; i++ ) {
          EggVertex vert;
          
          // There are some conversions needed from local matrix to global coords
          SAA_DVector local = cvertPos[i];
          SAA_DVector global = {0};
          
          _VCT_X_MAT( global, local, node_desc->matrix );
          
          cout << "indices[" << i << "] = " << indices[i] << "\n";
          cout << "cvert[" << i << "] = " << cvertPos[i].x << " " << cvertPos[i].y
               << " " << cvertPos[i].z << " " << cvertPos[i].w << "\n";
          cout << " global cvert[" << i << "] = " << global.x << " " << global.y
               << " " << global.z << " " << global.w << "\n";
          
          //      LPoint3d p3d(cvertPos[i].x, cvertPos[i].y, cvertPos[i].z);
          LPoint3d p3d(global.x, global.y, global.z);
          p3d = p3d * vertex_frame_inv;
          vert.set_pos(p3d);
          
          local = normals[i];
          _VCT_X_MAT( global, local, node_desc->matrix );
          
          cout << "normals[" << i <<"] = " << normals[i].x << " " <<  normals[i].y
               << " " << normals[i].z << " " <<  normals[i].w << "\n";
          cout << " global normals[" << i <<"] = " << global.x << " " <<  global.y
               << " " << global.z << " " <<  global.w << "\n";
          
          LVector3d n3d(global.x, global.y, global.z);
          n3d = n3d * vertex_frame_inv;
          vert.set_normal(n3d);
          
          // check to see if material is present
          float r,g,b,a;
          SAA_elementIsValid( &scene, &node_desc->materials[idx/3], &valid );
          // material present - get the color 
          if ( valid ) {
            SAA_materialGetDiffuse( &scene, &node_desc->materials[idx/3], &r, &g, &b );
            SAA_materialGetTransparency( &scene, &node_desc->materials[idx/3], &a );
            vert.set_color(Colorf(r, g, b, 1.0));
            cout << "color r = " << r << " g = " << g << " b = " << b << " a = " << a << "\n";
          }
          else {     // no material - default to white
            vert.set_color(Colorf(1.0, 1.0, 1.0, 1.0));
            cout << "default color\n";
          }
          
          // if texture present set the texture coordinates
          if (node_desc->textures) {
            float u, v;
            
            u = uCoords[i];
            v = 1.0f - vCoords[i];
            cout << "texcoords[" << i << "] = " << u << " " 
                 << v << endl;
            
            vert.set_uv(TexCoordd(u, v));
            //        vert.set_uv(TexCoordd(uCoords[i], vCoords[i]));
          }
          vert.set_external_index(indices[i]);
          egg_poly->add_vertex(vpool->create_unique_vertex(vert));
          cout << "\n";
        }
        
        // Now apply the shader.
        if (node_desc->textures != NULL) {
          if (node_desc->numTexLoc) {
            if (!strstr(node_desc->texNameArray[idx], "noIcon"))
              set_shader_attributes(node_desc, *egg_poly, node_desc->texNameArray[idx]);
            else
              cout << "texname :" << node_desc->texNameArray[idx] << endl;
          }
          else {
            if (!strstr(node_desc->texNameArray[0], "noIcon"))
              set_shader_attributes(node_desc, *egg_poly, node_desc->texNameArray[0]);
            else 
              cout << "texname :" << node_desc->texNameArray[0] << endl;
        }
      }
      }
#if 0
  come back to it later
  // Now that we've added all the polygons (and created all the
  // vertices), go back through the vertex pool and set up the
  // appropriate joint membership for each of the vertices.
  bool got_weights = false;

  pvector<EggGroup *> joints;
  MFloatArray weights;
  if (_animation_convert == AC_model) {
    got_weights = 
      get_vertex_weights(dag_path, mesh, joints, weights);
  }

  if (got_weights && !joints.empty()) {
    int num_joints = joints.size();
    int num_weights = (int)weights.length();
    int num_verts = num_weights / num_joints;
    // The number of weights should be an even multiple of verts *
    // joints.
    nassertv(num_weights == num_verts * num_joints);

    EggVertexPool::iterator vi;
    for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
      EggVertex *vert = (*vi);
      int maya_vi = vert->get_external_index();
      nassertv(maya_vi >= 0 && maya_vi < num_verts);

      for (int ji = 0; ji < num_joints; ++ji) {
        float weight = weights[maya_vi * num_joints + ji];
        if (weight != 0.0f) {
          EggGroup *joint = joints[ji];
          if (joint != (EggGroup *)NULL) {
            joint->ref_vertex(vert, weight);
          }
        }
      }
    }
  }
#endif
    }
}
////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::make_polyset
//       Access: Private
//  Description: Converts the indicated Soft polyset to a bunch of
//               EggPolygons and parents them to the indicated egg
//               group.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
handle_null(SAA_Elem *model, EggGroup *egg_group, SAA_ModelType type, const char *node_name) {
  const char *name = node_name;
  SAA_AlgorithmType    algo;
  
  SAA_modelGetAlgorithm( &scene, model, &algo );
  cout << "null algorithm: " << algo << endl;
  
  if ( algo == SAA_ALG_INV_KIN ) {
    //    MakeJoint( &scene, lastJoint, lastAnim,  model, name );
    cout << "encountered IK root: " << name << endl;
  }
  else if ( algo == SAA_ALG_INV_KIN_LEAF ) {
    //    MakeJoint( &scene, lastJoint, lastAnim, model, name );
    cout << "encountered IK leaf: " << name << endl;
  }
  else if ( algo == SAA_ALG_STANDARD ) {
    SAA_Boolean isSkeleton = FALSE;
    cout << "encountered Standard null: " << name << endl;

    SAA_modelIsSkeleton( &scene, model, &isSkeleton );

    // check to see if this NULL is used as a skeleton
    // or is animated via constraint only ( these nodes are
    // tagged by the animator with the keyword "joint"
    // somewhere in the nodes name)
    if ( isSkeleton || (strstr( name, "joint" ) != NULL) ) {
      //      MakeJoint( &scene, lastJoint, lastAnim, model, name );
      cout << "animating Standard null!!!\n";
    }
  }
  else
    cout << "encountered some other NULL: " << algo << endl;

#if 0 // no need to follow children, _tree already contains all the model nodes
  // check for children...
  int numChildren;
  int thisChild;
  SAA_Elem *children;
  
  SAA_modelGetNbChildren( &scene, model, &numChildren );
  cout << "Model children: " << numChildren << endl;

  if ( numChildren ) {
    children = new SAA_Elem[numChildren];
    SAA_modelGetChildren( &scene, model, numChildren, children );
    if ( children != NULL ) {
      for ( thisChild = 0; thisChild < numChildren; thisChild++ ) {
        cout << "\negging child " << thisChild << "...\n";
        //        MakeEgg( parent, lastJoint, lastAnim, scene, 
        //                 &children[thisChild] );
      }
    }
    else
      cout << "Not enough Memory for children...\n";
  }
  else
    cout << "Don't descend this branch!\n";
#endif
}

#if 0
////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::make_locator
//       Access: Private
//  Description: Locators are used in Soft to indicate a particular
//               position in space to the user or the modeler.  We
//               represent that in egg with an ordinary Group node,
//               which we transform by the locator's position, so that
//               the indicated point becomes the origin at this node
//               and below.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
make_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
             EggGroup *egg_group) {
  MStatus status;

  unsigned int num_children = dag_node.childCount();
  MObject locator;
  bool found_locator = false;
  for (unsigned int ci = 0; ci < num_children && !found_locator; ci++) {
    locator = dag_node.child(ci);
    found_locator = (locator.apiType() == MFn::kLocator);
  }

  if (!found_locator) {
    softegg_cat.error()
      << "Couldn't find locator within locator node " 
      << dag_path.fullPathName().asChar() << "\n";
    return;
  }

  LPoint3d p3d;
  if (!get_vec3d_attribute(locator, "localPosition", p3d)) {
    softegg_cat.error()
      << "Couldn't get position of locator " 
      << dag_path.fullPathName().asChar() << "\n";
    return;
  }

  // We need to convert the position to world coordinates.  For some
  // reason, Soft can only tell it to us in local coordinates.
  MMatrix mat = dag_path.inclusiveMatrix(&status);
  if (!status) {
    status.perror("Can't get coordinate space for locator");
    return;
  }
  LMatrix4d n2w(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
  p3d = p3d * n2w;

  // Now convert the locator point into the group's space.
  p3d = p3d * egg_group->get_node_frame_inv();

  egg_group->add_translate(p3d);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::get_vertex_weights
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
get_vertex_weights(const MDagPath &dag_path, const MFnMesh &mesh,
                   pvector<EggGroup *> &joints, MFloatArray &weights) {
  MStatus status;
  
  // Since we are working with a mesh the input attribute that 
  // creates the mesh is named "inMesh" 
  // 
  MObject attr = mesh.attribute("inMesh"); 
  
  // Create the plug to the "inMesh" attribute then use the 
  // DG iterator to walk through the DG, at the node level.
  // 
  MPlug history(mesh.object(), attr); 
  MItDependencyGraph it(history, MFn::kDependencyNode, 
                        MItDependencyGraph::kUpstream, 
                        MItDependencyGraph::kDepthFirst, 
                        MItDependencyGraph::kNodeLevel);

  while (!it.isDone()) {
    // We will walk along the node level of the DG until we 
    // spot a skinCluster node.
    // 
    MObject c_node = it.thisNode(); 
    if (c_node.hasFn(MFn::kSkinClusterFilter)) { 
      // We've found the cluster handle. Try to get the weight
      // data.
      // 
      MFnSkinCluster cluster(c_node, &status); 
      if (!status) {
        status.perror("MFnSkinCluster constructor");
        return false;
      }

      // Get the set of objects that influence the vertices of this
      // mesh.  Hopefully these will all be joints.
      MDagPathArray influence_objects;
      cluster.influenceObjects(influence_objects, &status); 
      if (!status) {
        status.perror("MFnSkinCluster::influenceObjects");

      } else {
        // Fill up the vector with the corresponding table of egg
        // groups for each joint.
        joints.clear();
        for (unsigned oi = 0; oi < influence_objects.length(); oi++) {
          MDagPath joint_dag_path = influence_objects[oi];
          SoftNodeDesc *joint_node_desc = _tree.build_node(joint_dag_path);
          EggGroup *joint = _tree.get_egg_group(joint_node_desc);
          joints.push_back(joint);
        }

        // Now use a component object to retrieve all of the weight
        // data in one API call.
        MFnSingleIndexedComponent sic; 
        MObject sic_object = sic.create(MFn::kMeshVertComponent); 
        sic.setCompleteData(mesh.numVertices()); 
        unsigned influence_count; 

        status = cluster.getWeights(dag_path, sic_object, 
                                    weights, influence_count); 
        if (!status) {
          status.perror("MFnSkinCluster::getWeights");
        } else {
          if (influence_count != influence_objects.length()) {
            softegg_cat.error()
              << "MFnSkinCluster::influenceObjects() returns " 
              << influence_objects.length()
              << " objects, but MFnSkinCluster::getWeights() reports "
              << influence_count << " objects.\n";
            
          } else {
            // We've got the weights and the set of objects.  That's all
            // we need.
            return true;
          }
        }
      }
    }

    it.next();
  }
  
  softegg_cat.error()
    << "Unable to find a cluster handle for the DG node.\n"; 
  return false;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: SoftShader::set_shader_attributes
//       Access: Private
//  Description: Applies the known shader attributes to the indicated
//               egg primitive.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
set_shader_attributes(SoftNodeDesc *node_desc, EggPrimitive &primitive, char *texName) {
  EggTexture tex(texName, "");

  Filename filename = Filename::from_os_specific(texName);
  Filename fullpath = _path_replace->match_path(filename, get_texture_path());
  tex.set_filename(_path_replace->store_path(fullpath));
  tex.set_fullpath(fullpath);
  //  tex.set_format(EggTexture::F_rgb);
  apply_texture_properties(tex, node_desc->uRepeat, node_desc->vRepeat);

  EggTexture *new_tex = _textures.create_unique_texture(tex, ~EggTexture::E_tref_name);
  primitive.set_texture(new_tex);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftShader::apply_texture_properties
//       Access: Private
//  Description: Applies all the appropriate texture properties to the
//               EggTexture object, including wrap modes and texture
//               matrix.
////////////////////////////////////////////////////////////////////
void SoftToEggConverter::
apply_texture_properties(EggTexture &tex, int uRepeat, int vRepeat) {
  // Let's mipmap all textures by default.
  tex.set_minfilter(EggTexture::FT_linear_mipmap_linear);
  tex.set_magfilter(EggTexture::FT_linear);

  EggTexture::WrapMode wrap_u = uRepeat > 0 ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  EggTexture::WrapMode wrap_v = vRepeat > 0 ? EggTexture::WM_repeat : EggTexture::WM_clamp;

  tex.set_wrap_u(wrap_u);
  tex.set_wrap_v(wrap_v);
  /*  
  LMatrix3d mat = color_def.compute_texture_matrix();
  if (!mat.almost_equal(LMatrix3d::ident_mat())) {
    tex.set_transform(mat);
  }
  */
}
#if 0
////////////////////////////////////////////////////////////////////
//     Function: SoftShader::compare_texture_properties
//       Access: Private
//  Description: Compares the texture properties already on the
//               texture (presumably set by a previous call to
//               apply_texture_properties()) and returns false if they
//               differ from that specified by the indicated color_def
//               object, or true if they match.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
compare_texture_properties(EggTexture &tex, 
                           const SoftShaderColorDef &color_def) {
  bool okflag = true;

  EggTexture::WrapMode wrap_u = color_def._wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  EggTexture::WrapMode wrap_v = color_def._wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  
  if (wrap_u != tex.determine_wrap_u()) {
    // Choose the more general of the two.
    if (wrap_u == EggTexture::WM_repeat) {
      tex.set_wrap_u(wrap_u);
    }
    okflag = false;
  }
  if (wrap_v != tex.determine_wrap_v()) {
    if (wrap_v == EggTexture::WM_repeat) {
      tex.set_wrap_v(wrap_v);
    }
    okflag = false;
  }
  
  LMatrix3d mat = color_def.compute_texture_matrix();
  if (!mat.almost_equal(tex.get_transform())) {
    okflag = false;
  }

  return okflag;
}
#endif
////////////////////////////////////////////////////////////////////
//     Function: SoftShader::reparent_decals
//       Access: Private
//  Description: Recursively walks the egg hierarchy, reparenting
//               "decal" type nodes below their corresponding
//               "decalbase" type nodes, and setting the flags.
//
//               Returns true on success, false if some nodes were
//               incorrect.
////////////////////////////////////////////////////////////////////
bool SoftToEggConverter::
reparent_decals(EggGroupNode *egg_parent) {
  bool okflag = true;

  // First, walk through all children of this node, looking for the
  // one decal base, if any.
  EggGroup *decal_base = (EggGroup *)NULL;
  pvector<EggGroup *> decal_children;

  EggGroupNode::iterator ci;
  for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
    EggNode *child =  (*ci);
    if (child->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = DCAST(EggGroup, child);
      if (child_group->has_object_type("decalbase")) {
        if (decal_base != (EggNode *)NULL) {
          softegg_cat.error()
            << "Two children of " << egg_parent->get_name()
            << " both have decalbase set: " << decal_base->get_name()
            << " and " << child_group->get_name() << "\n";
          okflag = false;
        }
        child_group->remove_object_type("decalbase");
        decal_base = child_group;

      } else if (child_group->has_object_type("decal")) {
        child_group->remove_object_type("decal");
        decal_children.push_back(child_group);
      }
    }
  }

  if (decal_base == (EggGroup *)NULL) {
    if (!decal_children.empty()) {
      softegg_cat.warning()
        << decal_children.front()->get_name()
        << " has decal, but no sibling node has decalbase.\n";
    }

  } else {
    if (decal_children.empty()) {
      softegg_cat.warning()
        << decal_base->get_name()
        << " has decalbase, but no sibling nodes have decal.\n";

    } else {
      // All the decal children get moved to be a child of decal base.
      // This usually will not affect the vertex positions, but it
      // could if the decal base has a transform and the decal child
      // is an instance node.  So don't do that.
      pvector<EggGroup *>::iterator di;
      for (di = decal_children.begin(); di != decal_children.end(); ++di) {
        EggGroup *child_group = (*di);
        decal_base->add_child(child_group);
      }

      // Also set the decal state on the base.
      decal_base->set_decal_flag(true);
    }
  }

  // Now recurse on each of the child nodes.
  for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
    EggNode *child =  (*ci);
    if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *child_group = DCAST(EggGroupNode, child);
      if (!reparent_decals(child_group)) {
        okflag = false;
      }
    }
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftShader::string_transform_type
//       Access: Public, Static
//  Description: Returns the TransformType value corresponding to the
//               indicated string, or TT_invalid.
////////////////////////////////////////////////////////////////////
SoftToEggConverter::TransformType SoftToEggConverter::
string_transform_type(const string &arg) {
  if (cmp_nocase(arg, "all") == 0) {
    return TT_all;
  } else if (cmp_nocase(arg, "model") == 0) {
    return TT_model;
  } else if (cmp_nocase(arg, "dcs") == 0) {
    return TT_dcs;
  } else if (cmp_nocase(arg, "none") == 0) {
    return TT_none;
  } else {
    return TT_invalid;
  }
}

/////////////////////////////////////////////////////////////////////////
//      Function: init_soft2egg
//        Access: 
//   Description: Invokes the softToEggConverter class
/////////////////////////////////////////////////////////////////////////
extern "C" int init_soft2egg (int argc, char **argv)
{
  stec._commandName = argv[0];
  stec.rsrc_path = "c:\\Softimage\\SOFT3D_3.9.2\\3D\\rsrc";
  if (stec.DoGetopts(argc, argv)) {

    // create a Filename object and convert the file
    Filename softFile(argv[1]);
    stec.convert_file(softFile);
  }

  return 0;
}
//
//
//
