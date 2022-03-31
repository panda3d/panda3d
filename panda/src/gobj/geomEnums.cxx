/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomEnums.cxx
 * @author drose
 * @date 2005-04-14
 */

#include "geomEnums.h"
#include "string_utils.h"
#include "config_gobj.h"

using std::istream;
using std::ostream;
using std::string;


/**
 *
 */
ostream &
operator << (ostream &out, GeomEnums::UsageHint usage_hint) {
  switch (usage_hint) {
  case GeomEnums::UH_client:
    return out << "client";

  case GeomEnums::UH_stream:
    return out << "stream";

  case GeomEnums::UH_dynamic:
    return out << "dynamic";

  case GeomEnums::UH_static:
    return out << "static";

  case GeomEnums::UH_unspecified:
    return out << "unspecified";
  }

  return out << "**invalid usage hint (" << (int)usage_hint << ")**";
}

/**
 *
 */
istream &
operator >> (istream &in, GeomEnums::UsageHint &usage_hint) {
  string word;
  in >> word;

  if (cmp_nocase(word, "client") == 0) {
    usage_hint = GeomEnums::UH_client;
  } else if (cmp_nocase(word, "stream") == 0) {
    usage_hint = GeomEnums::UH_stream;
  } else if (cmp_nocase(word, "dynamic") == 0) {
    usage_hint = GeomEnums::UH_dynamic;
  } else if (cmp_nocase(word, "static") == 0) {
    usage_hint = GeomEnums::UH_static;
  } else if (cmp_nocase(word, "unspecified") == 0) {
    usage_hint = GeomEnums::UH_unspecified;

  } else {
    gobj_cat->error() << "Invalid usage hint value: " << word << "\n";
    usage_hint = GeomEnums::UH_unspecified;
  }

  return in;
}

/**
 *
 */
ostream &
operator << (ostream &out, GeomEnums::NumericType numeric_type) {
  switch (numeric_type) {
  case GeomEnums::NT_uint8:
    return out << "uint8";

  case GeomEnums::NT_uint16:
    return out << "uint16";

  case GeomEnums::NT_uint32:
    return out << "uint32";

  case GeomEnums::NT_packed_dcba:
    return out << "packed_dcba";

  case GeomEnums::NT_packed_dabc:
    return out << "packed_dabc";

  case GeomEnums::NT_float32:
    return out << "float32";

  case GeomEnums::NT_float64:
    return out << "float64";

  case GeomEnums::NT_stdfloat:
    return out << "stdfloat";

  case GeomEnums::NT_int8:
    return out << "int8";

  case GeomEnums::NT_int16:
    return out << "int16";

  case GeomEnums::NT_int32:
    return out << "int32";

  case GeomEnums::NT_packed_ufloat:
    return out << "packed_ufloat";
  }

  return out << "**invalid numeric type (" << (int)numeric_type << ")**";
}

/**
 *
 */
ostream &
operator << (ostream &out, GeomEnums::Contents contents) {
  switch (contents) {
  case GeomEnums::C_other:
    return out << "other";

  case GeomEnums::C_point:
    return out << "point";

  case GeomEnums::C_clip_point:
    return out << "clip_point";

  case GeomEnums::C_vector:
    return out << "vector";

  case GeomEnums::C_texcoord:
    return out << "texcoord";

  case GeomEnums::C_color:
    return out << "color";

  case GeomEnums::C_index:
    return out << "index";

  case GeomEnums::C_morph_delta:
    return out << "morph_delta";

  case GeomEnums::C_matrix:
    return out << "matrix";

  case GeomEnums::C_normal:
    return out << "normal";
  }

  return out << "**invalid contents (" << (int)contents << ")**";
}
