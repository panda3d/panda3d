from PandaObject import *
from EntryScale import EntryScale

def adjust(command = None, min = 0.0, max = 1.0, text = 'Adjust'):
    tl = Toplevel()
    tl.title('Parameter Adjust')
    es = EntryScale(tl, command = command, min = min, max = max, text = text)
    es.pack(expand = 1, fill = X)

## Background Color ##
def setBackgroundColor(r,g,b):
    base.win.getGsg().setColorClearValue(VBase4(r, g, b, 1.0))

def lerpBackgroundColor(r,g,b,duration):
    def lerpColor(state):
        dt = globalClock.getDt()
        state.time += dt
        sf = state.time / state.duration
        if sf >= 1.0:
            setBackgroundColor(state.ec[0], state.ec[1], state.ec[2])
            return Task.done
        else:
            r = sf * state.ec[0] + (1 - sf) * state.sc[0]
            g = sf * state.ec[1] + (1 - sf) * state.sc[1]
            b = sf * state.ec[2] + (1 - sf) * state.sc[2]
            setBackgroundColor(r,g,b)
            return Task.cont
    taskMgr.removeTasksNamed('lerpBackgroundColor')
    t = taskMgr.spawnMethodNamed(lerpColor, 'lerpBackgroundColor')
    t.time = 0.0
    t.duration = duration
    t.sc = base.win.getGsg().getColorClearValue()
    t.ec = VBase4(r,g,b,1)

Q_EPSILON = 1e-10

# Quaternion interpolation
def qSlerp(startQuat, endQuat, t):
    startQ = Quat(startQuat)
    destQuat = Quat.identQuat()
    # Calc dot product
    cosOmega = (startQ.getI() * endQuat.getI() +
                startQ.getJ() * endQuat.getJ() + 
                startQ.getK() * endQuat.getK() +
                startQ.getR() * endQuat.getR())
    # If the above dot product is negative, it would be better to
    # go between the negative of the initial and the final, so that
    # we take the shorter path.  
    if ( cosOmega < 0.0 ):
        cosOmega *= -1
        startQ.setI(-1 * startQ.getI())
        startQ.setJ(-1 * startQ.getJ())
        startQ.setK(-1 * startQ.getK())
        startQ.setR(-1 * startQ.getR())
    if ((1.0 + cosOmega) > Q_EPSILON):
        # usual case
        if ((1.0 - cosOmega) > Q_EPSILON):
            # usual case
            omega = math.acos(cosOmega)
            sinOmega = math.sin(omega)
            startScale = math.sin((1.0 - t) * omega)/sinOmega
            endScale = math.sin(t * omega)/sinOmega
        else:
            # ends very close 
            startScale = 1.0 - t
            endScale = t
        destQuat.setI(startScale * startQ.getI() +
                      endScale * endQuat.getI())
        destQuat.setJ(startScale * startQ.getJ() +
                      endScale * endQuat.getJ())
        destQuat.setK(startScale * startQ.getK() +
                      endScale * endQuat.getK())
        destQuat.setR(startScale * startQ.getR() +
                      endScale * endQuat.getR())
    else:
        # ends nearly opposite
        destQuat.setI(-startQ.getJ())
        destQuat.setJ(startQ.getI())
        destQuat.setK(-startQ.getR())
        destQuat.setR(startQ.getK())
        startScale = math.sin((0.5 - t) * math.pi)
        endScale = math.sin(t * math.pi)
        destQuat.setI(startScale * startQ.getI() +
                      endScale * endQuat.getI())
        destQuat.setJ(startScale * startQ.getJ() +
                      endScale * endQuat.getJ())
        destQuat.setK(startScale * startQ.getK() +
                      endScale * endQuat.getK())
    return destQuat

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
        l = string.strip(line)
        if l:
            # If its a valid line, split on separator and
            # strip leading/trailing whitespace from each element
            data = map(string.strip, l.split(separator))
            fileData.append(data)
    return fileData

