/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVValue.I
 * @author rdb
 * @date 2026-07-08
 */

namespace glsl {

/**
 * Wraps the given id, which must name a value (not a pointer) available at
 * the builder's cursor position, as an expression.
 */
INLINE Value::
Value(SpirVBuilder &builder, Id id) :
  _builder(&builder),
  _id(id) {
}

/**
 * Returns the id holding this expression's value.
 */
INLINE Value::
operator SpirVId () const {
  return _id;
}

/**
 * Returns the builder this expression was emitted into.
 */
INLINE SpirVBuilder *Value::
get_builder() const {
  return _builder;
}

/**
 * Extracts the first two components of the vector.  Same as vec2(value)
 * for a vector operand, but does not accept a scalar to splat.
 */
INLINE Value Value::
get_xy() const {
  return truncate(2);
}

/**
 * Extracts the first three components of the vector.  Same as vec3(value)
 * for a vector operand, but does not accept a scalar to splat.
 */
INLINE Value Value::
get_xyz() const {
  return truncate(3);
}

/**
 * Rebinds this value to a copy with the first component replaced.
 */
INLINE void Value::
set_x(Argument x) {
  set_component(0, x);
}

/**
 * Rebinds this value to a copy with the second component replaced.
 */
INLINE void Value::
set_y(Argument y) {
  set_component(1, y);
}

/**
 * Rebinds this value to a copy with the third component replaced.
 */
INLINE void Value::
set_z(Argument z) {
  set_component(2, z);
}

/**
 * Rebinds this value to a copy with the fourth component replaced.
 */
INLINE void Value::
set_w(Argument w) {
  set_component(3, w);
}

/**
 *
 */
INLINE std::ostream &
operator << (std::ostream &out, const Value &value) {
  value.output(out);
  return out;
}

/**
 *
 */
INLINE Argument::
Argument(Value value) :
  _value(value),
  _is_literal(false) {
}

/**
 *
 */
INLINE Argument::
Argument(double literal) :
  _literal(literal),
  _is_literal(true) {
}

/**
 * Returns true if this argument was given as a plain number rather than an
 * expression.
 */
INLINE bool Argument::
is_literal() const {
  return _is_literal;
}

/**
 * Returns the number this argument was given as.  May only be called if
 * is_literal() is true.
 */
INLINE double Argument::
get_literal() const {
  nassertr(_is_literal, 0.0);
  return _literal;
}

/**
 * Returns the expression this argument was given as.  May only be called if
 * is_literal() is false.
 */
INLINE Value Argument::
get_value() const {
  nassertr(!_is_literal, Value());
  return _value;
}

/**
 * Returns the builder the expression was emitted into, or null if this
 * argument is a literal (which belongs to no builder until it is resolved).
 */
INLINE SpirVBuilder *Argument::
get_builder() const {
  return _is_literal ? nullptr : _value.get_builder();
}

/**
 *
 */
INLINE Value &operator += (Value &left, Value right) {
  return left = left + right;
}

/**
 *
 */
INLINE Value &operator += (Value &left, double right) {
  return left = left + right;
}

/**
 *
 */
INLINE Value &operator -= (Value &left, Value right) {
  return left = left - right;
}

/**
 *
 */
INLINE Value &operator -= (Value &left, double right) {
  return left = left - right;
}

/**
 *
 */
INLINE Value &operator *= (Value &left, Value right) {
  return left = left * right;
}

/**
 *
 */
INLINE Value &operator *= (Value &left, double right) {
  return left = left * right;
}

/**
 *
 */
INLINE Value &operator /= (Value &left, Value right) {
  return left = left / right;
}

/**
 *
 */
INLINE Value &operator /= (Value &left, double right) {
  return left = left / right;
}

}  // namespace glsl
