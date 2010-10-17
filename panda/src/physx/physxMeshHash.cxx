// Filename: physxMeshHash.cxx
// Created by: enn0x (13Sep10)
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

#include "physxMeshHash.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::quick_sort
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
quick_sort(pvector<int> &itemIndices, int l, int r) {

  int i, j, mi;
  int k, m;

  i = l; j = r; mi = (l + r)/2;
  m = itemIndices[mi];

  while (i <= j) {
    while(itemIndices[i] < m) i++;
    while(m < itemIndices[j]) j--;

    if (i <= j) {
      k = itemIndices[i]; itemIndices[i] = itemIndices[j]; itemIndices[j] = k;
      i++; j--;
    }
  }

  if (l < j) quick_sort(itemIndices, l, j);
  if (i < r) quick_sort(itemIndices, i, r);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::compress_indices
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
compress_indices(pvector<int> &itemIndices) {

  if (itemIndices.size() == 0) return;

  // Sort results
  quick_sort(itemIndices, 0, itemIndices.size() - 1);

  // Mark duplicates
  int i = 0;
  while (i < (int)itemIndices.size()) {
    int j = i+1;
    while (j < (int)itemIndices.size() && itemIndices[i] == itemIndices[j]) {
      itemIndices[j] = -1; j++;
    }
    i = j;
  }

  // Remove duplicates
  i = 0;
  while (i < (int)itemIndices.size()) {
    if (itemIndices[i] < 0) {
      itemIndices[i] = itemIndices[itemIndices.size()-1];
      itemIndices.pop_back();
    }
    else i++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::set_grid_spacing
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
set_grid_spacing(float spacing) {

  _spacing = spacing;
  _invSpacing = 1.0f / spacing;

  reset();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::reset
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
reset() {

  _time++;
  _entries.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::add
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
add(const NxBounds3 &bounds, int itemIndex) {

  int x1,y1,z1;
  int x2,y2,z2;
  int x,y,z;

  cell_coord_of(bounds.min, x1, y1, z1);
  cell_coord_of(bounds.max, x2, y2, z2);

  MeshHashEntry entry;
  entry.itemIndex = itemIndex;

  for (x = x1; x <= x2; x++) {
    for (y = y1; y <= y2; y++) {
      for (z = z1; z <= z2; z++) {

        int h = hash_function(x, y, z);
        MeshHashRoot &r = _hashIndex[h];
        int n = _entries.size();

        if (r.timeStamp != _time || r.first < 0)
          entry.next = -1;
        else 
          entry.next = r.first;

        r.first = n;
        r.timeStamp = _time;

        _entries.push_back(entry);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::add
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
add(const NxVec3 &pos, int itemIndex) {

  int x, y, z;

  cell_coord_of(pos, x, y, z);

  MeshHashEntry entry;
  entry.itemIndex = itemIndex;

  int h = hash_function(x, y, z);
  MeshHashRoot &r = _hashIndex[h];
  int n = _entries.size();

  if (r.timeStamp != _time || r.first < 0) 
    entry.next = -1;
  else 
    entry.next = r.first;

  r.first = n;
  r.timeStamp = _time;

  _entries.push_back(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::query
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
query(const NxBounds3 &bounds, pvector<int> &itemIndices, int maxIndices) {

  int x1, y1, z1;
  int x2, y2, z2;
  int x, y, z;

  cell_coord_of(bounds.min, x1, y1, z1);
  cell_coord_of(bounds.max, x2, y2, z2);

  itemIndices.clear();

  for (x=x1; x<=x2; x++) {
    for (y=y1; y<=y2; y++) {
      for (z=z1; z<=z2; z++) {

        int h = hash_function(x, y, z);

        MeshHashRoot &r = _hashIndex[h];
        if (r.timeStamp != _time) continue;
        int i = r.first;

        while (i >= 0) {
          MeshHashEntry &entry = _entries[i];
          itemIndices.push_back(entry.itemIndex);
          if (maxIndices >= 0 && (int)itemIndices.size() >= maxIndices) return;
          i = entry.next;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::query_unique
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
query_unique(const NxBounds3 &bounds, pvector<int> &itemIndices, int maxIndices) {

  query(bounds, itemIndices, maxIndices);
  compress_indices(itemIndices);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::query
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
query(const NxVec3 &pos, pvector<int> &itemIndices, int maxIndices) {

  int x, y, z;

  cell_coord_of(pos, x, y, z);

  itemIndices.clear();

  int h = hash_function(x, y, z);
  MeshHashRoot &r = _hashIndex[h];
  if (r.timeStamp != _time) return;
  int i = r.first;

  while (i >= 0) {
    MeshHashEntry &entry = _entries[i];
    itemIndices.push_back(entry.itemIndex);
    if (maxIndices >= 0 && (int)itemIndices.size() >= maxIndices) return;
    i = entry.next;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMeshHash::query_unique
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMeshHash::
query_unique(const NxVec3 &pos, pvector<int> &itemIndices, int maxIndices) {

  query(pos, itemIndices, maxIndices);
  compress_indices(itemIndices);
}

