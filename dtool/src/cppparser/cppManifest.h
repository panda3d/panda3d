/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppManifest.h
 * @author drose
 * @date 1999-10-22
 */

#ifndef CPPMANIFEST_H
#define CPPMANIFEST_H

#include "dtoolbase.h"

#include "cppFile.h"
#include "cppVisibility.h"
#include "cppBisonDefs.h"

#include "vector_string.h"

class CPPExpression;
class CPPType;

/**
 *
 */
class CPPManifest {
public:
  CPPManifest(const std::string &args, const cppyyltype &loc);
  CPPManifest(const std::string &macro, const std::string &definition);
  ~CPPManifest();

  static std::string stringify(const std::string &source);
  std::string expand(const vector_string &args = vector_string()) const;

  CPPType *determine_type() const;

  void output(std::ostream &out) const;

  std::string _name;
  bool _has_parameters;
  int _num_parameters;
  int _variadic_param;
  cppyyltype _loc;
  CPPExpression *_expr;

  // Manifests don't have a visibility in the normal sense.  Normally this
  // will be V_public.  But a manifest that is defined between __begin_publish
  // and __end_publish will have a visibility of V_published.
  CPPVisibility _vis;

private:
  void parse_parameters(const std::string &args, size_t &p,
                        vector_string &parameter_names);
  void save_expansion(const std::string &exp,
                      const vector_string &parameter_names);

  class ExpansionNode {
  public:
    ExpansionNode(int parm_number, bool stringify, bool paste);
    ExpansionNode(const std::string &str, bool paste = false);
    int _parm_number;
    bool _stringify;
    bool _paste;
    std::string _str;
  };
  typedef std::vector<ExpansionNode> Expansion;
  Expansion _expansion;
};

inline std::ostream &operator << (std::ostream &out, const CPPManifest &manifest) {
  manifest.output(out);
  return out;
}

#endif
