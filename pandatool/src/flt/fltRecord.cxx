/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltRecord.cxx
 * @author drose
 * @date 2000-08-24
 */

#include "fltRecord.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"
#include "fltGroup.h"
#include "fltObject.h"
#include "fltFace.h"
#include "fltCurve.h"
#include "fltMesh.h"
#include "fltLocalVertexPool.h"
#include "fltMeshPrimitive.h"
#include "fltVertexList.h"
#include "fltLOD.h"
#include "fltInstanceDefinition.h"
#include "fltInstanceRef.h"
#include "fltUnsupportedRecord.h"
#include "fltExternalReference.h"
#include "fltVectorRecord.h"
#include "config_flt.h"

#include "dcast.h"
#include "indent.h"
#include "datagramIterator.h"

#include <assert.h>

TypeHandle FltRecord::_type_handle;

/**
 *
 */
FltRecord::
FltRecord(FltHeader *header) :
  _header(header)
{
}

/**
 *
 */
FltRecord::
~FltRecord() {
}

/**
 * Returns the number of child records of this record.  This reflects the
 * normal scene graph hierarchy.
 */
int FltRecord::
get_num_children() const {
  return _children.size();
}

/**
 * Returns the nth child of this record.
 */
FltRecord *FltRecord::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), nullptr);
  return _children[n];
}

/**
 * Removes all children from this record.
 */
void FltRecord::
clear_children() {
  _children.clear();
}

/**
 * Adds a new child to the end of the list of children for this record.
 */
void FltRecord::
add_child(FltRecord *child) {
  _children.push_back(child);
}

/**
 * Returns the number of subface records of this record.  Normally, subfaces
 * will only be present on object records, although it is logically possible
 * for them to appear anywhere.
 */
int FltRecord::
get_num_subfaces() const {
  return _subfaces.size();
}

/**
 * Returns the nth subface of this record.
 */
FltRecord *FltRecord::
get_subface(int n) const {
  nassertr(n >= 0 && n < (int)_subfaces.size(), nullptr);
  return _subfaces[n];
}

/**
 * Removes all subfaces from this record.
 */
void FltRecord::
clear_subfaces() {
  _subfaces.clear();
}

/**
 * Adds a new subface to the end of the list of subfaces for this record.
 */
void FltRecord::
add_subface(FltRecord *subface) {
  _subfaces.push_back(subface);
}

/**
 * Returns the number of extension attribute records for this object.  These
 * are auxiliary nodes, presumably of type FO_extension, that have some local
 * meaning to the object.
 */
int FltRecord::
get_num_extensions() const {
  return _extensions.size();
}

/**
 * Returns the nth extension of this record.
 */
FltRecord *FltRecord::
get_extension(int n) const {
  nassertr(n >= 0 && n < (int)_extensions.size(), nullptr);
  return _extensions[n];
}

/**
 * Removes all extensions from this record.
 */
void FltRecord::
clear_extensions() {
  _extensions.clear();
}

/**
 * Adds a new extension to the end of the list of extensions for this record.
 * This should be a record of type FO_extension.
 */
void FltRecord::
add_extension(FltRecord *extension) {
  _extensions.push_back(extension);
}

/**
 * Returns the number of unsupported ancillary records of this record.  These
 * are ancillary records that appeared following this record in the flt file
 * but that aren't directly understood by the flt loader--normally, an
 * ancillary record is examined and decoded on the spot, and no pointer to it
 * is kept.
 */
int FltRecord::
get_num_ancillary() const {
  return _ancillary.size();
}

/**
 * Returns the nth unsupported ancillary record of this record.  See
 * get_num_ancillary().
 */
FltRecord *FltRecord::
get_ancillary(int n) const {
  nassertr(n >= 0 && n < (int)_ancillary.size(), nullptr);
  return _ancillary[n];
}

/**
 * Removes all unsupported ancillary records from this record.  See
 * get_num_ancillary().
 */
void FltRecord::
clear_ancillary() {
  _ancillary.clear();
}

/**
 * Adds a new unsupported ancillary record to the end of the list of ancillary
 * records for this record.  This record will be written to the flt file
 * following this record, without attempting to understand what is in it.
 *
 * Normally, there is no reason to use this function; if the data stored in
 * the FltRecord requires one or more ancillary record, the appropriate
 * records will automatically be generated when the record is written.  This
 * function is only required to output a record whose type is not supported by
 * the flt loader.  But it would be better to extend the flt loader to know
 * about this new kind of data record.
 */
void FltRecord::
add_ancillary(FltRecord *ancillary) {
  _ancillary.push_back(ancillary);
}

/**
 * Returns true if this record has a nonempty comment, false otherwise.
 */
bool FltRecord::
has_comment() const {
  return !_comment.empty();
}

/**
 * Retrieves the comment for this record, or empty string if the record has no
 * comment.
 */
const std::string &FltRecord::
get_comment() const {
  return _comment;
}

/**
 * Removes the comment for this record.
 */
void FltRecord::
clear_comment() {
  _comment = "";
}

/**
 * Changes the comment for this record.
 */
void FltRecord::
set_comment(const std::string &comment) {
  _comment = comment;
}

/**
 * Checks that the iterator has no bytes left, as it should at the end of a
 * successfully read record.  If there *are* remaining bytes, print a warning
 * message but otherwise don't worry about it.
 *
 * If we are attempting to read a flt file whose version is newer than the
 * newest this program understands, don't even print a warning message, since
 * this is exactly the sort of thing we expect.
 */
void FltRecord::
check_remaining_size(const DatagramIterator &di, const std::string &name) const {
  if (di.get_remaining_size() == 0) {
    return;
  }

  if (_header->get_flt_version() <= _header->max_flt_version()) {
    nout << "Warning!  Ignoring extra " << di.get_remaining_size()
         << " bytes at the end of a ";
    if (name.empty()) {
      nout << get_type();
    } else {
      nout << name;
    }
    nout << " record.\n";
  }
}

/**
 * Walks the hierarchy at this record and below and copies the
 * _converted_filename record into the _orig_filename record, so the flt file
 * will be written out with the converted filename instead of what was
 * originally read in.
 */
void FltRecord::
apply_converted_filenames() {
  Records::const_iterator ci;
  for (ci = _subfaces.begin(); ci != _subfaces.end(); ++ci) {
    (*ci)->apply_converted_filenames();
  }
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->apply_converted_filenames();
  }
}

/**
 * Writes a quick one-line description of the record, but not its children.
 * This is a human-readable description, primarily for debugging; to write a
 * flt file, use FltHeader::write_flt().
 */
void FltRecord::
output(std::ostream &out) const {
  out << get_type();
}

/**
 * Writes a multiple-line description of the record and all of its children.
 * This is a human-readable description, primarily for debugging; to write a
 * flt file, use FltHeader::write_flt().
 */
void FltRecord::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this;
  write_children(out, indent_level);
}

/**
 * Assuming the current write position has been left at the end of the last
 * line of the record description, writes out the list of children.
 */
void FltRecord::
write_children(std::ostream &out, int indent_level) const {
  if (!_ancillary.empty()) {
    out << " + " << _ancillary.size() << " ancillary";
  }
  if (!_extensions.empty()) {
    out << " + " << _extensions.size() << " extensions";
  }
  if (!_subfaces.empty()) {
    out << " [";
    Records::const_iterator ci;
    for (ci = _subfaces.begin(); ci != _subfaces.end(); ++ci) {
      out << " " << *(*ci);
    }
    out << " ]";
  }
  if (!_children.empty()) {
    out << " {\n";
    Records::const_iterator ci;
    for (ci = _children.begin(); ci != _children.end(); ++ci) {
      (*ci)->write(out, indent_level + 2);
    }
    indent(out, indent_level) << "}\n";
  } else {
    out << "\n";
  }
}

  /*
  virtual void write(ostream &out) const;
  virtual void build_record(Datagram &datagram) const;
  */

/**
 * Returns true if the indicated opcode corresponds to an ancillary record
 * type, false otherwise.  In general, this function is used to identify
 * ancillary records that are not presently supported by the FltReader; these
 * will be ignored.  Normally, ancillary records will be detected and
 * processed by extract_ancillary().
 */
bool FltRecord::
is_ancillary(FltOpcode opcode) {
  switch (opcode) {
  case FO_comment:
  case FO_long_id:
  case FO_multitexture:
  case FO_uv_list:
  case FO_replicate:
  case FO_road_zone:
  case FO_transform_matrix:
  case FO_rotate_about_edge:
  case FO_translate:
  case FO_scale:
  case FO_rotate_about_point:
  case FO_rotate_and_scale:
  case FO_put:
  case FO_general_matrix:
  case FO_vector:
  case FO_bounding_box:
  case FO_bounding_sphere:
  case FO_bounding_cylinder:
  case FO_bv_center:
  case FO_bv_orientation:
  case FO_local_vertex_pool:
  case FO_cat_data:

  case FO_14_material_palette:
  case FO_vertex_palette:
  case FO_vertex_c:
  case FO_vertex_cn:
  case FO_vertex_cnu:
  case FO_vertex_cu:
  case FO_color_palette:
  case FO_name_table:
  case FO_15_material:
  case FO_texture:
  case FO_eyepoint_palette:
  case FO_light_definition:
  case FO_texture_map_palette:
    return true;

  case FO_header:
  case FO_mesh:
  case FO_mesh_primitive:
  case FO_group:
  case FO_object:
  case FO_face:
  case FO_light_point:
  case FO_dof:
  case FO_vertex_list:
  case FO_morph_list:
  case FO_bsp:
  case FO_external_ref:
  case FO_lod:
  case FO_sound:
  case FO_light_source:
  case FO_road_segment:
  case FO_road_construction:
  case FO_road_path:
  case FO_clip_region:
  case FO_text:
  case FO_switch:
  case FO_cat:
  case FO_extension:
  case FO_curve:
    return false;

  case FO_push:
  case FO_pop:
  case FO_push_face:
  case FO_pop_face:
  case FO_push_attribute:
  case FO_pop_attribute:
  case FO_push_extension:
  case FO_pop_extension:
  case FO_instance:
  case FO_instance_ref:
    return false;

  default:
    nout << "Don't know whether " << opcode << " is ancillary.\n";
    return false;
  }
}

/**
 * Creates a new FltRecord corresponding to the opcode.  If the opcode is
 * unknown, creates a FltUnsupportedRecord.
 */
FltRecord *FltRecord::
create_new_record(FltOpcode opcode) const {
  switch (opcode) {
  case FO_group:
    return new FltGroup(_header);

  case FO_object:
    return new FltObject(_header);

  case FO_face:
    return new FltFace(_header);

  case FO_curve:
    return new FltCurve(_header);

  case FO_mesh:
    return new FltMesh(_header);

  case FO_local_vertex_pool:
    return new FltLocalVertexPool(_header);

  case FO_mesh_primitive:
    return new FltMeshPrimitive(_header);

  case FO_vertex_list:
    return new FltVertexList(_header);

  case FO_lod:
    return new FltLOD(_header);

  case FO_instance:
    return new FltInstanceDefinition(_header);

  case FO_instance_ref:
    return new FltInstanceRef(_header);

  case FO_external_ref:
    return new FltExternalReference(_header);

  case FO_vector:
    return new FltVectorRecord(_header);

  default:
    nout << "Ignoring unsupported record " << opcode << "\n";
    return new FltUnsupportedRecord(_header);
  }
}

/**
 * Extracts this record information from the current record presented in the
 * reader, then advances the reader and continues to read any children, if
 * present.  On return, the reader is position on the next sibling record to
 * this record.
 *
 * Returns FE_ok if successful, otherwise on error.
 */
FltError FltRecord::
read_record_and_children(FltRecordReader &reader) {
  if (!extract_record(reader)) {
    nout << "Could not extract record for " << *this << "\n";
    assert(!flt_error_abort);
    return FE_invalid_record;
  }
  FltError result = reader.advance();
  if (result == FE_end_of_file) {
    return FE_ok;
  } else if (result != FE_ok) {
    return result;
  }

  while (true) {
    if (extract_ancillary(reader)) {
      // Ok, a known ancillary record.  Fine.

    } else if (reader.get_opcode() == FO_push) {
      // A push begins a new list of children.
      result = reader.advance();
      if (result != FE_ok) {
        return result;
      }

      while (reader.get_opcode() != FO_pop) {
        PT(FltRecord) child = create_new_record(reader.get_opcode());
        FltError result = child->read_record_and_children(reader);
        if (result != FE_ok) {
          return result;
        }

        if (child->is_of_type(FltInstanceDefinition::get_class_type())) {
          // A special case for an instance definition.  These shouldn't
          // appear in the hierarchy, but should instead be added directly to
          // the header.
          _header->add_instance(DCAST(FltInstanceDefinition, child));

        } else {
          add_child(child);
        }

        if (reader.eof() || reader.error()) {
          assert(!flt_error_abort);
          return FE_end_of_file;
        }
      }

    } else if (reader.get_opcode() == FO_push_face) {
      // A push subface begins a new list of subfaces.
      result = reader.advance();
      if (result != FE_ok) {
        return result;
      }

      while (reader.get_opcode() != FO_pop_face) {
        PT(FltRecord) subface = create_new_record(reader.get_opcode());
        FltError result = subface->read_record_and_children(reader);
        if (result != FE_ok) {
          return result;
        }
        add_subface(subface);
        if (reader.eof() || reader.error()) {
          assert(!flt_error_abort);
          return FE_end_of_file;
        }
      }

    } else if (reader.get_opcode() == FO_push_extension) {
      // A push extension begins a new list of extensions.
      result = reader.advance();
      if (result != FE_ok) {
        return result;
      }

      while (reader.get_opcode() != FO_pop_extension) {
        PT(FltRecord) extension = create_new_record(reader.get_opcode());
        FltError result = extension->read_record_and_children(reader);
        if (result != FE_ok) {
          return result;
        }
        add_extension(extension);
        if (reader.eof() || reader.error()) {
          assert(!flt_error_abort);
          return FE_end_of_file;
        }
      }

    } else if (is_ancillary(reader.get_opcode())) {
      // An unsupported ancillary record.  Skip it.
      PT(FltRecord) ancillary = create_new_record(reader.get_opcode());
      ancillary->extract_record(reader);
      _ancillary.push_back(ancillary);

    } else {
      // None of the above: we're done.
      return FE_ok;
    }

    // Skip to the next record.  If that's the end, fine.
    result = reader.advance(true);
    if (reader.eof() || result != FE_ok) {
      return result;
    }
  }
}

/**
 * Fills in the information in this record based on the information given in
 * the indicated datagram, whose opcode has already been read.  Returns true
 * on success, false if the datagram is invalid.
 */
bool FltRecord::
extract_record(FltRecordReader &) {
  return true;
}

/**
 * Checks whether the given record, which follows this record sequentially in
 * the file, is an ancillary record of this record.  If it is, extracts the
 * relevant information and returns true; otherwise, leaves it alone and
 * returns false.
 */
bool FltRecord::
extract_ancillary(FltRecordReader &reader) {
  if (reader.get_opcode() == FO_comment) {
    DatagramIterator &di = reader.get_iterator();
    _comment = di.get_fixed_string(di.get_remaining_size());
    return true;
  }

  return false;
}

/**
 * Writes this record out to the flt file, along with all of its ancillary
 * records and children records.  Returns FE_ok on success, or something else
 * on error.
 */
FltError FltRecord::
write_record_and_children(FltRecordWriter &writer) const {
  // First, write the record.
  if (!build_record(writer)) {
    assert(!flt_error_abort);
    return FE_bad_data;
  }

  FltError result = writer.advance();
  if (result != FE_ok) {
    return result;
  }

  // Then the ancillary data.
  result = write_ancillary(writer);
  if (result != FE_ok) {
    return result;
  }
  Records::const_iterator ci;
  for (ci = _ancillary.begin(); ci != _ancillary.end(); ++ci) {
    if (!(*ci)->build_record(writer)) {
      assert(!flt_error_abort);
      return FE_bad_data;
    }
    result = writer.advance();
    if (result != FE_ok) {
      return result;
    }
  }

  // Any extensions?
  if (!_extensions.empty()) {
    result = writer.write_record(FO_push_face);
    if (result != FE_ok) {
      return result;
    }

    for (ci = _extensions.begin(); ci != _extensions.end(); ++ci) {
      (*ci)->write_record_and_children(writer);
    }

    result = writer.write_record(FO_pop_face);
    if (result != FE_ok) {
      return result;
    }
  }

  // Finally, write all the children.
  if (!_children.empty()) {
    result = writer.write_record(FO_push);
    if (result != FE_ok) {
      return result;
    }

    for (ci = _children.begin(); ci != _children.end(); ++ci) {
      (*ci)->write_record_and_children(writer);
    }

    result = writer.write_record(FO_pop);
    if (result != FE_ok) {
      return result;
    }
  }

  // We must write subfaces *after* the list of children, or Creator will
  // crash trying to load the file.
  if (!_subfaces.empty()) {
    result = writer.write_record(FO_push_face);
    if (result != FE_ok) {
      return result;
    }

    for (ci = _subfaces.begin(); ci != _subfaces.end(); ++ci) {
      (*ci)->write_record_and_children(writer);
    }

    result = writer.write_record(FO_pop_face);
    if (result != FE_ok) {
      return result;
    }
  }

  return FE_ok;
}

/**
 * Fills up the current record on the FltRecordWriter with data for this
 * record, but does not advance the writer.  Returns true on success, false if
 * there is some error.
 */
bool FltRecord::
build_record(FltRecordWriter &) const {
  return true;
}

/**
 * Writes whatever ancillary records are required for this record.  Returns
 * FE_ok on success, or something else if there is some error.
 */
FltError FltRecord::
write_ancillary(FltRecordWriter &writer) const {
  if (!_comment.empty()) {
    Datagram dc(_comment.data(), _comment.size());
    FltError result = writer.write_record(FO_comment, dc);
    if (result != FE_ok) {
      return result;
    }
  }
  return FE_ok;
}
