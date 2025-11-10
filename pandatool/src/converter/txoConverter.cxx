/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file txoConverter.cxx
 * @author RegDogg
 * @date 2025-11-10
 */

#include "txoConverter.h"
#include "pnmFileTypeRegistry.h"
#include "pnmFileType.h"

/**
 *
 */
TxoConverter::
TxoConverter() : WithOutputFile(true, false, true) {
  set_program_brief("convert various image formats to .txo file format");
  set_program_description
    ("This program reads the image date from the input file and "
     "outputs a txo files, suitable for viewing in Panda.");

  clear_runlines();
  add_runline("[opts] input output");
  add_runline("[opts] -o output input");

  add_option
    ("o", "filename", 0,
     "Specify the filename to which the resulting .bam file will be written.  "
     "If this option is omitted, the last parameter name is taken to be the "
     "name of the output file.",
     &TxoConverter::dispatch_filename, &_got_output_filename, &_output_filename);

  add_option
    ("alpha", "filename", 0,
     "Apply an RGB alpha file for image types such as JPEG.",
     &TxoConverter::dispatch_filename, &_got_rgb_filename, &_rgb_filename);
}

/**
 *
 */
void TxoConverter::
run() {
  nassertv(has_output_filename());
  Filename fullpath = Filename(_image_filename.get_fullpath());

  nout << "Reading " << fullpath << "...\n";

  PNMFileType *type = PNMFileTypeRegistry::get_global_ptr()->get_type_from_extension(fullpath);
  if (type == nullptr) {
    nout << "Cannot determine type of image file " << fullpath << ".\n";
    nout << "Currently supported image types:\n";
    PNMFileTypeRegistry::get_global_ptr()->write(nout, 2);
    nout << "\n";
    return;
  }

  PT(Texture) tex = new Texture("original image");

  if (_got_rgb_filename) {
    PNMFileType *type = PNMFileTypeRegistry::get_global_ptr()->get_type_from_extension(_rgb_filename);
    if (type == nullptr) {
      nout << "Image file type '" << _rgb_filename << "' is unknown.\n";
      return;
    }
    tex->read(_image_filename, _rgb_filename.get_fullpath(), 0, 0, LoaderOptions());
  }
  else {
    tex->read(_image_filename, LoaderOptions());
  }
  tex->get_ram_image();

  convert_txo(tex);

}


/**
 * If the indicated Texture was not already loaded from a txo file, writes it
 * to a txo file and updates the Texture object to reference the new file.
 */
void TxoConverter::
convert_txo(Texture *tex) {
  if (!tex->get_loaded_from_txo()) {

    Filename output = get_output_filename();
    output.make_dir();

    if (tex->write(output)) {
      nout << "Writing " << output << "...\n";
      tex->set_loaded_from_txo();
      tex->set_fullpath(output);
      tex->clear_alpha_fullpath();

      tex->set_filename(output);
      tex->clear_alpha_filename();
    }
  }
}

/**
 *
 */
bool TxoConverter::
handle_args(ProgramBase::Args &args) {
  if (!_got_output_filename && args.size() > 1) {
    _got_output_filename = true;
    _output_filename = Filename::from_os_specific(args.back());
    args.pop_back();
  }

  if (!_got_output_filename) {
    nout << "You must specify an output path.";
    return false;
  }

  if ((_output_filename.get_extension() != "txo")) {
    nout << "Output filename " << _output_filename << " must end in .txo\n";
    return false;
  }

  if (args.empty()) {
    nout << "You must specify the image file to read on the command line.\n";
    return false;
  }

  if (args.size() != 1) {
    nout << "Specify only one image on the command line.\n";
    return false;
  }

  _image_filename = Filename::from_os_specific(args[0]);

  return true;
}

int main(int argc, char *argv[]) {
  TxoConverter prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}