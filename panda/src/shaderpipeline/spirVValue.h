/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVValue.h
 * @author rdb
 * @date 2026-07-08
 */

#ifndef SPIRVVALUE_H
#define SPIRVVALUE_H

#include "spirVModule.h"

class SpirVBuilder;

/**
 * The expression builder API is modelled after GLSL, and accordingly mostly
 * uses free functions for operations.  To avoid conflict, we put everything
 * inside a namespace.  The Value class is defined inside too, so that ADL can
 * find the free functions automatically and no `using namespace glsl;` is
 * needed anywhere.  The global SpirVValue alias below refers to the class from
 * outside the namespace.
 */
namespace glsl {

class Argument;

/**
 * A SPIR-V intermediate value, as loaded from a pointer.  Holds a reference
 * to a SpirVBuilder and an id.  Any operations done on the Value will cause
 * corresponding instructions to be emitted on the builder immediately: there
 * is no lazy expression evaluation.
 *
 * To prevent mistakes due to accidental host-side use, this class lacks bool
 * conversion and comparison operators; see the free functions below instead.
 *
 * The usual way to obtain a Value is using load() on a SpirVPointer or by
 * using another operation that returns a Value, but it can be directly
 * constructed from a SpirVBuilder and Id instead as well.
 *
 * Note that a Value may not outlive its SpirVBuilder.  Block domination is not
 * checked.
 */
class EXPCL_PANDA_SHADERPIPELINE Value {
public:
  using Id = SpirVId;

  Value() = default;
  INLINE Value(SpirVBuilder &builder, Id id);

  INLINE operator Id () const;
  INLINE SpirVBuilder *get_builder() const;
  const ShaderType *get_type() const;

  explicit operator bool() const = delete;

  Value operator [](uint32_t index) const;
  Value operator [](Id) const = delete;

  Value convert(ShaderType::ScalarType scalar_type) const;
  Value bitcast(const ShaderType *type) const;

  Value swizzle(const pvector<uint32_t> &components) const;
  Value swizzle(std::initializer_list<uint32_t> components) const;
  Value truncate(uint32_t num_components) const;
  INLINE Value get_xy() const;
  INLINE Value get_xyz() const;

  // The setters don't touch memory, they just rebind the id.
  void set_component(uint32_t index, Argument value);
  INLINE void set_x(Argument x);
  INLINE void set_y(Argument y);
  INLINE void set_z(Argument z);
  INLINE void set_w(Argument w);
  void set_xyz(Argument new_xyz);

  void output(std::ostream &out) const;

private:
  SpirVBuilder *_builder = nullptr;
  Id _id;
};

INLINE std::ostream &operator << (std::ostream &out, const Value &value);

/**
 * Holds an argument that may be either a Value or a literal number.  A
 * literal materializes against the type it is used with: the constructed
 * scalar type for a vecN, the anchoring first operand's type for a function,
 * the destination's type for a sink (SpirVPointer::store, Value::set_x).
 *
 * Every function taking Arguments takes a plain Value first, so that the
 * builder and (where the result type is operand-directed) the scalar type are
 * always available without scanning; the vecN constructors are the exception,
 * since their scalar type is in their name and a literal is meaningful in any
 * position, so they assert instead that at least one argument is an
 * expression.
 */
class EXPCL_PANDA_SHADERPIPELINE Argument {
public:
  INLINE Argument(Value value);
  INLINE Argument(double literal);
  Argument(SpirVId) = delete;

  INLINE bool is_literal() const;
  INLINE double get_literal() const;
  INLINE Value get_value() const;
  INLINE SpirVBuilder *get_builder() const;

  Value resolve(SpirVBuilder &builder, const ShaderType *type) const;
  Value resolve(Value anchor) const;

private:
  Value _value;
  double _literal = 0.0;
  bool _is_literal;
};

// Arithmetic ops, with GLSL semantics.
EXPCL_PANDA_SHADERPIPELINE Value operator + (Value left, Value right);
EXPCL_PANDA_SHADERPIPELINE Value operator + (Value left, double right);
EXPCL_PANDA_SHADERPIPELINE Value operator + (double left, Value right);
EXPCL_PANDA_SHADERPIPELINE Value operator - (Value left, Value right);
EXPCL_PANDA_SHADERPIPELINE Value operator - (Value left, double right);
EXPCL_PANDA_SHADERPIPELINE Value operator - (double left, Value right);
EXPCL_PANDA_SHADERPIPELINE Value operator * (Value left, Value right);
EXPCL_PANDA_SHADERPIPELINE Value operator * (Value left, double right);
EXPCL_PANDA_SHADERPIPELINE Value operator * (double left, Value right);
EXPCL_PANDA_SHADERPIPELINE Value operator / (Value left, Value right);
EXPCL_PANDA_SHADERPIPELINE Value operator / (Value left, double right);
EXPCL_PANDA_SHADERPIPELINE Value operator / (double left, Value right);
EXPCL_PANDA_SHADERPIPELINE Value operator - (Value value);

// Compound assignments rebind the left-hand C++ variable to a fresh SSA
// value; they do not touch memory.
INLINE Value &operator += (Value &left, Value right);
INLINE Value &operator += (Value &left, double right);
INLINE Value &operator -= (Value &left, Value right);
INLINE Value &operator -= (Value &left, double right);
INLINE Value &operator *= (Value &left, Value right);
INLINE Value &operator *= (Value &left, double right);
INLINE Value &operator /= (Value &left, Value right);
INLINE Value &operator /= (Value &left, double right);

// Delete these to prevent a raw id silently materializing as a constant with
// the id's numeric value.
Value operator + (Value, SpirVId) = delete;
Value operator + (SpirVId, Value) = delete;
Value operator - (Value, SpirVId) = delete;
Value operator - (SpirVId, Value) = delete;
Value operator * (Value, SpirVId) = delete;
Value operator * (SpirVId, Value) = delete;
Value operator / (Value, SpirVId) = delete;
Value operator / (SpirVId, Value) = delete;

// These are defined as functions, since defining ordering operators on
// SpirVValue would cause op emission when they are used in maps and such.
EXPCL_PANDA_SHADERPIPELINE Value equal(Value left, Argument right);
EXPCL_PANDA_SHADERPIPELINE Value not_equal(Value left, Argument right);
EXPCL_PANDA_SHADERPIPELINE Value less_than(Value left, Argument right);
EXPCL_PANDA_SHADERPIPELINE Value less_than_equal(Value left, Argument right);
EXPCL_PANDA_SHADERPIPELINE Value greater_than(Value left, Argument right);
EXPCL_PANDA_SHADERPIPELINE Value greater_than_equal(Value left, Argument right);

// Collapses a bvec into a single bool (accepts scalar bools).
EXPCL_PANDA_SHADERPIPELINE Value any(Value value);
EXPCL_PANDA_SHADERPIPELINE Value all(Value value);

// The first argument for these math functions may not be a literal.
EXPCL_PANDA_SHADERPIPELINE Value abs(Value value);
EXPCL_PANDA_SHADERPIPELINE Value exp2(Value value);
EXPCL_PANDA_SHADERPIPELINE Value pow(Value base, Argument exponent);
EXPCL_PANDA_SHADERPIPELINE Value pow(double base, Value exponent);
EXPCL_PANDA_SHADERPIPELINE Value min(Value a, Argument b);
EXPCL_PANDA_SHADERPIPELINE Value max(Value a, Argument b);
EXPCL_PANDA_SHADERPIPELINE Value clamp(Value value, Argument min_value, Argument max_value);
EXPCL_PANDA_SHADERPIPELINE Value mix(Value a, Argument b, Argument t);
EXPCL_PANDA_SHADERPIPELINE Value dot(Value a, Argument b);
EXPCL_PANDA_SHADERPIPELINE Value length(Value vector);
EXPCL_PANDA_SHADERPIPELINE Value normalize(Value vector);
EXPCL_PANDA_SHADERPIPELINE Value transpose(Value matrix);

// Note that there cannot be a a vector constructor taking only literals;
// use builder.const_vec2(...) and friends for that instead.
EXPCL_PANDA_SHADERPIPELINE Value vec2(Value scalar_or_vector);
EXPCL_PANDA_SHADERPIPELINE Value vec3(Value scalar_or_vector);
EXPCL_PANDA_SHADERPIPELINE Value vec4(Value scalar_or_vector);
EXPCL_PANDA_SHADERPIPELINE Value vec2(Argument x, Argument y);
EXPCL_PANDA_SHADERPIPELINE Value vec3(Argument x, Argument y);
EXPCL_PANDA_SHADERPIPELINE Value vec3(Argument x, Argument y, Argument z);
EXPCL_PANDA_SHADERPIPELINE Value vec4(Argument x, Argument y);
EXPCL_PANDA_SHADERPIPELINE Value vec4(Argument x, Argument y, Argument z);
EXPCL_PANDA_SHADERPIPELINE Value vec4(Argument x, Argument y, Argument z, Argument w);

EXPCL_PANDA_SHADERPIPELINE Value mat3(Value matrix);
EXPCL_PANDA_SHADERPIPELINE Value mat3(Value r0, Value r1, Value r2);
EXPCL_PANDA_SHADERPIPELINE Value mat4(Value r0, Value r1, Value r2, Value r3);
EXPCL_PANDA_SHADERPIPELINE Value mat3x4(Value r0, Value r1, Value r2);

EXPCL_PANDA_SHADERPIPELINE Value texture(Value sampler, Value coord);
EXPCL_PANDA_SHADERPIPELINE Value texture_dref(Value sampler, Value coord, Value dref);
EXPCL_PANDA_SHADERPIPELINE Value texture_proj(Value sampler, Value coord);
EXPCL_PANDA_SHADERPIPELINE Value texture_proj_dref(Value sampler, Value coord, Value dref);
EXPCL_PANDA_SHADERPIPELINE Value texel_fetch(Value image, Value coord);

}  // namespace glsl

using SpirVValue = glsl::Value;

#include "spirVValue.I"

#endif
