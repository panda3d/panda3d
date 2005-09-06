// Filename: glShaderContext_src.cxx
// Created by: jyelon (01Sep05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

TypeHandle CLP(ShaderContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(CLP(GraphicsStateGuardian) *gsg, Shader *s) : ShaderContext(s) {
  string header;
  _gsg = gsg;
  _valid = false;
  s->parse_init();
  s->parse_line(header, true, true);
  
#ifdef HAVE_CGGL
  _cg_vprogram = 0;
  _cg_fprogram = 0;

  if (header == "Cg") {
    if (_gsg->_cg_context == 0) {
      cerr << "Cannot compile shader, no Cg context\n";
      return;
    }
    string commentary, vs, fs;
    s->parse_upto(commentary, "---*---", false);
    s->parse_upto(vs, "---*---", false);
    s->parse_upto(fs, "---*---", false);

    _cg_vprogram = cgCreateProgram(_gsg->_cg_context, CG_SOURCE,
                                   vs.c_str(), _gsg->_cg_vprofile,
                                   "main", (const char**)NULL);
    _cg_fprogram = cgCreateProgram(_gsg->_cg_context, CG_SOURCE,
                                   fs.c_str(), _gsg->_cg_fprofile,
                                   "main", (const char**)NULL);
    
    if ((_cg_vprogram==0)||(_cg_fprogram == 0)) {
      if (_cg_vprogram != 0) cgDestroyProgram(_cg_vprogram);
      if (_cg_fprogram != 0) cgDestroyProgram(_cg_fprogram);
      cerr << "Invalid Cg program" << s->_file << "\n";
      return;
    }
  }
#endif

  cerr << "Unrecognized shader language " << header << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Destructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
~CLP(ShaderContext)() {
}

