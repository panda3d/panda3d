/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompiler.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderCompiler.h"
#include "datagram.h"

TypeHandle ShaderCompiler::_type_handle;

/**
 *
 */
ShaderCompiler::
ShaderCompiler() {
}

/**
 *
 */
ShaderCompiler::
~ShaderCompiler() {
}

/**
 * Adds everything that identifies the output this compiler produces to the
 * datagram, which the ShaderPool hashes into the key that compiled modules
 * are cached under, alongside the source code and options.
 *
 * A compiler whose output may change from one build of Panda3D to the next
 * (because the underlying compiler library was upgraded, or because its own
 * code generation changed) must override this to write something that
 * changes along with it, such as a library version and a hand-bumped
 * revision number.  Otherwise, modules produced by an older build remain
 * valid in the on-disk cache indefinitely.
 *
 * The written data must be deterministic for a given build of Panda3D; it
 * may not contain pointers, timestamps, or anything else that varies from
 * one process to the next.
 */
void ShaderCompiler::
write_cache_key(Datagram &dg) const {
}
