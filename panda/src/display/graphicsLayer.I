// Filename: graphicsLayer.I
// Created by:  drose (18Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_channel
//       Access: Public
//  Description: Returns the GraphicsChannel that this layer is
//               associated with.  It is possible that the
//               GraphicsChannel might have been deleted while an
//               outstanding PT(GraphicsLayer) prevented all of its
//               children layers from also being deleted; in this
//               unlikely case, get_channel() may return NULL.
////////////////////////////////////////////////////////////////////
INLINE GraphicsChannel *GraphicsLayer::
get_channel() const {
  return _channel;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::set_active
//       Access: Public
//  Description: Sets the active flag on the layer.  If the layer
//               is marked as inactive, nothing will be rendered.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsLayer::
set_active(bool active) {
  _is_active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::is_active
//       Access: Public
//  Description: Returns the active flag on the layer.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsLayer::
is_active() const {
  return _is_active;
}

