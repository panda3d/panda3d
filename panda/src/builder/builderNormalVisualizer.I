// Filename: builderNormalVisualizer.I
// Created by:  drose (08Sep99)
// 
////////////////////////////////////////////////////////////////////

INLINE BuilderNormalVisualizer::
BuilderNormalVisualizer(BuilderBucket &bucket) : _bucket(bucket) {
  _num_vertices = 0;
  _net_vertex.set(0.0, 0.0, 0.0);
}

