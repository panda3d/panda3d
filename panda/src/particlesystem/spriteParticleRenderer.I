// Filename: spriteParticleRenderer.I
// Created by:  charles (13Jul00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Function : set_texture
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void SpriteParticleRenderer::
set_texture(Texture *tex) {
  _sprite_primitive->set_texture(tex);
}

////////////////////////////////////////////////////////////////////
// Function : get_texture
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE Texture *SpriteParticleRenderer::
get_texture(void) const {
  return _sprite_primitive->get_texture();
}

////////////////////////////////////////////////////////////////////
// Function : set_alpha_disable
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void SpriteParticleRenderer::
set_alpha_disable(bool ad) {
  _sprite_primitive->set_alpha_disable(ad);
}

////////////////////////////////////////////////////////////////////
// Function : get_alpha_disable
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE bool SpriteParticleRenderer::
get_alpha_disable(void) const {
  return _sprite_primitive->get_alpha_disable();
}

////////////////////////////////////////////////////////////////////
// Function : set_color
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void SpriteParticleRenderer::
set_color(const Colorf& color) {
  _color = color;
}

////////////////////////////////////////////////////////////////////
// Function : set_animation_flags
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void SpriteParticleRenderer::
set_animation_flags(bool animate_x_ratio,
		    bool animate_y_ratio,
		    bool animate_theta) {
  if (animate_x_ratio == true && _animate_x_ratio == false) {
    _x_texel_array = PTA_float(_pool_size);
    _sprite_primitive->set_x_texel_ratio(_x_texel_array, G_PER_PRIM);
  }
  else if (animate_x_ratio == false && _animate_x_ratio == true) {
    _x_texel_array = PTA_float(1);
    _sprite_primitive->set_x_texel_ratio(_x_texel_array, G_OVERALL);
  }

  _animate_x_ratio = animate_x_ratio;

  if (animate_y_ratio == true && _animate_y_ratio == false) {
    _y_texel_array = PTA_float(_pool_size);
    _sprite_primitive->set_y_texel_ratio(_y_texel_array, G_PER_PRIM);
  }
  else if (animate_y_ratio == false && _animate_y_ratio == true) {
    _y_texel_array = PTA_float(1);
    _sprite_primitive->set_y_texel_ratio(_y_texel_array, G_OVERALL);
  }

  _animate_y_ratio = animate_y_ratio;

  if (animate_theta == true && _animate_theta == false) {
    _theta_array = PTA_float(_pool_size);
    _sprite_primitive->set_thetas(_theta_array, G_PER_PRIM);
  }
  else if (animate_theta == false && _animate_theta == true) {
    _theta_array = PTA_float(_pool_size);
    _sprite_primitive->set_thetas(_theta_array, G_OVERALL);
  }

  _animate_theta = animate_theta;
}

////////////////////////////////////////////////////////////////////
// Function : set_x_ratios
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void SpriteParticleRenderer::
set_x_ratios(float initial_x_texel_ratio, 
	     float final_x_texel_ratio) {
  _initial_x_texel_ratio = initial_x_texel_ratio;
  _final_x_texel_ratio = final_x_texel_ratio;
}

////////////////////////////////////////////////////////////////////
// Function : set_y_ratios
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void SpriteParticleRenderer::
set_y_ratios(float initial_y_texel_ratio,
	     float final_y_texel_ratio) {
  _initial_y_texel_ratio = initial_y_texel_ratio;
  _final_y_texel_ratio = final_y_texel_ratio;
}

////////////////////////////////////////////////////////////////////
// Function : set_nonanimated_theta
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void SpriteParticleRenderer::
set_nonanimated_theta(float theta) {
  _theta = theta;
}

////////////////////////////////////////////////////////////////////
// Function : set_blend_type
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void SpriteParticleRenderer::
set_blend_type(ParticleRendererBlendMethod bm) {
  _blend_method = bm;
}
