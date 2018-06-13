/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaToEgg_server.cxx
 * @author cbrunner
 * @date 2009-11-09
 */

#if defined(WIN32_VC) || defined(WIN64_VC)
#include <direct.h>  // for chdir
#endif
#include "mayaToEgg_server.h"
#include "mayaToEggConverter.h"
#include "config_mayaegg.h"
#include "config_maya.h"  // for maya_cat
#include "globPattern.h"

/**
 *
 */
MayaToEggServer::
MayaToEggServer() :
  SomethingToEgg("Maya", ".mb")
{
  add_path_replace_options();
  add_path_store_options();
  add_animation_options();
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_brief("convert Maya model files to .egg");
  set_program_description
    ("This program converts Maya model files to egg.  Static and animatable "
     "models can be converted, with polygon or NURBS output.  Animation tables "
     "can also be generated to apply to an animatable model.");

  add_option
    ("p", "", 0,
     "Generate polygon output only.  Tesselate all NURBS surfaces to "
     "polygons via the built-in Maya tesselator.  The tesselation will "
     "be based on the tolerance factor given by -ptol.",
     &MayaToEggServer::dispatch_none, &_polygon_output);

  add_option
    ("ptol", "tolerance", 0,
     "Specify the fit tolerance for Maya polygon tesselation.  The smaller "
     "the number, the more polygons will be generated.  The default is "
     "0.01.",
     &MayaToEggServer::dispatch_double, nullptr, &_polygon_tolerance);

  add_option
    ("bface", "", 0,
     "Respect the Maya \"double sided\" rendering flag to indicate whether "
     "polygons should be double-sided or single-sided.  Since this flag "
     "is set to double-sided by default in Maya, it is often better to "
     "ignore this flag (unless your modelers are diligent in turning it "
     "off where it is not desired).  If this flag is not specified, the "
     "default is to treat all polygons as single-sided, unless an "
     "egg object type of \"double-sided\" is set.",
     &MayaToEggServer::dispatch_none, &_respect_maya_double_sided);

  add_option
    ("suppress-vcolor", "", 0,
     "Ignore vertex color for geometry that has a texture applied.  "
     "(This is the way Maya normally renders internally.)  The egg flag "
     "'vertex-color' may be applied to a particular model to override "
     "this setting locally.",
     &MayaToEggServer::dispatch_none, &_suppress_vertex_color);

  add_option
    ("keep-uvs", "", 0,
     "Convert all UV sets on all vertices, even those that do not appear "
     "to be referenced by any textures.",
     &MayaToEggServer::dispatch_none, &_keep_all_uvsets);

  add_option
    ("round-uvs", "", 0,
     "round up uv coordinates to the nearest 1/100th. i.e. -0.001 becomes"
     "0.0; 0.444 becomes 0.44; 0.778 becomes 0.78.",
     &MayaToEggServer::dispatch_none, &_round_uvs);

  add_option
    ("trans", "type", 0,
     "Specifies which transforms in the Maya file should be converted to "
     "transforms in the egg file.  The option may be one of all, model, "
     "dcs, or none.  The default is model, which means only transforms on "
     "nodes that have the model flag or the dcs flag are preserved.",
     &MayaToEggServer::dispatch_transform_type, nullptr, &_transform_type);

  add_option
    ("subroot", "name", 0,
     "Specifies that only a subroot of the geometry in the Maya file should "
     "be converted; specifically, the geometry under the node or nodes whose "
     "name matches the parameter (which may include globbing characters "
     "like * or ?).  This parameter may be repeated multiple times to name "
     "multiple roots.  If it is omitted altogether, the entire file is "
     "converted.",
     &MayaToEggServer::dispatch_vector_string, nullptr, &_subroots);

  add_option
    ("subset", "name", 0,
     "Specifies that only a subset of the geometry in the Maya file should "
     "be converted; specifically, the geometry under the node or nodes whose "
     "name matches the parameter (which may include globbing characters "
     "like * or ?).  This parameter may be repeated multiple times to name "
     "multiple roots.  If it is omitted altogether, the entire file is "
     "converted.",
     &MayaToEggServer::dispatch_vector_string, nullptr, &_subsets);

  add_option
    ("exclude", "name", 0,
     "Specifies that a subset of the geometry in the Maya file should "
     "not be converted; specifically, the geometry under the node or nodes whose "
     "name matches the parameter (which may include globbing characters "
     "like * or ?).  This parameter may be repeated multiple times to name "
     "multiple roots.",
     &MayaToEggServer::dispatch_vector_string, nullptr, &_excludes);

  add_option
    ("ignore-slider", "name", 0,
     "Specifies the name of a slider (blend shape deformer) that maya2egg "
     "should not process.  The slider will not be touched during conversion "
     "and it will not become a part of the animation.  This "
     "parameter may including globbing characters, and it may be repeated "
     "as needed.",
     &MayaToEggServer::dispatch_vector_string, nullptr, &_ignore_sliders);

  add_option
    ("force-joint", "name", 0,
     "Specifies the name of a DAG node that maya2egg "
     "should treat as a joint, even if it does not appear to be a Maya joint "
     "and does not appear to be animated.",
     &MayaToEggServer::dispatch_vector_string, nullptr, &_force_joints);

  add_option
    ("v", "", 0,
     "Increase verbosity.  More v's means more verbose.",
     &MayaToEggServer::dispatch_count, nullptr, &_verbose);

  add_option
    ("legacy-shaders", "", 0,
     "Use this flag to turn off modern (Phong) shader generation"
     "and treat all shaders as if they were Lamberts (legacy).",
     &MayaToEggServer::dispatch_none, &_legacy_shader);

  // Unfortunately, the Maya API doesn't allow us to differentiate between
  // relative and absolute pathnames--everything comes out as an absolute
  // pathname, even if it is stored in the Maya file as a relative path.  So
  // we can't support -noabs.
  remove_option("noabs");

  _verbose = 0;
  _polygon_tolerance = 0.01;
  _transform_type = MayaToEggConverter::TT_model;
  _got_tbnauto = true;
  qManager = new QueuedConnectionManager();
  qListener = new QueuedConnectionListener(qManager, 0);
  qReader = new QueuedConnectionReader(qManager, 0);
  cWriter = new ConnectionWriter(qManager, 0);
  dummy = new MayaToEggConverter();

  nout << "Initializing Maya...\n";
  if (!dummy->open_api()) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }
}
/**
 *
 */
MayaToEggServer::
~MayaToEggServer() {
  delete qManager;
  delete qReader;
  delete qListener;
  delete cWriter;
  delete dummy;
}

/**
 *
 */
void MayaToEggServer::
run() {
  // Make sure we have good clean data to start with
  _data = new EggData();

  // Set the verbose level by using Notify.
  if (_verbose >= 3) {
    maya_cat->set_severity(NS_spam);
    mayaegg_cat->set_severity(NS_spam);
  } else if (_verbose >= 2) {
    maya_cat->set_severity(NS_debug);
    mayaegg_cat->set_severity(NS_debug);
  } else if (_verbose >= 1) {
    maya_cat->set_severity(NS_info);
    mayaegg_cat->set_severity(NS_info);
  }

  // Let's convert the output file to a full path before we initialize Maya,
  // since Maya now has a nasty habit of changing the current directory.
  if (_got_output_filename) {
    _output_filename.make_absolute();
    _path_replace->_path_directory.make_absolute();
  }

  MayaToEggConverter converter(_program_name);

  // Copy in the command-line parameters.
  converter._polygon_output = _polygon_output;
  converter._polygon_tolerance = _polygon_tolerance;
  converter._respect_maya_double_sided = _respect_maya_double_sided;
  converter._always_show_vertex_color = !_suppress_vertex_color;
  converter._keep_all_uvsets = _keep_all_uvsets;
  converter._round_uvs = _round_uvs;
  converter._transform_type = _transform_type;
  converter._legacy_shader = _legacy_shader;

  vector_string::const_iterator si;
  if (!_subroots.empty()) {
    converter.clear_subroots();
    for (si = _subroots.begin(); si != _subroots.end(); ++si) {
      converter.add_subroot(GlobPattern(*si));
    }
  }

  if (!_subsets.empty()) {
    converter.clear_subsets();
    for (si = _subsets.begin(); si != _subsets.end(); ++si) {
      converter.add_subset(GlobPattern(*si));
    }
  }

  if (!_excludes.empty()) {
    converter.clear_excludes();
    for (si = _excludes.begin(); si != _excludes.end(); ++si) {
      converter.add_exclude(GlobPattern(*si));
    }
  }

  if (!_ignore_sliders.empty()) {
    converter.clear_ignore_sliders();
    for (si = _ignore_sliders.begin(); si != _ignore_sliders.end(); ++si) {
      converter.add_ignore_slider(GlobPattern(*si));
    }
  }

  if (!_force_joints.empty()) {
    converter.clear_force_joints();
    for (si = _force_joints.begin(); si != _force_joints.end(); ++si) {
      converter.add_force_joint(GlobPattern(*si));
    }
  }

  // Copy in the path and animation parameters.
  apply_parameters(converter);

  // Set the coordinate system to match Maya's.
  if (!_got_coordinate_system) {
    _coordinate_system = converter._maya->get_coordinate_system();
  }
  _data->set_coordinate_system(_coordinate_system);

  converter.set_egg_data(_data);

  if (!converter.convert_file(_input_filename)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  // Use the standard Maya units, if the user didn't specify otherwise.  This
  // always returns centimeters, which is the way all Maya files are stored
  // internally (and is the units returned by all of the API functions called
  // here).
  if (_input_units == DU_invalid) {
    _input_units = converter.get_input_units();
  }

  // Add the command line comment at the top of the egg file
  append_command_comment(_data);

  write_egg_file();

  // Clean and out
  close_output();
  _verbose = 0;
  _polygon_tolerance = 0.01;
  _polygon_output = 0;
  _transform_type = MayaToEggConverter::TT_model;
  _subsets.clear();
  _subroots.clear();
  _input_units = DU_invalid;
  _output_units = DU_invalid;
  _excludes.clear();
  _ignore_sliders.clear();
  _force_joints.clear();
  _got_transform = false;
  _transform = LMatrix4d::ident_mat();
  _normals_mode = NM_preserve;
  _normals_threshold = 0.0;
  _got_start_frame = false;
  _got_end_frame = false;
  _got_frame_inc = false;
  _got_neutral_frame = false;
  _got_input_frame_rate = false;
  _got_output_frame_rate = false;
  _got_output_filename = false;
  _merge_externals = false;
  _got_tbnall = false;
  _got_tbnauto = false;
  _got_transform = false;
  _coordinate_system = CS_yup_right;
  _noabs = false;
  _program_args.clear();
  _data->clear();
  _animation_convert = AC_none;
  _character_name = "";
  dummy->clear();
}

/**
 * Dispatches a parameter that expects a MayaToEggConverter::TransformType
 * option.
 */
bool MayaToEggServer::
dispatch_transform_type(const std::string &opt, const std::string &arg, void *var) {
  MayaToEggConverter::TransformType *ip = (MayaToEggConverter::TransformType *)var;
  (*ip) = MayaToEggConverter::string_transform_type(arg);

  if ((*ip) == MayaToEggConverter::TT_invalid) {
    nout << "Invalid type for -" << opt << ": " << arg << "\n"
         << "Valid types are all, model, dcs, and none.\n";
    return false;
  }

  return true;
}

/**
 * Checks for any network activity and handles it, if appropriate, and then
 * returns.  This must be called periodically
 */
void MayaToEggServer::
poll() {
  // Listen for new connections
  qListener->poll();

  // If we have a new connection from a client create a new connection pointer
  // and add it to the reader list
  if (qListener->new_connection_available()) {
    PT(Connection) con;
    PT(Connection) rv;
    NetAddress address;
    if (qListener->get_new_connection(rv, address, con)) {
      qReader->add_connection(con);
      _clients.insert(con);
    }
  }

  // Check for reset clients
  if (qManager->reset_connection_available()) {
    PT(Connection) connection;
    if (qManager->get_reset_connection(connection)) {
      _clients.erase(connection);
      qManager->close_connection(connection);
    }
  }

  // Poll the readers (created above) and if they have data process it
  qReader->poll();
  if (qReader->data_available()) {
    // Grab the incomming data and unpack it
    NetDatagram datagram;
    if (qReader->get_data(datagram)) {
      DatagramIterator data(datagram);
      // First data should be the "argc" (argument count) from the client
      int argc = data.get_uint8();

      // Now we have to get clever because the rest of the data comes as
      // strings and parse_command_line() expects arguments of the standard
      // argc, argv*[] variety.  First, we need a string vector to hold all
      // the strings from the datagram.  We also need a char * array to keep
      // track of all the pointers we're gonna malloc.  Needed later for
      // cleanup.
      vector_string vargv;
      std::vector<char *> buffers;

      // Get the strings from the datagram and put them into the string vector
      int i;
      for ( i = 0; i < argc; i++ ) {
        vargv.push_back(data.get_string());
      }

      // Last string is the current directory the client was run from.  Not
      // part of the argument list, but we still need it
      std::string cwd = data.get_string();

      // We allocate some memory to hold the pointers to the pointers we're
      // going to pass in to parse_command_line().
      char ** cargv = (char**) malloc(sizeof(char**) * argc);

      // Loop through the string arguments we got from the datagram and
      // convert them to const char *'s.  parse_command_line() expects char
      // *'s, so we have to copy these const versions into fresh char *, since
      // there is no casting from const char * to char *.
      for ( i = 0; i < argc; i++) {
        // string to const char *
        const char * cptr = vargv[i].c_str();
        // memory allocated for a new char * of size of the string
        char * buffer = (char*) malloc(vargv[i].capacity()+1);
        // Copy the const char * to the char *
        strcpy(buffer, cptr);
        // put this into the arry we defined above.  This is what will
        // eventually be passed to parse_command_line()
        cargv[i] = buffer;
        // keep track of the pointers to the  allocated memory for cleanup
        // later
        buffers.push_back(buffer);
      }
      // Change to the client's current dir
#ifdef WIN64_VC
      _chdir(cwd.c_str());
#else
      chdir(cwd.c_str());
#endif

      // Pass in the 'new' argc and argv we got from the client
      this->parse_command_line(argc, cargv);
      // Actually run the damn thing
      this->run();

      // Cleanup First, release the string vector
      vargv.clear();
      // No, iterate through the char * vector and cleanup the malloc'd
      // pointers
      std::vector<char *>::iterator vi;
      for ( vi = buffers.begin() ; vi != buffers.end(); vi++) {
        free(*vi);
      }
      // Clean up the malloc'd pointer pointer
      free(cargv);
    } // qReader->get_data

    Clients::iterator ci;
    for (ci = _clients.begin(); ci != _clients.end(); ++ci) {
      qManager->close_connection(*ci);
    }
  } // qReader->data_available
} // poll

int main(int argc, char *argv[]) {
  MayaToEggServer prog;
  // Open a rendezvous port for receiving new connections from the client
  PT(Connection) rend = prog.qManager->open_TCP_server_rendezvous(4242, 50);
  if (rend.is_null()) {
    nout << "port opened fail";
  }

  // Add this connection to the listeners list
  prog.qListener->add_connection(rend);

  // Main loop.  Keep polling for connections, but don't eat up all the CPU.
  while (true) {
    prog.poll();
    Thread::force_yield();
  }
  return 0;
}
