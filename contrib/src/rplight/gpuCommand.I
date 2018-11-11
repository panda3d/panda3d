/**
 *
 * RenderPipeline
 *
 * Copyright (c) 2014-2016 tobspr <tobias.springer1@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdint.h>

/**
 * @brief Appends an integer to the GPUCommand.
 * @details This adds an integer to the back of the GPUCommand. Depending on the
 *   setting in convert_int_to_float, this will either just convert the int to a
 *   float by casting it, or just do a bitwise copy.
 *
 * @param v The integer to append.
 */
inline void GPUCommand::push_int(int v) {
  push_float(convert_int_to_float(v));
}

/**
 * @brief Internal method to convert an integer to float
 * @details This methods gets called by the GPUCommand::push_int, and manages
 *   storing an integer in a floating point variable. There are two options,
 *   which are documented inside of the method.
 *
 * @param v Integer to convert
 * @return Float-representation of that integer, either casted or binary converted.s
 */
inline float GPUCommand::convert_int_to_float(int v) const {

  #if !PACK_INT_AS_FLOAT
    // Just round to float, can cause rounding issues tho
    return (float)v;

  #else
    assert(sizeof(float) == 4); // We really need this for packing! Better
                  // throw an error if the compiler uses more
                  // than 4 bytes.
    // Simple binary conversion, assuming sizeof(int) == sizeof(float)
    union { int32_t _int; float _float; } converter = { (int32_t)v };
    return converter._float;
  #endif
}

/**
 * @brief Appends a float to the GPUCommand.
 * @details This adds an integer to the back of the GPUCommand. Its used by all
 *   other push_xxx methods, and simply stores the value, then increments the write
 *   pointer. When the amount of floats exceeds the capacity of the GPUCommand,
 *   an error will be printed, and the method returns without doing anything else.
 *
 * @param v The float to append.
 */
inline void GPUCommand::push_float(float v) {
  if (_current_index >= GPU_COMMAND_ENTRIES) {
    gpucommand_cat.error() << "Out of bounds! Exceeded command size of " << GPU_COMMAND_ENTRIES << std::endl;
    return;
  }
  _data[_current_index++] = v;
}

/**
 * @brief Appends a 3-component floating point vector to the GPUCommand.
 * @details This appends a 3-component floating point vector to the command.
 *   It basically just calls push_float() for every component, in the order
 *   x, y, z, which causes the vector to occupy the space of 3 floats.
 *
 * @param v Int-Vector to append.
 */
inline void GPUCommand::push_vec3(const LVecBase3 &v) {
  push_float(v.get_x());
  push_float(v.get_y());
  push_float(v.get_z());
}


/**
 * @brief Appends a 3-component integer vector to the GPUCommand.
 * @details This appends a 3-component integer vector to the command.
 *   It basically just calls push_int() for every component, in the order
 *   x, y, z, which causes the vector to occupy the space of 3 floats.
 *
 * @param v Int-Vector to append.
 */
inline void GPUCommand::push_vec3(const LVecBase3i &v) {
  push_int(v.get_x());
  push_int(v.get_y());
  push_int(v.get_z());
}

/**
 * @brief Appends a 4-component floating point vector to the GPUCommand.
 * @details This appends a 4-component floating point vector to the command.
 *   It basically just calls push_float() for every component, in the order
 *   x, y, z, which causes the vector to occupy the space of 3 floats.
 *
 * @param v Int-Vector to append.
 */
inline void GPUCommand::push_vec4(const LVecBase4 &v) {
  push_float(v.get_x());
  push_float(v.get_y());
  push_float(v.get_z());
  push_float(v.get_w());
}

/**
 * @brief Appends a 4-component integer vector to the GPUCommand.
 * @details This appends a 4-component integer vector to the command.
 *   It basically just calls push_int() for every component, in the order
 *   x, y, z, w, which causes the vector to occupy the space of 4 floats.
 *
 * @param v Int-Vector to append.
 */
inline void GPUCommand::push_vec4(const LVecBase4i &v) {
  push_int(v.get_x());
  push_int(v.get_y());
  push_int(v.get_z());
  push_int(v.get_w());
}

/**
 * @brief Appends a floating point 3x3 matrix to the GPUCommand.
 * @details This appends a floating point 3x3 matrix to the GPUCommand, by
 *   pushing all components in row-order to the command. This occupies a space of
 *   9 floats.
 *
 * @param v Matrix to append
 */
inline void GPUCommand::push_mat3(const LMatrix3 &v) {
  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 3; ++j) {
      push_float(v.get_cell(i, j));
    }
  }
}

/**
 * @brief Appends a floating point 4x4 matrix to the GPUCommand.
 * @details This appends a floating point 4x4 matrix to the GPUCommand, by
 *   pushing all components in row-order to the command. This occupies a space of
 *   16 floats.
 *
 * @param v Matrix to append
 */
inline void GPUCommand::push_mat4(const LMatrix4 &v) {
  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      push_float(v.get_cell(i, j));
    }
  }
}

/**
 * @brief Returns whether integers are packed as floats.
 * @details This returns how integer are packed into the data stream. If the
 *   returned value is true, then integers are packed using their binary
 *   representation converted to floating point format. If the returned value
 *   is false, then integers are packed by simply casting them to float,
 *   e.g. val = (float)i;
 * @return The integer representation flag
 */
inline bool GPUCommand::get_uses_integer_packing() {
  return PACK_INT_AS_FLOAT;
}
