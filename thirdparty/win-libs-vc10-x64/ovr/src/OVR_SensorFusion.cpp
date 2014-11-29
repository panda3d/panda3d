/************************************************************************************

Filename    :   OVR_SensorFusion.cpp
Content     :   Methods that determine head orientation from sensor data over time
Created     :   October 9, 2012
Authors     :   Michael Antonov, Steve LaValle

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_SensorFusion.h"
#include "Kernel/OVR_Log.h"
#include "Kernel/OVR_System.h"
#include "OVR_JSON.h"
#include "OVR_Profile.h"

namespace OVR {

//-------------------------------------------------------------------------------------
// ***** Sensor Fusion

SensorFusion::SensorFusion(SensorDevice* sensor)
  : Handler(getThis()), pDelegate(0),
    Gain(0.05f), YawMult(1), EnableGravity(true), Stage(0), RunningTime(0), DeltaT(0.001f),
	EnablePrediction(true), PredictionDT(0.03f), PredictionTimeIncrement(0.001f),
    FRawMag(10), FAccW(20), FAngV(20),
    TiltCondCount(0), TiltErrorAngle(0), 
    TiltErrorAxis(0,1,0),
    MagCondCount(0), MagCalibrated(false), MagRefQ(0, 0, 0, 1), 
	MagRefM(0), MagRefYaw(0), YawErrorAngle(0), MagRefDistance(0.5f),
    YawErrorCount(0), YawCorrectionActivated(false), YawCorrectionInProgress(false), 
	EnableYawCorrection(false), MagNumReferences(0), MagHasNearbyReference(false),
    MotionTrackingEnabled(true)
{
   if (sensor)
       AttachToSensor(sensor);
   MagCalibrationMatrix.SetIdentity();
}

SensorFusion::~SensorFusion()
{
}


bool SensorFusion::AttachToSensor(SensorDevice* sensor)
{
    // clear the cached device information
    CachedSensorInfo.SerialNumber[0] = 0;   
    CachedSensorInfo.VendorId = 0;
    CachedSensorInfo.ProductId = 0;

    if (sensor != NULL)
    {
        // Cache the sensor device so we can access this information during
        // mag saving and loading (avoid holding a reference to sensor to prevent 
        // deadlock on shutdown)
        sensor->GetDeviceInfo(&CachedSensorInfo);   // save the device information
        MessageHandler* pCurrentHandler = sensor->GetMessageHandler();

        if (pCurrentHandler == &Handler)
        {
            Reset();
            return true;
        }

        if (pCurrentHandler != NULL)
        {
            OVR_DEBUG_LOG(
                ("SensorFusion::AttachToSensor failed - sensor %p already has handler", sensor));
            return false;
        }

        // Automatically load the default mag calibration for this sensor
        LoadMagCalibration();        
    }

    if (Handler.IsHandlerInstalled())
    {
        Handler.RemoveHandlerFromDevices();
    }

    if (sensor != NULL)
    {
        sensor->SetMessageHandler(&Handler);
    }

    Reset();
    return true;
}


    // Resets the current orientation
void SensorFusion::Reset()
{
    Lock::Locker lockScope(Handler.GetHandlerLock());
    Q                     = Quatf();
    QUncorrected          = Quatf();
    Stage                 = 0;
	RunningTime           = 0;
	MagNumReferences      = 0;
	MagHasNearbyReference = false;
}


void SensorFusion::handleMessage(const MessageBodyFrame& msg)
{
    if (msg.Type != Message_BodyFrame || !IsMotionTrackingEnabled())
        return;
  
    // Put the sensor readings into convenient local variables
    Vector3f angVel    = msg.RotationRate; 
    Vector3f rawAccel  = msg.Acceleration;
    Vector3f mag       = msg.MagneticField;

    // Set variables accessible through the class API
	DeltaT = msg.TimeDelta;
    AngV = msg.RotationRate;
    AngV.y *= YawMult;  // Warning: If YawMult != 1, then AngV is not true angular velocity
    A = rawAccel;

    // Allow external access to uncalibrated magnetometer values
    RawMag = mag;  

    // Apply the calibration parameters to raw mag
    if (HasMagCalibration())
    {
        mag.x += MagCalibrationMatrix.M[0][3];
        mag.y += MagCalibrationMatrix.M[1][3];
        mag.z += MagCalibrationMatrix.M[2][3];
    }

    // Provide external access to calibrated mag values
    // (if the mag is not calibrated, then the raw value is returned)
    CalMag = mag;

    float angVelLength = angVel.Length();
    float accLength    = rawAccel.Length();


    // Acceleration in the world frame (Q is current HMD orientation)
    Vector3f accWorld  = Q.Rotate(rawAccel);

    // Keep track of time
    Stage++;
    RunningTime += DeltaT;

    // Insert current sensor data into filter history
    FRawMag.AddElement(RawMag);
    FAccW.AddElement(accWorld);
    FAngV.AddElement(angVel);

    // Update orientation Q based on gyro outputs.  This technique is
    // based on direct properties of the angular velocity vector:
    // Its direction is the current rotation axis, and its magnitude
    // is the rotation rate (rad/sec) about that axis.  Our sensor
    // sampling rate is so fast that we need not worry about integral
    // approximation error (not yet, anyway).
    if (angVelLength > 0.0f)
    {
        Vector3f     rotAxis      = angVel / angVelLength;  
        float        halfRotAngle = angVelLength * DeltaT * 0.5f;
        float        sinHRA       = sin(halfRotAngle);
        Quatf        deltaQ(rotAxis.x*sinHRA, rotAxis.y*sinHRA, rotAxis.z*sinHRA, cos(halfRotAngle));

        Q =  Q * deltaQ;
    }
    
    // The quaternion magnitude may slowly drift due to numerical error,
    // so it is periodically normalized.
    if (Stage % 5000 == 0)
        Q.Normalize();
    
	// Maintain the uncorrected orientation for later use by predictive filtering
	QUncorrected = Q;

    // Perform tilt correction using the accelerometer data. This enables 
    // drift errors in pitch and roll to be corrected. Note that yaw cannot be corrected
    // because the rotation axis is parallel to the gravity vector.
    if (EnableGravity)
    {
        // Correcting for tilt error by using accelerometer data
        const float  gravityEpsilon = 0.4f;
        const float  angVelEpsilon  = 0.1f; // Relatively slow rotation
        const int    tiltPeriod     = 50;   // Required time steps of stability
        const float  maxTiltError   = 0.05f;
        const float  minTiltError   = 0.01f;

        // This condition estimates whether the only measured acceleration is due to gravity 
        // (the Rift is not linearly accelerating).  It is often wrong, but tends to average
        // out well over time.
        if ((fabs(accLength - 9.81f) < gravityEpsilon) &&
            (angVelLength < angVelEpsilon))
            TiltCondCount++;
        else
            TiltCondCount = 0;
    
        // After stable measurements have been taken over a sufficiently long period,
        // estimate the amount of tilt error and calculate the tilt axis for later correction.
        if (TiltCondCount >= tiltPeriod)
        {   // Update TiltErrorEstimate
            TiltCondCount = 0;
            // Use an average value to reduce noise (could alternatively use an LPF)
            Vector3f accWMean = FAccW.Mean();
            // Project the acceleration vector into the XZ plane
            Vector3f xzAcc = Vector3f(accWMean.x, 0.0f, accWMean.z);
            // The unit normal of xzAcc will be the rotation axis for tilt correction
            Vector3f tiltAxis = Vector3f(xzAcc.z, 0.0f, -xzAcc.x).Normalized();
            Vector3f yUp = Vector3f(0.0f, 1.0f, 0.0f);
            // This is the amount of rotation
            float    tiltAngle = yUp.Angle(accWMean);
            // Record values if the tilt error is intolerable
            if (tiltAngle > maxTiltError) 
            {
                TiltErrorAngle = tiltAngle;
                TiltErrorAxis = tiltAxis;
            }
        }

        // This part performs the actual tilt correction as needed
        if (TiltErrorAngle > minTiltError) 
        {
            if ((TiltErrorAngle > 0.4f)&&(RunningTime < 8.0f))
            {   // Tilt completely to correct orientation
                Q = Quatf(TiltErrorAxis, -TiltErrorAngle) * Q;
                TiltErrorAngle = 0.0f;
            }
            else 
            {
                //LogText("Performing tilt correction  -  Angle: %f   Axis: %f %f %f\n",
                //        TiltErrorAngle,TiltErrorAxis.x,TiltErrorAxis.y,TiltErrorAxis.z);
                //float deltaTiltAngle = -Gain*TiltErrorAngle*0.005f;
                // This uses aggressive correction steps while your head is moving fast
                float deltaTiltAngle = -Gain*TiltErrorAngle*0.005f*(5.0f*angVelLength+1.0f);
                // Incrementally "un-tilt" by a small step size
                Q = Quatf(TiltErrorAxis, deltaTiltAngle) * Q;
                TiltErrorAngle += deltaTiltAngle;
            }
        }
    }

    // Yaw drift correction based on magnetometer data.  This corrects the part of the drift
    // that the accelerometer cannot handle.
    // This will only work if the magnetometer has been enabled, calibrated, and a reference
    // point has been set.
    const float maxAngVelLength = 3.0f;
    const int   magWindow = 5;
    const float yawErrorMax = 0.1f;
    const float yawErrorMin = 0.01f;
    const int   yawErrorCountLimit = 50;
    const float yawRotationStep = 0.00002f;

    if (angVelLength < maxAngVelLength)
        MagCondCount++;
    else
        MagCondCount = 0;

	// Find, create, and utilize reference points for the magnetometer
	// Need to be careful not to set reference points while there is significant tilt error
    if ((EnableYawCorrection && MagCalibrated)&&(RunningTime > 10.0f)&&(TiltErrorAngle < 0.2f))
	{
	  if (MagNumReferences == 0)
      {
		  setMagReference(); // Use the current direction
      }
	  else if (Q.Distance(MagRefQ) > MagRefDistance) 
      {
		  MagHasNearbyReference = false;
          float bestDist = 100000.0f;
          int bestNdx = 0;
          float dist;
          for (int i = 0; i < MagNumReferences; i++)
          {
              dist = Q.Distance(MagRefTableQ[i]);
              if (dist < bestDist)
              {
                  bestNdx = i;
                  bestDist = dist;
              }
          }

          if (bestDist < MagRefDistance)
          {
              MagHasNearbyReference = true;
              MagRefQ   = MagRefTableQ[bestNdx];
              MagRefM   = MagRefTableM[bestNdx];
              MagRefYaw = MagRefTableYaw[bestNdx];
              //LogText("Using reference %d\n",bestNdx);
          }
          else if (MagNumReferences < MagMaxReferences)
              setMagReference();
	  }
	}

	YawCorrectionInProgress = false;
    if (EnableYawCorrection && MagCalibrated && (RunningTime > 2.0f) && (MagCondCount >= magWindow) &&
        MagHasNearbyReference)
    {
        // Use rotational invariance to bring reference mag value into global frame
        Vector3f grefmag = MagRefQ.Rotate(GetCalibratedMagValue(MagRefM));
        // Bring current (averaged) mag reading into global frame
        Vector3f gmag = Q.Rotate(GetCalibratedMagValue(FRawMag.Mean()));
        // Calculate the reference yaw in the global frame
        Anglef gryaw = Anglef(atan2(grefmag.x,grefmag.z));
        // Calculate the current yaw in the global frame
        Anglef gyaw = Anglef(atan2(gmag.x,gmag.z));
        // The difference between reference and current yaws is the perceived error
        Anglef YawErrorAngle = gyaw - gryaw;

        //LogText("Yaw error estimate: %f\n",YawErrorAngle.Get());
        // If the perceived error is large, keep count
        if ((YawErrorAngle.Abs() > yawErrorMax) && (!YawCorrectionActivated))
            YawErrorCount++;
        // After enough iterations of high perceived error, start the correction process
        if (YawErrorCount > yawErrorCountLimit)
            YawCorrectionActivated = true;
        // If the perceived error becomes small, turn off the yaw correction
        if ((YawErrorAngle.Abs() < yawErrorMin) && YawCorrectionActivated) 
        {
            YawCorrectionActivated = false;
            YawErrorCount = 0;
        }
        
        // Perform the actual yaw correction, due to previously detected, large yaw error
        if (YawCorrectionActivated) 
        {
			YawCorrectionInProgress = true;
            // Incrementally "unyaw" by a small step size
            Q = Quatf(Vector3f(0.0f,1.0f,0.0f), -yawRotationStep * YawErrorAngle.Sign()) * Q;
        }
    }
}

 
//  A predictive filter based on extrapolating the smoothed, current angular velocity
Quatf SensorFusion::GetPredictedOrientation(float pdt)
{		
	Lock::Locker lockScope(Handler.GetHandlerLock());
	Quatf        qP = QUncorrected;
	
    if (EnablePrediction)
    {
		// This method assumes a constant angular velocity
	    Vector3f angVelF  = FAngV.SavitzkyGolaySmooth8();
        float    angVelFL = angVelF.Length();

		// Force back to raw measurement
        angVelF  = AngV;
		angVelFL = AngV.Length();

		// Dynamic prediction interval: Based on angular velocity to reduce vibration
		const float minPdt   = 0.001f;
		const float slopePdt = 0.1f;
		float       newpdt   = pdt;
		float       tpdt     = minPdt + slopePdt * angVelFL;
		if (tpdt < pdt)
			newpdt = tpdt;
		//LogText("PredictonDTs: %d\n",(int)(newpdt / PredictionTimeIncrement + 0.5f));

        if (angVelFL > 0.001f)
        {
            Vector3f    rotAxisP      = angVelF / angVelFL;  
            float       halfRotAngleP = angVelFL * newpdt * 0.5f;
            float       sinaHRAP      = sin(halfRotAngleP);
		    Quatf       deltaQP(rotAxisP.x*sinaHRAP, rotAxisP.y*sinaHRAP,
                                rotAxisP.z*sinaHRAP, cos(halfRotAngleP));
            qP = QUncorrected * deltaQP;
		}
	}
    return qP;
}    


Vector3f SensorFusion::GetCalibratedMagValue(const Vector3f& rawMag) const
{
    Vector3f mag = rawMag;
    OVR_ASSERT(HasMagCalibration());
    mag.x += MagCalibrationMatrix.M[0][3];
    mag.y += MagCalibrationMatrix.M[1][3];
    mag.z += MagCalibrationMatrix.M[2][3];
    return mag;
}


void SensorFusion::setMagReference(const Quatf& q, const Vector3f& rawMag) 
{
    if (MagNumReferences < MagMaxReferences)
    {
        MagRefTableQ[MagNumReferences] = q;
        MagRefTableM[MagNumReferences] = rawMag; //FRawMag.Mean();

		//LogText("Inserting reference %d\n",MagNumReferences);
        
		MagRefQ   = q;
        MagRefM   = rawMag;

        float pitch, roll, yaw;
		Quatf q2 = q;
        q2.GetEulerAngles<Axis_X, Axis_Z, Axis_Y>(&pitch, &roll, &yaw);
        MagRefTableYaw[MagNumReferences] = yaw;
		MagRefYaw = yaw;

        MagNumReferences++;

        MagHasNearbyReference = true;
    }
}


SensorFusion::BodyFrameHandler::~BodyFrameHandler()
{
    RemoveHandlerFromDevices();
}

void SensorFusion::BodyFrameHandler::OnMessage(const Message& msg)
{
    if (msg.Type == Message_BodyFrame)
        pFusion->handleMessage(static_cast<const MessageBodyFrame&>(msg));
    if (pFusion->pDelegate)
        pFusion->pDelegate->OnMessage(msg);
}

bool SensorFusion::BodyFrameHandler::SupportsMessageType(MessageType type) const
{
    return (type == Message_BodyFrame);
}

// Writes the current calibration for a particular device to a device profile file
// sensor - the sensor that was calibrated
// cal_name - an optional name for the calibration or default if cal_name == NULL
bool SensorFusion::SaveMagCalibration(const char* calibrationName) const
{
    if (CachedSensorInfo.SerialNumber[0] == NULL || !HasMagCalibration())
        return false;
    
    // A named calibration may be specified for calibration in different
    // environments, otherwise the default calibration is used
    if (calibrationName == NULL)
        calibrationName = "default";

    // Generate a mag calibration event
    JSON* calibration = JSON::CreateObject();
    // (hardcoded for now) the measurement and representation method 
    calibration->AddStringItem("Version", "1.0");   
    calibration->AddStringItem("Name", "default");

    // time stamp the calibration
    char time_str[64];
   
#ifdef OVR_OS_WIN32
    struct tm caltime;
    localtime_s(&caltime, &MagCalibrationTime);
    strftime(time_str, 64, "%Y-%m-%d %H:%M:%S", &caltime);
#else
    struct tm* caltime;
    caltime = localtime(&MagCalibrationTime);
    strftime(time_str, 64, "%Y-%m-%d %H:%M:%S", caltime);
#endif
   
    calibration->AddStringItem("Time", time_str);

    // write the full calibration matrix
    Matrix4f calmat = GetMagCalibration();
    char matrix[128];
    int pos = 0;
    for (int r=0; r<4; r++)
    {
        for (int c=0; c<4; c++)
        {
            pos += (int)OVR_sprintf(matrix+pos, 128, "%g ", calmat.M[r][c]);
        }
    }
    calibration->AddStringItem("Calibration", matrix);

    
    String path = GetBaseOVRPath(true);
    path += "/Devices.json";

    // Look for a prexisting device file to edit
    Ptr<JSON> root = *JSON::Load(path);
    if (root)
    {   // Quick sanity check of the file type and format before we parse it
        JSON* version = root->GetFirstItem();
        if (version && version->Name == "Oculus Device Profile Version")
        {   // In the future I may need to check versioning to determine parse method
        }
        else
        {
            root->Release();
            root = NULL;
        }
    }

    JSON* device = NULL;
    if (root)
    {
        device = root->GetFirstItem();   // skip the header
        device = root->GetNextItem(device);
        while (device)
        {   // Search for a previous calibration with the same name for this device
            // and remove it before adding the new one
            if (device->Name == "Device")
            {   
                JSON* item = device->GetItemByName("Serial");
                if (item && item->Value == CachedSensorInfo.SerialNumber)
                {   // found an entry for this device
                    item = device->GetNextItem(item);
                    while (item)
                    {
                        if (item->Name == "MagCalibration")
                        {   
                            JSON* name = item->GetItemByName("Name");
                            if (name && name->Value == calibrationName)
                            {   // found a calibration of the same name
                                item->RemoveNode();
                                item->Release();
                                break;
                            } 
                        }
                        item = device->GetNextItem(item);
                    }

                    // update the auto-mag flag
                    item = device->GetItemByName("EnableYawCorrection");
                    if (item)
                        item->dValue = (double)EnableYawCorrection;
                    else
                        device->AddBoolItem("EnableYawCorrection", EnableYawCorrection);

                    break;
                }
            }

            device = root->GetNextItem(device);
        }
    }
    else
    {   // Create a new device root
        root = *JSON::CreateObject();
        root->AddStringItem("Oculus Device Profile Version", "1.0");
    }

    if (device == NULL)
    {
        device = JSON::CreateObject();
        device->AddStringItem("Product", CachedSensorInfo.ProductName);
        device->AddNumberItem("ProductID", CachedSensorInfo.ProductId);
        device->AddStringItem("Serial", CachedSensorInfo.SerialNumber);
        device->AddBoolItem("EnableYawCorrection", EnableYawCorrection);

        root->AddItem("Device", device);
    }

    // Create and the add the new calibration event to the device
    device->AddItem("MagCalibration", calibration);

    return root->Save(path);
}

// Loads a saved calibration for the specified device from the device profile file
// sensor - the sensor that the calibration was saved for
// cal_name - an optional name for the calibration or the default if cal_name == NULL
bool SensorFusion::LoadMagCalibration(const char* calibrationName)
{
    if (CachedSensorInfo.SerialNumber[0] == NULL)
        return false;

    // A named calibration may be specified for calibration in different
    // environments, otherwise the default calibration is used
    if (calibrationName == NULL)
        calibrationName = "default";

    String path = GetBaseOVRPath(true);
    path += "/Devices.json";

    // Load the device profiles
    Ptr<JSON> root = *JSON::Load(path);
    if (root == NULL)
        return false;

    // Quick sanity check of the file type and format before we parse it
    JSON* version = root->GetFirstItem();
    if (version && version->Name == "Oculus Device Profile Version")
    {   // In the future I may need to check versioning to determine parse method
    }
    else
    {
        return false;
    }

    bool autoEnableCorrection = false;    

    JSON* device = root->GetNextItem(version);
    while (device)
    {   // Search for a previous calibration with the same name for this device
        // and remove it before adding the new one
        if (device->Name == "Device")
        {   
            JSON* item = device->GetItemByName("Serial");
            if (item && item->Value == CachedSensorInfo.SerialNumber)
            {   // found an entry for this device

                JSON* autoyaw = device->GetItemByName("EnableYawCorrection");
                if (autoyaw)
                    autoEnableCorrection = (autoyaw->dValue != 0);

                item = device->GetNextItem(item);
                while (item)
                {
                    if (item->Name == "MagCalibration")
                    {   
                        JSON* calibration = item;
                        JSON* name = calibration->GetItemByName("Name");
                        if (name && name->Value == calibrationName)
                        {   // found a calibration with this name
                            
                            time_t now;
                            time(&now);

                            // parse the calibration time
                            time_t calibration_time = now;
                            JSON* caltime = calibration->GetItemByName("Time");
                            if (caltime)
                            {
                                const char* caltime_str = caltime->Value.ToCStr();

                                tm ct;
                                memset(&ct, 0, sizeof(tm));
                            
#ifdef OVR_OS_WIN32
                                struct tm nowtime;
                                localtime_s(&nowtime, &now);
                                ct.tm_isdst = nowtime.tm_isdst;
                                sscanf_s(caltime_str, "%d-%d-%d %d:%d:%d", 
                                    &ct.tm_year, &ct.tm_mon, &ct.tm_mday,
                                    &ct.tm_hour, &ct.tm_min, &ct.tm_sec);
#else
                                struct tm* nowtime = localtime(&now);
                                ct.tm_isdst = nowtime->tm_isdst;
                                sscanf(caltime_str, "%d-%d-%d %d:%d:%d", 
                                    &ct.tm_year, &ct.tm_mon, &ct.tm_mday,
                                    &ct.tm_hour, &ct.tm_min, &ct.tm_sec);
#endif
                                ct.tm_year -= 1900;
                                ct.tm_mon--;
                                calibration_time = mktime(&ct);
                            }
                                                        
                            // parse the calibration matrix
                            JSON* cal = calibration->GetItemByName("Calibration");
                            if (cal)
                            {
                                const char* data_str = cal->Value.ToCStr();
                                Matrix4f calmat;
                                for (int r=0; r<4; r++)
                                {
                                    for (int c=0; c<4; c++)
                                    {
                                        calmat.M[r][c] = (float)atof(data_str);
                                        while (data_str && *data_str != ' ')
                                            data_str++;

                                        if (data_str)
                                            data_str++;
                                    }
                                }

                                SetMagCalibration(calmat);
                                MagCalibrationTime  = calibration_time;
                                EnableYawCorrection = autoEnableCorrection;

                                return true;
                            }
                        } 
                    }
                    item = device->GetNextItem(item);
                }

                break;
            }
        }

        device = root->GetNextItem(device);
    }
    
    return false;
}



} // namespace OVR

