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

#ifndef POINTERSLOTSTORAGE_H
#define POINTERSLOTSTORAGE_H


#ifdef CPPPARSER

// Dummy implementation for interrogate
template <typename T, int SIZE>
class PointerSlotStorage {};

#else // CPPPARSER


#include "pandabase.h"

// Apple has an outdated libstdc++, so pull the class from TR1.
#if defined(__GLIBCXX__) && __GLIBCXX__ <= 20070719
#include <tr1/array>
using std::tr1::array;
#else
#include <array>
using std::array;
#endif

/**
 * @brief Class to keep a list of pointers and nullpointers.
 * @details This class stores a fixed size list of pointers, whereas pointers
 *   may be a nullptr as well. It provides functionality to find free slots,
 *   and also to find free consecutive slots, as well as taking care of reserving slots.
 *
 * @tparam T* Pointer-Type
 * @tparam SIZE Size of the storage
 */
template <typename T, int SIZE>
class PointerSlotStorage {
public:
  /**
   * @brief Constructs a new PointerSlotStorage
   * @details This constructs a new PointerSlotStorage, with all slots
   *   initialized to a nullptr.
   */
  PointerSlotStorage() {
#if defined(__GLIBCXX__) && __GLIBCXX__ <= 20070719
    _data.assign(nullptr);
#else
    _data.fill(nullptr);
#endif
    _max_index = 0;
    _num_entries = 0;
  }

  /**
   * @brief Returns the maximum index of the container
   * @details This returns the greatest index of any element which is not zero.
   *   This can be useful for iterating the container, since all elements
   *   coming after the returned index are guaranteed to be a nullptr.
   *
   *   If no elements are in this container, -1 is returned.
   * @return Maximum index of the container
   */
  int get_max_index() const {
    return _max_index;
  }

  /**
   * @brief Returns the amount of elements of the container
   * @details This returns the amount of elements in the container which are
   *   no nullptr.
   * @return Amount of elements
   */
  size_t get_num_entries() const {
    return _num_entries;
  }

  /**
   * @brief Finds a free slot
   * @details This finds the first slot which is a nullptr and returns it.
   *   This is most likely useful in combination with reserve_slot.
   *
   *   When no slot found was found, slot will be undefined, and false will
   *   be returned.
   *
   * @param slot Output-Variable, slot will be stored there
   * @return true if a slot was found, otherwise false
   */
  bool find_slot(size_t &slot) const {
    for (size_t i = 0; i < SIZE; ++i) {
      if (_data[i] == nullptr) {
        slot = i;
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Finds free consecutive slots
   * @details This behaves like find_slot, but it tries to find a slot
   *   after which <num_consecutive-1> free slots follow as well.
   *
   *   When no slot found was found, slot will be undefined, and false will
   *   be returned.
   *
   * @param slot Output-Variable, index of the first slot of the consecutive
   *   slots will be stored there.
   * @param num_consecutive Amount of consecutive slots to find, including the
   *   first slot.
   *
   * @return true if consecutive slots were found, otherwise false.
   */
  bool find_consecutive_slots(size_t &slot, size_t num_consecutive) const {
    nassertr(num_consecutive > 0, false);

    // Fall back to default search algorithm in case the parameters are equal
    if (num_consecutive == 1) {
      return find_slot(slot);
    }

    // Try to find consecutive slots otherwise
    for (size_t i = 0; i < SIZE; ++i) {
      bool any_taken = false;
      for (size_t k = 0; !any_taken && k < num_consecutive; ++k) {
        any_taken = _data[i + k] != nullptr;
      }
      if (!any_taken) {
        slot = i;
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Frees an allocated slot
   * @details This frees an allocated slot. If the slot was already freed
   *   before, this method throws an assertion.
   *
   * @param slot Slot to free
   */
  void free_slot(size_t slot) {
    nassertv(slot >= 0 && slot < SIZE);
    nassertv(_data[slot] != nullptr); // Slot was already empty!
    _data[slot] = nullptr;
    _num_entries--;

    // Update maximum index
    if ((int)slot == _max_index) {
      while (_max_index >= 0 && !_data[_max_index--]);
    }
  }

  /**
   * @brief Frees consecutive allocated slots
   * @details This behaves like PointerSlotStorage::free_slot, but deletes
   *   consecutive slots.
   *
   * @param slot Start of the consecutive slots to free
   * @param num_consecutive Number of consecutive slots
   */
  void free_consecutive_slots(size_t slot, size_t num_consecutive) {
    for (size_t i = slot; i < slot + num_consecutive; ++i) {
      free_slot(i);
    }
  }

  /**
   * @brief Reserves a slot
   * @details This reserves a slot by storing a pointer in it. If the slot
   *   was already taken, throws an assertion.
   *   If the ptr is a nullptr, also throws an assertion.
   *   If the slot was out of bounds, also throws an assertion.
   *
   * @param slot Slot to reserve
   * @param ptr Pointer to store
   */
  void reserve_slot(size_t slot, T ptr) {
    nassertv(slot >= 0 && slot < SIZE);
    nassertv(_data[slot] == nullptr); // Slot already taken!
    nassertv(ptr != nullptr); // nullptr passed as argument!
    _max_index = std::max(_max_index, (int)slot);
    _data[slot] = ptr;
    _num_entries++;
  }

  typedef array<T, SIZE> InternalContainer;

  /**
   * @brief Returns an iterator to the begin of the container
   * @details This returns an iterator to the beginning of the container
   * @return Begin-Iterator
   */
  typename InternalContainer::iterator begin() {
    return _data.begin();
  }

  /**
   * @brief Returns an iterator to the end of the container
   * @details This returns an iterator to the end of the iterator. This only
   *   iterates to PointerSlotStorage::get_max_index()
   * @return [description]
   */
  typename InternalContainer::iterator end() {
    return _data.begin() + _max_index + 1;
  }

private:
  int _max_index;
  size_t _num_entries;
  InternalContainer _data;
};

#endif // CPPPARSER

#endif // POINTERSLOTSTORAGE_H
