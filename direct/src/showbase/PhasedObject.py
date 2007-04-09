from direct.directnotify.DirectNotifyGlobal import *

class PhasedObject:
    """
    This class is governs the loading and unloading of successive
    phases in an ordered and automatic manner.

    An object can only have one phase at any given moment. At the
    completion of setPhase() the current and all previous phases are
    guaranteed to be loaded, while all later phases are guaranteed
    to be unloaded.

    In order to define a phase, simply define the functions:
    loadPhase<#> and unloadPhase<#> where # corresponds to the number
    of the phase to be defined and # >= 0.

    You also have the ability to define alias for phases so that
    your function definitions are more descriptive.  The way to do
    this is to provide an aliasMap to __init__().  The aliasMap is
    of the form {'alias':#, ...}. You can then call setPhase() with
    this alias as well.

    So for example, if you wanted to alias phase 0 to 'Far' you
    would define loadPhaseFar() and unloadPhaseFar(). Upon calling
    setPhase(0), setPhase('Far'), setPhase(<any phase greater than 0>),
    or setPhase(<any alias greater than 'Far'>), loadPhaseFar() will
    be invoked.

    For a skeleton example class, see the AnfaPhasedObject class
    definition lower in this file.
    """
    notify = directNotify.newCategory("PhasedObject")
    
    def __init__(self, aliasMap = {}):
        self.phase = -1
        self.phaseAliasMap = {}
        self.aliasPhaseMap = {}
        self.__phasing = False

        for alias,phase in aliasMap.items():
            self.setAlias(phase, alias)

    def __repr__(self):
        return 'PhasedObject(%s)' % str(self.aliasPhaseMap)

    def __str__(self):
        outStr = PhasedObject.__repr__(self)
        outStr += ' in phase \'%s\'' % self.getPhase()
        return outStr
        
    def setAlias(self, phase, alias):
        """
        Map an alias to a phase number.

        phase must be >= 0 and alias must be a string
        of characters suitable for python variable names.
        
        The mapping must be one-to-one.        
        """
        assert isinstance(phase,int) and phase >= 0
        assert isinstance(alias,str)
        
        self.phaseAliasMap[phase] = alias
        self.aliasPhaseMap[alias] = phase

    def getPhaseAlias(self, phase):
        """
        Returns the alias of a phase number, if it exists.
        Otherwise, returns the phase number.
        """
        return self.phaseAliasMap.get(phase, phase)
    
    def getAliasPhase(self, alias):
        """
        Returns the phase number of an alias, if it exists.
        Otherwise, returns the alias.
        """
        return self.aliasPhaseMap.get(alias, alias)
        
    def getPhase(self):
        """
        Returns the current phase (or alias, if defined)
        this object is currently in.
        """
        return self.getPhaseAlias(self.phase)

    def setPhase(self, aPhase):
        """
        aPhase can be either a phase number or a predefined alias.

        Will invoke a sequence of loadPhase*() or unloadPhase*()
        functions corresponding to the difference between the current
        phase and aPhase, starting at the current phase.
        """
        assert not self.__phasing, 'Already phasing. Cannot setPhase() while phasing in progress.'
        self.__phasing = True
        
        phase = self.aliasPhaseMap.get(aPhase,aPhase)
        assert isinstance(phase,int), 'Phase alias \'%s\' not found' % aPhase
        assert phase >= -1, 'Invalid phase number \'%s\'' % phase
        
        if phase > self.phase:
            for x in range(self.phase + 1, phase + 1):
                self.__loadPhase(x)
        elif phase < self.phase:
            for x in range(self.phase, phase, -1):
                self.__unloadPhase(x)

        self.__phasing = False

    def cleanup(self):
        """
        Will force the unloading, in correct order, of all currently
        loaded phases.
        """
        if self.phase >= 0:
            self.setPhase(-1)

    def __loadPhase(self, phase):
        aPhase = self.phaseAliasMap.get(phase,phase)
        getattr(self, 'loadPhase%s' % aPhase,
                lambda: self.__phaseNotFound('load',aPhase))()
        self.phase = phase

    def __unloadPhase(self, phase):
        aPhase = self.phaseAliasMap.get(phase,phase)
        getattr(self, 'unloadPhase%s' % aPhase,
                lambda: self.__phaseNotFound('unload',aPhase))()
        self.phase = (phase - 1)

    def __phaseNotFound(self, mode, aPhase):
        assert self.notify.debug('%s%s() not found!\n' % (mode,aPhase))
        
if __debug__:
    class AnfaPhasedObject(PhasedObject):
        """
        This is an example class to demonstrate the concept of
        alias mapping for PhasedObjects.

        As the distance between an observer and this object closes,
        we would set the phase level succesively higher, with an initial
        phase of 'Away' being set in __init__:

        setPhase('Far') -> invokes loadPhaseFar()
        setPhase('Near') -> invokes loadPhaseNear()

        Now let's say the objects start moving away from each other:

        setPhase('Far') -> invokes unloadPhaseNear()
        setPhase('Away') -> invokes unloadPhaseFar()

        Now one object teleports to the other:

        setPhase('At') -> invokes loadPhase('Far'),
                          then    loadPhase('Near'),
                          then    loadPhase('At')

        Now the phased object is destroyed, we must clean it up
        before removal:

        cleanup() -> invokes unloadPhase('At')
                     then    unloadPhase('Near')
                     then    unloadPhase('Far')
                     then    unloadPhase('Away')
        """
        def __init__(self):
            PhasedObject.__init__(self, {'At':3, 'Near':2, 'Far':1, 'Away':0})
            self.setPhase('Away')
            
        def loadPhaseAway(self):
            print 'loading Away'

        def unloadPhaseAway(self):
            print 'unloading Away'
                                
        def loadPhaseFar(self):
            print 'loading Far'

        def unloadPhaseFar(self):
            print 'unloading Far'
            
        def loadPhaseNear(self):
            print 'loading Near'
        
        def unloadPhaseNear(self):
            print 'unloading Near'
        
        def loadPhaseAt(self):
            print 'loading At'
        
        def unloadPhaseAt(self):
            print 'unloading At'
        
