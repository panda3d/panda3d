/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPacker.cxx
 * @author drose
 * @date 2004-06-15
 */

#include "dcPacker.h"
#include "dcSwitch.h"
#include "dcParserDefs.h"
#include "dcLexerDefs.h"
#include "dcClassParameter.h"
#include "dcSwitchParameter.h"
#include "dcClass.h"

using std::istream;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;

DCPacker::StackElement *DCPacker::StackElement::_deleted_chain = nullptr;
int DCPacker::StackElement::_num_ever_allocated = 0;

/**
 *
 */
DCPacker::
DCPacker() {
  _mode = M_idle;
  _unpack_data = nullptr;
  _unpack_length = 0;
  _owns_unpack_data = false;
  _unpack_p = 0;
  _live_catalog = nullptr;
  _parse_error = false;
  _pack_error = false;
  _range_error = false;
  _stack = nullptr;

  clear();
}

/**
 *
 */
DCPacker::
~DCPacker() {
  clear_data();
  clear();
}

/**
 * Begins a packing session.  The parameter is the DC object that describes
 * the packing format; it may be a DCParameter or DCField.
 *
 * Unless you call clear_data() between sessions, multiple packing sessions
 * will be concatenated together into the same buffer.  If you wish to add
 * bytes to the buffer between packing sessions, use append_data() or
 * get_write_pointer().
 */
void DCPacker::
begin_pack(const DCPackerInterface *root) {
  nassertv(_mode == M_idle);

  _mode = M_pack;
  _parse_error = false;
  _pack_error = false;
  _range_error = false;

  _root = root;
  _catalog = nullptr;
  _live_catalog = nullptr;

  _current_field = root;
  _current_parent = nullptr;
  _current_field_index = 0;
  _num_nested_fields = 0;
}

/**
 * Finishes a packing session.
 *
 * The return value is true on success, or false if there has been some error
 * during packing.
 */
bool DCPacker::
end_pack() {
  nassertr(_mode == M_pack, false);

  _mode = M_idle;

  if (_stack != nullptr || _current_field != nullptr || _current_parent != nullptr) {
    _pack_error = true;
  }

  clear();

  return !had_error();
}

/**
 * Sets up the unpack_data pointer.  You may call this before calling the
 * version of begin_unpack() that takes only one parameter.
 */
void DCPacker::
set_unpack_data(const vector_uchar &data) {
  nassertv(_mode == M_idle);

  char *buffer = new char[data.size()];
  memcpy(buffer, data.data(), data.size());
  set_unpack_data(buffer, data.size(), true);
}

/**
 * Sets up the unpack_data pointer.  You may call this before calling the
 * version of begin_unpack() that takes only one parameter.
 */
void DCPacker::
set_unpack_data(const char *unpack_data, size_t unpack_length,
                bool owns_unpack_data) {
  nassertv(_mode == M_idle);

  if (_owns_unpack_data) {
    delete[] _unpack_data;
  }
  _unpack_data = unpack_data;
  _unpack_length = unpack_length;
  _owns_unpack_data = owns_unpack_data;
  _unpack_p = 0;
}

/**
 * Begins an unpacking session.  You must have previously called
 * set_unpack_data() to specify a buffer to unpack.
 *
 * If there was data left in the buffer after a previous begin_unpack() ..
 * end_unpack() session, the new session will resume from the current point.
 * This method may be used, therefore, to unpack a sequence of objects from
 * the same buffer.
 */
void DCPacker::
begin_unpack(const DCPackerInterface *root) {
  nassertv(_mode == M_idle);
  nassertv(_unpack_data != nullptr);

  _mode = M_unpack;
  _parse_error = false;
  _pack_error = false;
  _range_error = false;

  _root = root;
  _catalog = nullptr;
  _live_catalog = nullptr;

  _current_field = root;
  _current_parent = nullptr;
  _current_field_index = 0;
  _num_nested_fields = 0;
}

/**
 * Finishes the unpacking session.
 *
 * The return value is true on success, or false if there has been some error
 * during unpacking (or if all fields have not been unpacked).
 */
bool DCPacker::
end_unpack() {
  nassertr(_mode == M_unpack, false);

  _mode = M_idle;

  if (_stack != nullptr || _current_field != nullptr || _current_parent != nullptr) {
    // This happens if we have not unpacked all of the fields.  However, this
    // is not an error if we have called seek() during the unpack session (in
    // which case the _catalog will be non-NULL).  On the other hand, if the
    // catalog is still NULL, then we have never called seek() and it is an
    // error not to unpack all values.
    if (_catalog == nullptr) {
      _pack_error = true;
    }
  }

  clear();

  return !had_error();
}

/**
 * Begins a repacking session.  You must have previously called
 * set_unpack_data() to specify a buffer to unpack.
 *
 * Unlike begin_pack() or begin_unpack() you may not concatenate the results
 * of multiple begin_repack() sessions in one buffer.
 *
 * Also, unlike in packing or unpacking modes, you may not walk through the
 * fields from beginning to end, or even pack two consecutive fields at once.
 * Instead, you must call seek() for each field you wish to modify and pack
 * only that one field; then call seek() again to modify another field.
 */
void DCPacker::
begin_repack(const DCPackerInterface *root) {
  nassertv(_mode == M_idle);
  nassertv(_unpack_data != nullptr);
  nassertv(_unpack_p == 0);

  _mode = M_repack;
  _parse_error = false;
  _pack_error = false;
  _range_error = false;
  _pack_data.clear();

  // In repack mode, we immediately get the catalog, since we know we'll need
  // it.
  _root = root;
  _catalog = _root->get_catalog();
  _live_catalog = _catalog->get_live_catalog(_unpack_data, _unpack_length);
  if (_live_catalog == nullptr) {
    _pack_error = true;
  }

  // We don't begin at the first field in repack mode.  Instead, you must
  // explicitly call seek().
  _current_field = nullptr;
  _current_parent = nullptr;
  _current_field_index = 0;
  _num_nested_fields = 0;
}

/**
 * Finishes the repacking session.
 *
 * The return value is true on success, or false if there has been some error
 * during repacking (or if all fields have not been repacked).
 */
bool DCPacker::
end_repack() {
  nassertr(_mode == M_repack, false);

  // Put the rest of the data onto the pack stream.
  _pack_data.append_data(_unpack_data + _unpack_p, _unpack_length - _unpack_p);

  _mode = M_idle;
  clear();

  return !had_error();
}

/**
 * Sets the current unpack (or repack) position to the named field.  In unpack
 * mode, the next call to unpack_*() or push() will begin to read the named
 * field.  In repack mode, the next call to pack_*() or push() will modify the
 * named field.
 *
 * Returns true if successful, false if the field is not known (or if the
 * packer is in an invalid mode).
 */
bool DCPacker::
seek(const string &field_name) {
  if (_catalog == nullptr) {
    _catalog = _root->get_catalog();
    _live_catalog = _catalog->get_live_catalog(_unpack_data, _unpack_length);
  }
  nassertr(_catalog != nullptr, false);
  if (_live_catalog == nullptr) {
    _pack_error = true;
    return false;
  }

  int seek_index = _live_catalog->find_entry_by_name(field_name);
  if (seek_index < 0) {
    // The field was not known.
    _pack_error = true;
    return false;
  }

  return seek(seek_index);
}

/**
 * Seeks to the field indentified by seek_index, which was returned by an
 * earlier call to DCField::find_seek_index() to get the index of some nested
 * field.  Also see the version of seek() that accepts a field name.
 *
 * Returns true if successful, false if the field is not known (or if the
 * packer is in an invalid mode).
 */
bool DCPacker::
seek(int seek_index) {
  if (_catalog == nullptr) {
    _catalog = _root->get_catalog();
    _live_catalog = _catalog->get_live_catalog(_unpack_data, _unpack_length);
  }
  nassertr(_catalog != nullptr, false);
  if (_live_catalog == nullptr) {
    _pack_error = true;
    return false;
  }

  if (_mode == M_unpack) {
    const DCPackerCatalog::Entry &entry = _live_catalog->get_entry(seek_index);

    // If we are seeking, we don't need to remember our current stack
    // position.
    clear_stack();
    _current_field = entry._field;
    _current_parent = entry._parent;
    _current_field_index = entry._field_index;
    _num_nested_fields = _current_parent->get_num_nested_fields();
    _unpack_p = _live_catalog->get_begin(seek_index);

    // We don't really need _push_marker and _pop_marker now, except that we
    // should set _push_marker in case we have just seeked to a switch
    // parameter, and we should set _pop_marker to 0 just so it won't get in
    // the way.
    _push_marker = _unpack_p;
    _pop_marker = 0;

    return true;

  } else if (_mode == M_repack) {
    nassertr(_catalog != nullptr, false);

    if (_stack != nullptr || _current_field != nullptr) {
      // It is an error to reseek while the stack is nonempty--that means we
      // haven't finished packing the current field.
      _pack_error = true;
      return false;
    }
    const DCPackerCatalog::Entry &entry = _live_catalog->get_entry(seek_index);

    if (entry._parent->as_switch_parameter() != nullptr) {
      // If the parent is a DCSwitch, that can only mean that the seeked field
      // is a switch parameter.  We can't support seeking to a switch
      // parameter and modifying it directly--what would happen to all of the
      // related fields?  Instead, you'll have to seek to the switch itself
      // and repack the whole entity.
      _pack_error = true;
      return false;
    }

    size_t begin = _live_catalog->get_begin(seek_index);
    if (begin < _unpack_p) {
      // Whoops, we are seeking fields out-of-order.  That means we need to
      // write the entire record and start again.
      _pack_data.append_data(_unpack_data + _unpack_p, _unpack_length - _unpack_p);
      size_t length = _pack_data.get_length();
      char *buffer = _pack_data.take_data();
      set_unpack_data(buffer, length, true);
      _unpack_p = 0;

      _catalog->release_live_catalog(_live_catalog);
      _live_catalog = _catalog->get_live_catalog(_unpack_data, _unpack_length);

      if (_live_catalog == nullptr) {
        _pack_error = true;
        return false;
      }

      begin = _live_catalog->get_begin(seek_index);
    }

    // Now copy the bytes from _unpack_p to begin from the _unpack_data to the
    // _pack_data.  These are the bytes we just skipped over with the call to
    // seek().
    _pack_data.append_data(_unpack_data + _unpack_p, begin - _unpack_p);

    // And set the packer up to pack the indicated field (but no subsequent
    // fields).
    _current_field = entry._field;
    _current_parent = entry._parent;
    _current_field_index = entry._field_index;
    _num_nested_fields = 1;
    _unpack_p = _live_catalog->get_end(seek_index);

    // Set up push_marker and pop_marker so we won't try to advance beyond
    // this field.
    _push_marker = begin;
    _pop_marker = _live_catalog->get_end(seek_index);

    return true;
  }

  // Invalid mode.
  _pack_error = true;
  return false;
}

/**
 * Marks the beginning of a nested series of fields.
 *
 * This must be called before filling the elements of an array or the
 * individual fields in a structure field.  It must also be balanced by a
 * matching pop().
 *
 * It is necessary to use push() / pop() only if has_nested_fields() returns
 * true.
 */
void DCPacker::
push() {
  if (!has_nested_fields()) {
    _pack_error = true;

  } else {
    StackElement *element = new StackElement;
    element->_current_parent = _current_parent;
    element->_current_field_index = _current_field_index;
    element->_push_marker = _push_marker;
    element->_pop_marker = _pop_marker;
    element->_next = _stack;
    _stack = element;
    _current_parent = _current_field;


    // Now deal with the length prefix that might or might not be before a
    // sequence of nested fields.
    int num_nested_fields = _current_parent->get_num_nested_fields();
    size_t length_bytes = _current_parent->get_num_length_bytes();

    if (_mode == M_pack || _mode == M_repack) {
      // Reserve length_bytes for when we figure out what the length is.
      _push_marker = _pack_data.get_length();
      _pop_marker = 0;
      _pack_data.append_junk(length_bytes);

    } else if (_mode == M_unpack) {
      // Read length_bytes to determine the end of this nested sequence.
      _push_marker = _unpack_p;
      _pop_marker = 0;

      if (length_bytes != 0) {
        if (_unpack_p + length_bytes > _unpack_length) {
          _pack_error = true;

        } else {
          size_t length;
          if (length_bytes == 4) {
            length = DCPackerInterface::do_unpack_uint32
              (_unpack_data + _unpack_p);
            _unpack_p += 4;
          } else {
            length = DCPackerInterface::do_unpack_uint16
              (_unpack_data + _unpack_p);
            _unpack_p += 2;
          }
          _pop_marker = _unpack_p + length;

          // The explicit length trumps the number of nested fields reported
          // by get_num_nested_fields().
          if (length == 0) {
            num_nested_fields = 0;
          } else {
            num_nested_fields = _current_parent->calc_num_nested_fields(length);
          }
        }
      }
    } else {
      _pack_error = true;
    }


    // Now point to the first field in the nested range.
    _num_nested_fields = num_nested_fields;
    _current_field_index = 0;

    if (_num_nested_fields >= 0 &&
        _current_field_index >= _num_nested_fields) {
      _current_field = nullptr;

    } else {
      _current_field = _current_parent->get_nested_field(_current_field_index);
    }
  }
}

/**
 * Marks the end of a nested series of fields.
 *
 * This must be called to match a previous push() only after all the expected
 * number of nested fields have been packed.  It is an error to call it too
 * early, or too late.
 */
void DCPacker::
pop() {
  if (_current_field != nullptr && _num_nested_fields >= 0) {
    // Oops, didn't pack or unpack enough values.
    _pack_error = true;

  } else if (_mode == M_unpack && _pop_marker != 0 &&
             _unpack_p != _pop_marker) {
    // Didn't unpack the right number of values.
    _pack_error = true;
  }

  if (_stack == nullptr) {
    // Unbalanced pop().
    _pack_error = true;

  } else {
    if (!_current_parent->validate_num_nested_fields(_current_field_index)) {
      // Incorrect number of nested elements.
      _pack_error = true;
    }

    if (_mode == M_pack || _mode == M_repack) {
      size_t length_bytes = _current_parent->get_num_length_bytes();
      if (length_bytes != 0) {
        // Now go back and fill in the length of the array.
        size_t length = _pack_data.get_length() - _push_marker - length_bytes;
        if (length_bytes == 4) {
          DCPackerInterface::do_pack_uint32
            (_pack_data.get_rewrite_pointer(_push_marker, 4), length);
        } else {
          DCPackerInterface::validate_uint_limits(length, 16, _range_error);
          DCPackerInterface::do_pack_uint16
            (_pack_data.get_rewrite_pointer(_push_marker, 2), length);
        }
      }
    }

    _current_field = _current_parent;
    _current_parent = _stack->_current_parent;
    _current_field_index = _stack->_current_field_index;
    _push_marker = _stack->_push_marker;
    _pop_marker = _stack->_pop_marker;
    _num_nested_fields = (_current_parent == nullptr) ? 0 : _current_parent->get_num_nested_fields();

    StackElement *next = _stack->_next;
    delete _stack;
    _stack = next;
  }

  advance();
}

/**
 * Adds the default value for the current element into the stream.  If no
 * default has been set for the current element, creates a sensible default.
 */
void DCPacker::
pack_default_value() {
  nassertv(_mode == M_pack || _mode == M_repack);
  if (_current_field == nullptr) {
    _pack_error = true;
  } else {
    if (_current_field->pack_default_value(_pack_data, _pack_error)) {
      advance();

    } else {
      // If the single field didn't know how to pack a default value, try
      // packing nested fields.
      push();
      while (more_nested_fields()) {
        pack_default_value();
      }
      pop();
    }
  }
}

/**
 * Internally unpacks the current numeric or string value and validates it
 * against the type range limits, but does not return the value.  If the
 * current field contains nested fields, validates all of them.
 */
void DCPacker::
unpack_validate() {
  nassertv(_mode == M_unpack);
  if (_current_field == nullptr) {
    _pack_error = true;

  } else {
    if (_current_field->unpack_validate(_unpack_data, _unpack_length, _unpack_p,
                                        _pack_error, _range_error)) {
      advance();
    } else {
      // If the single field couldn't be validated, try validating nested
      // fields.
      push();
      while (more_nested_fields()) {
        unpack_validate();
      }
      pop();
    }
  }
}

/**
 * Skips the current field without unpacking it and advances to the next
 * field.  If the current field contains nested fields, skips all of them.
 */
void DCPacker::
unpack_skip() {
  nassertv(_mode == M_unpack);
  if (_current_field == nullptr) {
    _pack_error = true;

  } else {
    if (_current_field->unpack_skip(_unpack_data, _unpack_length, _unpack_p,
                                    _pack_error)) {
      advance();

    } else {
      // If the single field couldn't be skipped, try skipping nested fields.
      push();
      while (more_nested_fields()) {
        unpack_skip();
      }
      pop();
    }
  }
}

/**
 * Parses an object's value according to the DC file syntax (e.g.  as a
 * default value string) and packs it.  Returns true on success, false on a
 * parse error.
 */
bool DCPacker::
parse_and_pack(const string &formatted_object) {
  istringstream strm(formatted_object);
  return parse_and_pack(strm);
}

/**
 * Parses an object's value according to the DC file syntax (e.g.  as a
 * default value string) and packs it.  Returns true on success, false on a
 * parse error.
 */
bool DCPacker::
parse_and_pack(istream &in) {
  dc_init_parser_parameter_value(in, "parse_and_pack", *this);
  dcyyparse();
  dc_cleanup_parser();

  bool parse_error = (dc_error_count() != 0);
  if (parse_error) {
    _parse_error = true;
  }

  return !parse_error;
}

/**
 * Unpacks an object and formats its value into a syntax suitable for parsing
 * in the dc file (e.g.  as a default value), or as an input to parse_object.
 */
string DCPacker::
unpack_and_format(bool show_field_names) {
  ostringstream strm;
  unpack_and_format(strm, show_field_names);
  return strm.str();
}

/**
 * Unpacks an object and formats its value into a syntax suitable for parsing
 * in the dc file (e.g.  as a default value), or as an input to parse_object.
 */
void DCPacker::
unpack_and_format(ostream &out, bool show_field_names) {
  DCPackType pack_type = get_pack_type();

  if (show_field_names && !get_current_field_name().empty()) {
    nassertv(_current_field != nullptr);
    const DCField *field = _current_field->as_field();
    if (field != nullptr &&
        field->as_parameter() != nullptr) {
      out << field->get_name() << " = ";
    }
  }

  switch (pack_type) {
  case PT_invalid:
    out << "<invalid>";
    break;

  case PT_double:
    out << unpack_double();
    break;

  case PT_int:
    out << unpack_int();
    break;

  case PT_uint:
    out << unpack_uint();
    break;

  case PT_int64:
    out << unpack_int64();
    break;

  case PT_uint64:
    out << unpack_uint64();
    break;

  case PT_string:
    enquote_string(out, '"', unpack_string());
    break;

  case PT_blob:
    output_hex_string(out, unpack_literal_value());
    break;

  default:
    {
      switch (pack_type) {
      case PT_array:
        out << '[';
        break;

      case PT_field:
      case PT_switch:
        out << '(';
        break;

      case PT_class:
      default:
        out << '{';
        break;
      }

      push();
      while (more_nested_fields() && !had_pack_error()) {
        unpack_and_format(out, show_field_names);

        if (more_nested_fields()) {
          out << ", ";
        }
      }
      pop();

      switch (pack_type) {
      case PT_array:
        out << ']';
        break;

      case PT_field:
      case PT_switch:
        out << ')';
        break;

      case PT_class:
      default:
        out << '}';
        break;
      }
    }
    break;
  }
}

/**
 * Outputs the indicated string within quotation marks.
 */
void DCPacker::
enquote_string(ostream &out, char quote_mark, const string &str) {
  out << quote_mark;
  for (string::const_iterator pi = str.begin();
       pi != str.end();
       ++pi) {
    if ((*pi) == quote_mark || (*pi) == '\\') {
      out << '\\' << (*pi);

    } else if (!isprint(*pi)) {
      char buffer[10];
      sprintf(buffer, "%02x", (unsigned char)(*pi));
      out << "\\x" << buffer;

    } else {
      out << (*pi);
    }
  }
  out << quote_mark;
}

/**
 * Outputs the indicated string as a hex constant.
 */
void DCPacker::
output_hex_string(ostream &out, const vector_uchar &str) {
  out << '<';
  for (vector_uchar::const_iterator pi = str.begin();
       pi != str.end();
       ++pi) {
    char buffer[10];
    sprintf(buffer, "%02x", (unsigned char)(*pi));
    out << buffer;
  }
  out << '>';
}

/**
 * When we advance past the key field on a switch record, we suddenly have
 * more fields available--all the appropriate alternate fields in the switch.
 *
 * This function is called when we detect this condition; it switches the
 * _current_parent to the appropriate case of the switch record.
 */
void DCPacker::
handle_switch(const DCSwitchParameter *switch_parameter) {
  // First, get the value from the key.  This is either found in the unpack or
  // the pack data, depending on what mode we're in.
  const DCPackerInterface *new_parent = nullptr;

  if (_mode == M_pack || _mode == M_repack) {
    const char *data = _pack_data.get_data();
    new_parent = switch_parameter->apply_switch
      (data + _push_marker, _pack_data.get_length() - _push_marker);

  } else if (_mode == M_unpack) {
    new_parent = switch_parameter->apply_switch
      (_unpack_data + _push_marker, _unpack_p - _push_marker);
  }

  if (new_parent == nullptr) {
    // This means an invalid value was packed for the key.
    _range_error = true;
    return;
  }

  _last_switch = switch_parameter;

  // Now substitute in the switch case for the previous parent (which replaces
  // the switch node itself).  This will suddenly make a slew of new fields
  // appear.
  _current_parent = new_parent;
  _num_nested_fields = _current_parent->get_num_nested_fields();

  if (_num_nested_fields < 0 ||
      _current_field_index < _num_nested_fields) {
    _current_field = _current_parent->get_nested_field(_current_field_index);
  }
}

/**
 * Resets the data structures after a pack or unpack sequence.
 */
void DCPacker::
clear() {
  clear_stack();
  _current_field = nullptr;
  _current_parent = nullptr;
  _current_field_index = 0;
  _num_nested_fields = 0;
  _push_marker = 0;
  _pop_marker = 0;
  _last_switch = nullptr;

  if (_live_catalog != nullptr) {
    _catalog->release_live_catalog(_live_catalog);
    _live_catalog = nullptr;
  }
  _catalog = nullptr;
  _root = nullptr;
}

/**
 * Empties the stack.
 */
void DCPacker::
clear_stack() {
  while (_stack != nullptr) {
    StackElement *next = _stack->_next;
    delete _stack;
    _stack = next;
  }
}
