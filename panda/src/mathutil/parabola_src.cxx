/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parabola_src.cxx
 * @author drose
 * @date 2007-10-10
 */

/**
 * Transforms the parabola by the indicated matrix.
 */
void FLOATNAME(LParabola)::
xform(const FLOATNAME(LMatrix4) &mat) {
  // Note that xform_vec() is the correct operation here, while
  // xform_vec_general() is not.
  _a = mat.xform_vec(_a);
  _b = mat.xform_vec(_b);
  _c = mat.xform_point(_c);
}

/**
 *
 */
void FLOATNAME(LParabola)::
output(std::ostream &out) const {
  out << "LParabola(" << _a << ", " << _b << ", " << _c << ")";
}

/**
 *
 */
void FLOATNAME(LParabola)::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 * Writes the parabola to the Datagram using add_float32() or add_float64(),
 * depending on the type of floats in the parabola, regardless of the setting
 * of Datagram::set_stdfloat_double().  This is appropriate when you want to
 * write a fixed-width value to the datagram, especially when you are not
 * writing a bam file.
 */
void FLOATNAME(LParabola)::
write_datagram_fixed(Datagram &destination) const {
  _a.write_datagram_fixed(destination);
  _b.write_datagram_fixed(destination);
  _c.write_datagram_fixed(destination);
}

/**
 * Reads the parabola from the Datagram using get_float32() or get_float64().
 * See write_datagram_fixed().
 */
void FLOATNAME(LParabola)::
read_datagram_fixed(DatagramIterator &source) {
  _a.read_datagram_fixed(source);
  _b.read_datagram_fixed(source);
  _c.read_datagram_fixed(source);
}

/**
 * Writes the parabola to the Datagram using add_stdfloat().  This is
 * appropriate when you want to write the vector using the standard width
 * setting, especially when you are writing a bam file.
 */
void FLOATNAME(LParabola)::
write_datagram(Datagram &destination) const {
  _a.write_datagram(destination);
  _b.write_datagram(destination);
  _c.write_datagram(destination);
}

/**
 * Reads the parabola from the Datagram using get_stdfloat().
 */
void FLOATNAME(LParabola)::
read_datagram(DatagramIterator &source) {
  _a.read_datagram(source);
  _b.read_datagram(source);
  _c.read_datagram(source);
}
