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
 * @date 2025-04-09
 */

#include "txoConverter.h"

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
  add_runline("[opts] input.png [input.png ... ]");

  add_option
    ("ow", "", 0,
     "Overwrite all existing TXO files.",
     &TxoConverter::dispatch_none, &_txo_overwrite);
}

/**
 *
 */
void TxoConverter::
run() {
    Filenames::iterator fi;
    for (fi = _filenames.begin(); fi != _filenames.end(); ++fi) {
        PNMImage image;
        Filename input = Filename(*fi);
        Filename fullpath = Filename(input.get_fullpath());

        if (!fullpath.exists()) {
          nout << "  The file '" << fullpath << "' does not exist, skipping...\n";
          continue;
        }

        image.read(fullpath);
        PT(Texture) tex = new Texture("original image");
        tex->load(image);
        tex->get_ram_image();

        convert_txo(tex, fullpath);

    }
}

/**
 * If the indicated Texture was not already loaded from a txo file, writes it
 * to a txo file and updates the Texture object to reference the new file.
 */
void TxoConverter::
convert_txo(Texture *tex, Filename output) {
  if (!tex->get_loaded_from_txo()) {

    output.set_extension("txo");

    if (output.exists() && !_txo_overwrite) {
      nout << "  The file '" << output << "' already exists, skipping...\n";
      return;
    }
    else {
      if (tex->write(output)) {
        nout << "  Writing " << output;
        if (tex->get_ram_image_compression() != Texture::CM_off) {
          nout << " (compressed " << tex->get_ram_image_compression() << ")";
        }
        nout << "\n";
        tex->set_loaded_from_txo();
        tex->set_fullpath(output);
        tex->clear_alpha_fullpath();
  
        tex->set_filename(output);
        tex->clear_alpha_filename();
      }
    }
  }
}

/**
 *
 */
bool TxoConverter::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the image file(s) to read on the command line.\n";
    return false;
  }

  ProgramBase::Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    _filenames.push_back(*ai);
  }

  return true;
}

int main(int argc, char *argv[]) {
  TxoConverter prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}