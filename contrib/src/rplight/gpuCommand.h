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

#ifndef GPUCOMMAND_H
#define GPUCOMMAND_H

#include "pandabase.h"
#include "luse.h"

NotifyCategoryDecl(gpucommand, EXPORT_CLASS, EXPORT_TEMPL);

#define GPU_COMMAND_ENTRIES 32

// Packs integers by storing their binary representation in floats
// This only works if the command and light buffer is 32bit floating point.
#define PACK_INT_AS_FLOAT 0

/**
 * @brief Class for storing data to be transferred to the GPU.
 * @details This class can be seen like a packet, to be transferred to the GPU.
 *   It has a command type, which tells the GPU what to do once it recieved this
 *   "packet". It stores a limited amount of floating point components.
 */
class GPUCommand {
PUBLISHED:
  /**
   * The different types of GPUCommands. Each type has a special case in
   * the command queue processor. When adding new types, those need to
   * be handled in the command target, too.
   */
  enum CommandType {
    CMD_invalid = 0,
    CMD_store_light = 1,
    CMD_remove_light = 2,
    CMD_store_source = 3,
    CMD_remove_sources = 4,
  };

  GPUCommand(CommandType command_type);

  inline void push_int(int v);
  inline void push_float(float v);
  inline void push_vec3(const LVecBase3 &v);
  inline void push_vec3(const LVecBase3i &v);
  inline void push_vec4(const LVecBase4 &v);
  inline void push_vec4(const LVecBase4i &v);
  inline void push_mat3(const LMatrix3 &v);
  inline void push_mat4(const LMatrix4 &v);

  inline static bool get_uses_integer_packing();

  void write_to(const PTA_uchar &dest, size_t command_index);
  void write(std::ostream &out) const;

private:

  inline float convert_int_to_float(int v) const;

  CommandType _command_type;
  size_t _current_index;
  float _data[GPU_COMMAND_ENTRIES];
};

#include "gpuCommand.I"

#endif // GPUCOMMAND_H
