"""MsgTypes module: contains distributed object message types"""

# Sent by the server when it is dropping the connection deliberately.
CLIENT_GO_GET_LOST =                          4

CLIENT_OBJECT_UPDATE_FIELD =                 24
CLIENT_OBJECT_UPDATE_FIELD_RESP =            24
CLIENT_OBJECT_DISABLE_RESP =                 25
CLIENT_OBJECT_DELETE_RESP =                  27
CLIENT_SET_ZONE =                            29
CLIENT_SET_SHARD =                           31
CLIENT_CREATE_OBJECT_REQUIRED =              34
CLIENT_CREATE_OBJECT_REQUIRED_OTHER =        35

CLIENT_HEARTBEAT =                           52

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
