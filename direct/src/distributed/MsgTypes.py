"""MsgTypes module: contains distributed object message types"""

from direct.showbase.PythonUtil import invertDictLossless

MsgName2Id = {
    # 2 new params: passwd, char bool 0/1 1 = new account
    # 2 new return values: 129 = not found, 12 = bad passwd, 
    'CLIENT_LOGIN':                                  1,
    'CLIENT_LOGIN_RESP':                             2,
    'CLIENT_GET_AVATARS':                            3,
    # Sent by the server when it is dropping the connection deliberately.
    'CLIENT_GO_GET_LOST':                            4,
    'CLIENT_GET_AVATARS_RESP':                       5,
    'CLIENT_CREATE_AVATAR':                          6,
    'CLIENT_CREATE_AVATAR_RESP':                     7,
    'CLIENT_GET_FRIEND_LIST':                        10,
    'CLIENT_GET_FRIEND_LIST_RESP':                   11,
    'CLIENT_GET_AVATAR_DETAILS':                     14,
    'CLIENT_GET_AVATAR_DETAILS_RESP':                15,
    'CLIENT_LOGIN_2':                                16,
    'CLIENT_LOGIN_2_RESP':                           17,

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

    'CLIENT_GET_STATE_RESP':                         47,
    'CLIENT_DONE_INTEREST_RESP':                     48,

    'CLIENT_DELETE_AVATAR':                          49,

    'CLIENT_DELETE_AVATAR_RESP':                     5,

    'CLIENT_HEARTBEAT':                              52,
    'CLIENT_FRIEND_ONLINE':                          53,
    'CLIENT_FRIEND_OFFLINE':                         54,
    'CLIENT_REMOVE_FRIEND':                          56,

    'CLIENT_CHANGE_PASSWORD':                        65,

    'CLIENT_SET_NAME_PATTERN':                       67,
    'CLIENT_SET_NAME_PATTERN_ANSWER':                68,

    'CLIENT_SET_WISHNAME':                           70,
    'CLIENT_SET_WISHNAME_RESP':                      71,
    'CLIENT_SET_WISHNAME_CLEAR':                     72,
    'CLIENT_SET_SECURITY':                           73,

    'CLIENT_SET_DOID_RANGE':                         74,

    'CLIENT_GET_AVATARS_RESP2':                      75,
    'CLIENT_CREATE_AVATAR2':                         76,
    'CLIENT_SYSTEM_MESSAGE':                         78,
    'CLIENT_SET_AVTYPE':                             80,

    'CLIENT_GET_PET_DETAILS':                        81,
    'CLIENT_GET_PET_DETAILS_RESP':                   82,

    'CLIENT_ADD_INTEREST':                           97,
    'CLIENT_REMOVE_INTEREST':                        99,
    'CLIENT_OBJECT_LOCATION':                        102,

    'CLIENT_LOGIN_3':                                111,
    'CLIENT_LOGIN_3_RESP':                           110,

    'CLIENT_GET_FRIEND_LIST_EXTENDED':               115,
    'CLIENT_GET_FRIEND_LIST_EXTENDED_RESP':          116,

    'CLIENT_SET_FIELD_SENDABLE':                     120,
    
    'CLIENT_SYSTEMMESSAGE_AKNOWLEDGE':               123,
    'CLIENT_CHANGE_GENERATE_ORDER':                  124,

    # new toontown specific login message, adds last logged in, and if child account has parent acount
    'CLIENT_LOGIN_TOONTOWN':                         125,
    'CLIENT_LOGIN_TOONTOWN_RESP':                    126,  
    
    

    'STATESERVER_OBJECT_GENERATE_WITH_REQUIRED':           2001,
    'STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER':     2003,
    'STATESERVER_OBJECT_UPDATE_FIELD':                     2004,
    'STATESERVER_OBJECT_CREATE_WITH_REQUIRED_CONTEXT':     2050,
    'STATESERVER_OBJECT_CREATE_WITH_REQUIR_OTHER_CONTEXT': 2051,
    'STATESERVER_BOUNCE_MESSAGE':                          2086,
    }

# create id->name table for debugging
MsgId2Names = invertDictLossless(MsgName2Id)
    
# put msg names in module scope, assigned to msg value
globals().update(MsgName2Id)

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
