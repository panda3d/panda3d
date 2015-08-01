// Filename: glUniformBufferContext_src.cxx
// Created by:  rdb (29Jul15)
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

#ifndef OPENGLES

TypeHandle CLP(UniformBufferContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CLP(UniformBufferContext)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLP(UniformBufferContext)::
CLP(UniformBufferContext)(CLP(GraphicsStateGuardian) *glgsg,
                          CPT(GeomVertexArrayFormat) layout) :
  BufferContext(&glgsg->_ubuffer_residency),
  _glgsg(glgsg),
  _layout(MOVE(layout)),
  _index(0),
  _mat_deps(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GLUniformBufferContext::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLP(UniformBufferContext)::
~CLP(UniformBufferContext)() {
  // TODO: unregister uniform buffer from GSG.
  if (_index != 0) {
    _glgsg->_glDeleteBuffers(1, &_index);
    _index = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLUniformBufferContext::update
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(UniformBufferContext)::
update_data(const ShaderAttrib *attrib) {
  //if (ClockObject::get_global_clock()->get_frame_count() == _frame) {
  //  return;
  //}
  _frame = ClockObject::get_global_clock()->get_frame_count();

  // Change the generic buffer binding target for the map operation.
  if (_glgsg->_current_ubuffer_index != _index) {
    _glgsg->_glBindBuffer(GL_UNIFORM_BUFFER, _index);
    _glgsg->_current_ubuffer_index = _index;
  }

  void *buffer;
  GLsizeiptr size = _layout->get_pad_to();

  if (_glgsg->_glMapBufferRange != NULL) {
    // If we have glMapBufferRange, we can tell it we don't need the
    // previous contents for future draw calls any more, preventing
    // an unnecessary synchronization.
    buffer = _glgsg->_glMapBufferRange(GL_UNIFORM_BUFFER, 0, size,
      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

  } else {
    // This old trick achieves more or less the same effect.
    _glgsg->_glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STREAM_DRAW);
    buffer = _glgsg->_glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
  }

  nassertv(buffer != NULL);

  for (int i = 0; i < _layout->get_num_columns(); ++i) {
    const GeomVertexColumn *column = _layout->get_column(i);
    const InternalName *name = column->get_name();

    const ShaderInput *input = attrib->get_shader_input(name);
    if (input == NULL || input->get_value_type() == ShaderInput::M_invalid) {
      GLCAT.error()
        << "Missing shader input: " << *name << "\n";
      continue;
    }

    switch (column->get_numeric_type()) {
    case GeomEnums::NT_float32:
      {
        float *into = (float *)((char *)buffer + column->get_start());
        nassertd(input->extract_data(into, column->get_num_values(), column->get_num_elements())) continue;
      }
      break;

    case GeomEnums::NT_float64:
      {
        double *into = (double *)((char *)buffer + column->get_start());
        nassertd(input->extract_data(into, column->get_num_values(), column->get_num_elements())) continue;
      }
      break;

    //TODO: support unsigned int
    case GeomEnums::NT_int32:
    case GeomEnums::NT_uint32:
      {
        int *into = (int *)((char *)buffer + column->get_start());
        nassertd(input->extract_data(into, column->get_num_values(), column->get_num_elements())) continue;
      }
      break;

    default:
      continue;
    }
  }

  _glgsg->_glUnmapBuffer(GL_UNIFORM_BUFFER);
}

#endif
