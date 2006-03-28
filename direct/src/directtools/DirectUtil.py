
from DirectGlobals import *

# Routines to adjust values
def ROUND_TO(value, divisor):
    return round(value/float(divisor)) * divisor

def ROUND_INT(val):
    return int(round(val))

def CLAMP(val, minVal, maxVal):
    return min(max(val, minVal), maxVal)

# Create a tk compatible color string
def getTkColorString(color):
    """
    Print out a Tk compatible version of a color string
    """
    def toHex(intVal):
        val = int(round(intVal))
        if val < 16:
            return "0" + hex(val)[2:]
        else:
            return hex(val)[2:]
    r = toHex(color[0])
    g = toHex(color[1])
    b = toHex(color[2])
    return "#" + r + g + b

## Background Color ##
def lerpBackgroundColor(r, g, b, duration):
    """
    Function to lerp background color to a new value
    """
    def lerpColor(state):
        dt = globalClock.getDt()
        state.time += dt
        sf = state.time / state.duration
        if sf >= 1.0:
            base.setBackgroundColor(state.ec[0], state.ec[1], state.ec[2])
            return Task.done
        else:
            r = sf * state.ec[0] + (1 - sf) * state.sc[0]
            g = sf * state.ec[1] + (1 - sf) * state.sc[1]
            b = sf * state.ec[2] + (1 - sf) * state.sc[2]
            base.setBackgroundColor(r, g, b)
            return Task.cont
    taskMgr.remove('lerpBackgroundColor')
    t = taskMgr.add(lerpColor, 'lerpBackgroundColor')
    t.time = 0.0
    t.duration = duration
    t.sc = base.getBackgroundColor()
    t.ec = VBase4(r, g, b, 1)

# Set direct drawing style for an object
# Never light object or draw in wireframe
def useDirectRenderStyle(nodePath, priority = 0):
    """
    Function to force a node path to use direct render style:
    no lighting, and no wireframe
    """
    nodePath.setLightOff(priority)
    nodePath.setRenderModeFilled()

# File data util
def getFileData(filename, separator = ','):
    """
    Open the specified file and strip out unwanted whitespace and
    empty lines.  Return file as list of lists, one file line per element,
    list elements based upon separator
    """
    f = open(filename.toOsSpecific(), 'r')
    rawData = f.readlines()
    f.close()
    fileData = []
    for line in rawData:
        # First strip whitespace from both ends of line
        l = line.strip()
        if l:
            # If its a valid line, split on separator and
            # strip leading/trailing whitespace from each element
            data = map(lambda s: s.strip(), l.split(separator))
            fileData.append(data)
    return fileData

