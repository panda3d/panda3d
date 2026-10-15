/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_shaderattrib_bam.cxx
 */

#include "bamReader.h"
#include "bamWriter.h"
#include "datagramBuffer.h"
#include "panda.h"
#include "pandaNode.h"
#include "paramNodePath.h"
#include "paramTexture.h"
#include "shaderAttrib.h"
#include "shaderBuffer.h"

#include "catch_amalgamated.hpp"

#include <algorithm>

namespace {
  CPT(ShaderAttrib)
  round_trip(const ShaderAttrib *attrib, int minor_version = 47) {
    init_libpanda();
    DatagramBuffer buffer;
    BamWriter writer(&buffer);
    writer.set_file_minor_ver(minor_version);
    REQUIRE(writer.init());
    PT(PandaNode) node = new PandaNode("root");
    node->set_attrib(attrib);
    REQUIRE(writer.write_object(node));

    BamReader reader(&buffer);
    REQUIRE(reader.init());
    PT(PandaNode) restored = DCAST(PandaNode, reader.read_object());
    REQUIRE(restored != nullptr);
    REQUIRE(reader.resolve());
    CPT(ShaderAttrib) result = DCAST(ShaderAttrib,
      restored->get_attrib(ShaderAttrib::get_class_slot()));
    REQUIRE(result != nullptr);
    return result;
  }

  // Numeric storage is not exposed to Python, so check its type, size and
  // contents here, including after the BamReader has been destroyed.
  void
  check_numeric(const ShaderInput &original) {
    CPT(ShaderAttrib) attrib = DCAST(ShaderAttrib, ShaderAttrib::make());
    attrib = DCAST(ShaderAttrib, attrib->set_shader_input(original));
    CPT(ShaderAttrib) restored = round_trip(attrib);
    const ShaderInput &input = restored->get_shader_input(original.get_name());
    REQUIRE(input.get_value_type() == original.get_value_type());
    CHECK(input.get_priority() == original.get_priority());
    const auto &before = original.get_ptr();
    const auto &after = input.get_ptr();
    REQUIRE((int)after._type == (int)before._type);
    REQUIRE(after._size == before._size);
    if (after._size != 0) {
      REQUIRE(after._ptr != nullptr);
    }
    for (size_t i = 0; i < before._size; ++i) {
      switch (before._type) {
      case ShaderType::ST_float:
        CHECK(((float *)after._ptr)[i] == ((float *)before._ptr)[i]);
        break;
      case ShaderType::ST_double:
        CHECK(((double *)after._ptr)[i] == ((double *)before._ptr)[i]);
        break;
      case ShaderType::ST_int:
        CHECK(((int *)after._ptr)[i] == ((int *)before._ptr)[i]);
        break;
      default:
        FAIL("Unexpected numeric input type");
      }
    }
    if (input.get_value_type() == ShaderInput::M_vector) {
      CHECK(input.get_vector() == original.get_vector());
    }
  }
}

TEST_CASE("ShaderAttrib BAM preserves numeric inputs", "[pgraph][bam]") {
  CPT_InternalName name = InternalName::make("numeric");
  SECTION("float vector") {
    check_numeric(ShaderInput(name, LVecBase3f(1.25f, -2.5f, 3.75f), 5));
  }
  SECTION("double vector") {
    check_numeric(ShaderInput(name, LVecBase4d(1.0 / 3.0, -2.0, 3.0, 4.0), -7));
  }
  SECTION("integer vector") {
    check_numeric(ShaderInput(name, LVecBase4i(-1, 2, 123456789, 4), 2));
  }
  SECTION("float array") {
    PTA_float values = PTA_float::empty_array(5);
    for (size_t i = 0; i < values.size(); ++i) {
      values[i] = (float)i * 1.25f;
    }
    check_numeric(ShaderInput(name, values, 3));
  }
  SECTION("double array") {
    PTA_double values = PTA_double::empty_array(3);
    values[0] = 1.0 / 3.0;
    values[1] = -1e100;
    values[2] = 1e-100;
    check_numeric(ShaderInput(name, values));
  }
  SECTION("integer array") {
    PTA_int values = PTA_int::empty_array(3);
    values[0] = -123456789;
    values[1] = 0;
    values[2] = 123456789;
    check_numeric(ShaderInput(name, values));
  }
  SECTION("empty array") {
    check_numeric(ShaderInput(name, PTA_float::empty_array(0)));
  }
  SECTION("float matrix") {
    check_numeric(ShaderInput(name, LMatrix4f::translate_mat(1, 2, 3)));
  }
  SECTION("double matrix") {
    check_numeric(ShaderInput(name, LMatrix4d::scale_mat(1.0 / 3.0, 2, 3)));
  }
}

TEST_CASE("ShaderAttrib BAM preserves object inputs and shared textures", "[pgraph][bam]") {
  PT(Texture) texture = new Texture("shader texture");
  texture->setup_2d_texture(2, 2, Texture::T_unsigned_byte, Texture::F_rgba8);
  PTA_uchar pixels = texture->make_ram_image();
  for (size_t i = 0; i < pixels.size(); ++i) {
    pixels[i] = (unsigned char)i;
  }
  SamplerState sampler;
  sampler.set_minfilter(SamplerState::FT_nearest);
  sampler.set_wrap_u(SamplerState::WM_clamp);
  PT(ShaderBuffer) buffer = new ShaderBuffer("shader buffer", 32, GeomEnums::UH_static);
  vector_uchar initial_data = {1, 2, 3, 4};
  PT(ShaderBuffer) initialized_buffer = new ShaderBuffer(
    "initialized buffer", initial_data, GeomEnums::UH_dynamic);
  PT(ParamVecBase4f) param = new ParamVecBase4f(LVecBase4f(1, 2, 3, 4));
  NodePath node("input node");
  node.set_pos(1, 2, 3);

  CPT(ShaderAttrib) attrib = DCAST(ShaderAttrib, ShaderAttrib::make());
  auto add = [&](const ShaderInput &input) {
    attrib = DCAST(ShaderAttrib, attrib->set_shader_input(input));
  };
  add(ShaderInput(InternalName::make("texture"), texture, 1));
  add(ShaderInput(InternalName::make("sampler"), texture, sampler, 2));
  add(ShaderInput(InternalName::make("image"), texture, true, false, 0, 1, 3));
  add(ShaderInput(InternalName::make("node"), node, 4));
  add(ShaderInput(InternalName::make("buffer"), buffer, 5));
  add(ShaderInput(InternalName::make("param"), param, 6));
  add(ShaderInput(InternalName::make("vector"), LVecBase4f(4, 3, 2, 1), 7));
  add(ShaderInput(InternalName::make("initialized_buffer"), initialized_buffer, 8));

  CPT(ShaderAttrib) restored = round_trip(attrib);
  for (const char *name : {"texture", "sampler", "image", "node", "buffer",
                           "param", "vector", "initialized_buffer"}) {
    const ShaderInput &before = attrib->get_shader_input(name);
    const ShaderInput &after = restored->get_shader_input(name);
    REQUIRE(after.get_value_type() == before.get_value_type());
    CHECK(after.get_name() == before.get_name());
    CHECK(after.get_priority() == before.get_priority());
  }
  const ShaderInput &tex_input = restored->get_shader_input("texture");
  REQUIRE(tex_input.get_texture() != nullptr);
  CHECK(tex_input.get_texture() != texture);
  CHECK(tex_input.get_texture()->get_name() == texture->get_name());
  CHECK(tex_input.get_texture()->get_x_size() == 2);
  CPTA_uchar restored_pixels = tex_input.get_texture()->get_ram_image();
  REQUIRE(restored_pixels.size() == pixels.size());
  CHECK(std::equal(pixels.begin(), pixels.end(), restored_pixels.begin()));
  const ShaderInput &sampler_input = restored->get_shader_input("sampler");
  CHECK(sampler_input.get_texture() == tex_input.get_texture());
  CHECK(sampler_input.get_sampler() == sampler);
  const auto *image = DCAST(ParamTextureImage,
    restored->get_shader_input("image").get_value());
  REQUIRE(image != nullptr);
  CHECK(image->get_texture() == tex_input.get_texture());
  CHECK(image->has_read_access());
  CHECK_FALSE(image->has_write_access());
  CHECK(image->get_bind_layer() == 0);
  CHECK(image->get_bind_level() == 1);
  NodePath restored_node = restored->get_shader_input("node").get_nodepath();
  CHECK(restored_node != node);
  CHECK(restored_node.get_name() == node.get_name());
  CHECK(restored_node.get_pos() == node.get_pos());
  const auto *restored_buffer = DCAST(ShaderBuffer,
    restored->get_shader_input("buffer").get_value());
  REQUIRE(restored_buffer != nullptr);
  CHECK(restored_buffer->get_name() == buffer->get_name());
  CHECK(restored_buffer->get_data_size_bytes() == 32);
  CHECK(restored_buffer->get_usage_hint() == GeomEnums::UH_static);
  const auto *restored_initialized_buffer = DCAST(ShaderBuffer,
    restored->get_shader_input("initialized_buffer").get_value());
  REQUIRE(restored_initialized_buffer != nullptr);
  CHECK(restored_initialized_buffer->get_name() == initialized_buffer->get_name());
  CHECK(restored_initialized_buffer->get_usage_hint() == GeomEnums::UH_dynamic);
  REQUIRE(restored_initialized_buffer->get_data_size_bytes() == initial_data.size());
  REQUIRE(restored_initialized_buffer->get_initial_data() != nullptr);
  CHECK(std::equal(initial_data.begin(), initial_data.end(),
                   restored_initialized_buffer->get_initial_data()));
  const auto *restored_param = DCAST(ParamVecBase4f,
    restored->get_shader_input("param").get_value());
  REQUIRE(restored_param != nullptr);
  CHECK(restored_param->get_value() == param->get_value());
  CHECK(restored->get_shader_input("vector").get_vector() == LVecBase4(4, 3, 2, 1));
}

TEST_CASE("ShaderAttrib BAM supports older files without inputs", "[pgraph][bam]") {
  CPT(ShaderAttrib) attrib = DCAST(ShaderAttrib, ShaderAttrib::make());
  attrib = DCAST(ShaderAttrib, attrib->set_shader_input(
    ShaderInput(InternalName::make("value"), LVecBase4f(1, 2, 3, 4))));
  attrib = DCAST(ShaderAttrib, attrib->set_flag(ShaderAttrib::F_hardware_skinning, true));
  attrib = DCAST(ShaderAttrib, attrib->set_instance_count(3));
  CPT(ShaderAttrib) restored = round_trip(attrib, 46);
  CHECK_FALSE(restored->has_shader_input(InternalName::make("value")));
  CHECK(restored->get_flag(ShaderAttrib::F_hardware_skinning));
  CHECK(restored->get_instance_count() == 3);
}
