/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCharacterFilter.h
 * @author drose
 * @date 2001-02-23
 */

#ifndef EGGCHARACTERFILTER_H
#define EGGCHARACTERFILTER_H

#include "pandatoolbase.h"

#include "eggMultiFilter.h"

class EggCharacterData;
class EggCharacterCollection;

/**
 * This is the base class for a family of programs that operate on a number of
 * character models and their associated animation files together.  It reads
 * in a number of egg files, any combination of model files or character files
 * which must all represent the same character skeleton, and maintains a
 * single hierarchy of joints and sliders that may be operated on before
 * writing the files back out.
 */
class EggCharacterFilter : public EggMultiFilter {
public:
  EggCharacterFilter();
  virtual ~EggCharacterFilter();

  void add_fixrest_option();

protected:
  virtual bool post_command_line();
  virtual void write_eggs();

  virtual EggCharacterCollection *make_collection();

  EggCharacterCollection *_collection;
  bool _force_initial_rest_frame;
};

#endif
