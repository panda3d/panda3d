// Filename: WebBrowserTexture.h
// Created by: Bei Yang (03Aug2010)
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

#ifndef WebBrowserTexture_H
#define WebBrowserTexture_H

#include "pandabase.h"
#include "texture.h"
#include "awWebView.h"


////////////////////////////////////////////////////////////////////
//       Class : WebBrowserTexture
// Description : A Wrapper class for Awesomium webview.  This
//               implements most of Awesomium's features and
//               updates on the cull_traverser callback much
//               much like a movie texture.
//               
//               The use of class means that you will have to
//               follow Awesomium license agreement give below
//               http://www.khrona.com/products/awesomium/licensing/
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAAWESOMIUM WebBrowserTexture : public Texture {
protected:
    AwWebView* _aw_web_view;
    bool _update_active;
    bool _flip_texture_active;

//Constructors & Destructors ------------
private:
    WebBrowserTexture(const WebBrowserTexture &copy);
PUBLISHED:
    WebBrowserTexture(const string &name, AwWebView* aw_web_view = NULL);
    virtual ~WebBrowserTexture();


//methods --------------
protected:
    bool get_keep_ram_image() const;
    void do_reload_ram_image();
public:
    virtual bool has_cull_callback() const;
    virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;
PUBLISHED:
    void set_web_view(AwWebView* aw_web_view);
    AwWebView* get_web_view() const;
    void set_update_active(bool active_flag);
    bool get_update_active() const;
    void set_flip_texture_active(bool active_flag);
    bool get_flip_texture_active() const;

//Type handles ----------------
public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        Texture::init_type();
        register_type(_type_handle, "WebBrowserTexture",
            Texture::get_class_type());
    }
    virtual TypeHandle get_type() const {
        return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
    static TypeHandle _type_handle;
};
#endif
