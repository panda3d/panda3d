// Filename: WebBrowserTexture.cxx
// Created by:  bei yang (Mar 2010)
//
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_awesomium.h"
#include "WebBrowserTexture.h"

TypeHandle WebBrowserTexture::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::WebBrowserTexture
//       Access: Published
//  Description: Copy constructor for web browser texture.  The behavior
//               of copying a webtexture is that will be the same
//               as a standard texture copy.  However, the content
//               will remain the system until set_web_view is called.
////////////////////////////////////////////////////////////////////
WebBrowserTexture::WebBrowserTexture(const WebBrowserTexture &copy):
Texture(copy)
{
    //this kind of assumes that the previous texture
    //was initialized properly
    _aw_web_view = copy._aw_web_view;
    _update_active = copy._update_active;
    _flip_texture_active = copy._flip_texture_active;
}

////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::WebBrowserTexture
//       Access: Published
//  Description: This initializes a web browser texture with the given
//               AwWebView class.
////////////////////////////////////////////////////////////////////
WebBrowserTexture::WebBrowserTexture(const string &name, AwWebView* aw_web_view):
Texture(name),
_update_active(true),
_flip_texture_active(false)
{
    set_web_view(aw_web_view);
    set_minfilter(FT_linear);
    set_magfilter(FT_linear);

}

////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::~WebBrowserTexture
//       Access: Published
//  Description: Standard destructor... doesn't do anything. All
//               destructing happens in parent texture class.
////////////////////////////////////////////////////////////////////
WebBrowserTexture::~WebBrowserTexture()
{
    //do nothing
}


////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::~WebBrowserTexture
//       Access: Published
//  Description: Standard destructor... doesn't do anything. All
//               destructing happens in parent texture class.
////////////////////////////////////////////////////////////////////
bool WebBrowserTexture::get_keep_ram_image() const {
    return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::reload_ram_image
//       Access: Protected, Virtual
//  Description: A WebBrowserTexture must always keep its ram image.
//               This is essentially a sub.
////////////////////////////////////////////////////////////////////
void WebBrowserTexture::do_reload_ram_image() {
    // A MovieTexture should never dump its RAM image.
    // Therefore, this is not needed.
}

////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.  
//               
//               This one returns true because it uses
//               the cull traverser method to do the texture udpate.
////////////////////////////////////////////////////////////////////
bool WebBrowserTexture::has_cull_callback() const {
    return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::set_web_view
//       Access: Published
//  Description: Sets the internal AwWebView of this texture.
//               After calling this, the texture will automatically
//               set it's width and height to match the AwWebView
//               at the next time it is culled and rendered.
////////////////////////////////////////////////////////////////////
void WebBrowserTexture::set_web_view(AwWebView* aw_web_view){
    _aw_web_view = aw_web_view;
}


////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::get_web_view
//       Access: Published
//  Description: Gets the current internal AwWebView of this texture.
////////////////////////////////////////////////////////////////////
AwWebView* WebBrowserTexture::get_web_view() const{
    return _aw_web_view;
}


////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::set_update_active
//       Access: Published
//  Description: Gives the ability to toggle updating this texture
//               or not.  This can be disabled to improve performance
//               so that only the one that needs to be active is
//               active.
////////////////////////////////////////////////////////////////////
void WebBrowserTexture::set_update_active(bool active_flag){
    _update_active = active_flag;
}


////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::get_update_active
//       Access: Published
//  Description: Gets whether or not this texture is updating
//               itself every time it is rendered.
////////////////////////////////////////////////////////////////////
bool WebBrowserTexture::get_update_active() const{
    return _update_active;
}


////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::set_flip_texture_active
//       Access: Published
//  Description: This toggles on/off automatic flipping of the
//               of the texture at a source level.  Awesomium renders
//               things that are flipped vertically.  This enables
//               automatic flipping of that.
//
//               Since it is doing byte manipulation, this can get 
//               rather slow. Turning this on should be avoided.
//               Instead, flipping should be taken care of via UV
//               coordinates or shaders.
////////////////////////////////////////////////////////////////////
void WebBrowserTexture::set_flip_texture_active(bool active_flag){
    _flip_texture_active = active_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::get_flip_texture_active
//       Access: Published
//  Description: Returns whether automatic texture flipping is
//               enabled.
////////////////////////////////////////////////////////////////////
bool WebBrowserTexture::get_flip_texture_active() const {
    return _flip_texture_active;
}


////////////////////////////////////////////////////////////////////
//     Function: WebBrowserTexture::cull_callback
//       Access: Public, Virtual
//  Description: This function will be called during the cull 
//               traversal to update the WebBrowserTexture.  This
//               method calls the render method of AwWebView but
//               does not call the update method of AwWebCore.
////////////////////////////////////////////////////////////////////
bool WebBrowserTexture::cull_callback(CullTraverser *trav, const CullTraverserData &data) const{
    //see if we are in a state where udpates can happen. else just return
    if( !_update_active ) return true;
    if( _aw_web_view == NULL ) return true;

    //do we even need to update?
    if( !_aw_web_view->is_dirty() ) return true;

    //see if we're the same size, if not we need to make sure this texture
    //matches the webview
    if( _aw_web_view->get_width() != get_x_size() || _aw_web_view->get_height() != get_y_size() || get_texture_type() != TT_2d_texture){
        //these casts are so dirty especially when the method itself is
        //labled as const.  Really Texture::cull_callback should be not const
        //first clean up
        ((WebBrowserTexture*)this)->clear_ram_mipmap_images();
        ((WebBrowserTexture*)this)->clear_ram_image();
        //now set up the texture again
        ((WebBrowserTexture*)this)->setup_2d_texture( _aw_web_view->get_width(), _aw_web_view->get_height(), T_unsigned_byte, F_rgba );
        //should be good to go at this point
    }

    //get the pointer
    PTA_uchar ram_image = ((WebBrowserTexture*)this)->modify_ram_image();
    unsigned char* cp_data = ram_image.p();
    //render it
    _aw_web_view->render((void*)cp_data, get_x_size()*4, 4);

    if(_flip_texture_active){
        //flips the texture around... this is super slow. Really this should
        //never be enabled.  However beginners might find this useful
        size_t width = get_x_size();
        size_t height = get_y_size();
        for(size_t i=0; i < height/2; i++){
            for(size_t j=0; j < width; j++){
                unsigned char tmp[4];
                size_t a_pos = j+width*i;
                size_t b_pos = j + width*(height-i-1);
                memcpy(tmp,&cp_data[4*a_pos], 4); //tmp = a
                memcpy(&cp_data[4*a_pos], &cp_data[4*b_pos], 4); //a = b
                memcpy(&cp_data[4*b_pos], tmp, 4); //b = tmp
            }
        }
    }
    //success
    return true;
}