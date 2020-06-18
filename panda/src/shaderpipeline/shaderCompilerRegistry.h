/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerRegistry.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef SHADERCOMPILERREGISTRY_H
#define SHADERCOMPILERREGISTRY_H

#include "pandabase.h"

#include "pvector.h"
#include "pmap.h"
#include "shader.h"

class ShaderCompiler;

/**
 * This class maintains the set of all known ShaderCompilers in the universe.
 */
class EXPCL_PANDA_SHADERPIPELINE ShaderCompilerRegistry {
protected:
  ShaderCompilerRegistry();

public:
  ~ShaderCompilerRegistry();

  void register_compiler(ShaderCompiler *compiler);
  void register_deferred_compiler(Shader::ShaderLanguage language, const std::string &library);

PUBLISHED:
  int get_num_compilers() const;
  ShaderCompiler *get_compiler(int n) const;
  MAKE_SEQ(get_compilers, get_num_compilers, get_compiler);
  MAKE_SEQ_PROPERTY(compilers, get_num_compilers, get_compiler);
  ShaderCompiler *get_compiler_from_language(Shader::ShaderLanguage language);

  void write(std::ostream &out, int indent_level = 0) const;

  static ShaderCompilerRegistry *get_global_ptr();

private:
  void record_language(Shader::ShaderLanguage language, ShaderCompiler *compiler);

private:
  typedef pvector<ShaderCompiler *> Compilers;
  Compilers _compilers;

  typedef pmap<Shader::ShaderLanguage, ShaderCompiler *> Languages;
  Languages _languages;

  typedef pmap<Shader::ShaderLanguage, std::string> DeferredCompilers;
  DeferredCompilers _deferred_compilers;

  static ShaderCompilerRegistry *_global_ptr;
};

#endif
