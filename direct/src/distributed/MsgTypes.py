"""MsgTypes module: contains distributed object message types"""

from direct.showbase.PythonUtil import invertDictLossless

MsgName2Id = {
    'CLIENT_HELLO':                                  1,
    'CLIENT_HELLO_RESP':                             2,

    # Sent by the server when it is dropping the connection deliberately.
    'CLIENT_GO_GET_LOST':                            4,

    'CLIENT_OBJECT_UPDATE_FIELD':                    24,
    'CLIENT_OBJECT_UPDATE_FIELD_RESP':               24,
    'CLIENT_OBJECT_DISABLE':                         25,
    'CLIENT_OBJECT_DISABLE_RESP':                    25,
    'CLIENT_OBJECT_DISABLE_OWNER':                   26,
    'CLIENT_OBJECT_DISABLE_OWNER_RESP':              26,
    'CLIENT_OBJECT_DELETE':                          27,
    'CLIENT_OBJECT_DELETE_RESP':                     27,
    'CLIENT_SET_ZONE_CMU':                           29,
    'CLIENT_REMOVE_ZONE':                            30,
    'CLIENT_SET_AVATAR':                             32,
    'CLIENT_CREATE_OBJECT_REQUIRED':                 34,
    'CLIENT_CREATE_OBJECT_REQUIRED_RESP':            34,
    'CLIENT_CREATE_OBJECT_REQUIRED_OTHER':           35,
    'CLIENT_CREATE_OBJECT_REQUIRED_OTHER_RESP':      35,
    'CLIENT_CREATE_OBJECT_REQUIRED_OTHER_OWNER':     36,
    'CLIENT_CREATE_OBJECT_REQUIRED_OTHER_OWNER_RESP':36,

    'CLIENT_REQUEST_GENERATES':                      36,

    'CLIENT_DISCONNECT':                             37,

    'CLIENT_DONE_INTEREST_RESP':                     48,

    'CLIENT_HEARTBEAT':                              52,

    'CLIENT_SET_DOID_RANGE':                         74,

    'CLIENT_ADD_INTEREST':                           97,
    'CLIENT_REMOVE_INTEREST':                        99,
    'CLIENT_OBJECT_LOCATION':                        102,


    # These are sent internally inside the Astron cluster.

    # Message Director control messages:
    'CONTROL_CHANNEL':                                  4001,
    'CONTROL_ADD_CHANNEL':                              2001,
    'CONTROL_REMOVE_CHANNEL':                           2002,
    'CONTROL_ADD_RANGE':                                2008,
    'CONTROL_REMOVE_RANGE':                             2009,
    'CONTROL_ADD_POST_REMOVE':                          2010,
    'CONTROL_CLEAR_POST_REMOVE':                        2011,

    # State Server control messages:
    'STATESERVER_OBJECT_GENERATE_WITH_REQUIRED':        2001,
    'STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER':  2003,
    'STATESERVER_OBJECT_UPDATE_FIELD':                  2004,


    }

# create id->name table for debugging
MsgId2Names = invertDictLossless(MsgName2Id)
    
# put msg names in module scope, assigned to msg value
for name, value in MsgName2Id.items():
    exec '%s = %s' % (name, value)
del name, value

# These messages are ignored when the client is headed to the quiet zone
QUIET_ZONE_IGNORED_LIST = [

    # We mustn't ignore updates, because some updates for localToon
    # are always important.
    #CLIENT_OBJECT_UPDATE_FIELD,
    
    # These are now handled. If it is a create for a class that is in the
    # uber zone, we should create it.
    #CLIENT_CREATE_OBJECT_REQUIRED,
    #CLIENT_CREATE_OBJECT_REQUIRED_OTHER,

    ]

# The following is a different set of numbers from above.
# These are the sub-message types for CLIENT_LOGIN_2.
CLIENT_LOGIN_2_GREEN = 1       # Disney's GoReg subscription token, not used.
CLIENT_LOGIN_2_PLAY_TOKEN = 2  # VR Studio PlayToken.
CLIENT_LOGIN_2_BLUE = 3        # The international GoReg token.
CLIENT_LOGIN_3_DISL_TOKEN = 4  # SSL encoded blob from DISL system.
