"""MsgTypes module: contains distributed object message types"""

CLIENT_OBJECT_UPDATE_FIELD =                 24
CLIENT_OBJECT_UPDATE_FIELD_RESP =            24
CLIENT_OBJECT_DISABLE_RESP =                 25
CLIENT_OBJECT_DELETE_RESP =                  27
CLIENT_SET_ZONE =                            29
CLIENT_SET_SHARD =                           31
CLIENT_CREATE_OBJECT_REQUIRED =              34
CLIENT_CREATE_OBJECT_REQUIRED_OTHER =        35

# These messages are ignored when the client is headed to the quiet zone
QUIET_ZONE_IGNORED_LIST = [
    CLIENT_OBJECT_UPDATE_FIELD,
    CLIENT_CREATE_OBJECT_REQUIRED,
    CLIENT_CREATE_OBJECT_REQUIRED_OTHER,
    ]
