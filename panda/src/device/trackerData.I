// Filename: trackerData.I
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TrackerData::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TrackerData::
TrackerData() :
  ptime(0), vtime(0), vquat_dt(0), atime(0), aquat_dt(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerData::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TrackerData::
TrackerData(const TrackerData &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerData::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TrackerData &TrackerData::
operator = (const TrackerData &copy) {
  ptime = copy.ptime;
  position = copy.position;
  pquat = copy.pquat;

  vtime = copy.vtime;
  velocity = copy.velocity;
  vquat = copy.vquat;
  vquat_dt = copy.vquat_dt;

  atime = copy.atime;
  acceleration = copy.acceleration;
  aquat = copy.aquat;
  aquat_dt = copy.aquat_dt;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerData::none
//       Access: Public, Static
//  Description: Returns an empty TrackerData object
////////////////////////////////////////////////////////////////////
INLINE const TrackerData &TrackerData::
none() {
 return _none; 
}
