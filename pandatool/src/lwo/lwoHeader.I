// Filename: lwoHeader.I
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: LwoHeader::get_lwid
//       Access: Public
//  Description: Returns the Lightwave ID of the file.  This is the
//               first ID read within the header, and indicates the
//               version of the Lightwave file in some mysterious way.
////////////////////////////////////////////////////////////////////
IffId LwoHeader::
get_lwid() const {
  return _lwid;
}
