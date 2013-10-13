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
    'STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE':         2005,
    'STATESERVER_OBJECT_DELETE_RAM':                    2007,
    'STATESERVER_OBJECT_SET_ZONE':                      2008,
    'STATESERVER_OBJECT_CHANGE_ZONE':                   2009,
    'STATESERVER_OBJECT_LOCATE':                        2022,
    'STATESERVER_OBJECT_LOCATE_RESP':                   2023,
    'STATESERVER_OBJECT_SET_AI_CHANNEL':                2045,
    'STATESERVER_OBJECT_QUERY_ALL':                     2020,
    'STATESERVER_OBJECT_QUERY_FIELD':                   2024,
    'STATESERVER_OBJECT_QUERY_FIELD_RESP':              2062,
    'STATESERVER_OBJECT_QUERY_FIELDS':                  2080,
    'STATESERVER_OBJECT_QUERY_FIELDS_RESP':             2081,
    'STATESERVER_OBJECT_QUERY_ALL_RESP':                2030,
    'STATESERVER_OBJECT_QUERY_ZONE_ALL':                2021,
    'STATESERVER_OBJECT_QUERY_ZONE_ALL_DONE':           2046,
    'STATESERVER_SHARD_RESET':                          2061,
    'STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED':       2065,
    'STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER': 2066,
    'STATESERVER_OBJECT_ENTER_AI_RECV':                 2067,
    'STATESERVER_OBJECT_SET_OWNER_RECV':                2070,
    'STATESERVER_OBJECT_CHANGE_OWNER_RECV':             2069,
    'STATESERVER_OBJECT_ENTER_OWNER_RECV':              2068,
    'STATESERVER_OBJECT_LEAVING_AI_INTEREST':           2033,
    'STATESERVER_OBJECT_QUERY_MANAGING_AI':             2083,
    'STATESERVER_OBJECT_NOTIFY_MANAGING_AI':            2047,

    # Database Server control messages:
    'DBSERVER_OBJECT_CREATE':                           4000,
    'DBSERVER_OBJECT_CREATE_RESP':                      4001,
    'DBSERVER_OBJECT_DELETE':                           4002,
    'DBSERVER_OBJECT_GET_FIELD':                        4010,
    'DBSERVER_OBJECT_GET_FIELD_RESP':                   4011,
    'DBSERVER_OBJECT_GET_FIELDS':                       4012,
    'DBSERVER_OBJECT_GET_FIELDS_RESP':                  4013,
    'DBSERVER_OBJECT_GET_ALL':                          4014,
    'DBSERVER_OBJECT_GET_ALL_RESP':                     4015,
    'DBSERVER_OBJECT_SET_FIELD':                        4020,
    'DBSERVER_OBJECT_SET_FIELDS':                       4021,
    'DBSERVER_OBJECT_SET_FIELD_IF_EQUALS':              4022,
    'DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP':         4023,
    'DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS':             4024,
    'DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP':        4025,
    'DBSERVER_OBJECT_SET_FIELD_IF_EMPTY':               4026,
    'DBSERVER_OBJECT_SET_FIELD_IF_EMPTY_RESP':          4027,
    'DBSERVER_OBJECT_DELETE_FIELD':                     4030,
    'DBSERVER_OBJECT_DELETE_FIELDS':                    4031,

    # Client Agent control messages:
    'CLIENTAGENT_OPEN_CHANNEL':                         3104,
    'CLIENTAGENT_CLOSE_CHANNEL':                        3105,
    'CLIENTAGENT_ADD_INTEREST':                         3106,
    'CLIENTAGENT_REMOVE_INTEREST':                      3107,
    'CLIENTAGENT_ADD_POST_REMOVE':                      3108,
    'CLIENTAGENT_CLEAR_POST_REMOVE':                    3109,
    'CLIENTAGENT_DISCONNECT':                           3101,
    'CLIENTAGENT_DROP':                                 3102,
    'CLIENTAGENT_SEND_DATAGRAM':                        3100,
    'CLIENTAGENT_SET_SENDER_ID':                        3103,
    'CLIENTAGENT_SET_STATE':                            3110,
    'CLIENTAGENT_ADD_SESSION_OBJECT':                   3112,
    'CLIENTAGENT_REMOVE_SESSION_OBJECT':                3113,
    'CLIENTAGENT_DECLARE_OBJECT':                       3114,
    'CLIENTAGENT_UNDECLARE_OBJECT':                     3115,
    'CLIENTAGENT_SET_FIELDS_SENDABLE':                  3111,
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
