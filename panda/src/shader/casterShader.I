// Filename: casterShader.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
//     Function: CasterShader::add_caster
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int CasterShader::add_caster(NamedNode* node)
{
  make_dirty();
  _casters.push_back(node);
  return (_casters.size());
}

////////////////////////////////////////////////////////////////////
//     Function: CasterShader::remove_caster
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int CasterShader::remove_caster(NamedNode* node)
{
  make_dirty();
  NamedNodeVector::iterator i;
  for (i = _casters.begin(); i != _casters.end(); ++i) {
    if ((*i) == node) {
      _casters.erase(i);
      return true;
    }
  }
  return false;
}
