// Filename: eggSAnimData.I
// Created by:  drose (19Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggSAnimData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggSAnimData::
EggSAnimData(const string &name) : EggAnimData(name) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggSAnimData::Copy constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggSAnimData::
EggSAnimData(const EggSAnimData &copy) : EggAnimData(copy) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggSAnimData::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggSAnimData &EggSAnimData::
operator = (const EggSAnimData &copy) {
  EggAnimData::operator = (copy);

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggSAnimData::get_num_rows
//       Access: Public
//  Description: Returns the number of rows in the table.  For an
//               SAnim table, each row has one column.
////////////////////////////////////////////////////////////////////
INLINE int EggSAnimData::
get_num_rows() const {
  return get_size();
}


////////////////////////////////////////////////////////////////////
//     Function: EggSAnimData::get_value
//       Access: Public
//  Description: Returns the value at the indicated row.  Row must be
//               in the range 0 <= row < get_num_rows().
////////////////////////////////////////////////////////////////////
INLINE double EggSAnimData::
get_value(int row) const {
  nassertr(row >= 0 && row < get_num_rows(), 0.0);
  return _data[row];
}


