"""ProjectileInterval module: contains the ProjectileInterval class"""

__all__ = ['ProjectileInterval']

from panda3d.core import *
from panda3d.direct import *
from direct.directnotify.DirectNotifyGlobal import *
from .Interval import Interval
from direct.showbase import PythonUtil

class ProjectileInterval(Interval):
    """ProjectileInterval class: moves a nodepath through the trajectory
    of a projectile under the influence of gravity"""

    # create ProjectileInterval DirectNotify category
    notify = directNotify.newCategory('ProjectileInterval')

    # serial num for unnamed intervals
    projectileIntervalNum = 1

    # g ~ 9.8 m/s^2 ~ 32 ft/s^2
    gravity = 32.

    # the projectile's velocity is constant in the X and Y directions.
    # the projectile's motion in the Z (up) direction is parabolic
    # due to the constant force of gravity, which acts in the -Z direction

    def __init__(self, node, startPos = None,
                 endPos = None, duration = None,
                 startVel = None, endZ = None,
                 wayPoint = None, timeToWayPoint = None,
                 gravityMult = None, name = None,
                 collNode = None):
        """
        You may specify several different sets of input parameters.
        (If startPos is not provided, it will be obtained from the node's
        position at the time that the interval is first started. Note that
        in this case you must provide a duration of some kind.)

        # go from startPos to endPos in duration seconds
        startPos, endPos, duration
        # given a starting velocity, go for a specific time period
        startPos, startVel, duration
        # given a starting velocity, go until you hit a given Z plane
        startPos, startVel, endZ
        # pass through wayPoint at time 'timeToWayPoint'. Go until
        # you hit a given Z plane
        startPos, wayPoint, timeToWayPoint, endZ

        You may alter gravity by providing a multiplier in 'gravityMult'.
        '2.' will make gravity twice as strong, '.5' half as strong.
        '-1.' will reverse gravity

        If collNode is not None, it should be an empty CollisionNode
        which will be filled with an appropriate CollisionParabola
        when the interval starts.  This CollisionParabola will be set
        to match the interval's parabola, and its t1, t2 values will
        be updated automatically as the interval plays.  It will *not*
        be automatically removed from the node when the interval
        finishes.

        """
        self.node = node
        self.collNode = collNode
        if self.collNode:
            if isinstance(self.collNode, NodePath):
                self.collNode = self.collNode.node()
            assert self.collNode.getNumSolids() == 0

        if name == None:
            name = '%s-%s' % (self.__class__.__name__,
                              self.projectileIntervalNum)
            ProjectileInterval.projectileIntervalNum += 1

        args = (startPos, endPos, duration, startVel, endZ,
                wayPoint, timeToWayPoint, gravityMult)
        self.implicitStartPos = 0
        if startPos is None:
            if duration is None:
                self.notify.error('must provide either startPos or duration')
            self.duration = duration
            # we can't calc the trajectory until we know our starting
            # position; delay until the interval is actually started
            self.trajectoryArgs = args
            self.implicitStartPos = 1
        else:
            self.trajectoryArgs = args
            self.__calcTrajectory(*args)

        Interval.__init__(self, name, self.duration)

    def __calcTrajectory(self, startPos = None,
                         endPos = None, duration = None,
                         startVel = None, endZ = None,
                         wayPoint = None, timeToWayPoint = None,
                         gravityMult = None):
        if startPos is None:
            startPos = self.node.getPos()

        def doIndirections(*items):
            result = []
            for item in items:
                if callable(item):
                    item = item()
                result.append(item)
            return result

        startPos, endPos, startVel, endZ, gravityMult, wayPoint, \
                  timeToWayPoint = \
                  doIndirections(startPos, endPos, startVel, endZ, gravityMult,
                                 wayPoint, timeToWayPoint)

        # we're guaranteed to know the starting position at this point
        self.startPos = startPos

        # gravity is applied in the -Z direction
        self.zAcc = -self.gravity
        if gravityMult:
            self.zAcc *= gravityMult

        def calcStartVel(startPos, endPos, duration, zAccel):
            # p(t) = p_0 + t*v_0 + .5*a*t^2
            # v_0 = [p(t) - p_0 - .5*a*t^2] / t
            if (duration == 0):
                return Point3(0, 0, 0)
            else:
                return Point3((endPos[0] - startPos[0]) / duration,
                          (endPos[1] - startPos[1]) / duration,
                          (endPos[2] - startPos[2] - (.5*zAccel*duration*duration)) / duration)

        def calcTimeOfImpactOnPlane(startHeight, endHeight, startVel, accel):
            return PythonUtil.solveQuadratic(accel * .5, startVel,
                                             startHeight-endHeight)

        def calcTimeOfLastImpactOnPlane(startHeight, endHeight,
                                        startVel, accel):
            time = calcTimeOfImpactOnPlane(startHeight, endHeight,
                                           startVel, accel)
            if not time:
                return None
            if type(time) == type([]):
                # projectile hits plane once going up, once going down
                # assume they want the one on the way down
                assert self.notify.debug('projectile hits plane twice at times: %s' %
                                  time)
                time = max(*time)
            else:
                assert self.notify.debug('projectile hits plane once at time: %s' %
                                  time)
            return time

        # now all we need is startVel, duration, and endPos.

        # which set of input parameters do we have?
        if (None not in (endPos, duration)):
            assert not startVel
            assert not endZ
            assert not wayPoint
            assert not timeToWayPoint
            self.duration = duration
            self.endPos = endPos
            self.startVel = calcStartVel(self.startPos, self.endPos,
                                         self.duration, self.zAcc)
        elif (None not in (startVel, duration)):
            assert not endPos
            assert not endZ
            assert not wayPoint
            assert not timeToWayPoint
            self.duration = duration
            self.startVel = startVel
            self.endPos = None
        elif (None not in (startVel, endZ)):
            assert not endPos
            assert not duration
            assert not wayPoint
            assert not timeToWayPoint
            self.startVel = startVel
            time = calcTimeOfLastImpactOnPlane(self.startPos[2], endZ,
                                               self.startVel[2], self.zAcc)
            if time is None:
                self.notify.error(
                    'projectile never reaches plane Z=%s' % endZ)
            self.duration = time
            self.endPos = None
        elif (None not in (wayPoint, timeToWayPoint, endZ)):
            assert not endPos
            assert not duration
            assert not startVel
            # we pass through wayPoint at time 'timeToWayPoint', and we
            # stop when we reach endZ
            self.startVel = calcStartVel(self.startPos, wayPoint,
                                         timeToWayPoint, self.zAcc)
            time = calcTimeOfLastImpactOnPlane(
                self.startPos[2], endZ, self.startVel[2], self.zAcc)
            if time is None:
                self.notify.error(
                    'projectile never reaches plane Z=%s' % endZ)
            self.duration = time
            self.endPos = None
        else:
            self.notify.error('invalid set of inputs to ProjectileInterval')

        self.parabola = LParabola(VBase3(0, 0, 0.5 * self.zAcc),
                                  self.startVel,
                                  self.startPos)

        if not self.endPos:
            self.endPos = self.__calcPos(self.duration)

        # these are the parameters that we need to know:
        assert self.notify.debug('startPos: %s' % repr(self.startPos))
        assert self.notify.debug('endPos:   %s' % repr(self.endPos))
        assert self.notify.debug('duration: %s' % self.duration)
        assert self.notify.debug('startVel: %s' % repr(self.startVel))
        assert self.notify.debug('z-accel:  %s' % self.zAcc)

    def __initialize(self):
        if self.implicitStartPos:
            self.__calcTrajectory(*self.trajectoryArgs)

    def testTrajectory(self):
        try:
            self.__calcTrajectory(*self.trajectoryArgs)
        except Exception:
            assert self.notify.error('invalid projectile parameters')
            return False
        return True

    def privInitialize(self, t):
        self.__initialize()
        if self.collNode:
            self.collNode.clearSolids()
            csolid = CollisionParabola(self.parabola, 0, 0)
            self.collNode.addSolid(csolid)

        Interval.privInitialize(self, t)

    def privInstant(self):
        self.__initialize()
        Interval.privInstant(self)
        if self.collNode:
            self.collNode.clearSolids()
            csolid = CollisionParabola(self.parabola, 0, self.duration)
            self.collNode.addSolid(csolid)

    def __calcPos(self, t):
        return self.parabola.calcPoint(t)

    def privStep(self, t):
        self.node.setFluidPos(self.__calcPos(t))
        Interval.privStep(self, t)
        if self.collNode and self.collNode.getNumSolids() > 0:
            csolid = self.collNode.modifySolid(0)
            csolid.setT1(csolid.getT2())
            csolid.setT2(t)

"""
        ##################################################################
          TODO: support arbitrary sets of inputs
        ##################################################################
        You must provide a few of the parameters, and the others will be
        computed. The input parameters in question are:
          duration, endZ, endPos, startVel, gravityMult

        Valid sets of input parameters (AA),
        (trivially computed/default parameters) (BB),
        non-trivial computed parameters (CC):
        AA && BB => CC

        # one parameter
        duration && (startVel, gravityMult) => endZ, endPos
        endZ     && (startVel, gravityMult) => duration, endPos
        endPos   && (endZ, gravityMult    ) => duration, startVel

        # two parameters
        duration, endZ        && (endPos, gravityMult) => startVel
        duration, endPos      && (endZ, gravityMult  ) => startVel
        duration, startVel    && (gravityMult        ) => endZ, endPos
        duration, gravityMult && (startVel           ) => endZ, endPos
        endZ, startVel        && (gravityMult        ) => duration, endPos
        endZ, gravityMult     && (endPos, startVel   ) => duration
        endPos, gravityMult   && (endZ               ) => duration, startVel

        # three parameters
        duration, endZ, startVel        && (      ) => endPos, gravityMult
        duration, endZ, gravityMult     && (endPos) => startVel
        duration, endPos, gravityMult   && (endZ  ) => startVel
        duration, startVel, gravityMult && (      ) => endZ, endPos
        endZ, startVel, gravityMult     && (      ) => duration, endPos

        # four parameters
        duration, endZ, startVel, gravityMult && () => endPos
        ##################################################################
        ##################################################################
"""
