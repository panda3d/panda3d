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
ClientConfigs = {
    'mono-modelcave-pipe0': [{'pos' : Vec3(0),
                              'hpr' : Vec3(0)}
                             ],
    'single-server'       : [{'pos' : Vec3(0),
                              'hpr' : Vec3(0)}
                             ],
    'two-server'          : [{'pos' : Vec3(0),
                              'hpr' : Vec3(-60,0,0)},
                             {'pos' : Vec3(0),
                              'hpr' : Vec3(60,0,0)}
                             ],
    'cavetest'            : [{'pos' : Vec3(-0.105, -0.020, 5.000),
                              'hpr' : Vec3(51.213, 0.000, 0.000),
                              'focal length' : 0.809,
                              'film size' : (1.000, 0.831),
                              'film offset' : (0.000, 0.173),
                              },
                             {'pos' : Vec3(-0.105, -0.020, 5.000),
                              'hpr' : Vec3(-0.370, 0.000, 0.000),
                              'focal length' : 0.815,
                              'film size' : (1.000, 0.831),
                              'film offset' : (0.000, 0.173),
                              },
                             {'pos' : Vec3(-0.105, -0.020, 5.000),
                              'hpr' : Vec3(-51.675, 0.000, 0.000),
                              'focal length' : 0.820,
                              'film size' : (1.000, 0.830),
                              'film offset' : (-0.000, 0.173),
                              },
                             {'pos' : Vec3(0.105, -0.020, 5.000),
                              'hpr' : Vec3(51.675, 0.000, 0.000),
                              'focal length' : 0.820,
                              'film size' : (1.000, 0.830),
                              'film offset' : (0.000, 0.173),
                              },
                             ],
    'cavetest-all'        : [{'pos' : Vec3(-0.105, -0.020, 5.000),
                              'hpr' : Vec3(51.213, 0.000, 0.000),
                              'focal length' : 0.809,
                              'film size' : (1.000, 0.831),
                              'film offset' : (0.000, 0.173),
                              },
                             {'pos' : Vec3(-0.105, -0.020, 5.000),
                              'hpr' : Vec3(-0.370, 0.000, 0.000),
                              'focal length' : 0.815,
                              'film size' : (1.000, 0.831),
                              'film offset' : (0.000, 0.173),
                              },
                             {'pos' : Vec3(-0.105, -0.020, 5.000),
                              'hpr' : Vec3(-51.675, 0.000, 0.000),
                              'focal length' : 0.820,
                              'film size' : (1.000, 0.830),
                              'film offset' : (-0.000, 0.173),
                              },
                             {'pos' : Vec3(0.105, -0.020, 5.000),
                              'hpr' : Vec3(51.675, 0.000, 0.000),
                              'focal length' : 0.820,
                              'film size' : (1.000, 0.830),
                              'film offset' : (0.000, 0.173),
                              },
                             {'pos' : Vec3(0.105, -0.020, 5.000),
                              'hpr' : Vec3(0.370, 0.000, 0.000),
                              'focal length' : 0.815,
                              'film size' : (1.000, 0.831),
                              'film offset' : (0.000, 0.173),
                              },
                             {'pos' : Vec3(0.105, -0.020, 5.000),
                              'hpr' : Vec3(-51.213, 0.000, 0.000),
                              'focal length' : 0.809,
                              'film size' : (1.000, 0.831),
                              'film offset' : (-0.000, 0.173),
                              },
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
    configList = ClientConfigs[clusterConfig]
    numConfigs = len(configList)
    for i in range(numConfigs):
        configData = configList[i]
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
            cci = ClusterConfigItem(
                serverConfigName,
                serverName,
                port)
            # Init Cam Offset
            pos = configData.get('pos', Vec3(0))
            hpr = configData.get('hpr', Vec3(0))
            cci.setCamOffset(pos, hpr)
            # Init Frustum if specified
            fl = configData.get('focalLength', None)
            fs = configData.get('filmSize', None)
            fo = configData.get('filmOffset', None)
            if fl and fs and fo:
                cci.setCamFrustum(fl, fs, fo)
            displayConfigs.append(cci)
    # Create Cluster Managers (opening connections to servers)
    # Are the servers going to be synced?
    synced = base.config.GetBool('sync-display', 0)
    if synced:
        base.win.setSync(1)
        return ClusterManagerSync(displayConfigs)
    else:
            return ClusterManager(displayConfigs)
    


