from PandaObject import *
from ClusterClient import *
import string

# this is an array of offsets for the various servers.  For example,
# mono-modelcave-pipe0 has one server with both a pos and hpr of 0.
# For mono-modelcave-pipe0, I decided to set the offsets in the
# actual configuration for the display.

# Specify offset from client for each server's camera group
# Note: If server chan-config consists of multiple display regions
# each display region can have an additional offset as specified in
# DirectCamConfig.py
ClientConfigs = {'mono-modelcave-pipe0': [ [Vec3(0),Vec3(0)] ],
                 'single-server': [ [Vec3(0),Vec3(0)] ],
                 'two-server'   : [ [Vec3(0),Vec3(-60,0,0)],
                                    [Vec3(0),Vec3(60,0,0)]
                                    ],
                 'cavetest'     : [ [Vec3(0), Vec3(0)],
                                    [Vec3(0), Vec3(0)],
                                    [Vec3(0), Vec3(0)],
                                    [Vec3(0), Vec3(0)]
                                    ],
                 }


def createClusterManager():
    # setup camera offsets based on cluster-config
    clusterConfig = base.config.GetString('cluster-config', 'single-server')
    # No cluster config specified!
    if not ClientConfigs.has_key(clusterConfig):
        base.notify.warning(
            'display-client flag set, but %s cluster-config is undefined.' %
            clusterConfig)
        return None
    # Get display config for each server in the cluster
    displayConfigs = []
    configData = ClientConfigs[clusterConfig]
    numConfigs = len(configData)
    for i in range(numConfigs):
        serverConfigName = 'display%d' % i
        serverString = base.config.GetString(serverConfigName, '')
        if serverString == '':
            base.notify.warning(
                '%s undefined in Configrc: expected by %s display client.' %
                (serverConfigName,clusterConfig))
            base.notify.warning('%s will not be used.' % serverConfigName)
        else:
            serverInfo = string.split(serverString)
            serverName = serverInfo[0]
            if len(serverInfo) > 1:
                port = int(serverInfo[1])
            else:
                # Use default port
                port = CLUSTER_PORT
            displayConfigs.append(ClusterConfigItem(
                serverConfigName,
                serverName,
                # Pos Offset
                configData[i][0],
                # Hpr Offset
                configData[i][1],
                port))
    # Create Cluster Managers (opening connections to servers)
    # Are the servers going to be synced?
    synced = base.config.GetBool('sync-display', 0)
    if synced:
        base.win.setSync(1)
        return ClusterManagerSync(displayConfigs)
    else:
            return ClusterManager(displayConfigs)
    


