/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggReader.cxx
 * @author drose
 * @date 2000-02-14
 */

#include "eggReader.h"

#include "pnmImage.h"
#include "config_putil.h"
#include "eggTextureCollection.h"
#include "eggGroup.h"
#include "eggGroupNode.h"
#include "eggSwitchCondition.h"
#include "string_utils.h"
#include "dcast.h"

/**
 *
 */
EggReader::
EggReader() {
  clear_runlines();
  add_runline("[opts] input.egg");

  redescribe_option
    ("cs",
     "Specify the coordinate system to operate in.  This may be "
     " one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default "
     "is the coordinate system of the input egg file.");

  add_option
    ("f", "", 80,
     "Force complete loading: load up the egg file along with all of its "
     "external references.",
     &EggReader::dispatch_none, &_force_complete);

  add_option
    ("noabs", "", 0,
     "Don't allow the input egg file to have absolute pathnames.  "
     "If it does, abort with an error.  This option is designed to help "
     "detect errors when populating or building a standalone model tree, "
     "which should be self-contained and include only relative pathnames.",
     &EggReader::dispatch_none, &_noabs);

  _tex_type = nullptr;
  _delod = -1.0;

  _got_tex_dirname = false;
  _got_tex_extension = false;
}

/**
 * Adds -td, -te, etc.  as valid options for this program.  If the user
 * specifies one of the options on the command line, the textures will be
 * copied and converted as each egg file is read.
 *
 * Note that if you call this function to add these options, you must call
 * do_reader_options() at the appropriate point before or during processing to
 * execute the options if the user specified them.
 */
void EggReader::
add_texture_options() {
  add_option
    ("td", "dirname", 40,
     "Copy textures to the indicated directory.  The copy is performed "
     "only if the destination file does not exist or is older than the "
     "source file.",
     &EggReader::dispatch_filename, &_got_tex_dirname, &_tex_dirname);

  add_option
    ("te", "ext", 40,
     "Rename textures to have the indicated extension.  This also "
     "automatically copies them to the new filename (possibly in a "
     "different directory if -td is also specified), and may implicitly "
     "convert to a different image format according to the extension.",
     &EggReader::dispatch_string, &_got_tex_extension, &_tex_extension);

  add_option
    ("tt", "type", 40,
     "Explicitly specifies the image format to convert textures to "
     "when copying them via -td or -te.  Normally, this is unnecessary as "
     "the image format can be determined by the extension, but sometimes "
     "the extension is insufficient to unambiguously specify an image "
     "type.",
     &EggReader::dispatch_image_type, nullptr, &_tex_type);
}

/**
 * Adds -delod as a valid option for this program.
 *
 * Note that if you call this function to add these options, you must call
 * do_reader_options() at the appropriate point before or during processing to
 * execute the options if the user specified them.
 */
void EggReader::
add_delod_options(double default_delod) {
  _delod = default_delod;

  if (default_delod < 0) {
    add_option
      ("delod", "dist", 40,
       "Eliminate LOD's by choosing the level that would be appropriate for "
       "a camera at the indicated fixed distance from each LOD.  "
       "Use -delod -1 to keep all the LOD's as they are, which is "
       "the default.\n",
       &EggReader::dispatch_double, nullptr, &_delod);

  } else {
    add_option
      ("delod", "dist", 40,
       "Eliminate LOD's by choosing the level that would be appropriate for "
       "a camera at the indicated fixed distance from each LOD.  "
       "Use -delod -1 to keep all the LOD's as they are.  The default value "
       "is " + format_string(default_delod) + ".",
       &EggReader::dispatch_double, nullptr, &_delod);
  }
}

/**
 * Returns this object as an EggReader pointer, if it is in fact an EggReader,
 * or NULL if it is not.
 *
 * This is intended to work around the C++ limitation that prevents downcasts
 * past virtual inheritance.  Since both EggReader and EggWriter inherit
 * virtually from EggSingleBase, we need functions like this to downcast to
 * the appropriate pointer.
 */
EggReader *EggReader::
as_reader() {
  return this;
}

/**
 * Performs any processing of the egg file that is appropriate after reading
 * it in.
 *
 * Normally, you should not need to call this function directly; it is called
 * automatically at startup.
 */
void EggReader::
pre_process_egg_file() {
}

/**
 *
 */
bool EggReader::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the egg file(s) to read on the command line.\n";
    return false;
  }

  // Any separate egg files that are listed on the command line will get
  // implicitly loaded up into one big egg file.

  if (!args.empty()) {
    _data->set_egg_filename(Filename::from_os_specific(args[0]));
  }
  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    Filename filename = Filename::from_os_specific(*ai);

    EggData file_data;
    if (filename != "-") {
      if (!file_data.read(filename)) {
        // Rather than returning false, we simply exit here, so the ProgramBase
        // won't try to tell the user how to run the program just because we got
        // a bad egg file.
        exit(1);
      }
    } else {
      if (!file_data.read(std::cin)) {
        exit(1);
      }
    }

    if (_noabs && file_data.original_had_absolute_pathnames()) {
      nout << filename.get_basename()
           << " includes absolute pathnames!\n";
      exit(1);
    }

    DSearchPath file_path;
    file_path.append_directory(filename.get_dirname());

    if (_force_complete) {
      if (!file_data.load_externals(file_path)) {
        exit(1);
      }
    }

    // Now resolve the filenames again according to the user's specified
    // _path_replace.
    convert_paths(&file_data, _path_replace, file_path);

    _data->merge(file_data);
  }

  pre_process_egg_file();

  return true;
}

/**
 * This is called after the command line has been completely processed, and it
 * gives the program a chance to do some last-minute processing and validation
 * of the options and arguments.  It should return true if everything is fine,
 * false if there is an error.
 */
bool EggReader::
post_command_line() {
  return EggSingleBase::post_command_line();
}

/**
 * Postprocesses the egg file as the user requested according to whatever
 * command-line options are in effect.  Returns true if everything is done
 * correctly, false if there was some problem.
 */
bool EggReader::
do_reader_options() {
  bool okflag = true;

  if (_got_tex_dirname || _got_tex_extension) {
    if (!copy_textures()) {
      okflag = false;
    }
  }

  if (_delod >= 0.0) {
    do_delod(_data);
  }

  return okflag;
}

/**
 * Renames and copies the textures referenced in the egg file, if so specified
 * by the -td and -te options.  Returns true if all textures are copied
 * successfully, false if any one of them failed.
 */
bool EggReader::
copy_textures() {
  bool success = true;
  EggTextureCollection textures;
  textures.find_used_textures(_data);

  EggTextureCollection::const_iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    EggTexture *tex = (*ti);
    Filename orig_filename = tex->get_filename();
    if (!orig_filename.exists()) {
      bool found = orig_filename.resolve_filename(get_model_path());
      if (!found) {
        nout << "Cannot find " << orig_filename << "\n";
        success = false;
        continue;
      }
    }

    Filename new_filename = orig_filename;
    if (_got_tex_dirname) {
      new_filename.set_dirname(_tex_dirname);
    }
    if (_got_tex_extension) {
      new_filename.set_extension(_tex_extension);
    }

    if (orig_filename != new_filename) {
      tex->set_filename(new_filename);

      // The new filename is different; does it need copying?
      int compare =
        orig_filename.compare_timestamps(new_filename, true, true);
      if (compare > 0) {
        // Yes, it does.  Copy it!
        nout << "Reading " << orig_filename << "\n";
        PNMImage image;
        if (!image.read(orig_filename)) {
          nout << "  unable to read!\n";
          success = false;
        } else {
          nout << "Writing " << new_filename << "\n";
          if (!image.write(new_filename, _tex_type)) {
            nout << "  unable to write!\n";
            success = false;
          }
        }
      }
    }
  }

  return success;
}

/**
 * Removes all the LOD's in the egg file by treating the camera as being
 * _delod distance from each LOD. Returns true if this particular group should
 * be preserved, false if it should be removed.
 */
bool EggReader::
do_delod(EggNode *node) {
  if (node->is_of_type(EggGroup::get_class_type())) {
    EggGroup *group = DCAST(EggGroup, node);
    if (group->has_lod()) {
      const EggSwitchCondition &cond = group->get_lod();
      if (cond.is_of_type(EggSwitchConditionDistance::get_class_type())) {
        const EggSwitchConditionDistance *dist =
          DCAST(EggSwitchConditionDistance, &cond);
        if (_delod >= dist->_switch_out && _delod < dist->_switch_in) {
          // Preserve this group node, but not the LOD information itself.
          nout << "Preserving LOD " << node->get_name()
               << " (" << dist->_switch_out << " to " << dist->_switch_in
               << ")\n";
          group->clear_lod();
        } else {
          // Remove this group node.
          nout << "Eliminating LOD " << node->get_name()
               << " (" << dist->_switch_out << " to " << dist->_switch_in
               << ")\n";
          return false;
        }
      }
    }
  }

  // Now process all the children.
  if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, node);
    EggGroupNode::iterator ci;
    ci = group->begin();
    while (ci != group->end()) {
      EggNode *child = *ci;
      ++ci;

      if (!do_delod(child)) {
        group->remove_child(child);
      }
    }
  }

  return true;
}
