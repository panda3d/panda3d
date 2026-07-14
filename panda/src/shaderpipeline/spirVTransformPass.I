/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformPass.I
 * @author rdb
 * @date 2024-10-08
 */

/**
 *
 */
INLINE void SpirVTransformPass::AccessChain::
prepend(uint32_t id) {
  _chain.insert(_chain.begin(), id);
}

/**
 *
 */
INLINE void SpirVTransformPass::AccessChain::
append(uint32_t id) {
  _chain.push_back(id);
}

/**
 *
 */
INLINE void SpirVTransformPass::AccessChain::
extend(const AccessChain &other) {
  _chain.insert(_chain.end(), other._chain.begin(), other._chain.end());
}

/**
 *
 */
INLINE bool SpirVTransformPass::AccessChain::
startswith(const AccessChain &other) const {
  if (_var_id != other._var_id || other._chain.size() > _chain.size()) {
    return false;
  }
  for (size_t i = 0; i < other.size(); ++i) {
    if (_chain[i] != other._chain[i]) {
      return false;
    }
  }
  return true;
}

/**
 *
 */
INLINE bool SpirVTransformPass::AccessChain::
operator < (const AccessChain &other) const {
  if (_var_id != other._var_id) {
    return _var_id < other._var_id;
  }
  return _chain < other._chain;
}

/**
 *
 */
INLINE void SpirVTransformPass::AccessChain::
output(std::ostream &out) const {
  out << _var_id;
  for (size_t i = 0; i < _chain.size(); ++i) {
    out << '[' << _chain[i] << ']';
  }
}

/**
 *
 */
INLINE std::ostream &
operator << (std::ostream &out, const SpirVTransformPass::AccessChain &obj) {
  obj.output(out);
  return out;
}
