

from direct.directnotify import DirectNotifyGlobal
from direct.showbase import DirectObject

# internal class, don't create these on your own
class InputStateToken:
    _SerialGen = SerialNumGen()
    Inval = 'invalidatedToken'
    def __init__(self, inputState):
        self._id = InputStateToken._SerialGen.next()
        self._hash = self._id
        self._inputState = inputState
    def release(self):
        # subclasses will override
        assert False
    def isValid(self):
        return self._id != InputStateToken.Inval
    def invalidate(self):
        self._id = InputStateToken.Inval
    def __hash__(self):
        return self._hash

class InputStateWatchToken(InputStateToken, DirectObject.DirectObject):
    def release(self):
        self._inputState._ignore(self)
        self.ignoreAll()
class InputStateForceToken(InputStateToken):
    def release(self):
        self._inputState._unforce(self)

class InputStateTokenGroup:
    def __init__(self):
        self._tokens = []
    def addToken(self, token):
        self._tokens.append(token)
    def release(self):
        for token in self._tokens:
            token.release()
        self._tokens = []

class InputState(DirectObject.DirectObject):
    """
    InputState is for tracking the on/off state of some events.
    The initial usage is to watch some keyboard keys so that another
    task can poll the key states.  By the way, in general polling is
    not a good idea, but it is useful in some situations.  Know when
    to use it:)  If in doubt, don't use this class and listen for
    events instead.
    """

    notify = DirectNotifyGlobal.directNotify.newCategory("InputState")

    # standard input sources
    WASD = 'WASD'
    QE = 'QE'
    ArrowKeys = 'ArrowKeys'
    Keyboard = 'Keyboard'
    Mouse = 'Mouse'

    def __init__(self):
        # stateName->set(SourceNames)
        self._state = {}
        # stateName->set(SourceNames)
        self._forcingOn = {}
        # stateName->set(SourceNames)
        self._forcingOff = {}
        # tables to look up the info needed to undo operations
        self._token2inputSource = {}
        self._token2forceInfo = {}
        # inputSource->token->(name, eventOn, eventOff)
        self._watching = {}
        assert self.debugPrint("InputState()")

    def delete(self):
        del self._watching
        del self._token2forceInfo
        del self._token2inputSource
        del self._forcingOff
        del self._forcingOn
        del self._state
        self.ignoreAll()

    def isSet(self, name, inputSource=None):
        """
        returns True/False
        """
        #assert self.debugPrint("isSet(name=%s)"%(name))
        if name in self._forcingOn:
            return True
        elif name in self._forcingOff:
            return False
        if inputSource:
            s = self._state.get(name)
            if s:
                return inputSource in s
            else:
                return False
        else:
            return name in self._state

    def getEventName(self, name):
        return "InputState-%s" % (name,)

    def set(self, name, isActive, inputSource=None):
        assert self.debugPrint("set(name=%s, isActive=%s, inputSource=%s)"%(name, isActive, inputSource))
        # inputSource is a string that identifies where this input change
        # is coming from (like 'WASD', 'ArrowKeys', etc.)
        # Each unique inputSource is allowed to influence this input item
        # once: it's either 'active' or 'not active'. If at least one source
        # activates this input item, the input item is considered to be active
        if inputSource is None:
            inputSource = 'anonymous'
        if isActive:
            self._state.setdefault(name, set())
            self._state[name].add(inputSource)
        else:
            if name in self._state:
                self._state[name].discard(inputSource)
                if len(self._state[name]) == 0:
                    del self._state[name]
        # We change the name before sending it because this may
        # be the same name that messenger used to call InputState.set()
        # this avoids running in circles:
        messenger.send(self.getEventName(name), [self.isSet(name)])

    def releaseInputs(self, name):
        # call this to act as if all inputs affecting this state have been released
        del self._state[name]

    def watch(self, name, eventOn, eventOff, startState=False, inputSource=None):
        """
        This returns a token; hold onto the token and call token.release() when you
        no longer want to watch for these events.

        # set up
        token = inputState.watch('forward', 'w', 'w-up', inputSource=inputState.WASD)
         ...
        # tear down
        token.release()
        """
        assert self.debugPrint(
            "watch(name=%s, eventOn=%s, eventOff=%s, startState=%s)"%(
            name, eventOn, eventOff, startState))
        if inputSource is None:
            inputSource = "eventPair('%s','%s')" % (eventOn, eventOff)
        # Do we really need to reset the input state just because
        # we're watching it?  Remember, there may be multiple things
        # watching this input state.
        self.set(name, startState, inputSource)
        token = InputStateWatchToken(self)
        # make the token listen for the events, to allow multiple listeners for the same event
        token.accept(eventOn, self.set, [name, True, inputSource])
        token.accept(eventOff, self.set, [name, False, inputSource])
        self._token2inputSource[token] = inputSource
        self._watching.setdefault(inputSource, {})
        self._watching[inputSource][token] = (name, eventOn, eventOff)
        return token

    def watchWithModifiers(self, name, event, startState=False, inputSource=None):
        patterns = ('%s', 'control-%s', 'shift-control-%s', 'alt-%s',
                    'control-alt-%s', 'shift-%s', 'shift-alt-%s')
        tGroup = InputStateTokenGroup()
        for pattern in patterns:
            tGroup.addToken(self.watch(name, pattern % event, '%s-up' % event, startState=startState, inputSource=inputSource))
        return tGroup

    def _ignore(self, token):
        """
        Undo a watch(). Don't call this directly, call release() on the token that watch() returned.
        """
        inputSource = self._token2inputSource.pop(token)
        name, eventOn, eventOff = self._watching[inputSource].pop(token)
        token.invalidate()
        DirectObject.DirectObject.ignore(self, eventOn)
        DirectObject.DirectObject.ignore(self, eventOff)
        if len(self._watching[inputSource]) == 0:
            del self._watching[inputSource]

        # I commented this out because we shouldn't be modifying an
        # input state simply because we're not looking at it anymore.
        # self.set(name, False, inputSource)


    def force(self, name, value, inputSource):
        """
        Force isSet(name) to return 'value'.

        This returns a token; hold onto the token and call token.release() when you
        no longer want to force the state.

        example:
        # set up
        token=inputState.force('forward', True, inputSource='myForwardForcer')
         ...
        # tear down
        token.release()
        """
        token = InputStateForceToken(self)
        self._token2forceInfo[token] = (name, inputSource)
        if value:
            if name in self._forcingOff:
                self.notify.error(
                    "%s is trying to force '%s' to ON, but '%s' is already being forced OFF by %s" %
                    (inputSource, name, name, self._forcingOff[name])
                    )
            self._forcingOn.setdefault(name, set())
            self._forcingOn[name].add(inputSource)
        else:
            if name in self._forcingOn:
                self.notify.error(
                    "%s is trying to force '%s' to OFF, but '%s' is already being forced ON by %s" %
                    (inputSource, name, name, self._forcingOn[name])
                    )
            self._forcingOff.setdefault(name, set())
            self._forcingOff[name].add(inputSource)
        return token

    def _unforce(self, token):
        """
        Stop forcing a value. Don't call this directly, call release() on your token.
        """
        name, inputSource = self._token2forceInfo[token]
        token.invalidate()
        if name in self._forcingOn:
            self._forcingOn[name].discard(inputSource)
            if len(self._forcingOn[name]) == 0:
                del self._forcingOn[name]
        if name in self._forcingOff:
            self._forcingOff[name].discard(inputSource)
            if len(self._forcingOff[name]) == 0:
                del self._forcingOff[name]

    def debugPrint(self, message):
        """for debugging"""
        return self.notify.debug(
            "%s (%s) %s"%(id(self), len(self._state), message))
