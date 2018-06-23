/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggRenderMode.cxx
 * @author drose
 * @date 1999-01-20
 */

#include "eggRenderMode.h"
#include "indent.h"
#include "string_utils.h"
#include "pnotify.h"

using std::istream;
using std::ostream;
using std::string;

TypeHandle EggRenderMode::_type_handle;

/**
 *
 */
EggRenderMode::
EggRenderMode() {
  _alpha_mode = AM_unspecified;
  _depth_write_mode = DWM_unspecified;
  _depth_test_mode = DTM_unspecified;
  _visibility_mode = VM_unspecified;
  _depth_offset = 0;
  _has_depth_offset = false;
  _draw_order = 0;
  _has_draw_order = false;
}

/**
 *
 */
EggRenderMode &EggRenderMode::
operator = (const EggRenderMode &copy) {
  _alpha_mode = copy._alpha_mode;
  _depth_write_mode = copy._depth_write_mode;
  _depth_test_mode = copy._depth_test_mode;
  _visibility_mode = copy._visibility_mode;
  _depth_offset = copy._depth_offset;
  _has_depth_offset = copy._has_depth_offset;
  _draw_order = copy._draw_order;
  _has_draw_order = copy._has_draw_order;
  return *this;
}

/**
 * Writes the attributes to the indicated output stream in Egg format.
 */
void EggRenderMode::
write(ostream &out, int indent_level) const {
  if (get_alpha_mode() != AM_unspecified) {
    indent(out, indent_level)
      << "<Scalar> alpha { " << get_alpha_mode() << " }\n";
  }
  if (get_depth_write_mode() != DWM_unspecified) {
    indent(out, indent_level)
      << "<Scalar> depth_write { " << get_depth_write_mode() << " }\n";
  }
  if (get_depth_test_mode() != DTM_unspecified) {
    indent(out, indent_level)
      << "<Scalar> depth_test { " << get_depth_test_mode() << " }\n";
  }
  if (get_visibility_mode() != VM_unspecified) {
    indent(out, indent_level)
      << "<Scalar> visibility { " << get_visibility_mode() << " }\n";
  }
  if (has_depth_offset()) {
    indent(out, indent_level)
      << "<Scalar> depth-offset { " << get_depth_offset() << " }\n";
  }
  if (has_draw_order()) {
    indent(out, indent_level)
      << "<Scalar> draw-order { " << get_draw_order() << " }\n";
  }
  if (has_bin()) {
    indent(out, indent_level)
      << "<Scalar> bin { " << get_bin() << " }\n";
  }
}

/**
 *
 */
bool EggRenderMode::
operator == (const EggRenderMode &other) const {
  if (_alpha_mode != other._alpha_mode ||
      _depth_write_mode != other._depth_write_mode ||
      _depth_test_mode != other._depth_test_mode ||
      _visibility_mode != other._visibility_mode ||
      _has_depth_offset != other._has_depth_offset ||
      _has_draw_order != other._has_draw_order) {
    return false;
  }

  if (_has_depth_offset) {
    if (_depth_offset != other._depth_offset) {
      return false;
    }
  }

  if (_has_draw_order) {
    if (_draw_order != other._draw_order) {
      return false;
    }
  }

  if (_bin != other._bin) {
    return false;
  }

  return true;
}

/**
 *
 */
bool EggRenderMode::
operator < (const EggRenderMode &other) const {
  if (_alpha_mode != other._alpha_mode) {
    return (int)_alpha_mode < (int)other._alpha_mode;
  }
  if (_depth_write_mode != other._depth_write_mode) {
    return (int)_depth_write_mode < (int)other._depth_write_mode;
  }
  if (_depth_test_mode != other._depth_test_mode) {
    return (int)_depth_test_mode < (int)other._depth_test_mode;
  }
  if (_visibility_mode != other._visibility_mode) {
    return (int)_visibility_mode < (int)other._visibility_mode;
  }

  if (_has_depth_offset != other._has_depth_offset) {
    return (int)_has_depth_offset < (int)other._has_depth_offset;
  }
  if (_has_draw_order != other._has_draw_order) {
    return (int)_has_draw_order < (int)other._has_draw_order;
  }

  if (_has_depth_offset) {
    if (_depth_offset != other._depth_offset) {
      return _depth_offset < other._depth_offset;
    }
  }
  if (_has_draw_order) {
    if (_draw_order != other._draw_order) {
      return _draw_order < other._draw_order;
    }
  }

  if (_bin != other._bin) {
    return _bin < other._bin;
  }

  return false;
}

/**
 * Returns the AlphaMode value associated with the given string
 * representation, or AM_unspecified if the string does not match any known
 * AlphaMode value.
 */
EggRenderMode::AlphaMode EggRenderMode::
string_alpha_mode(const string &string) {
  if (cmp_nocase_uh(string, "off") == 0) {
    return AM_off;
  } else if (cmp_nocase_uh(string, "on") == 0) {
    return AM_on;
  } else if (cmp_nocase_uh(string, "blend") == 0) {
    return AM_blend;
  } else if (cmp_nocase_uh(string, "blend_no_occlude") == 0) {
    return AM_blend_no_occlude;
  } else if (cmp_nocase_uh(string, "ms") == 0) {
    return AM_ms;
  } else if (cmp_nocase_uh(string, "ms_mask") == 0) {
    return AM_ms_mask;
  } else if (cmp_nocase_uh(string, "binary") == 0) {
    return AM_binary;
  } else if (cmp_nocase_uh(string, "dual") == 0) {
    return AM_dual;
  } else if (cmp_nocase_uh(string, "premultiplied") == 0) {
    return AM_premultiplied;
  } else {
    return AM_unspecified;
  }
}

/**
 * Returns the DepthWriteMode value associated with the given string
 * representation, or DWM_unspecified if the string does not match any known
 * DepthWriteMode value.
 */
EggRenderMode::DepthWriteMode EggRenderMode::
string_depth_write_mode(const string &string) {
  if (cmp_nocase_uh(string, "off") == 0) {
    return DWM_off;
  } else if (cmp_nocase_uh(string, "on") == 0) {
    return DWM_on;
  } else {
    return DWM_unspecified;
  }
}

/**
 * Returns the DepthTestMode value associated with the given string
 * representation, or DTM_unspecified if the string does not match any known
 * DepthTestMode value.
 */
EggRenderMode::DepthTestMode EggRenderMode::
string_depth_test_mode(const string &string) {
  if (cmp_nocase_uh(string, "off") == 0) {
    return DTM_off;
  } else if (cmp_nocase_uh(string, "on") == 0) {
    return DTM_on;
  } else {
    return DTM_unspecified;
  }
}

/**
 * Returns the HiddenMode value associated with the given string
 * representation, or VM_unspecified if the string does not match any known
 * HiddenMode value.
 */
EggRenderMode::VisibilityMode EggRenderMode::
string_visibility_mode(const string &string) {
  if (cmp_nocase_uh(string, "hidden") == 0) {
    return VM_hidden;
  } else if (cmp_nocase_uh(string, "normal") == 0) {
    return VM_normal;
  } else {
    return VM_unspecified;
  }
}


/**
 *
 */
ostream &operator << (ostream &out, EggRenderMode::AlphaMode mode) {
  switch (mode) {
  case EggRenderMode::AM_unspecified:
    return out << "unspecified";
  case EggRenderMode::AM_off:
    return out << "off";
  case EggRenderMode::AM_on:
    return out << "on";
  case EggRenderMode::AM_blend:
    return out << "blend";
  case EggRenderMode::AM_blend_no_occlude:
    return out << "blend_no_occlude";
  case EggRenderMode::AM_ms:
    return out << "ms";
  case EggRenderMode::AM_ms_mask:
    return out << "ms_mask";
  case EggRenderMode::AM_binary:
    return out << "binary";
  case EggRenderMode::AM_dual:
    return out << "dual";
  case EggRenderMode::AM_premultiplied:
    return out << "premultiplied";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
istream &operator >> (istream &in, EggRenderMode::AlphaMode &mode) {
  string word;
  in >> word;
  mode = EggRenderMode::string_alpha_mode(word);
  return in;
}

/**
 *
 */
ostream &operator << (ostream &out, EggRenderMode::DepthWriteMode mode) {
  switch (mode) {
  case EggRenderMode::DWM_unspecified:
    return out << "unspecified";
  case EggRenderMode::DWM_off:
    return out << "off";
  case EggRenderMode::DWM_on:
    return out << "on";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggRenderMode::DepthTestMode mode) {
  switch (mode) {
  case EggRenderMode::DTM_unspecified:
    return out << "unspecified";
  case EggRenderMode::DTM_off:
    return out << "off";
  case EggRenderMode::DTM_on:
    return out << "on";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}



/**
 *
 */
ostream &operator << (ostream &out, EggRenderMode::VisibilityMode mode) {
  switch (mode) {
  case EggRenderMode::VM_unspecified:
    return out << "unspecified";
  case EggRenderMode::VM_hidden:
    return out << "hidden";
  case EggRenderMode::VM_normal:
    return out << "normal";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}
