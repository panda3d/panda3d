"""MsgTypes module: contains distributed object message types"""

# 2 new params: passwd, char bool 0/1 1 = new account
# 2 new return values: 129 = not found, 12 = bad passwd, 
CLIENT_LOGIN =                                1
CLIENT_LOGIN_RESP =                           2
CLIENT_GET_AVATARS =                          3
# Sent by the server when it is dropping the connection deliberately.
CLIENT_GO_GET_LOST =                          4
CLIENT_GET_AVATARS_RESP =                     5
CLIENT_CREATE_AVATAR =                        6
CLIENT_CREATE_AVATAR_RESP =                   7
CLIENT_GET_SHARD_LIST =                       8
CLIENT_GET_SHARD_LIST_RESP =                  9
CLIENT_GET_FRIEND_LIST =                     10
CLIENT_GET_FRIEND_LIST_RESP =                11
CLIENT_GET_FRIEND_DETAILS =                  12
CLIENT_GET_FRIEND_DETAILS_RESP =             13
CLIENT_GET_AVATAR_DETAILS =                  14
CLIENT_GET_AVATAR_DETAILS_RESP =             15
CLIENT_LOGIN_2 =                             16
CLIENT_LOGIN_2_RESP =                        17

CLIENT_OBJECT_UPDATE_FIELD =                 24
CLIENT_OBJECT_UPDATE_FIELD_RESP =            24
CLIENT_OBJECT_DISABLE_RESP =                 25
CLIENT_OBJECT_DELETE_RESP =                  27
CLIENT_SET_ZONE =                            29
CLIENT_SET_SHARD =                           31
CLIENT_SET_AVATAR =                          32
CLIENT_CREATE_OBJECT_REQUIRED =              34
CLIENT_CREATE_OBJECT_REQUIRED_OTHER =        35

CLIENT_DISCONNECT =                          37

CLIENT_CHANGE_IP_ADDRESS_RESP =              45
CLIENT_GET_STATE =                           46
CLIENT_GET_STATE_RESP =                      47
CLIENT_DONE_SET_ZONE_RESP =                  48
CLIENT_DELETE_AVATAR =                       49
CLIENT_DELETE_AVATAR_RESP =                  50

CLIENT_HEARTBEAT =                           52
CLIENT_FRIEND_ONLINE =                       53
CLIENT_FRIEND_OFFLINE =                      54
CLIENT_REMOVE_FRIEND =                       56

CLIENT_SERVER_UP =                           57
CLIENT_SERVER_DOWN =                         58

CLIENT_CHANGE_PASSWORD =                     65

CLIENT_SET_NAME_PATTERN =                    67
CLIENT_SET_NAME_PATTERN_ANSWER =             68

CLIENT_SET_WISHNAME =                        70
CLIENT_SET_WISHNAME_RESP =                   71
CLIENT_SET_WISHNAME_CLEAR =                  72
CLIENT_SET_SECURITY =                        73

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
