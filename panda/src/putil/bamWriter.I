/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamWriter.I
 * @author jason
 * @date 2000-06-08
 */

/**
 * Returns the current target of the BamWriter as set by set_target() or the
 * constructor.
 */
INLINE DatagramSink *BamWriter::
get_target() {
  return _target;
}

/**
 * If a BAM is a file, then the BamWriter should contain the name of the file.
 * This enables the writer to convert pathnames in the BAM to relative to the
 * directory containing the BAM.
 */
INLINE const Filename &BamWriter::
get_filename() const {
  if (_target != nullptr) {
    return _target->get_filename();
  }
  static const Filename empty_filename;
  return empty_filename;
}

/**
 * Returns the major version number of the Bam file currently being written.
 */
INLINE int BamWriter::
get_file_major_ver() const {
  return _file_major;
}

/**
 * Changes the minor .bam version to write.  This should be called before
 * init().  Each Panda version has only a fairly narrow range of versions it
 * is able to write; consult the .bam documentation for more information.
 */
INLINE void BamWriter::
set_file_minor_ver(int minor_ver) {
  _file_minor = minor_ver;
}

/**
 * Returns the minor version number of the Bam file currently being written.
 */
INLINE int BamWriter::
get_file_minor_ver() const {
  return _file_minor;
}

/**
 * Returns the endian preference indicated by the Bam file currently being
 * written.  This does not imply that every number is stored using the
 * indicated convention, but individual objects may choose to respect this
 * flag when recording data.
 */
INLINE BamWriter::BamEndian BamWriter::
get_file_endian() const {
  return _file_endian;
}

/**
 * Returns true if the file will store all "standard" floats as 64-bit
 * doubles, or false if they are 32-bit floats.  This isn't runtime settable;
 * it's based on the compilation flags of the version of Panda that generated
 * this file.
 */
INLINE bool BamWriter::
get_file_stdfloat_double() const {
  return _file_stdfloat_double;
}

/**
 * Returns the BamTextureMode preference indicated by the Bam file currently
 * being written.  Texture objects written to this Bam file will be encoded
 * according to the specified mode.
 */
INLINE BamWriter::BamTextureMode BamWriter::
get_file_texture_mode() const {
  return _file_texture_mode;
}

/**
 * Changes the BamTextureMode preference for the Bam file currently being
 * written.  Texture objects written to this Bam file will be encoded
 * according to the specified mode.
 */
INLINE void BamWriter::
set_file_texture_mode(BamTextureMode file_texture_mode) {
  _file_texture_mode = file_texture_mode;
}

/**
 * Returns the root node of the part of the scene graph we are currently
 * writing out.  This is used for determining what to make NodePaths relative
 * to.
 */
INLINE TypedWritable *BamWriter::
get_root_node() const {
  return _root_node;
}

/**
 * Sets the root node of the part of the scene graph we are currently writing
 * out.  NodePaths written to this bam file will be relative to this node.
 */
INLINE void BamWriter::
set_root_node(TypedWritable *root_node) {
  _root_node = root_node;
}
