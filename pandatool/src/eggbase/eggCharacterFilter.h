// Filename: eggCharacterFilter.h
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGCHARACTERFILTER_H
#define EGGCHARACTERFILTER_H

#include <pandatoolbase.h>

#include "eggMultiFilter.h"

class EggCharacterData;

////////////////////////////////////////////////////////////////////
// 	 Class : EggCharacterFilter
// Description : This is the base class for a family of programs that
//               operate on a number of character models and their
//               associated animation files together.  It reads in a
//               number of egg files, any combination of model files
//               or character files which must all represent the same
//               character skeleton, and maintains a single hierarchy
//               of joints and sliders that may be operated on before
//               writing the files back out.
////////////////////////////////////////////////////////////////////
class EggCharacterFilter : public EggMultiFilter {
public:
  EggCharacterFilter();
  virtual ~EggCharacterFilter();

protected:
  virtual bool post_command_line();

  virtual EggCharacterData *make_character_data();

  EggCharacterData *_character_data;
};

#endif


