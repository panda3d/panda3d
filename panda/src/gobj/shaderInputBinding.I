/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInputBinding.I
 * @author rdb
 * @date 2024-09-22
 */

/**
 * Convenient shorthand for creating an input that fetches a single texture
 * using an arbitrary callable.
 */
template<class Callable>
INLINE ShaderInputBinding *ShaderInputBinding::
make_data(int dep, Callable callable) {
  class InlineShaderInputBinding final : public ShaderInputBinding {
  public:
    InlineShaderInputBinding(Callable callable, int dep) :
      _callable(std::move(callable)),
      _dep(dep) {
    }

    virtual int get_state_dep() const override final {
      return _dep;
    }

    virtual void fetch_data(const State &state, void *into, bool packed) const override final {
      _callable(state, into, packed);
    }

  private:
    Callable _callable;
    int _dep;
  };
  return new InlineShaderInputBinding(std::move(callable), dep);
}

/**
 * Convenient shorthand for creating an input that fetches a single texture
 * using an arbitrary callable.
 */
template<class Callable>
INLINE ShaderInputBinding *ShaderInputBinding::
make_texture(int dep, Callable callable) {
  class InlineShaderInputBinding final : public ShaderInputBinding {
  public:
    InlineShaderInputBinding(Callable callable, int dep) :
      _callable(std::move(callable)),
      _dep(dep) {
    }

    virtual int get_state_dep() const override final {
      return _dep;
    }

    virtual PT(Texture) fetch_texture(const State &state, ResourceId resource_id, SamplerState &sampler, int &view) const override final {
      return _callable(state, sampler, view);
    }

  private:
    Callable _callable;
    int _dep;
  };
  return new InlineShaderInputBinding(std::move(callable), dep);
}

/**
 * Convenient shorthand for creating an input that fetches a single image using
 * an arbitrary callable.
 */
template<class Callable>
INLINE ShaderInputBinding *ShaderInputBinding::
make_texture_image(int dep, Callable callable) {
  class InlineShaderInputBinding final : public ShaderInputBinding {
  public:
    InlineShaderInputBinding(Callable callable, int dep) :
      _callable(std::move(callable)),
      _dep(dep) {
    }

    virtual int get_state_dep() const override final {
      return _dep;
    }

    virtual PT(Texture) fetch_texture_image(const State &state, ResourceId resource_id, ShaderType::Access &access, int &z, int &n) const override final {
      return _callable(state, access, z, n);
    }

  private:
    Callable _callable;
    int _dep;
  };
  return new InlineShaderInputBinding(std::move(callable), dep);
}
