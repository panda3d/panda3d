// Filename: pStatThreadData.I
// Created by:  drose (10Jul00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_client_data
//       Access: Public
//  Description: Returns a pointer to the ClientData structure
//               associated with this data.
////////////////////////////////////////////////////////////////////
INLINE const PStatClientData *PStatThreadData::
get_client_data() const {
  return _client_data;
}
