/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileParseData.h
 * @author drose
 * @date 2004-10-07
 */

#ifndef XFILEPARSEDATA_H
#define XFILEPARSEDATA_H

#include "pandatoolbase.h"
#include "xFileDataObject.h"
#include "pointerTo.h"
#include "pta_int.h"
#include "pta_double.h"
#include "pvector.h"

/**
 * This class is used to fill up the data into an XFileDataNodeTemplate object
 * as the data values are parsed out of the X file.  It only has a temporary
 * lifespan; it will be converted into actual data by
 * XFileDataNodeTemplate::finalize_parse_data().
 */
class XFileParseData {
public:
  XFileParseData();

  void yyerror(const std::string &message) const;

  enum ParseFlags {
    PF_object     = 0x001,
    PF_reference  = 0x002,
    PF_double     = 0x004,
    PF_int        = 0x008,
    PF_string     = 0x010,
    PF_any_data   = 0x01f,
  };

  PT(XFileDataObject) _object;
  PTA_double _double_list;
  PTA_int _int_list;
  std::string _string;
  int _parse_flags;

  int _line_number;
  int _col_number;
  std::string _current_line;
};

/**
 * A container for a pvector of the above objects.  We need this wrapper class
 * to avoid circular #includes; this allows XFileNode to define a forward
 * reference to this class (without having to include this file or know that
 * it contains a template class).
 */
class XFileParseDataList {
public:
  typedef pvector<XFileParseData> List;
  List _list;
};

#include "xFileParseData.I"

#endif
