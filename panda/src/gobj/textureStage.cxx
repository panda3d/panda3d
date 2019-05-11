/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureStage.cxx
 * @author MAsaduzz
 * @date 2004-07-16
 */

#include "textureStage.h"
#include "internalName.h"
#include "bamReader.h"
#include "bamWriter.h"

using std::ostream;

PT(TextureStage) TextureStage::_default_stage;
UpdateSeq TextureStage::_sort_seq;

TypeHandle TextureStage::_type_handle;

/**
 * Initialize the texture stage at construction
 */
TextureStage::
TextureStage(const std::string &name) : _used_by_auto_shader(false) {
  _name = name;
  _sort = 0;
  _priority = 0;
  _texcoord_name = InternalName::get_texcoord();
  _mode = M_modulate;
  _color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _rgb_scale = 1;
  _alpha_scale = 1;
  _saved_result = false;
  _tex_view_offset = 0;
  _combine_rgb_mode = CM_undefined;
  _num_combine_rgb_operands = 0;
  _combine_rgb_source0 = CS_undefined;
  _combine_rgb_operand0 = CO_undefined;
  _combine_rgb_source1 = CS_undefined;
  _combine_rgb_operand1 = CO_undefined;
  _combine_rgb_source2 = CS_undefined;
  _combine_rgb_operand2 = CO_undefined;
  _combine_alpha_mode = CM_undefined;
  _num_combine_alpha_operands = 0;
  _combine_alpha_source0 = CS_undefined;
  _combine_alpha_operand0 = CO_undefined;
  _combine_alpha_source1 = CS_undefined;
  _combine_alpha_operand1 = CO_undefined;
  _combine_alpha_source2 = CS_undefined;
  _combine_alpha_operand2 = CO_undefined;

  _uses_color = false;
  _involves_color_scale = false;
}

/**
 * just copy the members of other to this
 */
void TextureStage::
operator = (const TextureStage &other) {
  _name = other._name;
  _sort = other._sort;
  _priority = other._priority;
  _texcoord_name = other._texcoord_name;
  _mode = other._mode;
  _color = other._color;
  _rgb_scale = other._rgb_scale;
  _alpha_scale = other._alpha_scale;
  _saved_result = other._saved_result;
  _tex_view_offset = other._tex_view_offset;

  _combine_rgb_mode = other._combine_rgb_mode;
  _combine_rgb_source0 = other._combine_rgb_source0;
  _combine_rgb_operand0 = other._combine_rgb_operand0;
  _combine_rgb_source1 = other._combine_rgb_source1;
  _combine_rgb_operand1 = other._combine_rgb_operand1;
  _combine_rgb_source2 = other._combine_rgb_source2;
  _combine_rgb_operand2 = other._combine_rgb_operand2;
  _combine_alpha_mode = other._combine_alpha_mode;
  _combine_alpha_source0 = other._combine_alpha_source0;
  _combine_alpha_operand0 = other._combine_alpha_operand0;
  _combine_alpha_source1 = other._combine_alpha_source1;
  _combine_alpha_operand1 = other._combine_alpha_operand1;
  _combine_alpha_source2 = other._combine_alpha_source2;
  _combine_alpha_operand2 = other._combine_alpha_operand2;

  _uses_color = other._uses_color;
  _involves_color_scale = other._involves_color_scale;

  _used_by_auto_shader = false;
}

/**
 *
 */
TextureStage::
~TextureStage() {
}

/**
 * Returns a number less than zero if this TextureStage sorts before the other
 * one, greater than zero if it sorts after, or zero if they are equivalent.
 * The sorting order is arbitrary and largely meaningless, except to
 * differentiate different stages.
 */
int TextureStage::
compare_to(const TextureStage &other) const {
  // We put the sort parameter first, so that we sorting a list of
  // TextureStages will happen to put them in sorted order, even though we
  // don't promise to do that.  But there's no reason not to do so, and it
  // might be more convenient for the developer.
  if (get_sort() != other.get_sort()) {
    return get_sort() < other.get_sort() ? -1 : 1;
  }

  // The remaining parameters are arbitrary.  We start with the name, because
  // that's most likely to be consistent between similar TextureStages, and
  // different between different TextureStages.
  int compare = strcmp(get_name().c_str(), other.get_name().c_str());
  if (compare != 0) {
    return compare;
  }

  if (get_priority() != other.get_priority()) {
    return get_priority() < other.get_priority() ? -1 : 1;
  }
  if (get_texcoord_name() != other.get_texcoord_name()) {
    return get_texcoord_name() < other.get_texcoord_name() ? -1 : 1;
  }
  if (get_mode() != other.get_mode()) {
    return get_mode() < other.get_mode() ? -1 : 1;
  }
  if (get_rgb_scale() != other.get_rgb_scale()) {
    return get_rgb_scale() < other.get_rgb_scale() ? -1 : 1;
  }
  if (get_alpha_scale() != other.get_alpha_scale()) {
    return get_alpha_scale() < other.get_alpha_scale() ? -1 : 1;
  }
  if (get_saved_result() != other.get_saved_result()) {
    return get_saved_result() < other.get_saved_result() ? -1 : 1;
  }
  if (get_tex_view_offset() != other.get_tex_view_offset()) {
    return get_tex_view_offset() < other.get_tex_view_offset() ? -1 : 1;
  }
  if (get_mode() == M_combine) {
    if (get_combine_rgb_mode() != other.get_combine_rgb_mode()) {
      return get_combine_rgb_mode() < other.get_combine_rgb_mode() ? -1 : 1;
    }

    if (get_num_combine_rgb_operands() != other.get_num_combine_rgb_operands()) {
      return get_num_combine_rgb_operands() < other.get_num_combine_rgb_operands() ? -1 : 1;
    }
    if (get_num_combine_rgb_operands() >= 1) {
      if (get_combine_rgb_source0() != other.get_combine_rgb_source0()) {
        return get_combine_rgb_source0() < other.get_combine_rgb_source0() ? -1 : 1;
      }
      if (get_combine_rgb_operand0() != other.get_combine_rgb_operand0()) {
        return get_combine_rgb_operand0() < other.get_combine_rgb_operand0() ? -1 : 1;
      }
    }
    if (get_num_combine_rgb_operands() >= 2) {
      if (get_combine_rgb_source1() != other.get_combine_rgb_source1()) {
        return get_combine_rgb_source1() < other.get_combine_rgb_source1() ? -1 : 1;
      }
      if (get_combine_rgb_operand1() != other.get_combine_rgb_operand1()) {
        return get_combine_rgb_operand1() < other.get_combine_rgb_operand1() ? -1 : 1;
      }
    }
    if (get_num_combine_rgb_operands() >= 3) {
      if (get_combine_rgb_source2() != other.get_combine_rgb_source2()) {
        return get_combine_rgb_source2() < other.get_combine_rgb_source2() ? -1 : 1;
      }
      if (get_combine_rgb_operand2() != other.get_combine_rgb_operand2()) {
        return get_combine_rgb_operand2() < other.get_combine_rgb_operand2() ? -1 : 1;
      }
    }
    if (get_combine_alpha_mode() != other.get_combine_alpha_mode()) {
      return get_combine_alpha_mode() < other.get_combine_alpha_mode() ? -1 : 1;
    }

    if (get_num_combine_alpha_operands() != other.get_num_combine_alpha_operands()) {
      return get_num_combine_alpha_operands() < other.get_num_combine_alpha_operands() ? -1 : 1;
    }
    if (get_num_combine_alpha_operands() >= 1) {
      if (get_combine_alpha_source0() != other.get_combine_alpha_source0()) {
        return get_combine_alpha_source0() < other.get_combine_alpha_source0() ? -1 : 1;
      }
      if (get_combine_alpha_operand0() != other.get_combine_alpha_operand0()) {
        return get_combine_alpha_operand0() < other.get_combine_alpha_operand0() ? -1 : 1;
      }
    }
    if (get_num_combine_alpha_operands() >= 2) {
      if (get_combine_alpha_source1() != other.get_combine_alpha_source1()) {
        return get_combine_alpha_source1() < other.get_combine_alpha_source1() ? -1 : 1;
      }
      if (get_combine_alpha_operand1() != other.get_combine_alpha_operand1()) {
        return get_combine_alpha_operand1() < other.get_combine_alpha_operand1() ? -1 : 1;
      }
    }
    if (get_num_combine_alpha_operands() >= 3) {
      if (get_combine_alpha_source2() != other.get_combine_alpha_source2()) {
        return get_combine_alpha_source2() < other.get_combine_alpha_source2() ? -1 : 1;
      }
      if (get_combine_alpha_operand2() != other.get_combine_alpha_operand2()) {
        return get_combine_alpha_operand2() < other.get_combine_alpha_operand2() ? -1 : 1;
      }
    }
  }

  return 0;
}

/**
 * Writes the details of this stage
 */
void TextureStage::
write(ostream &out) const {
  out << "TextureStage " << get_name() << ", sort = " << get_sort() << ", priority = " << get_priority() << "\n"
      << "  texcoords = " << get_texcoord_name()->get_name()
      << ", mode = " << get_mode() << ", color = " << get_color()
      << ", scale = " << get_rgb_scale() << ", " << get_alpha_scale()
      << ", saved_result = " << get_saved_result()
      << ", tex_view_offset = " << get_tex_view_offset() << "\n";

  if (get_mode() == M_combine) {
    out << "  RGB combine mode =  " << get_combine_rgb_mode() << "\n";
    if (get_num_combine_rgb_operands() >= 1) {
      out << "    0: " << get_combine_rgb_source0() << ", "
          << get_combine_rgb_operand0() << "\n";
    }
    if (get_num_combine_rgb_operands() >= 2) {
      out << "    1: " << get_combine_rgb_source1() << ", "
          << get_combine_rgb_operand1() << "\n";
    }
    if (get_num_combine_rgb_operands() >= 3) {
      out << "    2: " << get_combine_rgb_source2() << ", "
          << get_combine_rgb_operand2() << "\n";
    }
    out << "  alpha combine mode =  " << get_combine_alpha_mode() << "\n";
    if (get_num_combine_alpha_operands() >= 1) {
      out << "    0: " << get_combine_alpha_source0() << ", "
          << get_combine_alpha_operand0() << "\n";
    }
    if (get_num_combine_alpha_operands() >= 2) {
      out << "    1: " << get_combine_alpha_source1() << ", "
          << get_combine_alpha_operand1() << "\n";
    }
    if (get_num_combine_alpha_operands() >= 3) {
      out << "    2: " << get_combine_alpha_source2() << ", "
          << get_combine_alpha_operand2() << "\n";
    }
  }
}

/**
 * Just a single line output
 */
void TextureStage::
output(ostream &out) const {
  out << "TextureStage " << get_name();
}

/**
 * Returns the number of combine operands expected with the indicated combine
 * mode (0, 1, 2, or 3).
 */
int TextureStage::
get_expected_num_combine_operands(TextureStage::CombineMode cm) {
  switch (cm) {
  case CM_undefined:
    return 0;

  case CM_replace:
    return 1;

  case CM_modulate:
  case CM_add:
  case CM_add_signed:
  case CM_subtract:
  case CM_dot3_rgb:
  case CM_dot3_rgba:
    return 2;

  case CM_interpolate:
    return 3;
  }

  return 0;
}

/**
 * Returns true if the indicated CombineOperand is valid for one of the RGB
 * modes, false otherwise.
 */
bool TextureStage::
operand_valid_for_rgb(TextureStage::CombineOperand co) {
  switch (co) {
  case CO_undefined:
    return false;

  case CO_src_color:
  case CO_one_minus_src_color:
  case CO_src_alpha:
  case CO_one_minus_src_alpha:
    return true;
  }

  return false;
}

/**
 * Returns true if the indicated CombineOperand is valid for one of the alpha
 * modes, false otherwise.
 */
bool TextureStage::
operand_valid_for_alpha(TextureStage::CombineOperand co) {
  switch (co) {
  case CO_undefined:
  case CO_src_color:
  case CO_one_minus_src_color:
    return false;

  case CO_src_alpha:
  case CO_one_minus_src_alpha:
    return true;
  }

  return false;
}

/**
 * Factory method to generate a TextureStage object
 */
void TextureStage::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_TextureStage);
}

/**
 * Factory method to generate a TextureStage object
 */
TypedWritable* TextureStage::
make_TextureStage(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  bool is_default = scan.get_bool();
  if (is_default) {
    return get_default();
  } else {
    TextureStage *me = new TextureStage("");
    me->fillin(scan, manager);
    return me;
  }
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void TextureStage::
fillin(DatagramIterator &scan, BamReader *manager) {
  _name = scan.get_string();
  _sort = scan.get_int32();
  _priority = scan.get_int32();

  manager->read_pointer(scan);

  _mode = (TextureStage::Mode) scan.get_uint8();
  _color.read_datagram(scan);

  _rgb_scale = scan.get_uint8();
  _alpha_scale = scan.get_uint8();
  _saved_result = scan.get_bool();
  _tex_view_offset = 0;
  if (manager->get_file_minor_ver() >= 26) {
    _tex_view_offset = scan.get_int32();
  }

  _combine_rgb_mode = (TextureStage::CombineMode) scan.get_uint8();
  _num_combine_rgb_operands = scan.get_uint8();
  _combine_rgb_source0 = (TextureStage::CombineSource) scan.get_uint8();
  _combine_rgb_operand0 = (TextureStage::CombineOperand) scan.get_uint8();
  _combine_rgb_source1 = (TextureStage::CombineSource) scan.get_uint8();
  _combine_rgb_operand1 = (TextureStage::CombineOperand) scan.get_uint8();
  _combine_rgb_source2 = (TextureStage::CombineSource) scan.get_uint8();
  _combine_rgb_operand2 = (TextureStage::CombineOperand) scan.get_uint8();

  _combine_alpha_mode = (TextureStage::CombineMode) scan.get_uint8();
  _num_combine_alpha_operands = scan.get_uint8();
  _combine_alpha_source0 = (TextureStage::CombineSource) scan.get_uint8();
  _combine_alpha_operand0 = (TextureStage::CombineOperand) scan.get_uint8();
  _combine_alpha_source1 = (TextureStage::CombineSource) scan.get_uint8();
  _combine_alpha_operand1 = (TextureStage::CombineOperand) scan.get_uint8();
  _combine_alpha_source2 = (TextureStage::CombineSource) scan.get_uint8();
  _combine_alpha_operand2 = (TextureStage::CombineOperand) scan.get_uint8();

  update_color_flags();
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int TextureStage::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  _texcoord_name = DCAST(InternalName, p_list[pi++]);

  return pi;
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void TextureStage::
write_datagram(BamWriter *manager, Datagram &me) {
  // These properties are read in again by fillin(), above.
  if (this == get_default()) {
    me.add_bool(true);
  } else {
    me.add_bool(false);
    me.add_string(_name);
    me.add_int32(_sort);
    me.add_int32(_priority);

    manager->write_pointer(me, _texcoord_name);

    me.add_uint8(_mode);
    _color.write_datagram(me);
    me.add_uint8(_rgb_scale);
    me.add_uint8(_alpha_scale);
    me.add_bool(_saved_result);

    if (manager->get_file_minor_ver() >= 26) {
      me.add_int32(_tex_view_offset);
    }

    me.add_uint8(_combine_rgb_mode);
    me.add_uint8(_num_combine_rgb_operands);
    me.add_uint8(_combine_rgb_source0);
    me.add_uint8(_combine_rgb_operand0);
    me.add_uint8(_combine_rgb_source1);
    me.add_uint8(_combine_rgb_operand1);
    me.add_uint8(_combine_rgb_source2);
    me.add_uint8(_combine_rgb_operand2);

    me.add_uint8(_combine_alpha_mode);
    me.add_uint8(_num_combine_alpha_operands);
    me.add_uint8(_combine_alpha_source0);
    me.add_uint8(_combine_alpha_operand0);
    me.add_uint8(_combine_alpha_source1);
    me.add_uint8(_combine_alpha_operand1);
    me.add_uint8(_combine_alpha_source2);
    me.add_uint8(_combine_alpha_operand2);
  }
}

ostream &
operator << (ostream &out, TextureStage::Mode mode) {
  switch (mode) {
  case TextureStage::M_modulate:
    return out << "modulate";

  case TextureStage::M_decal:
    return out << "decal";

  case TextureStage::M_blend:
    return out << "blend";

  case TextureStage::M_replace:
    return out << "replace";

  case TextureStage::M_add:
    return out << "add";

  case TextureStage::M_combine:
    return out << "combine";

  case TextureStage::M_blend_color_scale:
    return out << "blend_color_scale";

  case TextureStage::M_modulate_glow:
    return out << "modulate_glow";

  case TextureStage::M_modulate_gloss:
    return out << "modulate_gloss";

  case TextureStage::M_normal:
    return out << "normal";

  case TextureStage::M_normal_height:
    return out << "normal_height";

  case TextureStage::M_glow:
    return out << "glow";

  case TextureStage::M_gloss:
    return out << "gloss";

  case TextureStage::M_height:
    return out << "height";

  case TextureStage::M_selector:
    return out << "selector";

  case TextureStage::M_normal_gloss:
    return out << "normal_gloss";
  }

  return out << "**invalid Mode(" << (int)mode << ")**";
}

ostream &
operator << (ostream &out, TextureStage::CombineMode cm) {
  switch (cm) {
  case TextureStage::CM_undefined:
    return out << "undefined";

  case TextureStage::CM_replace:
    return out << "replace";

  case TextureStage::CM_modulate:
    return out << "modulate";

  case TextureStage::CM_add:
    return out << "add";

  case TextureStage::CM_add_signed:
    return out << "add_signed";

  case TextureStage::CM_interpolate:
    return out << "interpolate";

  case TextureStage::CM_subtract:
    return out << "subtract";

  case TextureStage::CM_dot3_rgb:
    return out << "dot3_rgb";

  case TextureStage::CM_dot3_rgba:
    return out << "dot3_rgba";
  }

  return out << "**invalid CombineMode(" << (int)cm << ")**";
}

ostream &
operator << (ostream &out, TextureStage::CombineSource cs) {
  switch (cs) {
  case TextureStage::CS_undefined:
    return out << "undefined";

  case TextureStage::CS_texture:
    return out << "texture";

  case TextureStage::CS_constant:
    return out << "constant";

  case TextureStage::CS_primary_color:
    return out << "primary_color";

  case TextureStage::CS_previous:
    return out << "previous";

  case TextureStage::CS_constant_color_scale:
    return out << "constant_color_scale";

  case TextureStage::CS_last_saved_result:
    return out << "last_saved_result";
  }

  return out << "**invalid CombineSource(" << (int)cs << ")**";
}

ostream &
operator << (ostream &out, TextureStage::CombineOperand co) {
  switch (co) {
  case TextureStage::CO_undefined:
    return out << "undefined";

  case TextureStage::CO_src_color:
    return out << "src_color";

  case TextureStage::CO_one_minus_src_color:
    return out << "one_minus_src_color";

  case TextureStage::CO_src_alpha:
    return out << "src_alpha";

  case TextureStage::CO_one_minus_src_alpha:
    return out << "one_minus_src_alpha";
  }

  return out << "**invalid CombineOperand(" << (int)co << ")**";
}
