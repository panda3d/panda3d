/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTextGlyph.cxx
 * @author drose
 * @date 2005-03-31
 */

#include "geomTextGlyph.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "indent.h"

TypeHandle GeomTextGlyph::_type_handle;


/**
 *
 */
GeomTextGlyph::
GeomTextGlyph(const TextGlyph *glyph, const GeomVertexData *data) :
  Geom(data)
{
  // Initially, there is only one glyph in the Geom.  There might be
  // additional Glyphs later when we flatten the graph and call Geom::unify().
  if (glyph != nullptr) {
    _glyphs.reserve(1);
    _glyphs.push_back(glyph);
  }
}

/**
 *
 */
GeomTextGlyph::
GeomTextGlyph(const GeomVertexData *data) :
  Geom(data)
{
  // With this constructor, there are no glyphs initially.
}

/**
 *
 */
GeomTextGlyph::
GeomTextGlyph(const GeomTextGlyph &copy) :
  Geom(copy),
  _glyphs(copy._glyphs)
{
}

/**
 *
 */
GeomTextGlyph::
GeomTextGlyph(const Geom &copy, const TextGlyph *glyph) :
  Geom(copy)
{
  if (glyph != nullptr) {
    _glyphs.reserve(1);
    _glyphs.push_back(glyph);
  }
}

/**
 *
 */
void GeomTextGlyph::
operator = (const GeomTextGlyph &copy) {
  Geom::operator = (copy);
  _glyphs = copy._glyphs;
}

/**
 *
 */
GeomTextGlyph::
~GeomTextGlyph() {
}

/**
 * Returns a newly-allocated Geom that is a shallow copy of this one.  It will
 * be a different Geom pointer, but its internal data may or may not be shared
 * with that of the original Geom.
 */
Geom *GeomTextGlyph::
make_copy() const {
  return new GeomTextGlyph(*this);
}

/**
 * Copies the primitives from the indicated Geom into this one.  This does
 * require that both Geoms contain the same fundamental type primitives, both
 * have a compatible shade model, and both use the same GeomVertexData.  Both
 * Geoms must also be the same specific class type (i.e.  if one is a
 * GeomTextGlyph, they both must be.)
 *
 * Returns true if the copy is successful, or false otherwise (because the
 * Geoms were mismatched).
 */
bool GeomTextGlyph::
copy_primitives_from(const Geom *other) {
  if (!Geom::copy_primitives_from(other)) {
    return false;
  }

  const GeomTextGlyph *tother;
  DCAST_INTO_R(tother, other, false);

  // Also copy the glyph pointers.
  _glyphs.reserve(_glyphs.size() + tother->_glyphs.size());
  _glyphs.insert(_glyphs.end(), tother->_glyphs.begin(), tother->_glyphs.end());

  return true;
}

/**
 * Records the reference count of the other Geom within this Geom, as if the
 * primitives were copied in via copy_primitives_from() (but does not actually
 * copy any primitives).  This is particularly necessary for GeomTextGlyph's
 * reference counting mechanism.
 *
 * Does nothing if the other Geom is not a GeomTextGlyph.
 */
void GeomTextGlyph::
count_geom(const Geom *other) {
  if (other->is_of_type(GeomTextGlyph::get_class_type())) {
    const GeomTextGlyph *tother;
    DCAST_INTO_V(tother, other);

    _glyphs.reserve(_glyphs.size() + tother->_glyphs.size());
    _glyphs.insert(_glyphs.end(), tother->_glyphs.begin(), tother->_glyphs.end());
  }
}

/**
 *
 */
void GeomTextGlyph::
output(std::ostream &out) const {
  Geom::output(out);
  out << ", glyphs: [";
  Glyphs::const_iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    const TextGlyph *glyph = (*gi);
    nassertv(glyph != nullptr);
    out << " " << glyph->get_character();
  }
  out << " ]";
}

/**
 *
 */
void GeomTextGlyph::
write(std::ostream &out, int indent_level) const {
  Geom::write(out, indent_level);
  indent(out, indent_level)
    << "Glyphs: [";
  Glyphs::const_iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    const TextGlyph *glyph = (*gi);
    nassertv(glyph != nullptr);
    out << " " << glyph->get_character();
  }
  out << " ]\n";
}

/**
 * Adds a glyph to the list of glyphs referenced by this Geom.
 */
void GeomTextGlyph::
add_glyph(const TextGlyph *glyph) {
  _glyphs.push_back(glyph);
}

/**
 * Factory method to generate a GeomTextGlyph object
 */
void GeomTextGlyph::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomTextGlyph);
}

/**
 * Factory method to generate a GeomTextGlyph object
 */
TypedWritable* GeomTextGlyph::
make_GeomTextGlyph(const FactoryParams &params) {
  GeomTextGlyph *me = new GeomTextGlyph(nullptr,
                                        nullptr);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}
