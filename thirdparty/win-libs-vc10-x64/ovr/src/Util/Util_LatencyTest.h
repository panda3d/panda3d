/************************************************************************************

PublicHeader:   OVR.h
Filename    :   Util_LatencyTest.h
Content     :   Wraps the lower level LatencyTesterDevice and adds functionality.
Created     :   February 14, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_Util_LatencyTest_h
#define OVR_Util_LatencyTest_h

#include "../OVR_Device.h"

#include "../Kernel/OVR_String.h"
#include "../Kernel/OVR_List.h"

namespace OVR { namespace Util {


//-------------------------------------------------------------------------------------
// ***** LatencyTest
//
// LatencyTest utility class wraps the low level LatencyTestDevice and manages the scheduling
// of a latency test. A single test is composed of a series of individual latency measurements
// which are used to derive min, max, and an average latency value.
//
// Developers are required to call the following methods:
//      SetDevice - Sets the LatencyTestDevice to be used for the tests.
//      ProcessInputs - This should be called at the same place in the code where the game engine
//                      reads the headset orientation from LibOVR (typically done by calling
//                      'GetOrientation' on the SensorFusion object). Calling this at the right time
//                      enables us to measure the same latency that occurs for headset orientation
//                      changes.
//      DisplayScreenColor -    The latency tester works by sensing the color of the pixels directly
//                              beneath it. The color of these pixels can be set by drawing a small
//                              quad at the end of the rendering stage. The quad should be small
//                              such that it doesn't significantly impact the rendering of the scene,
//                              but large enough to be 'seen' by the sensor. See the SDK
//                              documentation for more information.
//		GetResultsString -	Call this to get a string containing the most recent results.
//							If the string has already been gotten then NULL will be returned.
//							The string pointer will remain valid until the next time this 
//							method is called.
//

class LatencyTest : public NewOverrideBase
{
public:
    LatencyTest(LatencyTestDevice* device = NULL);
    ~LatencyTest();
    
    // Set the Latency Tester device that we'll use to send commands to and receive
    // notification messages from.
    bool        SetDevice(LatencyTestDevice* device);

    // Returns true if this LatencyTestUtil has a Latency Tester device.
    bool        HasDevice() const
    { return Handler.IsHandlerInstalled(); }

    void        ProcessInputs();
    bool        DisplayScreenColor(Color& colorToDisplay);
	const char*	GetResultsString();

    // Begin test. Equivalent to pressing the button on the latency tester.
    void BeginTest();

private:
    LatencyTest* getThis()  { return this; }

    enum LatencyTestMessageType
    {
        LatencyTest_None,
        LatencyTest_Timer,
        LatencyTest_ProcessInputs,
    };
    
    UInt32 getRandomComponent(UInt32 range);
    void handleMessage(const Message& msg, LatencyTestMessageType latencyTestMessage = LatencyTest_None);
    void reset();
    void setTimer(UInt32 timeMilliS);
    void clearTimer();

    class LatencyTestHandler : public MessageHandler
    {
        LatencyTest*    pLatencyTestUtil;
    public:
        LatencyTestHandler(LatencyTest* latencyTester) : pLatencyTestUtil(latencyTester) { }
        ~LatencyTestHandler();

        virtual void OnMessage(const Message& msg);
    };

    bool areResultsComplete();
    void processResults();
    void updateForTimeouts();

    Ptr<LatencyTestDevice>      Device;
    LatencyTestHandler          Handler;

    enum TesterState
    {
        State_WaitingForButton,
        State_WaitingForSettlePreCalibrationColorBlack,
        State_WaitingForSettlePostCalibrationColorBlack,
        State_WaitingForSettlePreCalibrationColorWhite,
        State_WaitingForSettlePostCalibrationColorWhite,
        State_WaitingToTakeMeasurement,
        State_WaitingForTestStarted,
        State_WaitingForColorDetected,
        State_WaitingForSettlePostMeasurement
    };
    TesterState                 State;

    bool                        HaveOldTime;
    UInt32                      OldTime;
    UInt32                      ActiveTimerMilliS;

    Color                       RenderColor;

    struct MeasurementResult : public ListNode<MeasurementResult>, public NewOverrideBase
    {
        MeasurementResult()
         :  DeviceMeasuredElapsedMilliS(0),
            TimedOutWaitingForTestStarted(false),
            TimedOutWaitingForColorDetected(false),
            StartTestTicksMicroS(0),
            TestStartedTicksMicroS(0)
        {}

        Color                   TargetColor;

        UInt32                  DeviceMeasuredElapsedMilliS;

        bool                    TimedOutWaitingForTestStarted;
        bool                    TimedOutWaitingForColorDetected;

        UInt64                  StartTestTicksMicroS;
        UInt64                  TestStartedTicksMicroS;
    };

    List<MeasurementResult>     Results;
    void clearMeasurementResults();

    MeasurementResult*          getActiveResult();

    StringBuffer			    ResultsString;
	String					    ReturnedResultString;
};

}} // namespace OVR::Util

#endif // OVR_Util_LatencyTest_h
