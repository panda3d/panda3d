// Filename: cLwoLayer.I
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CLwoLayer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLwoLayer::
CLwoLayer(LwoToEggConverter *converter, const LwoLayer *layer) : 
  _converter(converter),
  _layer(layer) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoLayer::get_number
//       Access: Public
//  Description: Returns the index number associated with this
//               particular layer.  This should be unique among all
//               Lightwave layers in a single file.
////////////////////////////////////////////////////////////////////
INLINE int CLwoLayer::
get_number() const {
  return _layer->_number;
}
