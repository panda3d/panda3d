// Filename: animChannelFixed.I
// Created by:  drose (24Feb99)
// 
////////////////////////////////////////////////////////////////////

template<class SwitchType>
TypeHandle AnimChannelFixed<SwitchType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::Constructor
//       Access: Public
//  Description: This is the flavor of AnimChannelFixed that puts it
//               in a hierarchy.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE AnimChannelFixed<SwitchType>::
AnimChannelFixed(AnimGroup *parent, const string &name,
		 const ValueType &value) 
  : AnimChannel<SwitchType>(parent, name),
    _value(value) {
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::Constructor
//       Access: Public
//  Description: This flavor creates an AnimChannelFixed that is not
//               in a hierarchy.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
INLINE AnimChannelFixed<SwitchType>::
AnimChannelFixed(const string &name, const ValueType &value) 
  : AnimChannel<SwitchType>(name),
    _value(value) {
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::has_changed
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class SwitchType>
bool AnimChannelFixed<SwitchType>::
has_changed(int, int) {
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::get_value
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannelFixed<SwitchType>::
get_value(int, ValueType &value) {
  value = _value;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelFixed::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class SwitchType>
void AnimChannelFixed<SwitchType>::
output(ostream &out) const {
  AnimChannel<SwitchType>::output(out);
  out << " = " << _value;
}
