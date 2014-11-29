/************************************************************************************

PublicHeader:   OVR.h
Filename    :   Util_MagCalibration.h
Content     :   Procedures for calibrating the magnetometer
Created     :   April 16, 2013
Authors     :   Steve LaValle, Andrew Reisse

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_Util_MagCalibration_h
#define OVR_Util_MagCalibration_h

#include "../OVR_SensorFusion.h"
#include "../Kernel/OVR_String.h"
#include "../Kernel/OVR_Log.h"

namespace OVR { namespace Util {

class MagCalibration
{
public:
    enum MagStatus
    {
        Mag_Uninitialized = 0,
        Mag_AutoCalibrating = 1,
        Mag_ManuallyCalibrating = 2,
        Mag_Calibrated  = 3
    };

    MagCalibration() :
        Stat(Mag_Uninitialized),
        MinMagDistance(0.2f), MinQuatDistance(0.5f),
        SampleCount(0)
    {
        MinMagDistanceSq = MinMagDistance * MinMagDistance;
        MinQuatDistanceSq = MinQuatDistance * MinQuatDistance;
        MinMagValues = Vector3f(10000.0f,10000.0f,10000.0f);
        MaxMagValues = Vector3f(-10000.0f,-10000.0f,-10000.0f);
		MinQuatValues = Quatf(1.0f,1.0f,1.0f,1.0f);
		MaxQuatValues = Quatf(0.0f,0.0f,0.0f,0.0f);
		}

    // Methods that are useful for either auto or manual calibration
    bool     IsUnitialized() const       { return Stat == Mag_Uninitialized; }
    bool     IsCalibrated() const        { return Stat == Mag_Calibrated; }
    int      NumberOfSamples() const     { return SampleCount; }
    int      RequiredSampleCount() const { return 4; }
	void     AbortCalibration()
	{
        Stat = Mag_Uninitialized;
        SampleCount = 0;
	}

    void     ClearCalibration(SensorFusion& sf) 
    {
        Stat = Mag_Uninitialized;
        SampleCount = 0;
        sf.ClearMagCalibration();
	};
  
    // Methods for automatic magnetometer calibration
    void     BeginAutoCalibration(SensorFusion& sf);
    unsigned UpdateAutoCalibration(SensorFusion& sf);
    bool     IsAutoCalibrating() const { return Stat == Mag_AutoCalibrating; }

    // Methods for building a manual (user-guided) calibraton procedure
    void     BeginManualCalibration(SensorFusion& sf);
    bool     IsAcceptableSample(const Quatf& q, const Vector3f& m);
    bool     InsertIfAcceptable(const Quatf& q, const Vector3f& m);
    // Returns true if successful, requiring that SampleCount = 4
    bool     SetCalibration(SensorFusion& sf);
    bool     IsManuallyCalibrating() const { return Stat == Mag_ManuallyCalibrating; }

    // This is the minimum acceptable distance (Euclidean) between raw
    // magnetometer values to be acceptable for usage in calibration.
    void SetMinMagDistance(float dist) 
    { 
        MinMagDistance = dist; 
        MinMagDistanceSq = MinMagDistance * MinMagDistance;
    }

    // The minimum acceptable distance (4D Euclidean) between orientations
    // to be acceptable for calibration usage.
    void SetMinQuatDistance(float dist) 
    { 
        MinQuatDistance = dist; 
        MinQuatDistanceSq = MinQuatDistance * MinQuatDistance;
    }

    // A result of the calibration, which is the center of a sphere that 
    // roughly approximates the magnetometer data.
    Vector3f GetMagCenter() const { return MagCenter; }
    // Retrieves the full magnetometer calibration matrix
    Matrix4f GetMagCalibration() const;
    // Retrieves the range of each quaternion term during calibration
    Quatf GetCalibrationQuatSpread() const { return QuatSpread; }
    // Retrieves the range of each magnetometer term during calibration
    Vector3f GetCalibrationMagSpread() const { return MagSpread; }

private:
    // Determine the unique sphere through 4 non-coplanar points
    Vector3f CalculateSphereCenter(const Vector3f& p1, const Vector3f& p2,
                                   const Vector3f& p3, const Vector3f& p4);

    // Distance from p4 to the nearest point on a plane through p1, p2, p3
    float PointToPlaneDistance(const Vector3f& p1, const Vector3f& p2,
                               const Vector3f& p3, const Vector3f& p4);

    Vector3f MagCenter;
    unsigned Stat;
    float    MinMagDistance;
    float    MinQuatDistance;
    float    MinMagDistanceSq;
    float    MinQuatDistanceSq;
	// For gathering statistics during calibration
	Vector3f    MinMagValues;
	Vector3f    MaxMagValues;
	Vector3f    MagSpread;
	Quatf		MinQuatValues;
	Quatf		MaxQuatValues;
	Quatf       QuatSpread;

    unsigned SampleCount;
    Vector3f MagSamples[4];
    Quatf    QuatSamples[4];

};

}}

#endif
