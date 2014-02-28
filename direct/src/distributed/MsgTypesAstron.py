"""MsgTypesAstron module: contains distributed object message types"""

from direct.showbase.PythonUtil import invertDictLossless

MsgName2Id = {
    'CLIENT_HELLO':                               1,
    'CLIENT_HELLO_RESP':                          2,
    'CLIENT_DISCONNECT':                          3,
    'CLIENT_EJECT':                               4,
    'CLIENT_HEARTBEAT':                           5,
    'CLIENT_ENTER_OBJECT_REQUIRED':             142,
    'CLIENT_ENTER_OBJECT_REQUIRED_OTHER':       143,
    'CLIENT_ENTER_OBJECT_REQUIRED_OWNER':       172,
    'CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER': 173,
    'CLIENT_OBJECT_SET_FIELD':                  120,
    'CLIENT_OBJECT_SET_FIELDS':                 121,
    'CLIENT_OBJECT_LEAVING':                    132,
    'CLIENT_OBJECT_LOCATION':                   140,
    'CLIENT_ADD_INTEREST':                      200,
    'CLIENT_ADD_INTEREST_MULTIPLE':             201,
    'CLIENT_REMOVE_INTEREST':                   203,
    'CLIENT_DONE_INTEREST_RESP':                204,
    }

# create id->name table for debugging
MsgId2Names = invertDictLossless(MsgName2Id)
    
# put msg names in module scope, assigned to msg value
for name, value in MsgName2Id.items():
    exec '%s = %s' % (name, value)
del name, value

