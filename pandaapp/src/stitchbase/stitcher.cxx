// Filename: stitcher.cxx
// Created by:  drose (09Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "stitcher.h"
#include "stitchImage.h"
#include "stitchPoint.h"

#include "rotate_to.h"
#include "compose_matrix.h"
#include <algorithm>


Stitcher::MatchingPoint::
MatchingPoint(StitchPoint *p, const LPoint2d &got_uv) :
  _p(p),
  _got_uv(got_uv)
{
  _need_uv.set(0.0, 0.0);
  _diff = 0.0;
}

Stitcher::
Stitcher() {
  _show_points = false;
}

Stitcher::
~Stitcher() {
}

void Stitcher::
add_image(StitchImage *image) {
  image->_index = _images.size();
  _images.push_back(image);

  // Record all of the points in the image as well.
  StitchImage::Points::const_iterator pi;
  for (pi = image->_points.begin(); pi != image->_points.end(); ++pi) {
    string name = (*pi).first;

    Points::iterator ppi;
    ppi = _points.find(name);
    StitchPoint *sp;
    if (ppi != _points.end()) {
      // Previously used point.
      sp = (*ppi).second;
    } else {
      // New point.
      sp = new StitchPoint(name);
      _points.insert(Points::value_type(name, sp));
    }
    sp->_images.insert(image);
  }
}

void Stitcher::
add_point(const string &name, const LVector3d &vec) {
  Points::iterator ppi;
  ppi = _points.find(name);
  StitchPoint *sp;
  if (ppi != _points.end()) {
    // Previously used point.
    sp = (*ppi).second;
  } else {
    // New point.
    sp = new StitchPoint(name);
    _points.insert(Points::value_type(name, sp));
  }

  sp->set_space(normalize(vec));
  _loose_points.push_back(sp);
}


void Stitcher::
show_points(double radius, const Colord &color) {
  _show_points = true;
  _point_radius = radius;
  _point_color = color;
}


void Stitcher::
stitch() {
  if (_images.empty()) {
    return;
  }

  // First place the reference image.  All of its points are fixed
  // where they are.
  if (_loose_points.empty()) {
    StitchImage *image = _images.front();
    assert(image != NULL);

    StitchImage::Points::const_iterator pi;
    for (pi = image->_points.begin(); pi != image->_points.end(); ++pi) {
      string name = (*pi).first;
      LPoint2d uv = (*pi).second;

      Points::iterator ppi;
      ppi = _points.find(name);
      assert(ppi != _points.end());
      StitchPoint *sp = (*ppi).second;

      LVector3d space = normalize(image->extrude(uv));
      sp->set_space(space);
    }

    _placed.push_back(image);

    // Report the reference image.
    nout << "\n" << *image << "\n";
    _images.erase(_images.begin());
  }

  // Now place each of the other images relative to the already-known
  // points.
  double net_stitch_score = 0.0;
  bool done = false;

  while (!_images.empty() && !done) {
    // Find the image with the greatest number of known points.
    int max_score = 0;
    Images::iterator best_image = _images.end();

    Images::iterator ii;
    for (ii = _images.begin(); ii != _images.end(); ++ii) {
      int score = score_image(*ii);
      if (score > max_score) {
        max_score = score;
        best_image = ii;
      }
    }
    if (best_image == _images.end()) {
      // Bad news.  None of the images had a score greater than zero,
      // so we can't stitch them in--not enough shared points.
      done = true;

    } else {
      // Now stitch this image in and remove it from the set.
      net_stitch_score += stitch_image(*best_image);
      _placed.push_back(*best_image);
      _images.erase(best_image);
    }
  }

  // Any of the unplaced images with explicit hpr's get placed where
  // they are.
  Images::iterator ii = _images.begin();
  while (ii != _images.end()) {
    StitchImage *image = (*ii);
    if (image->_hpr_set) {
      net_stitch_score += stitch_image(image);
      _placed.push_back(image);
      _images.erase(ii);
    } else {
      ++ii;
    }
  }

  if (!_images.empty()) {
    nout << "Not enough shared points; " << _images.size()
         << " images remain unstitched.\n";
  }

  nout << "Net score is " << net_stitch_score << "\n";

  // Reorder all of the images by index number order.
  sort(_placed.begin(), _placed.end(), StitchImageByIndex());

  // And feather the edges between them nicely.  We don't need to
  // feather the first image.
  if (_placed.size() > 1) {
    nout << "Feathering edges\n";
    Images::iterator ii;
    ii = _placed.begin();
    for (++ii; ii != _placed.end(); ++ii) {
      feather_image(*ii);
    }
  }
}

int Stitcher::
score_image(StitchImage *image) {
  // Give the image one point for each StitchPoint it has that has a
  // known location in space.
  int score = 0;

  StitchImage::Points::const_iterator pi;
  for (pi = image->_points.begin(); pi != image->_points.end(); ++pi) {
    string name = (*pi).first;

    Points::iterator ppi;
    ppi = _points.find(name);
    assert(ppi != _points.end());
    StitchPoint *sp = (*ppi).second;

    if (sp->_space_known) {
      score++;
    }
  }

  // We must have at least two points in common to stitch an image.
  if (score < 2) {
    score = 0;
  }
  return score;
}


double Stitcher::
stitch_image(StitchImage *image) {
  // First, collect all the points we have that exist somewhere in
  // known space.
  MatchingPoints mp;

  StitchImage::Points::const_iterator pi;
  for (pi = image->_points.begin(); pi != image->_points.end(); ++pi) {
    string name = (*pi).first;
    LPoint2d uv = (*pi).second;

    Points::iterator ppi;
    ppi = _points.find(name);
    assert(ppi != _points.end());
    StitchPoint *sp = (*ppi).second;

    if (sp->_space_known) {
      mp.push_back(MatchingPoint(sp, uv));
    }
  }

  // We need at least two points in common, or one point and an
  // explicit hpr to stitch.
  if (mp.size() < 2 && !image->_hpr_set) {
    nout << "cannot stitch " << image->get_name() << "\n\n";
    return 0.0;
  }

  double best_score = 0.0;

  if (mp.empty()) {
    // If we have no points, we can at least place it where the hpr says to.
    nout << *image << "placed explicitly.\n\n";

  } else {
    // If we have at least one point, we can stitch something.

    // Reset the image's total transform, since we'll be changing it.
    image->clear_transform();

    // Find the best match.
    int best_i = -1;
    int best_j = -1;

    if (mp.size() < 2) {
      // If we don't have two points, there's nothing to choose.
      best_i = 0;
      best_j = 0;
    } else {
      for (int i = 0; i < (int)mp.size(); i++) {
        for (int j = 0; j < (int)mp.size(); j++) {
          if (j != i) {
            LMatrix3d rot;
            double score = try_match(image, rot, mp, i, j);
            if (score < best_score || best_i == -1) {
              best_i = i;
              best_j = j;
              best_score = score;
            }
          }
        }
      }
    }

    // Now go back and actually use the best match.
    LMatrix3d rot;
    try_match(image, rot, mp, best_i, best_j);

    image->set_transform(rot);

    if (mp.size() < 2) {
      nout << *image << "placed semi-explicitly.\n\n";
    } else {
      nout << *image << "score is " << best_score << "\n\n";
    }

    // Now compute the degree of success.
    MatchingPoints::iterator mi;
    for (mi = mp.begin(); mi != mp.end(); ++mi) {
      (*mi)._need_uv = image->project((*mi)._p->_space);
      (*mi)._diff = (*mi)._need_uv - (*mi)._got_uv;
    }

    // Now morph the image out the last few pixels so that all the
    // points will match up exactly.

    int x_verts = image->get_x_verts();
    int y_verts = image->get_y_verts();
    image->_morph.init(x_verts, y_verts);
    int x, y;
    for (y = 0; y < y_verts; y++) {
      for (x = 0; x < x_verts; x++) {
        LPoint2d p = image->get_grid_uv(x, y);
        LVector2d offset(0.0, 0.0);
        double net = 0.0;

        MatchingPoints::const_iterator cmi;
        bool done = false;
        for (cmi = mp.begin(); cmi != mp.end() && !done; ++cmi) {
          LVector2d v = p - (*cmi)._got_uv;
          double d = pow(dot(v, v), 0.1);
          if (d < 0.0001) {
            // This one is dead on; stop here and never mind.
            offset = (*cmi)._diff;
            net = 1.0;
            done = true;
          } else {
            double scale = 1.0 / d;
            offset += (*cmi)._diff * scale;
            net += scale;
          }
        }

        offset /= net;
        image->_morph._table[y][x]._p[MorphGrid::TT_out] += offset;
      }
    }
    image->_morph.recompute();


  /*

  for (mi = mp.begin(); mi != mp.end(); ++mi) {
    LVector3d va = normalize(LVector3d(image->extrude((*mi)._got_uv)));
    LVector3d vb = (*mi)._p->_space;
    nout << 1000.0 * (1.0 - dot(va, vb)) << " for " << (*mi)._p->_name
         << "\n   at " << va << " vs. " << vb << "\n";
  }
  nout << "\n";

  // Report the final results, including the morphs, to the user.
  for (mi = mp.begin(); mi != mp.end(); ++mi) {
    (*mi)._need_uv = image->project((*mi)._p->_space);
    (*mi)._diff = (*mi)._need_uv - (*mi)._got_uv;

    nout << (*mi)._p->_name
         << " "<< (*mi)._need_uv << " vs. " << (*mi)._got_uv
         << " diff is " << length((*mi)._diff * image->_uv_to_pixels)
         << " pixels\n";
  }
  */

    // Finally, mark all of the other points in this image as now known
    // points in space.
    for (pi = image->_points.begin(); pi != image->_points.end(); ++pi) {
      string name = (*pi).first;
      LPoint2d uv = (*pi).second;

      Points::iterator ppi;
      ppi = _points.find(name);
      assert(ppi != _points.end());
      StitchPoint *sp = (*ppi).second;

      if (!sp->_space_known) {
        LVector3d space = normalize(image->extrude(uv));
        sp->set_space(space);
      }
    }
  }

  return best_score;
}

void Stitcher::
feather_image(StitchImage *image) {
  // Feather the edges of the image wherever it overlaps with an image
  // we have laid down previously.  We do this by first determining
  // which morph points overlap with some other image.
  int x_verts = image->get_x_verts();
  int y_verts = image->get_y_verts();

  if (image->_morph.is_empty()) {
    image->_morph.init(x_verts, y_verts);
    image->_morph.recompute();
  }

  int x, y;
  for (y = 0; y < y_verts; y++) {
    for (x = 0; x < x_verts; x++) {
      LVector3d space = image->get_grid_vector(x, y);
      Images::const_iterator ii;
      for (ii = _placed.begin();
           ii != _placed.end() &&
             !image->_morph._table[y][x]._over_another;
           ++ii) {
        StitchImage *other = (*ii);
        if (other->_index < image->_index) {
          LPoint2d uv = other->project(space);
          if (uv[0] >= 0.0 && uv[0] <= 1.0 &&
              uv[1] >= 0.0 && uv[1] <= 1.0) {
            // This point is over the other image.
            image->_morph._table[y][x]._over_another = true;
          }
        }
      }
    }
  }

  image->_morph.fill_alpha();
}


double Stitcher::
try_match(StitchImage *image, LMatrix3d &rot,
          const Stitcher::MatchingPoints &mp, int zero, int one) {

  // Now rotate this image relative to the other so the first pair of
  // points exactly coincide.
  LVector3d v0a = normalize(image->extrude(mp[zero]._got_uv));
  LVector3d v0b = mp[zero]._p->_space;

  rotate_to(rot, v0a, v0b);

  if (zero == one) {
    // Here's a special case: only one matching point.  In this case,
    // we roll by the explicit angle given by the user.
    if (image->_hpr_set) {
      rot = rot * LMatrix3d::rotate_mat(image->_hpr[2], v0b);
    }

  } else {
    // Now (v0a * rot) == v0b.  Roll about this vector till the
    // second pair of points comes as close as possible to coinciding.
    LVector3d v1a = normalize(image->extrude(mp[one]._got_uv));
    LVector3d v1b = mp[one]._p->_space;

    v1a = v1a * rot;

    // We need to determine the appropriate angle to roll.  This is the
    // angle between the plane that contains v0 and v1a, and the plane
    // that contains v0 and v1b.

    LVector3d normal_a = normalize(cross(v0b, v1a));
    LVector3d normal_b = normalize(cross(v0b, v1b));

    double cos_theta = dot(normal_a, normal_b);
    double theta = rad_2_deg(acos(cos_theta));

    rot = rot * LMatrix3d::rotate_mat(-theta, v0b);
  }

  // Now compute the score.
  double score = 0.0;

  MatchingPoints::const_iterator mi;
  for (mi = mp.begin(); mi != mp.end(); ++mi) {
    LVector3d va = normalize(LVector3d(image->extrude((*mi)._got_uv) * rot));
    LVector3d vb = (*mi)._p->_space;
    score += 1.0 - dot(va, vb);
  }

  return 1000.0 * score;
}
