/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file daeMaterials.h
 * @author rdb
 * @date 2008-10-03
 */

#ifndef DAEMATERIALS_H
#define DAEMATERIALS_H

#include "pandatoolbase.h"
#include "eggMaterial.h"
#include "eggTexture.h"
#include "eggPrimitive.h"
#include "eggGroup.h"
#include "pointerTo.h"
#include "pt_EggTexture.h"
#include "pt_EggMaterial.h"

#include "pre_fcollada_include.h"
#include <FCollada.h>
#include <FCDocument/FCDGeometryInstance.h>
#include <FCDocument/FCDMaterialInstance.h>
#include <FCDocument/FCDEffectStandard.h>
#include <FCDocument/FCDEffectParameterSampler.h>
#include <FCDocument/FCDExtra.h>

/**
 * This class is seperated from the converter file because otherwise it would
 * get too big and needlessly complicated.
 */
class DaeMaterials : public TypedReferenceCount {
public:
  DaeMaterials(const FCDGeometryInstance* geometry_instance);
  virtual ~DaeMaterials() {};

  void add_material_instance(const FCDMaterialInstance* instance);
  void apply_to_primitive(const std::string semantic, const PT(EggPrimitive) to);
  void apply_to_group(const std::string semantic, const PT(EggGroup) to, bool invert_transparency=false);
  const std::string get_uvset_name(const std::string semantic, FUDaeGeometryInput::Semantic input_semantic, int32 input_set);

  static EggTexture::TextureType convert_texture_type(const FCDEffectParameterSampler::SamplerType orig_type);
  static EggTexture::WrapMode convert_wrap_mode(const FUDaeTextureWrapMode::WrapMode orig_mode);
  static EggTexture::FilterType convert_filter_type(const FUDaeTextureFilterFunction::FilterFunction orig_type);

private:
  // Holds stuff for color blend attribs.
  struct DaeBlendSettings : public ReferenceCount {
    bool _enabled;
    LColor _color;
    EggGroup::BlendOperand _operand_a;
    EggGroup::BlendOperand _operand_b;
  };

  // Holds information to bind texcoord inputs to textures.
  struct DaeVertexInputBinding : public ReferenceCount {
    int32 _input_set;
    FUDaeGeometryInput::Semantic _input_semantic;
    std::string _semantic;
  };

  // Holds stuff for an individual material.
  struct DaeMaterial : public ReferenceCount {
    pvector<PT_EggTexture> _egg_textures;
    PT_EggMaterial _egg_material;
    bool _double_sided;
    pvector<PT(DaeVertexInputBinding)> _uvsets;
    PT(DaeBlendSettings) _blend;
  };

  void process_texture_bucket(const std::string semantic, const FCDEffectStandard* effect_common, FUDaeTextureChannel::Channel bucket, EggTexture::EnvType envtype = EggTexture::ET_unspecified, EggTexture::Format format = EggTexture::F_unspecified);
  void process_extra(const std::string semantic, const FCDExtra* extra);
  static PT(DaeBlendSettings) convert_blend(FCDEffectStandard::TransparencyMode mode, const LColor &transparent, double transparency);

  pmap<const std::string, PT(DaeMaterial)> _materials;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "DaeMaterials",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
