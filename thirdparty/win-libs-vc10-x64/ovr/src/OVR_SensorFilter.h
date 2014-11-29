/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_SensorFilter.h
Content     :   Basic filtering of sensor data
Created     :   March 7, 2013
Authors     :   Steve LaValle, Anna Yershova

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_SensorFilter_h
#define OVR_SensorFilter_h

#include "Kernel/OVR_Math.h"


namespace OVR {

// This class maintains a sliding window of sensor data taken over time and implements
// various simple filters, most of which are linear functions of the data history.
class SensorFilter
{
    enum
    {
        MaxFilterSize     = 100,
        DefaultFilterSize = 20
    };

private:
    int         LastIdx;                    // The index of the last element that was added to the array
    int         Size;                       // The window size (number of elements)
    Vector3f    Elements[MaxFilterSize]; 

public:
    // Create a new filter with default size
    SensorFilter() 
    {
        LastIdx = -1;
        Size = DefaultFilterSize;
    };

    // Create a new filter with size i
    SensorFilter(int i) 
    {
        OVR_ASSERT(i <= MaxFilterSize);
        LastIdx = -1;
        Size = i;
    };


    // Create a new element to the filter
    void AddElement (const Vector3f &e) 
    {
        if (LastIdx == Size - 1) 
            LastIdx = 0;
        else                            
            LastIdx++;

        Elements[LastIdx] = e;
    };

    // Get element i.  0 is the most recent, 1 is one step ago, 2 is two steps ago, ...
    Vector3f GetPrev(int i) const
    {
		OVR_ASSERT(i >= 0); // 
        int idx = (LastIdx - i);
        if (idx < 0) // Fix the wraparound case
            idx += Size;
		OVR_ASSERT(idx >= 0); // Multiple wraparounds not allowed
        return Elements[idx];
    };

    // Simple statistics
    Vector3f Total() const;
    Vector3f Mean() const;
    Vector3f Median() const;
    Vector3f Variance() const; // The diagonal of covariance matrix
    Matrix4f Covariance() const;
    Vector3f PearsonCoefficient() const;

    // A popular family of smoothing filters and smoothed derivatives
    Vector3f SavitzkyGolaySmooth8() const;
    Vector3f SavitzkyGolayDerivative4() const;
    Vector3f SavitzkyGolayDerivative5() const;
    Vector3f SavitzkyGolayDerivative12() const; 
    Vector3f SavitzkyGolayDerivativeN(int n) const;

    ~SensorFilter() {};
};

} //namespace OVR

#endif // OVR_SensorFilter_h
