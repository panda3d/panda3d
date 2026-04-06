/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filenameUnifier.h
 * @author drose
 * @date 2000-12-05
 */

#ifndef FILENAMEUNIFIER_H
#define FILENAMEUNIFIER_H

#include "pandatoolbase.h"

#include "filename.h"

#include "pmap.h"

/**
 * This static class does the job of converting filenames from relative to
 * absolute to canonical or whatever is appropriate.  Its main purpose is to
 * allow us to write relative pathnames to the bam file and turn them back
 * into absolute pathnames on read, so that a given bam file does not get tied
 * to absolute pathnames.
 */
class FilenameUnifier {
public:
  static void set_txa_filename(const Filename &txa_filename);
  static void set_rel_dirname(const Filename &rel_dirname);

  static Filename make_bam_filename(Filename filename);
  static Filename get_bam_filename(Filename filename);
  static Filename make_egg_filename(Filename filename);
  static Filename make_user_filename(Filename filename);
  static void make_canonical(Filename &filename);

private:

  static Filename _txa_filename;
  static Filename _txa_dir;
  static Filename _rel_dirname;

  typedef pmap<std::string, std::string> CanonicalFilenames;
  static CanonicalFilenames _canonical_filenames;
};

#endif
