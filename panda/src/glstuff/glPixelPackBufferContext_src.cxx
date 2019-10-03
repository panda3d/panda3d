/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glPixelPackBufferContext_src.cxx
 * @author rdb
 * @date 2019-10-02
 */

#ifndef OPENGLES

TypeHandle CLP(PixelPackBufferContext)::_type_handle;

/**
 * Deletes the buffer.
 */
CLP(PixelPackBufferContext)::
~CLP(PixelPackBufferContext)() {
  if (_index != 0) {
    _glgsg->_glDeleteBuffers(1, &_index);
    _index = 0;
  }
}

/**
 * Returns true if the extraction has been performed.  This may return false
 * positives if sync objects are not supported.
 */
bool CLP(PixelPackBufferContext)::
is_transfer_done() const {
#ifndef OPENGLES
  if (_sync == 0) {
    return true;
  }
  GLint status = GL_UNSIGNALED;
  _glgsg->_glGetSynciv(_sync, GL_SYNC_STATUS, 1, nullptr, &status);
  return (status == GL_SIGNALED);
#else
  return true;
#endif
}

/**
 * Finishes the extraction process.
 */
void CLP(PixelPackBufferContext)::
finish_transfer() {
  nassertv(_index != 0);

  _glgsg->_glBindBuffer(GL_PIXEL_PACK_BUFFER, _index);

  void *data = _glgsg->_glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

  for (ExtractTexture &et : _textures) {
    et._texture->set_x_size(et._width);
    et._texture->set_y_size(et._height);
    et._texture->set_z_size(et._depth);
    et._texture->set_component_type(et._type);
    et._texture->set_format(et._format);

    unsigned char *begin = (unsigned char *)data + et._offset;
    unsigned char *end = (unsigned char *)begin + (et._page_size * et._depth);
    et._texture->set_ram_image(PTA_uchar(begin, end, Texture::get_class_type()),
                               et._compression, et._page_size);

    //PTA_uchar image = PTA_uchar::empty_array(et._page_size * et._depth);
    //_glgsg->_glGetBufferSubData(GL_PIXEL_PACK_BUFFER, et._offset, image.size(), image.p());
    //et._texture->set_ram_image(std::move(image), et._compression, et._page_size);
  }

  _glgsg->_glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

  notify_done();

  _glgsg->_glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

  // Delete the fence, which is no longer needed.
  if (_sync != 0) {
    _glgsg->_glDeleteSync(_sync);
    _sync = 0;
  }

  _textures.clear();
}

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
/*void CLP(PixelPackBufferContext)::
evict_lru() {
  if (!_textures.empty()) {
    finish_extraction();
  }

  dequeue_lru();

  // Free the buffer.
  _glgsg->_glDeleteBuffers(1, &_index);

  update_data_size_bytes(0);
  mark_unloaded();
}*/

#endif  // !OPENGLES
