// Filename: fltRecord.I
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

INLINE ostream &
operator << (ostream &out, const FltRecord &record) {
  record.output(out);
  return out;
}
