// Filename: eggPalettize.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "eggPalettize.h"
#include "attribFile.h"
#include "pTexture.h"
#include "string_utils.h"
#include "sourceEgg.h"
#include "textureOmitReason.h"

#include <pnmImage.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////
//     Function: EggPalettize::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggPalettize::
EggPalettize() : EggMultiFilter(true) {
  set_program_description
    ("egg-palettize attempts to pack several texture maps from various models "
     "together into one or more palette images, for improved rendering performance "
     "and ease of texture management.  It can also resize textures on the fly, "
     "whether or not they are actually placed on a palette.\n\n"
     
     "egg-palettize reads and writes an AttributesFile, which contains instructions "
     "from the user about resizing particular textures, as well as the complete "
     "information necessary to reconstruct the palettization from past runs, "
     "including references to other egg files that may share this palette.  This "
     "is designed to allow multiple egg files to use the same palette, without "
     "having to process them all at once.\n\n"
     
     "Note that it is not even necessary to specify any egg files at all on the "
     "command line; egg-palettize can be run on an existing AttributesFiles by "
     "itself to freshen up a palette when necessary.");


  clear_runlines();
  add_runline("[opts] attribfile.txa file.egg [file.egg ...]");

  // We always have EggMultiBase's -f on: force complete load.  In
  // fact, we use -f for our own purposes, below.
  remove_option("f");
  _force_complete = true;

  add_option
    ("s", "", 0, 
     "Do not process anything, but report statistics on all palette "
     "information files read.",
     &EggPalettize::dispatch_none, &_statistics_only);
  redescribe_option
    ("d",
     "The directory in which to write the palettized egg files.  This is "
     "only necessary if more than one egg file is processed at the same "
     "time; if it is included, each egg file will be processed and written "
     "into the indicated directory.");
  add_option
    ("dm", "dirname", 0, 
     "The directory in which to place all maps: generated palettes, "
     "as well as images which were not placed on palettes "
     "(but may have been resized).  It is often best if this is a "
     "fully-qualified directory name rather than a relative directory name, "
     "particularly if -d is used to write the egg files to a directory "
     "different than the current directory, as the same name is written "
     "into the egg files.",
     &EggPalettize::dispatch_filename, &_got_map_dirname, &_map_dirname);
  add_option
    ("f", "", 0, 
     "Force an optimal packing.  By default, textures are added to "
     "existing palettes without disturbing them, which can lead to "
     "suboptimal packing.  Including this switch forces the palettes "
     "to be rebuilt if necessary to optimize the packing, but this "
     "may invalidate other egg files which share this palette.",
     &EggPalettize::dispatch_none, &_force_optimal);
  add_option
    ("F", "", 0, 
     "Force a redo of everything.  This is useful in case something "
     "has gotten out of sync and the old palettes are just bad.",
     &EggPalettize::dispatch_none, &_force_redo_all);
  add_option
    ("R", "", 0, 
     "Resize mostly-empty palettes to their minimal size.",
     &EggPalettize::dispatch_none, &_optimal_resize);
  add_option
    ("t", "", 0, 
     "Touch any additional egg files that share this palette and will "
     "need to be refreshed, but were not included on the command "
     "line.  Presumably a future make pass will cause them to be run "
     "through egg-palettize again.",
     &EggPalettize::dispatch_none, &_touch_eggs);
  add_option
    ("T", "", 0, 
     "When touching egg files, consider an egg file to be invalidated "
     "if textures have changed in any way, rather than simply moving "
     "around within their palettes.  You should use this switch "
     "if the texture images themselves are to be stored within bam files "
     "generated from the eggs, or some such nonsense.",
     &EggPalettize::dispatch_none, &_eggs_include_images);
  add_option
    ("C", "", 0, 
     "Aggressively keep the map directory clean by deleting unused "
     "textures from previous passes.",
     &EggPalettize::dispatch_none, &_got_aggressively_clean_mapdir);
  add_option
    ("r", "", 0, 
     "Respect any repeat/clamp flags given in the egg files.  The "
     "default is to override a repeat flag if a texture's UV's don't "
     "exceed the range [0..1].",
     &EggPalettize::dispatch_none, &_dont_override_repeats);
  add_option
    ("z", "fuzz", 0, 
     "The fuzz factor when applying the above repeat test.  UV's are "
     "considered to be in the range [0..1] if they are actually in "
     "the range [0-fuzz .. 1+fuzz].  The default is 0.01.",
     &EggPalettize::dispatch_double, NULL, &_fuzz_factor);
  add_option
    ("m", "margin", 0, 
     "Specify the default margin size.",
     &EggPalettize::dispatch_int, &_got_default_margin, &_default_margin);
  add_option
    ("P", "x,y", 0, 
     "Specify the default palette size.",
     &EggPalettize::dispatch_int_pair, &_got_palette_size, _pal_size);
  add_option
    ("2", "", 0, 
     "Force textures that have been left out of the palette to a size "
     "which is an even power of 2.  They will be scaled down to "
     "achieve this.",
     &EggPalettize::dispatch_none, &_got_force_power_2);
  add_option
    ("k", "", 0, 
     "Kill lines from the attributes file that aren't used on any "
     "texture.",
     &EggPalettize::dispatch_none, &_remove_unused_lines);
  add_option
    ("H", "", 0, 
     "Describe the syntax of the attributes file.",
     &EggPalettize::dispatch_none, &_describe_input_file);
  
  _fuzz_factor = 0.01;
  _aggressively_clean_mapdir = false;
  _force_power_2 = false;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPalettize::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggPalettize::
handle_args(ProgramBase::Args &args) {
  if (_describe_input_file) {
    describe_input_file();
    exit(1);
  }


  Args egg_names;
  Args txa_names;

  // First, collect all the filenames.
  Args::iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    Filename filename = (*ai);
    string ext = filename.get_extension();

    if (ext == "egg") {
      egg_names.push_back(filename);
    } else if (ext == "txa" || ext == "pi") {
      txa_names.push_back(filename);
    } else {
      nout << "Don't know what kind of file " << filename << " is.\n";
      return false;
    }
  }

  // Now sanity check them.  Either we have zero egg files, and one or
  // more txa files, or we have some egg files and exactly one txa
  // file.
  if (egg_names.empty()) {
    if (txa_names.empty()) {
      nout << "No files specified.\n";
      return false;
    }

    for (ai = txa_names.begin(); ai != txa_names.end(); ++ai) {
      AttribFile *af = new AttribFile(*ai);
      _attrib_files.push_back(af);
    }

  } else {
    if (txa_names.empty()) {
      nout << "Must include an attribs.txa file.\n";
      return false;
    }
    if (txa_names.size() != 1) {
      nout << "Can only read one attribs.txa file when processing egg files.\n";
      return false;
    }

    AttribFile *af = new AttribFile(txa_names[0]);
    _attrib_files.push_back(af);
  }

  return EggMultiFilter::handle_args(egg_names);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPalettize::read_egg
//       Access: Protected, Virtual
//  Description: Allocates and returns a new EggData structure that
//               represents the indicated egg file.  If the egg file
//               cannot be read for some reason, returns NULL. 
//
//               This can be overridden by derived classes to control
//               how the egg files are read, or to extend the
//               information stored with each egg structure, by
//               deriving from EggData.
////////////////////////////////////////////////////////////////////
EggData *EggPalettize::
read_egg(const Filename &filename) {
  // We should only call this function if we have exactly one .txa file.
  nassertr(_attrib_files.size() == 1, (EggData *)NULL);

  SourceEgg *data = _attrib_files[0]->get_egg(filename);
  if (!data->read(filename)) {
    // Failure reading.
    delete data;
    return (EggData *)NULL;
  }

  if (_force_complete) {
    if (!data->resolve_externals()) {
      delete data;
      return (EggData *)NULL;
    }
  }
   
  return data;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPalettize::describe_input_file
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggPalettize::
describe_input_file() {
  nout <<
    "An attributes file consists mostly of lines describing desired sizes of "
    "texture maps.  The format resembles, but is not identical to, that of "
    "the qtess input file.  Examples:\n\n"

    "  texturename.rgb : 64 64\n"
    "  texture-a.rgb texture-b.rgb : 32 16 2\n"
    "  *.rgb : 50%\n"
    "  eyelids.rgb : 16 16 omit\n\n"

    "In general, each line consists of one or more filenames (and can "
    "contain shell globbing characters like '*' or '?'), and a colon "
    "followed by a size request.  For each texture appearing in an egg "
    "file, the input list is scanned from the beginning and the first "
    "line that matches the filename defines the size of the texture.\n\n"

    "A size request may be either a pair of numbers, giving a specific x y "
    "size of the texture, or it may be a scale factor in the form of a "
    "percentage.  It may also include an additional number, giving a margin "
    "for this particular texture (otherwise the default margin is "
    "applied).  Finally, the keyword 'omit' may be included along with the "
    "size to specify that the texture should not be placed in a palette.\n\n"

    "The attributes file may also assign certain egg files into various "
    "named palette groups.  The syntax is similar to the above:\n\n"

    "  car-blue.egg : main\n"
    "  road.egg house.egg : main\n"
    "  plane.egg : phase2 main\n"
    "  car*.egg : phase2\n\n"

    "Any number of egg files may be named on one line, and the group of "
    "egg files may be simultaneously assigned to one or more groups.  Each "
    "named group represents a semi-independent collection of textures; a "
    "different set of palette images will be created for each group.  Each "
    "texture that is referenced by a given egg file will be palettized "
    "in one of the groups assigned to the egg file.  Also see the "
    ":group command, below, which defines relationships between the "
    "different groups.\n\n"

    "There are some other special lines that may appear in this second, "
    "along with the resize commands.  They begin with a colon to "
    "distinguish them from the resize commands.  They are:\n\n"

    "  :palette xsize ysize\n\n"

    "This specifies the size of the palette file(s) to be created.  It "
    "overrides the -s command-line option.\n\n"

    "  :margin msize\n\n"

    "This specifies the size of the default margin for all subsequent "
    "resize commands.  This may appear several times in a given file.\n\n"

    "  :group groupname1 with groupname2 [groupname3 ...]\n\n"

    "This indicates that the palette group named by groupname1 should "
    "be allowed to shared textures with those on groupname2 or groupname3, "
    "etc.  In other words, that whenever palette group groupname1 is in "
    "texture memory, we can assume that palette groups groupname2 and "
    "groupname3 will also be in memory.  Textures that already exist on "
    "groupname2 and other dependent groups will not be added to groupname1; "
    "instead, egg files will reference the textures directly from the "
    "other palettes.\n\n"

    "Comments may appear freely throughout the file, and are set off by a "
    "hash mark (#).\n";
}


////////////////////////////////////////////////////////////////////
//     Function: EggPalettize::format_space
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string EggPalettize::
format_space(int size_pixels, bool verbose) {
  int size_bytes = size_pixels * 4;
  int size_k = (size_bytes + 512) / 1024;
  int mm_size_k = (size_bytes * 4 / 3 + 512) / 1024;
  char str[128];
  if (verbose) {
    sprintf(str, "%dk, w/mm %dk", size_k, mm_size_k);
  } else {
    sprintf(str, "%dk, %dk", size_k, mm_size_k);
  }
  assert(strlen(str) < 128);

  return str;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPalettize::report_statistics
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggPalettize::
report_statistics() {
  /*
  // Look for textures in common.
  map<string, PTexture *> textures;
  map<string, int> dup_textures;

  AttribFiles::iterator afi;
  for (afi = _attrib_files.begin(); afi != _attrib_files.end(); ++afi) {
    AttribFile &af = *(*afi);
    af.check_dup_textures(textures, dup_textures);
  }

  if (dup_textures.empty()) {
    cout << "\nEach texture appears in only one palette group.\n";
  } else {
    cout << "\nThe following textures appear in more than one palette group:\n";
    int net_wasted_size = 0;
    map<string, int>::const_iterator di;
    for (di = dup_textures.begin(); di != dup_textures.end(); ++di) {
      cout << "  " << (*di).first << " (" 
	   << format_space((*di).second) << ")\n";
      net_wasted_size += (*di).second;
    }
    cout << "Total wasted memory from common textures is "
	 << format_space(net_wasted_size, true) << "\n";
  }

  int net_palette_size = 0;
  int net_num_palettes = 0;

  for (afi = _attrib_files.begin(); afi != _attrib_files.end(); ++afi) {
    AttribFile &af = *(*afi);
    int num_textures, num_placed, num_palettes;
    int orig_size, resized_size, palette_size, unplaced_size;
    af.collect_statistics(num_textures, num_placed, num_palettes,
			  orig_size, resized_size, 
			  palette_size, unplaced_size);
    cout << "\nPalette group " << af.get_name() << ":\n"
	 << "   " << num_textures << " textures, " 
	 << num_placed << " of which are packed onto " 
	 << num_palettes << " palettes (" << num_textures - num_placed
	 << " unplaced)"
	 << "\n   Original texture memory: " 
	 << format_space(orig_size, true)
	 << "\n   After resizing:          " 
	 << format_space(resized_size, true)
	 << "\n   After palettizing:       " 
	 << format_space(palette_size + unplaced_size, true)
	 << "\n";

    net_palette_size += palette_size;
    net_num_palettes += num_palettes;
  }

  int net_num_textures = textures.size();
  int net_num_placed = 0;
  int net_orig_size = 0;
  int net_resized_size = 0;
  int net_unplaced_size = 0;
  typedef map<TextureOmitReason, pair<int, int> > UnplacedReasons;
  UnplacedReasons unplaced_reasons;

  map<string, PTexture *>::iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    PTexture *texture = (*ti).second;

    int xsize, ysize, zsize;
    int rxsize, rysize;
    int rsize = 0;
    if (texture->get_size(xsize, ysize, zsize) && 
	texture->get_last_req(rxsize, rysize)) {
      net_orig_size += xsize * ysize;
      net_resized_size += rxsize * rysize;
      rsize = rxsize * rysize;
    }


    if (texture->is_really_packed()) {
      net_num_placed++;

    } else {
      net_unplaced_size += rsize;

      // Here's an unplaced texture; add its size to the unplaced
      // reasons table.
      UnplacedReasons::iterator uri = 
	unplaced_reasons.find(texture->get_omit());
      if (uri == unplaced_reasons.end()) {
	unplaced_reasons.insert
	  (UnplacedReasons::value_type(texture->get_omit(), 
				       pair<int, int>(1, rsize)));
      } else {
	(*uri).second.first++;
	(*uri).second.second += rsize;
      }
    }
  }

  cout << "\nOverall:\n"
       << "   " << net_num_textures << " textures, " 
       << net_num_placed << " of which are packed onto " 
       << net_num_palettes << " palettes (" 
       << net_num_textures - net_num_placed << " unplaced)"
       << "\n   Original texture memory: "
       << format_space(net_orig_size, true)
       << "\n   After resizing:          " 
       << format_space(net_resized_size, true)
       << "\n   After palettizing:       " 
       << format_space(net_palette_size + net_unplaced_size, true)
       << "\n\n";

  UnplacedReasons::iterator uri;
  for (uri = unplaced_reasons.begin(); 
       uri != unplaced_reasons.end();
       ++uri) {
    TextureOmitReason reason = (*uri).first;
    int count = (*uri).second.first;
    int size = (*uri).second.second;
    cout << count << " textures (" << format_space(size)
	 << ") unplaced because ";
    switch (reason) {
    case PTexture::OR_none:
      cout << "of no reason--textures should have been placed\n";
      break;
      
    case PTexture::OR_size:
      cout << "size was too large for palette\n";
      break;
      
    case PTexture::OR_repeats:
      cout << "repeating\n";
      break;
      
    case PTexture::OR_omitted:
      cout << "explicitly omitted\n";
      break;
      
    case PTexture::OR_unused:
      cout << "unused by any egg file\n";
      break;
      
    case PTexture::OR_unknown:
      cout << "texture file is missing\n";
      break;
      
    case PTexture::OR_cmdline:
      cout << "-x was given on command line\n";
      break;
      
    case PTexture::OR_solitary:
      cout << "texture was alone on a palette\n";
      break;
      
    default:
      cout << "unknown reason\n";
    }
  }

  cout << "\n";
  */
}

////////////////////////////////////////////////////////////////////
//     Function: EggPalettize::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggPalettize::
run() {
  _force_power_2 = _got_force_power_2;
  _aggressively_clean_mapdir = _got_aggressively_clean_mapdir;

  bool okflag = true;

  // We'll repeat the processing steps for each attrib file.  If we
  // have multiple attrib files, then we have no egg files, so all of
  // the egg loops below fall out.  On the other hand, if we do have
  // egg files, then we have only one attrib file, so this outer loop
  // falls out.

  AttribFiles::iterator afi;
  for (afi = _attrib_files.begin(); afi != _attrib_files.end(); ++afi) {
    AttribFile &af = *(*afi);

    if (!af.grab_lock()) {
      // Failing to grab the write lock on the attribute file is a
      // fatal error.
      exit(1);
    }

    if (_statistics_only) {
      nout << "Reading " << af.get_name() << "\n";
      okflag = af.read(false);

    } else {
      nout << "Processing " << af.get_name() << "\n";

      if (!af.read(_force_redo_all)) {
	// Failing to read the attribute file is a fatal error.
	exit(1);
      }
      
      af.update_params(this);
      
      // Get all the texture references out of the egg files.
      Eggs::iterator ei;
      for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
	SourceEgg *egg = DCAST(SourceEgg, *ei);
	egg->get_textures(this);
      }

      // Assign textures into the appropriate groups.
      af.get_egg_group_requests();
      
      // Apply the user's requested size changes onto the textures.
      af.get_size_requests();
      af.update_texture_flags();
      
      if (af.prepare_repack(_force_optimal) || _force_redo_all) {
	if (_force_redo_all || _force_optimal) {
	  af.repack_all_textures();
	} else {
	  af.repack_some_textures();
	}
	
      } else {
	nout << "No changes to palette arrangement are required.\n";
      }
      
      af.finalize_palettes();
      
      if (_optimal_resize) {
	af.optimal_resize();
      }
      
      if (_remove_unused_lines) {
	af.remove_unused_lines();
      }
      
      if (!af.write()) {
	// Failing to rewrite the attribute file is a fatal error.
	exit(1);
      }
      
      // And rebuild whatever images are necessary.
      okflag = af.generate_palette_images() && okflag;
      okflag = af.transfer_unplaced_images(_force_redo_all) && okflag;
      
      // Now apply the palettization effects to the egg files.
      for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
	SourceEgg *egg = DCAST(SourceEgg, *ei);
	egg->update_trefs();
      }
      
      // Write out the egg files.
      write_eggs();
      
      if (_touch_eggs) {
	af.touch_dirty_egg_files(_force_redo_all, _eggs_include_images);
      }
    }

    okflag = af.release_lock() && okflag;
  }

  if (_statistics_only) {
    report_statistics();
  }

  if (!okflag) {
    exit(1);
  }
}

int 
main(int argc, char *argv[]) {
  EggPalettize prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
