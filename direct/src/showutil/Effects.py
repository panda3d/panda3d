""" Showutil Effects module: contains code for useful showcode effects. """

from panda3d.core import *
from direct.interval.IntervalGlobal import *

# bounce types
SX_BOUNCE = 0
SY_BOUNCE = 1
SZ_BOUNCE = 2
TX_BOUNCE = 3
TY_BOUNCE = 4
TZ_BOUNCE = 5
H_BOUNCE = 6
P_BOUNCE = 7
R_BOUNCE = 8


# convenience funcs
#
def createScaleXBounce(nodeObj, numBounces, startValues, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValues, totalTime,
                        amplitude, SX_BOUNCE)

def createScaleYBounce(nodeObj, numBounces, startValues, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValues, totalTime,
                        amplitude, SY_BOUNCE)

def createScaleZBounce(nodeObj, numBounces, startValue, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValue, totalTime,
                        amplitude, SZ_BOUNCE)

def createXBounce(nodeObj, numBounces, startValues, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValues, totalTime,
                        amplitude, TX_BOUNCE)

def createYBounce(nodeObj, numBounces, startValues, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValues, totalTime,
                        amplitude, TY_BOUNCE)

def createZBounce(nodeObj, numBounces, startValues, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValues, totalTime,
                        amplitude, TZ_BOUNCE)

def createHBounce(nodeObj, numBounces, startValues, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValues, totalTime,
                        amplitude, H_BOUNCE)

def createPBounce(nodeObj, numBounces, startValues, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValues, totalTime,
                        amplitude, P_BOUNCE)

def createRBounce(nodeObj, numBounces, startValues, totalTime, amplitude):
    return createBounce(nodeObj, numBounces, startValues, totalTime,
                        amplitude, R_BOUNCE)

def createBounce(nodeObj, numBounces, startValues, totalTime, amplitude,
                 bounceType=SZ_BOUNCE):
    """
    createBounce: create and return a list of intervals to make a
    given nodePath bounce a given number of times over a give total time.
    """
    if not nodeObj or numBounces < 1 or totalTime == 0:
        self.notify.warning(
            "createBounceIvals called with invalid parameter")
        return

    result = Sequence()

    # calculate how long, in seconds, each bounce should last
    bounceTime = totalTime/float(numBounces)
    currTime = bounceTime
    currAmplitude = amplitude

    # determine the how much of a change in value the first bounce
    # will produce based on the given start value and amplitude
    #
    if ((bounceType == SX_BOUNCE) or (bounceType == TX_BOUNCE) or
        (bounceType == H_BOUNCE)):
        index = 0
    elif ((bounceType == SY_BOUNCE) or (bounceType == TY_BOUNCE) or
          (bounceType == P_BOUNCE)):
        index = 1
    elif ((bounceType == SZ_BOUNCE) or (bounceType == TZ_BOUNCE) or
          (bounceType == R_BOUNCE)):
        index = 2
    currBounceVal = startValues[index]

    # create a lerp interval for each bounce, making sure to
    # figure out the new value, which progressively gets closer
    # to our start value
    #
    for bounceNum in range(numBounces*2):
        # determine the direction that this value should go,
        # alternating for each lerp interval to simulate
        # a spring effect
        #
        if bounceNum % 2:
            # bounce down
            currBounceVal = startValues[index]
            blend = 'easeIn'
        else:
            # bounce up
            currBounceVal = startValues[index] + currAmplitude
            blend = 'easeOut'

        newVec3 = Vec3(startValues)
        newVec3.setCell(index, currBounceVal)
        print("### newVec3 = %s" % newVec3)

        # create the right type of lerp
        if ((bounceType == SX_BOUNCE) or (bounceType == SY_BOUNCE) or
            (bounceType == SZ_BOUNCE)):
            result.append(LerpScaleInterval(
                nodeObj, currTime, newVec3, blendType=blend))
        elif ((bounceType == TX_BOUNCE) or (bounceType == TY_BOUNCE) or
              (bounceType == TZ_BOUNCE)):
            result.append(LerpPosInterval(
                nodeObj, currTime, newVec3, blendType=blend))
        elif ((bounceType == H_BOUNCE) or (bounceType == P_BOUNCE) or
              (bounceType == R_BOUNCE)):
            result.append(LerpHprInterval(
                nodeObj, currTime, newVec3, blendType=blend))

        # the value diff from the base gets smaller for each
        # consecutive bounce, and make sure to update for
        # possibly a new amount of time the next bounce will
        # take
        #
        currAmplitude *= 0.5
        currTime = bounceTime

    return result


