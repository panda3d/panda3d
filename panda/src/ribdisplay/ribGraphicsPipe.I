// Filename: ribGraphicsPipe.I
// Created by:  drose (17Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE RIBGraphicsPipe::
RIBGraphicsPipe(const PipeSpecifier& spec)
  : NoninteractiveGraphicsPipe(spec), _filename(spec.get_file_name()) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::get_file_name
//       Access: Public
//  Description: gets the base filename
////////////////////////////////////////////////////////////////////
INLINE std::string RIBGraphicsPipe::
get_file_name(void) const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE RIBGraphicsPipe::
RIBGraphicsPipe(void) {
  cerr << "RIBGraphicsPipes should not be created with default constructor"
       << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE RIBGraphicsPipe::
RIBGraphicsPipe(const RIBGraphicsPipe&) {
  cerr << "RIBGraphicsPipes should not be copied" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::operator=
//       Access: Public
//  Description: assignment operator
////////////////////////////////////////////////////////////////////
INLINE RIBGraphicsPipe& RIBGraphicsPipe::
operator=(const RIBGraphicsPipe&) {
  cerr << "RIBGraphicsPipes should not be assigned" << endl;
  return *this;
}
