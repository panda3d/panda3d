// Filename: lerpchans.h
// Created by:  frang (11Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __LERPCHANS_H__
#define __LERPCHANS_H__

// There are three fundamental types of lerp channel: tabular/keyframe,
// procedural, and composite.  Tabular channels may have an interpolator
// associated with them, to determine values between samples.  Proceedural
// channels compute each data point as needed.  Composite channels can be
// either blending or concatenation objects.  Blending objects take some
// number of channels as input, apply a blending function to them in order
// to yield what appears to be a single channel.  Concatenation objects
// take some number of channels as input and string them end-to-end to yield
// what appears to be a single channel.

class LerpChannelRange {
private:
  float _low, _high;
public:
  INLINE LerpChannelRange(float low, float high) : _low(low), _high(high) {
    if (low > high) {
      _low = high;
      _high = low;
    }
  }
  INLINE LerpChannelRange(const LerpChannelRange& c) : _low(c._low),
						       _high(c._high) {}
  INLINE ~LerpChannelRange(void) {}
  INLINE float GetLow(void) { return _low; }
  INLINE float GetHigh(void) { return _high; }
  INLINE void SetLow(float l) {
    if (l > _high) {
      _low = _high;
      _high = l;
    } else
      _low = l;
  }
  INLINE void SetHigh(float h) {
    if (h < _low) {
      _high = _low;
      _low = h;
    } else
      _high = h;
  }
  INLINE void SetRange(float l, float h) {
    if (l > h) {
      _low = h;
      _high = l;
    } else {
      _low = l;
      _high = h;
    }
  }
  INLINE LerpChannelRange& operator=(const LerpChannelRange& c) {
    _low = c._low;
    _high = c._high;
    return *this;
  }
};

template <class value>
class LerpChannel {
public:
  virtual GetValue(float p);
};

template <class value>
class TabularChannel : public LerpChannel {
};

template <class value>
class ProceduralChannel : public LerpChannel {
};

template <class value>
class CompositeChannel : public LerpChannel {
};

template <class value>
class BlendChannel : public CompositeChannel {
};

template <class value>
class ConcatenationChannel : public CompositeChannel {
};

#endif /* __LERPCHANS_H__ */
