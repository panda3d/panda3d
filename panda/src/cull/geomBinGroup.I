// Filename: geomBinGroup.I
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinGroup::
GeomBinGroup(const string &name, CullTraverser *traverser, int sort) :
  GeomBin(name, traverser, sort) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::add_sub_bin
//       Access: Public
//  Description: Appends the indicated bin to the end of the sub_bin
//               list, and returns the new index.
////////////////////////////////////////////////////////////////////
INLINE int GeomBinGroup::
add_sub_bin(GeomBin *sub_bin) {
  _sub_bins.push_back(sub_bin);
  return _sub_bins.size() - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::get_num_bins
//       Access: Public
//  Description: Returns the number of available sub bins.
////////////////////////////////////////////////////////////////////
INLINE int GeomBinGroup::
get_num_bins() const {
  return _sub_bins.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::get_bin
//       Access: Public
//  Description: Returns the nth sub bin.
////////////////////////////////////////////////////////////////////
INLINE GeomBin *GeomBinGroup::
get_bin(int n) {
  nassertr(n >= 0 && n < (int)_sub_bins.size(), NULL);
  return _sub_bins[n];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::remove_bin
//       Access: Public
//  Description: Removes the nth bin.  All subsequent index numbers
//               shift down by one.  The return value is the
//               just-removed bin, which may be deleted if it is not
//               immediately saved.
////////////////////////////////////////////////////////////////////
INLINE PT(GeomBin) GeomBinGroup::
remove_bin(int n) {
  nassertr(n >= 0 && n < (int)_sub_bins.size(), NULL);
  PT(GeomBin) keep = get_bin(n);
  _sub_bins.erase(_sub_bins.begin() + n);
  return keep;
}
