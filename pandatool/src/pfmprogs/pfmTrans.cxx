/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfmTrans.cxx
 * @author drose
 * @date 2010-12-23
 */

#include "pfmTrans.h"
#include "config_pfmprogs.h"
#include "pfmFile.h"
#include "pfmVizzer.h"
#include "texture.h"
#include "texturePool.h"
#include "pointerTo.h"
#include "string_utils.h"
#include "pandaFileStream.h"

using std::string;

/**
 *
 */
PfmTrans::
PfmTrans() {
  _no_data_nan_num_channels = 0;
  _got_transform = false;
  _transform = LMatrix4::ident_mat();
  _rotate = 0;

  add_transform_options();

  set_program_brief("transform .pfm files");
  set_program_description
    ("pfm-trans reads an pfm file and transforms it, filters it, "
     "operates on it, writing the output to another pfm file.  A pfm "
     "file contains a 2-d table of floating-point values.");

  add_option
    ("z", "", 0,
     "Treats (0,0,0) in the pfm file as a special don't-touch value.",
     &PfmTrans::dispatch_none, &_got_zero_special);

  add_option
    ("nan", "num_channels", 0,
     "Treats a NaN in any of the first num_channels channels as a special don't-touch value.",
     &PfmTrans::dispatch_int, &_got_no_data_nan, &_no_data_nan_num_channels);

  add_option
    ("resize", "width,height", 0,
     "Resamples the pfm file to scale it to the indicated grid size.  "
     "A simple box filter is applied during the scale.  Don't confuse this "
     "with -TS, which scales the individual point values, but doesn't "
     "change the number of points.",
     &PfmTrans::dispatch_int_pair, &_got_resize, &_resize);

  add_option
    ("crop", "xbegin,xend,ybegin,yend", 0,
     "Crops the pfm file to the indicated subregion.",
     &PfmTrans::dispatch_int_quad, &_got_crop, &_crop);

  add_option
    ("autocrop", "", 0,
     "Automatically crops to the smallest possible rectangle that includes "
     "all points.  Requires -z or -nan.",
     &PfmTrans::dispatch_none, &_got_autocrop);

  add_option
    ("rotate", "degrees", 0,
     "Rotates the pfm file the specified number of degrees counterclockwise, "
     "which must be a multiple of 90.",
     &PfmTrans::dispatch_int, nullptr, &_rotate);

  add_option
    ("mirror_x", "", 0,
     "Flips the pfm file about the x axis.",
     &PfmTrans::dispatch_none, &_got_mirror_x);

  add_option
    ("mirror_y", "", 0,
     "Flips the pfm file about the y axis.",
     &PfmTrans::dispatch_none, &_got_mirror_y);

  add_option
    ("o", "filename", 50,
     "Specify the filename to which the resulting pfm file will be written.  "
     "This is only valid when there is only one input pfm file on the command "
     "line.  If you want to process multiple files simultaneously, you must "
     "use -d.",
     &PfmTrans::dispatch_filename, &_got_output_filename, &_output_filename);

  add_option
    ("d", "dirname", 50,
     "Specify the name of the directory in which to write the processed pfm "
     "files.  If you are processing only one pfm file, this may be omitted "
     "in lieu of the -o option.",
     &PfmTrans::dispatch_filename, &_got_output_dirname, &_output_dirname);

  add_option
    ("vis", "filename.bam", 60,
     "Generates a bam file that represents a visualization of the pfm file "
     "as a 3-D geometric mesh.  If -vistex is specified, the mesh is "
     "textured.",
     &PfmTrans::dispatch_filename, &_got_vis_filename, &_vis_filename);

  add_option
    ("visinv", "", 60,
     "Inverts the visualization, generating a uniform 2-d mesh with the "
     "3-d depth values encoded in the texture coordinates.",
     &PfmTrans::dispatch_none, &_got_vis_inverse);

  add_option
    ("vis2d", "", 60,
     "Respect only the first two components of each depth value, ignoring z.",
     &PfmTrans::dispatch_none, &_got_vis_2d);

  add_option
    ("vistex", "texture.jpg", 60,
     "Specifies the name of the texture to apply to the visualization.",
     &PfmTrans::dispatch_filename, &_got_vistex_filename, &_vistex_filename);

  add_option
    ("ls", "filename.txt", 60,
     "Lists the points in the file to the indicated text file.",
     &PfmTrans::dispatch_filename, &_got_ls_filename, &_ls_filename);
}


/**
 *
 */
void PfmTrans::
run() {
  if ((int)(_rotate / 90) * 90 != _rotate) {
    nout << "-rotate can only accept a multiple of 90 degrees.\n";
    exit(1);
  }

  if (_got_vis_filename) {
    _mesh_root = NodePath("mesh_root");
  }

  Filenames::const_iterator fi;
  for (fi = _input_filenames.begin(); fi != _input_filenames.end(); ++fi) {
    PfmFile file;
    if (!file.read(*fi)) {
      nout << "Cannot read " << *fi << "\n";
      exit(1);
    }
    if (!process_pfm(*fi, file)) {
      exit(1);
    }
  }

  if (_got_vis_filename) {
    _mesh_root.write_bam_file(_vis_filename);
  }
}

/**
 * Handles a single pfm file.
 */
bool PfmTrans::
process_pfm(const Filename &input_filename, PfmFile &file) {
  PfmVizzer vizzer(file);
  if (_got_no_data_nan) {
    file.set_no_data_nan(_no_data_nan_num_channels);
  } else if (_got_zero_special) {
    file.set_zero_special(true);
  }
  vizzer.set_vis_inverse(_got_vis_inverse);
  vizzer.set_vis_2d(_got_vis_2d);

  if (_got_autocrop) {
    _got_crop = file.calc_autocrop(_crop[0], _crop[1], _crop[2], _crop[3]);
  }

  if (_got_crop) {
    file.apply_crop(_crop[0], _crop[1], _crop[2], _crop[3]);
  }

  if (_got_resize) {
    file.resize(_resize[0], _resize[1]);
  }

  if (_rotate != 0) {
    int r = (_rotate / 90) % 4;
    if (r < 0) {
      r += 4;
    }
    switch (r) {
    case 0:
      break;
    case 1:
      // Rotate 90 degrees ccw.
      file.flip(true, false, true);
      break;
    case 2:
      // Rotate 180 degrees.

      // Not sure right now why we can't flip both axes at once.  But it works
      // if we do one at a time.  file.flip(true, true, false);
      file.flip(true, false, false);
      file.flip(false, true, false);
      break;
    case 3:
      // Rotate 90 degrees cw.
      file.flip(false, true, true);
      break;
    default:
      nassertr(false, false);
    }
  }

  if (_got_mirror_x) {
    file.flip(true, false, false);
  }

  if (_got_mirror_y) {
    file.flip(false, true, false);
  }

  if (_got_transform) {
    file.xform(LCAST(PN_float32, _transform));
  }

  if (_got_vis_filename) {
    NodePath mesh = vizzer.generate_vis_mesh(PfmVizzer::MF_both);
    if (_got_vistex_filename) {
      PT(Texture) tex = TexturePool::load_texture(_vistex_filename);
      if (tex == nullptr) {
        nout << "Couldn't find " << _vistex_filename << "\n";
      } else {
        tex->set_minfilter(SamplerState::FT_linear_mipmap_linear);
        mesh.set_texture(tex);
        if (tex->has_alpha(tex->get_format())) {
          mesh.set_transparency(TransparencyAttrib::M_dual);
        }
      }
    }
    mesh.set_name(input_filename.get_basename_wo_extension());
    mesh.reparent_to(_mesh_root);
  }

  if (_got_ls_filename) {
    pofstream out;
    _ls_filename.set_text();
    if (_ls_filename.open_write(out, true)) {
      for (int yi = 0; yi < file.get_y_size(); ++yi) {
        for (int xi = 0; xi < file.get_x_size(); ++xi) {
          if (file.has_point(xi, yi)) {
            out << "(" << xi << ", " << yi << "):";
            for (int ci = 0; ci < file.get_num_channels(); ++ci) {
              out << " " << file.get_channel(xi, yi, ci);
            }
            out << "\n";
          }
        }
      }
    }
  }

  Filename output_filename;
  if (_got_output_filename) {
    output_filename = _output_filename;
  } else if (_got_output_dirname) {
    output_filename = Filename(_output_dirname, input_filename.get_basename());
  }

  if (!output_filename.empty()) {
    return file.write(output_filename);
  }

  return true;
}

/**
 * Adds -TS, -TT, etc.  as valid options for this program.  If the user
 * specifies one of the options on the command line, the data will be
 * transformed when the egg file is written out.
 */
void PfmTrans::
add_transform_options() {
  add_option
    ("TS", "sx[,sy,sz]", 49,
     "Scale the model uniformly by the given factor (if only one number "
     "is given) or in each axis by sx, sy, sz (if three numbers are given).",
     &PfmTrans::dispatch_scale, &_got_transform, &_transform);

  add_option
    ("TR", "x,y,z", 49,
     "Rotate the model x degrees about the x axis, then y degrees about the "
     "y axis, and then z degrees about the z axis.",
     &PfmTrans::dispatch_rotate_xyz, &_got_transform, &_transform);

  add_option
    ("TA", "angle,x,y,z", 49,
     "Rotate the model angle degrees counterclockwise about the given "
     "axis.",
     &PfmTrans::dispatch_rotate_axis, &_got_transform, &_transform);

  add_option
    ("TT", "x,y,z", 49,
     "Translate the model by the indicated amount.\n\n"
     "All transformation options (-TS, -TR, -TA, -TT) are cumulative and are "
     "applied in the order they are encountered on the command line.",
     &PfmTrans::dispatch_translate, &_got_transform, &_transform);
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool PfmTrans::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the pfm file(s) to read on the command line.\n";
    return false;
  }

  if (_got_output_filename && args.size() == 1) {
    if (_got_output_dirname) {
      nout << "Cannot specify both -o and -d.\n";
      return false;
    }

  } else {
    if (_got_output_filename) {
      nout << "Cannot use -o when multiple pfm files are specified.\n";
      return false;
    }
  }

  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    _input_filenames.push_back(Filename::from_os_specific(*ai));
  }

  return true;
}

/**
 * Handles -TS, which specifies a scale transform.  Var is an LMatrix4.
 */
bool PfmTrans::
dispatch_scale(const string &opt, const string &arg, void *var) {
  LMatrix4 *transform = (LMatrix4 *)var;

  vector_string words;
  tokenize(arg, words, ",");

  PN_stdfloat sx, sy, sz;

  bool okflag = false;
  if (words.size() == 3) {
    okflag =
      string_to_stdfloat(words[0], sx) &&
      string_to_stdfloat(words[1], sy) &&
      string_to_stdfloat(words[2], sz);

  } else if (words.size() == 1) {
    okflag =
      string_to_stdfloat(words[0], sx);
    sy = sz = sx;
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires one or three numbers separated by commas.\n";
    return false;
  }

  *transform = (*transform) * LMatrix4::scale_mat(sx, sy, sz);

  return true;
}

/**
 * Handles -TR, which specifies a rotate transform about the three cardinal
 * axes.  Var is an LMatrix4.
 */
bool PfmTrans::
dispatch_rotate_xyz(ProgramBase *self, const string &opt, const string &arg, void *var) {
  PfmTrans *base = (PfmTrans *)self;
  return base->ns_dispatch_rotate_xyz(opt, arg, var);
}

/**
 * Handles -TR, which specifies a rotate transform about the three cardinal
 * axes.  Var is an LMatrix4.
 */
bool PfmTrans::
ns_dispatch_rotate_xyz(const string &opt, const string &arg, void *var) {
  LMatrix4 *transform = (LMatrix4 *)var;

  vector_string words;
  tokenize(arg, words, ",");

  LVecBase3 xyz;

  bool okflag = false;
  if (words.size() == 3) {
    okflag =
      string_to_stdfloat(words[0], xyz[0]) &&
      string_to_stdfloat(words[1], xyz[1]) &&
      string_to_stdfloat(words[2], xyz[2]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires three numbers separated by commas.\n";
    return false;
  }

  LMatrix4 mat =
    LMatrix4::rotate_mat(xyz[0], LVector3(1.0, 0.0, 0.0)) *
    LMatrix4::rotate_mat(xyz[1], LVector3(0.0, 1.0, 0.0)) *
    LMatrix4::rotate_mat(xyz[2], LVector3(0.0, 0.0, 1.0));

  *transform = (*transform) * mat;

  return true;
}

/**
 * Handles -TA, which specifies a rotate transform about an arbitrary axis.
 * Var is an LMatrix4.
 */
bool PfmTrans::
dispatch_rotate_axis(ProgramBase *self, const string &opt, const string &arg, void *var) {
  PfmTrans *base = (PfmTrans *)self;
  return base->ns_dispatch_rotate_axis(opt, arg, var);
}

/**
 * Handles -TA, which specifies a rotate transform about an arbitrary axis.
 * Var is an LMatrix4.
 */
bool PfmTrans::
ns_dispatch_rotate_axis(const string &opt, const string &arg, void *var) {
  LMatrix4 *transform = (LMatrix4 *)var;

  vector_string words;
  tokenize(arg, words, ",");

  PN_stdfloat angle;
  LVecBase3 axis;

  bool okflag = false;
  if (words.size() == 4) {
    okflag =
      string_to_stdfloat(words[0], angle) &&
      string_to_stdfloat(words[1], axis[0]) &&
      string_to_stdfloat(words[2], axis[1]) &&
      string_to_stdfloat(words[3], axis[2]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires four numbers separated by commas.\n";
    return false;
  }

  *transform = (*transform) * LMatrix4::rotate_mat(angle, axis);

  return true;
}

/**
 * Handles -TT, which specifies a translate transform.  Var is an LMatrix4.
 */
bool PfmTrans::
dispatch_translate(const string &opt, const string &arg, void *var) {
  LMatrix4 *transform = (LMatrix4 *)var;

  vector_string words;
  tokenize(arg, words, ",");

  LVector3 trans;

  bool okflag = false;
  if (words.size() == 3) {
    okflag =
      string_to_stdfloat(words[0], trans[0]) &&
      string_to_stdfloat(words[1], trans[1]) &&
      string_to_stdfloat(words[2], trans[2]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires three numbers separated by commas.\n";
    return false;
  }

  *transform = (*transform) * LMatrix4::translate_mat(trans);

  return true;
}


int main(int argc, char *argv[]) {
  PfmTrans prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
