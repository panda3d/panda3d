"""EditorGlobals module: contains global editor data"""

from direct.showbase.PythonUtil import uniqueElements

# levels should put themselves into the bboard under this posting
# to assert themselves as the level to be edited by ~edit
EditTargetPostName = 'inGameEditTarget'

EntIdRange = 10000
# Once a range has been assigned to a user, please don't change it.
username2entIdBase = {
    'darren':   1*EntIdRange,
    'samir':    2*EntIdRange,
    'skyler':   3*EntIdRange,
    'joe':      4*EntIdRange,
    'DrEvil':   5*EntIdRange,
    'asad':     6*EntIdRange,
    'drose':    7*EntIdRange,
    'pappy':    8*EntIdRange,
    'patricia': 9*EntIdRange,
    }
assert uniqueElements(username2entIdBase.values())

usernameConfigVar = 'level-edit-username'
undefinedUsername = 'UNDEFINED_USERNAME'
editUsername = config.GetString(usernameConfigVar, undefinedUsername)

# call this to make sure things have been set up correctly
def checkNotReadyToEdit():
    # returns error string if not ready, None if ready
    if editUsername == undefinedUsername:
        return "you must config '%s'; see %s.py" % (
            usernameConfigVar, __name__)
    # Feel free to add your name to the table if it's not in there
    if editUsername not in username2entIdBase:
        return "unknown editor username '%s'; see %s.py" % (
            editUsername, __name__)
    return None

def assertReadyToEdit():
    msg = checkNotReadyToEdit()
    if msg is not None:
        assert False, msg

def getEditUsername():
    return editUsername

def getEntIdAllocRange():
    """range of valid entId values for this user.
    returns [min, max+1] (values taken by range() and xrange())"""
    baseId = username2entIdBase[editUsername]
    return [baseId, baseId+EntIdRange]
