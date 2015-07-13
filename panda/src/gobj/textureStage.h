// Filename: textureStage.h
// Created by:  drose (14Jul04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTURESTAGE_H
#define TEXTURESTAGE_H

#include "pandabase.h"

#include "internalName.h"
#include "pointerTo.h"
#include "typedWritableReferenceCount.h"
#include "updateSeq.h"
#include "luse.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : TextureStage
// Description : Defines the properties of a named stage of the
//               multitexture pipeline.  The TextureAttrib will
//               associated a number of these stages with Texture
//               objects, and the GSG will render geometry by sorting
//               all of the currently active TextureStages in order
//               and then issuing the appropriate rendering calls to
//               activate them.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ TextureStage : public TypedWritableReferenceCount {
PUBLISHED:
  explicit TextureStage(const string &name);
  INLINE TextureStage(TextureStage &copy);
  void operator = (const TextureStage &copy);

  virtual ~TextureStage();

  enum Mode {
    // Modes that pertain to the fixed-function pipeline.

    M_modulate,
    M_decal,
    M_blend,
    M_replace,
    M_add,
    M_combine,
    M_blend_color_scale,

    M_modulate_glow,  // When fixed-function, equivalent to modulate.
    M_modulate_gloss, // When fixed-function, equivalent to modulate.

    // Modes that are only relevant to shader-based rendering.

    M_normal,
    M_normal_height,
    M_glow,         // Rarely used: modulate_glow  is more efficient.
    M_gloss,        // Rarely used: modulate_gloss is more efficient.
    M_height,       // Rarely used: normal_height  is more efficient.
    M_selector,
    M_normal_gloss,
  };

  enum CombineMode {
    CM_undefined,
    CM_replace,
    CM_modulate,
    CM_add,
    CM_add_signed,
    CM_interpolate,
    CM_subtract,

    // The following are valid only for combine_rgb, not
    // combine_alpha.
    CM_dot3_rgb,
    CM_dot3_rgba,
  };

  enum CombineSource {
    CS_undefined,
    CS_texture,
    CS_constant,
    CS_primary_color,
    CS_previous,
    CS_constant_color_scale,
    CS_last_saved_result,
  };

  enum CombineOperand {
    CO_undefined,
    CO_src_color,
    CO_one_minus_src_color,
    CO_src_alpha,
    CO_one_minus_src_alpha,
  };

  INLINE void set_name(const string &name);
  INLINE const string &get_name() const;

  INLINE void set_sort(int sort);
  INLINE int get_sort() const;

  INLINE void set_priority(int priority);
  INLINE int get_priority() const;

  INLINE void set_texcoord_name(InternalName *name);
  INLINE void set_texcoord_name(const string &texcoord_name);
  INLINE InternalName *get_texcoord_name() const;
  INLINE InternalName *get_tangent_name() const;
  INLINE InternalName *get_binormal_name() const;

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE bool is_fixed_function() const;

  INLINE void set_color(const LColor &color);
  INLINE LColor get_color() const;

  INLINE void set_rgb_scale(int rgb_scale);
  INLINE int get_rgb_scale() const;

  INLINE void set_alpha_scale(int alpha_scale);
  INLINE int get_alpha_scale() const;

  INLINE void set_saved_result(bool saved_result);
  INLINE bool get_saved_result() const;

  INLINE void set_tex_view_offset(int tex_view_offset);
  INLINE int get_tex_view_offset() const;

  INLINE void set_combine_rgb(CombineMode mode, 
                              CombineSource source0, CombineOperand operand0);
  INLINE void set_combine_rgb(CombineMode mode, 
                              CombineSource source0, CombineOperand operand0,
                              CombineSource source1, CombineOperand operand1);
  INLINE void set_combine_rgb(CombineMode mode, 
                              CombineSource source0, CombineOperand operand0,
                              CombineSource source1, CombineOperand operand1,
                              CombineSource source2, CombineOperand operand2);
  INLINE CombineMode get_combine_rgb_mode() const;
  INLINE int get_num_combine_rgb_operands() const;
  INLINE CombineSource get_combine_rgb_source0() const;
  INLINE CombineOperand get_combine_rgb_operand0() const;
  INLINE CombineSource get_combine_rgb_source1() const;
  INLINE CombineOperand get_combine_rgb_operand1() const;
  INLINE CombineSource get_combine_rgb_source2() const;
  INLINE CombineOperand get_combine_rgb_operand2() const;

  INLINE void set_combine_alpha(CombineMode mode, 
                                CombineSource source0, CombineOperand operand0);
  INLINE void set_combine_alpha(CombineMode mode, 
                                CombineSource source0, CombineOperand operand0,
                                CombineSource source1, CombineOperand operand1);
  INLINE void set_combine_alpha(CombineMode mode, 
                                CombineSource source0, CombineOperand operand0,
                                CombineSource source1, CombineOperand operand1,
                                CombineSource source2, CombineOperand operand2);
  INLINE CombineMode get_combine_alpha_mode() const;
  INLINE int get_num_combine_alpha_operands() const;
  INLINE CombineSource get_combine_alpha_source0() const;
  INLINE CombineOperand get_combine_alpha_operand0() const;
  INLINE CombineSource get_combine_alpha_source1() const;
  INLINE CombineOperand get_combine_alpha_operand1() const;
  INLINE CombineSource get_combine_alpha_source2() const;
  INLINE CombineOperand get_combine_alpha_operand2() const;

  INLINE bool involves_color_scale() const;
  INLINE bool uses_color() const;
  INLINE bool uses_primary_color() const;
  INLINE bool uses_last_saved_result() const;

  INLINE bool operator == (const TextureStage &other) const;
  INLINE bool operator != (const TextureStage &other) const;
  INLINE bool operator < (const TextureStage &other) const;

  int compare_to(const TextureStage &other) const;

  void write(ostream &out) const;
  void output(ostream &out) const;

  INLINE static TextureStage *get_default();

public:
  INLINE static UpdateSeq get_sort_seq();

private:
  INLINE void update_color_flags();

  static int get_expected_num_combine_operands(CombineMode cm);
  static bool operand_valid_for_rgb(CombineOperand co);
  static bool operand_valid_for_alpha(CombineOperand co);

  string _name;
  int _sort;
  int _priority;
  PT(InternalName) _texcoord_name;
  Mode _mode;
  LColor _color;
  int _rgb_scale;
  int _alpha_scale;
  bool _saved_result;
  int _tex_view_offset;
  bool _involves_color_scale;
  bool _uses_color;
  bool _uses_primary_color;
  bool _uses_last_saved_result;

  CombineMode _combine_rgb_mode;
  int _num_combine_rgb_operands;
  CombineSource _combine_rgb_source0;
  CombineOperand _combine_rgb_operand0;
  CombineSource _combine_rgb_source1;
  CombineOperand _combine_rgb_operand1;
  CombineSource _combine_rgb_source2;
  CombineOperand _combine_rgb_operand2;

  CombineMode _combine_alpha_mode;
  int _num_combine_alpha_operands;
  CombineSource _combine_alpha_source0;
  CombineOperand _combine_alpha_operand0;
  CombineSource _combine_alpha_source1;
  CombineOperand _combine_alpha_operand1;
  CombineSource _combine_alpha_source2;
  CombineOperand _combine_alpha_operand2;

  static PT(TextureStage) _default_stage;
  static UpdateSeq _sort_seq;

public:
  // Datagram stuff
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

  static TypedWritable *make_TextureStage(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "TextureStage",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const TextureStage &ts);

EXPCL_PANDA_GOBJ ostream &operator << (ostream &out, TextureStage::Mode mode);
EXPCL_PANDA_GOBJ ostream &operator << (ostream &out, TextureStage::CombineMode cm);
EXPCL_PANDA_GOBJ ostream &operator << (ostream &out, TextureStage::CombineSource cs);
EXPCL_PANDA_GOBJ ostream &operator << (ostream &out, TextureStage::CombineOperand co);


#include "textureStage.I"

#endif
