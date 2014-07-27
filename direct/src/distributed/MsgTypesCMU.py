""" MsgTypesCMU module: defines the various message type codes as used
by the CMU ServerRepository/ClientRepository code in this directory.
It replaces the MsgTypes module, which is not used by the CMU
implementation. """

from direct.showbase.PythonUtil import invertDictLossless

MsgName2Id = {
    'SET_DOID_RANGE_CMU'                      : 9001,
    'CLIENT_OBJECT_GENERATE_CMU'              : 9002,
    'OBJECT_GENERATE_CMU'                     : 9003,
    'OBJECT_UPDATE_FIELD_CMU'                 : 9004,
    'OBJECT_DISABLE_CMU'                      : 9005,
    'OBJECT_DELETE_CMU'                       : 9006,
    'REQUEST_GENERATES_CMU'                   : 9007,
    'CLIENT_DISCONNECT_CMU'                   : 9008,
    'CLIENT_SET_INTEREST_CMU'                 : 9009,
    'OBJECT_SET_ZONE_CMU'                     : 9010,
    'CLIENT_HEARTBEAT_CMU'                    : 9011,
    'CLIENT_OBJECT_UPDATE_FIELD_TARGETED_CMU'  : 9011,

    'CLIENT_OBJECT_UPDATE_FIELD' : 24,  # Matches MsgTypes.CLIENT_OBJECT_UPDATE_FIELD
    }

# create id->name table for debugging
MsgId2Names = invertDictLossless(MsgName2Id)
    
# put msg names in module scope, assigned to msg value
globals().update(MsgName2Id)
