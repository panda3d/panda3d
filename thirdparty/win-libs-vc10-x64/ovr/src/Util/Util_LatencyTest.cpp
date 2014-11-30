/************************************************************************************

Filename    :   Util_LatencyTest.cpp
Content     :   Wraps the lower level LatencyTester interface and adds functionality.
Created     :   February 14, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "Util_LatencyTest.h"

#include "../Kernel/OVR_Log.h"
#include "../Kernel/OVR_Timer.h"

namespace OVR { namespace Util {

static const UInt32     TIME_TO_WAIT_FOR_SETTLE_PRE_CALIBRATION = 16*10;
static const UInt32     TIME_TO_WAIT_FOR_SETTLE_POST_CALIBRATION = 16*10;
static const UInt32     TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT = 16*5;
static const UInt32     TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT_RANDOMNESS = 16*5;
static const UInt32     DEFAULT_NUMBER_OF_SAMPLES = 10;                 // For both color 1->2 and color 2->1 transitions.
static const UInt32     INITIAL_SAMPLES_TO_IGNORE = 4;
static const UInt32     TIMEOUT_WAITING_FOR_TEST_STARTED = 1000;
static const UInt32     TIMEOUT_WAITING_FOR_COLOR_DETECTED = 4000;
static const Color      CALIBRATE_BLACK(0, 0, 0);
static const Color      CALIBRATE_WHITE(255, 255, 255);
static const Color      COLOR1(0, 0, 0);
static const Color      COLOR2(255, 255, 255);
static const Color      SENSOR_DETECT_THRESHOLD(128, 255, 255);
static const float      BIG_FLOAT = 1000000.0f;
static const float      SMALL_FLOAT = -1000000.0f;

//-------------------------------------------------------------------------------------
// ***** LatencyTest

LatencyTest::LatencyTest(LatencyTestDevice* device)
 :  Handler(getThis())
{
    if (device != NULL)
    {
        SetDevice(device);
    }

    reset();

    srand(Timer::GetTicksMs());
}

LatencyTest::~LatencyTest()
{
     clearMeasurementResults();
}

bool LatencyTest::SetDevice(LatencyTestDevice* device)
{

    if (device != Device)
    {
        if (device != NULL)
        {
            if (device->GetMessageHandler() != NULL)
            {
                OVR_DEBUG_LOG(
                    ("LatencyTest::AttachToDevice failed - device %p already has handler", device));
                return false;
            }
        }

        if (Device != NULL)
        {
            Device->SetMessageHandler(0);
        }
        Device = device;

        if (Device != NULL)
        {
            Device->SetMessageHandler(&Handler);

            // Set trigger threshold.
            LatencyTestConfiguration configuration(SENSOR_DETECT_THRESHOLD, false);     // No samples streaming.
            Device->SetConfiguration(configuration, true);

            // Set display to intial (3 dashes).
            LatencyTestDisplay ltd(2, 0x40400040);
            Device->SetDisplay(ltd);
        }
    }

    return true;
}

UInt32 LatencyTest::getRandomComponent(UInt32 range)
{
    UInt32 val = rand() % range;
    return val;
}

void LatencyTest::BeginTest()
{
     if (State == State_WaitingForButton)
    {
        // Set color to black and wait a while.
        RenderColor = CALIBRATE_BLACK;

        State = State_WaitingForSettlePreCalibrationColorBlack;
        OVR_DEBUG_LOG(("State_WaitingForButton -> State_WaitingForSettlePreCalibrationColorBlack."));

        setTimer(TIME_TO_WAIT_FOR_SETTLE_PRE_CALIBRATION);
    }
}

void LatencyTest::handleMessage(const Message& msg, LatencyTestMessageType latencyTestMessage)
{
    // For debugging.
/*  if (msg.Type == Message_LatencyTestSamples)
    {
        MessageLatencyTestSamples* pSamples = (MessageLatencyTestSamples*) &msg;

        if (pSamples->Samples.GetSize() > 0)
        {
            // Just show the first one for now.
            Color c = pSamples->Samples[0];
            OVR_DEBUG_LOG(("%d %d %d", c.R, c.G, c.B));
        }
        return;
    }
*/

    if (latencyTestMessage == LatencyTest_Timer)
    {
        if (!Device)
        {
            reset();
            return;
        }
        
        if (State == State_WaitingForSettlePreCalibrationColorBlack)
        {
            // Send calibrate message to device and wait a while.
            Device->SetCalibrate(CALIBRATE_BLACK);

            State = State_WaitingForSettlePostCalibrationColorBlack;
            OVR_DEBUG_LOG(("State_WaitingForSettlePreCalibrationColorBlack -> State_WaitingForSettlePostCalibrationColorBlack."));

            setTimer(TIME_TO_WAIT_FOR_SETTLE_POST_CALIBRATION);
        }
        else if (State == State_WaitingForSettlePostCalibrationColorBlack)
        {
            // Change color to white and wait a while.
            RenderColor = CALIBRATE_WHITE;

            State = State_WaitingForSettlePreCalibrationColorWhite;
            OVR_DEBUG_LOG(("State_WaitingForSettlePostCalibrationColorBlack -> State_WaitingForSettlePreCalibrationColorWhite."));

            setTimer(TIME_TO_WAIT_FOR_SETTLE_PRE_CALIBRATION);
        }
        else if (State == State_WaitingForSettlePreCalibrationColorWhite)
        {
            // Send calibrate message to device and wait a while.
            Device->SetCalibrate(CALIBRATE_WHITE);

            State = State_WaitingForSettlePostCalibrationColorWhite;
            OVR_DEBUG_LOG(("State_WaitingForSettlePreCalibrationColorWhite -> State_WaitingForSettlePostCalibrationColorWhite."));

            setTimer(TIME_TO_WAIT_FOR_SETTLE_POST_CALIBRATION);
        }
        else if (State == State_WaitingForSettlePostCalibrationColorWhite)
        {
            // Calibration is done. Switch to color 1 and wait for it to settle.
            RenderColor = COLOR1;

            State = State_WaitingForSettlePostMeasurement;
            OVR_DEBUG_LOG(("State_WaitingForSettlePostCalibrationColorWhite -> State_WaitingForSettlePostMeasurement."));

            UInt32 waitTime = TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT + getRandomComponent(TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT_RANDOMNESS);
            setTimer(waitTime);
        }
        else if (State == State_WaitingForSettlePostMeasurement)
        {
            // Prepare for next measurement.

            // Create a new result object.
            MeasurementResult* pResult = new MeasurementResult();
            Results.PushBack(pResult);

            State = State_WaitingToTakeMeasurement;
            OVR_DEBUG_LOG(("State_WaitingForSettlePostMeasurement -> State_WaitingToTakeMeasurement."));
        }
        else if (State == State_WaitingForTestStarted)
        {
            // We timed out waiting for 'TestStarted'. Abandon this measurement and setup for the next.
            getActiveResult()->TimedOutWaitingForTestStarted = true;

            State = State_WaitingForSettlePostMeasurement;
            OVR_DEBUG_LOG(("** Timed out waiting for 'TestStarted'."));
            OVR_DEBUG_LOG(("State_WaitingForTestStarted -> State_WaitingForSettlePostMeasurement."));

            UInt32 waitTime = TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT + getRandomComponent(TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT_RANDOMNESS);
            setTimer(waitTime);
        }
        else if (State == State_WaitingForColorDetected)
        {
            // We timed out waiting for 'ColorDetected'. Abandon this measurement and setup for the next.
            getActiveResult()->TimedOutWaitingForColorDetected = true;

            State = State_WaitingForSettlePostMeasurement;
            OVR_DEBUG_LOG(("** Timed out waiting for 'ColorDetected'."));
            OVR_DEBUG_LOG(("State_WaitingForColorDetected -> State_WaitingForSettlePostMeasurement."));

            UInt32 waitTime = TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT + getRandomComponent(TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT_RANDOMNESS);
            setTimer(waitTime);
        }
    }
    else if (latencyTestMessage == LatencyTest_ProcessInputs)
    {
        if (State == State_WaitingToTakeMeasurement)
        {
            if (!Device)
            {
                reset();
                return;
            }
            
            // Send 'StartTest' feature report with opposite target color.
            if (RenderColor == COLOR1)
            {
                RenderColor = COLOR2;
            }
            else
            {
                RenderColor = COLOR1;
            }

            getActiveResult()->TargetColor = RenderColor;
            
            // Record time so we can determine usb roundtrip time.
            getActiveResult()->StartTestTicksMicroS = Timer::GetTicks();

            Device->SetStartTest(RenderColor);

            State = State_WaitingForTestStarted;
            OVR_DEBUG_LOG(("State_WaitingToTakeMeasurement -> State_WaitingForTestStarted."));

            setTimer(TIMEOUT_WAITING_FOR_TEST_STARTED);

            LatencyTestDisplay ltd(2, 0x40090040);
            Device->SetDisplay(ltd);
        }
    }
    else if (msg.Type == Message_LatencyTestButton)
    {
        BeginTest();
    }
    else if (msg.Type == Message_LatencyTestStarted)
    {
        if (State == State_WaitingForTestStarted)
        {
            clearTimer();

            // Record time so we can determine usb roundtrip time.
            getActiveResult()->TestStartedTicksMicroS = Timer::GetTicks();
            
            State = State_WaitingForColorDetected;
            OVR_DEBUG_LOG(("State_WaitingForTestStarted -> State_WaitingForColorDetected."));

            setTimer(TIMEOUT_WAITING_FOR_COLOR_DETECTED);
        }
    }
    else if (msg.Type == Message_LatencyTestColorDetected)
    {
        if (State == State_WaitingForColorDetected)
        {
            // Record time to detect color.
            MessageLatencyTestColorDetected* pDetected = (MessageLatencyTestColorDetected*) &msg;
            UInt16 elapsedTime = pDetected->Elapsed;
            OVR_DEBUG_LOG(("Time to 'ColorDetected' = %d", elapsedTime));
            
            getActiveResult()->DeviceMeasuredElapsedMilliS = elapsedTime;

            if (areResultsComplete())
            {
                // We're done.
                processResults();
                reset();
            }
            else
            {
                // Run another measurement.
                State = State_WaitingForSettlePostMeasurement;
                OVR_DEBUG_LOG(("State_WaitingForColorDetected -> State_WaitingForSettlePostMeasurement."));

                UInt32 waitTime = TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT + getRandomComponent(TIME_TO_WAIT_FOR_SETTLE_POST_MEASUREMENT_RANDOMNESS);
                setTimer(waitTime);

                LatencyTestDisplay ltd(2, 0x40400040);
                Device->SetDisplay(ltd);
            }
        }
    }
    else if (msg.Type == Message_DeviceRemoved)
    {
        reset();
    }
}

LatencyTest::MeasurementResult* LatencyTest::getActiveResult()
{
    OVR_ASSERT(!Results.IsEmpty());    
    return Results.GetLast();
}

void LatencyTest::setTimer(UInt32 timeMilliS)
{
    ActiveTimerMilliS = timeMilliS;
}

void LatencyTest::clearTimer()
{
    ActiveTimerMilliS = 0;
}

void LatencyTest::reset()
{
    clearMeasurementResults();
    State = State_WaitingForButton;

    HaveOldTime = false;
    ActiveTimerMilliS = 0;
}

void LatencyTest::clearMeasurementResults()
{
    while(!Results.IsEmpty())
    {
        MeasurementResult* pElem = Results.GetFirst();
        pElem->RemoveNode();
        delete pElem;
    }
}

LatencyTest::LatencyTestHandler::~LatencyTestHandler()
{
    RemoveHandlerFromDevices();
}

void LatencyTest::LatencyTestHandler::OnMessage(const Message& msg)
{
    pLatencyTestUtil->handleMessage(msg);
}

void LatencyTest::ProcessInputs()
{
    updateForTimeouts();
    handleMessage(Message(), LatencyTest_ProcessInputs);
}

bool LatencyTest::DisplayScreenColor(Color& colorToDisplay)
{
    updateForTimeouts();

    if (State == State_WaitingForButton)
    {
        return false;
    }

    colorToDisplay = RenderColor;
    return true;
}

const char*	LatencyTest::GetResultsString()
{
	if (!ResultsString.IsEmpty() && ReturnedResultString != ResultsString.ToCStr())
	{
		ReturnedResultString = ResultsString;
		return ReturnedResultString.ToCStr();
	}
    
	return NULL;
}

bool LatencyTest::areResultsComplete()
{
    UInt32 initialMeasurements = 0;

    UInt32 measurements1to2 = 0;
    UInt32 measurements2to1 = 0;

    MeasurementResult* pCurr = Results.GetFirst();
    while(true)
    {
        // Process.
        if (!pCurr->TimedOutWaitingForTestStarted &&
            !pCurr->TimedOutWaitingForColorDetected)
        {
            initialMeasurements++;

            if (initialMeasurements > INITIAL_SAMPLES_TO_IGNORE)
            {
                if (pCurr->TargetColor == COLOR2)
                {
                    measurements1to2++;
                }
                else
                {
                    measurements2to1++;
                }
            }
        }

        if (Results.IsLast(pCurr))
        {
            break;
        }
        pCurr = Results.GetNext(pCurr);
    }

    if (measurements1to2 >= DEFAULT_NUMBER_OF_SAMPLES &&
        measurements2to1 >= DEFAULT_NUMBER_OF_SAMPLES)
    {
        return true;
    }

    return false;
}

void LatencyTest::processResults()
{

    UInt32 minTime1To2 = UINT_MAX;
    UInt32 maxTime1To2 = 0;
    float averageTime1To2 = 0.0f;
    UInt32 minTime2To1 = UINT_MAX;
    UInt32 maxTime2To1 = 0;
    float averageTime2To1 = 0.0f;

    float minUSBTripMilliS = BIG_FLOAT;
    float maxUSBTripMilliS = SMALL_FLOAT;
    float averageUSBTripMilliS = 0.0f;
    UInt32 countUSBTripTime = 0;

    UInt32 measurementsCount = 0;
    UInt32 measurements1to2 = 0;
    UInt32 measurements2to1 = 0;

    MeasurementResult* pCurr = Results.GetFirst();
    UInt32 count = 0;
    while(true)
    {
        count++;

        if (!pCurr->TimedOutWaitingForTestStarted &&
            !pCurr->TimedOutWaitingForColorDetected)
        {
            measurementsCount++;

            if (measurementsCount > INITIAL_SAMPLES_TO_IGNORE)
            {
                if (pCurr->TargetColor == COLOR2)
                {
                    measurements1to2++;

                    if (measurements1to2 <= DEFAULT_NUMBER_OF_SAMPLES)
                    {
                        UInt32 elapsed = pCurr->DeviceMeasuredElapsedMilliS;

                        minTime1To2 = Alg::Min(elapsed, minTime1To2);
                        maxTime1To2 = Alg::Max(elapsed, maxTime1To2);

                        averageTime1To2 += (float) elapsed;
                    }
                }
                else
                {
                    measurements2to1++;

                    if (measurements2to1 <= DEFAULT_NUMBER_OF_SAMPLES)
                    {
                        UInt32 elapsed = pCurr->DeviceMeasuredElapsedMilliS;

                        minTime2To1 = Alg::Min(elapsed, minTime2To1);
                        maxTime2To1 = Alg::Max(elapsed, maxTime2To1);

                        averageTime2To1 += (float) elapsed;
                    }
                }

                float usbRountripElapsedMilliS = 0.001f * (float) (pCurr->TestStartedTicksMicroS - pCurr->StartTestTicksMicroS);
                minUSBTripMilliS = Alg::Min(usbRountripElapsedMilliS, minUSBTripMilliS);
                maxUSBTripMilliS = Alg::Max(usbRountripElapsedMilliS, maxUSBTripMilliS);
                averageUSBTripMilliS += usbRountripElapsedMilliS;
                countUSBTripTime++;
            }
        }

        if (measurements1to2 >= DEFAULT_NUMBER_OF_SAMPLES &&
            measurements2to1 >= DEFAULT_NUMBER_OF_SAMPLES)
        {
            break;
        }

        if (Results.IsLast(pCurr))
        {
            break;
        }
        pCurr = Results.GetNext(pCurr);
    }

    averageTime1To2 /= (float) DEFAULT_NUMBER_OF_SAMPLES;      
    averageTime2To1 /= (float) DEFAULT_NUMBER_OF_SAMPLES;

    averageUSBTripMilliS /= countUSBTripTime;
    
    float finalResult = 0.5f * (averageTime1To2 + averageTime2To1);
    finalResult += averageUSBTripMilliS;

    ResultsString.Clear();
    ResultsString.AppendFormat("RESULT=%.1f (add half Tracker period) [b->w %d|%.1f|%d] [w->b %d|%.1f|%d] [usb rndtrp %.1f|%.1f|%.1f] [cnt %d] [tmouts %d]",  
                finalResult, 
                minTime1To2, averageTime1To2, maxTime1To2, 
                minTime2To1, averageTime2To1, maxTime2To1,
                minUSBTripMilliS, averageUSBTripMilliS, maxUSBTripMilliS,
                DEFAULT_NUMBER_OF_SAMPLES*2, count - measurementsCount);
    
    // Display result on latency tester display.
    LatencyTestDisplay ltd(1, (int)finalResult);
    Device->SetDisplay(ltd);
}

void LatencyTest::updateForTimeouts()
{
    if (!HaveOldTime)
    {
        HaveOldTime = true;
        OldTime = Timer::GetTicksMs();
        return;
    }

    UInt32 newTime = Timer::GetTicksMs();
    UInt32 elapsedMilliS = newTime - OldTime;
    if (newTime < OldTime)
    {
        elapsedMilliS = OldTime - newTime;
        elapsedMilliS = UINT_MAX - elapsedMilliS;
    }
    OldTime = newTime;

    elapsedMilliS = Alg::Min(elapsedMilliS, (UInt32) 100);   // Clamp at 100mS in case we're not being called very often.


    if (ActiveTimerMilliS == 0)
    {
        return;
    }

    if (elapsedMilliS >= ActiveTimerMilliS)
    {
        ActiveTimerMilliS = 0;
        handleMessage(Message(), LatencyTest_Timer);
        return;
    }

    ActiveTimerMilliS -= elapsedMilliS;
}

}} // namespace OVR::Util
