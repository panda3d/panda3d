"""EditorGlobals module: contains global editor data"""

from PythonUtil import uniqueElements

EntIdRange = 10000
username2entIdBase = {
    'darren': 1*EntIdRange,
    'samir':  2*EntIdRange,
    'skyler': 3*EntIdRange,
    'joe':    4*EntIdRange,
    'mark':   5*EntIdRange,
    }
assert uniqueElements(username2entIdBase.values())

usernameConfigVar = 'level-edit-username'
undefinedUsername = 'UNDEFINED_USERNAME'
editUsername = config.GetString(usernameConfigVar, undefinedUsername)

# call this to make sure things have been set up correctly
def assertReadyToEdit():
    assert editUsername != undefinedUsername, (
        "you must config '%s'" % usernameConfigVar)
    # Feel free to add your name to the table if it's not in there
    assert editUsername in username2entIdBase, (
        "unknown editor username '%s'" % username)

def getEntIdAllocRange():
    """range of valid entId values for this user.
    returns [min, max+1] (values taken by range() and xrange())"""
    baseId = username2entIdBase[editUsername]
    return [baseId, baseId+EntIdRange]
