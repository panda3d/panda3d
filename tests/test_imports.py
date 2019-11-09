# These tests import Panda3D modules just to make sure that there are no
# missing imports.  It is useful for a quick and dirty test to make sure
# that there are no obvious build issues.
import pytest

# This will print out imports on the command line.
#import direct.showbase.VerboseImport



def test_imports_panda3d():
    import imp, importlib, os
    import panda3d
    dir = os.path.dirname(panda3d.__file__)

    # Iterate over the things in the panda3d package that look like modules.
    extensions = set()
    for suffix in imp.get_suffixes():
        extensions.add(suffix[0])

    for basename in os.listdir(dir):
        if basename.startswith('lib'):
            # This not a Python module.
            continue

        module = basename.split('.', 1)[0]
        ext = basename[len(module):]

        if ext in extensions:
            importlib.import_module('panda3d.%s' % (module))


def test_imports_direct():
    import direct.actor.Actor
    import direct.actor.DistributedActor
    import direct.cluster.ClusterClient
    import direct.cluster.ClusterConfig
    import direct.cluster.ClusterMsgs
    import direct.cluster.ClusterServer
    import direct.controls.BattleWalker
    import direct.controls.ControlManager
    import direct.controls.DevWalker
    import direct.controls.GhostWalker
    import direct.controls.GravityWalker
    import direct.controls.InputState
    import direct.controls.NonPhysicsWalker
    import direct.controls.ObserverWalker
    import direct.controls.PhysicsWalker
    import direct.controls.SwimWalker
    import direct.controls.TwoDWalker
    import direct.directnotify.DirectNotify
    import direct.directnotify.DirectNotifyGlobal
    import direct.directnotify.Logger
    import direct.directnotify.LoggerGlobal
    import direct.directnotify.Notifier
    import direct.directnotify.RotatingLog
    import direct.directtools.DirectCameraControl
    import direct.directtools.DirectGeometry
    import direct.directtools.DirectGlobals
    import direct.directtools.DirectGrid
    import direct.directtools.DirectLights
    import direct.directtools.DirectManipulation
    import direct.directtools.DirectSelection
    import direct.directtools.DirectUtil
    import direct.directutil.DeltaProfiler
    import direct.directutil.DistributedLargeBlobSender
    import direct.directutil.DistributedLargeBlobSenderAI
    import direct.directutil.LargeBlobSenderConsts
    import direct.directutil.Mopath
    import direct.directutil.Verify
    import direct.directutil.WeightedChoice
    import direct.dist.FreezeTool
    import direct.dist.icon
    import direct.dist.commands
    import direct.distributed.AsyncRequest
    import direct.distributed.CRCache
    import direct.distributed.CRDataCache
    import direct.distributed.CachedDOData
    import direct.distributed.CartesianGridBase
    import direct.distributed.ClientRepository
    import direct.distributed.ClientRepositoryBase
    import direct.distributed.ClockDelta
    import direct.distributed.ConnectionRepository
    import direct.distributed.DistributedCamera
    import direct.distributed.DistributedCameraAI
    import direct.distributed.DistributedCameraOV
    import direct.distributed.DistributedCartesianGrid
    import direct.distributed.DistributedCartesianGridAI
    import direct.distributed.DistributedNode
    import direct.distributed.DistributedNodeAI
    import direct.distributed.DistributedNodeUD
    import direct.distributed.DistributedObject
    import direct.distributed.DistributedObjectAI
    import direct.distributed.DistributedObjectBase
    import direct.distributed.DistributedObjectGlobal
    import direct.distributed.DistributedObjectGlobalAI
    import direct.distributed.DistributedObjectGlobalUD
    import direct.distributed.DistributedObjectOV
    import direct.distributed.DistributedObjectUD
    import direct.distributed.DistributedSmoothNodeAI
    import direct.distributed.DistributedSmoothNodeBase
    import direct.distributed.DoCollectionManager
    import direct.distributed.DoHierarchy
    import direct.distributed.DoInterestManager
    import direct.distributed.GridChild
    import direct.distributed.GridParent
    import direct.distributed.InterestWatcher
    import direct.distributed.MsgTypes
    import direct.distributed.MsgTypesCMU
    import direct.distributed.NetMessenger
    import direct.distributed.ParentMgr
    import direct.distributed.PyDatagram
    import direct.distributed.PyDatagramIterator
    import direct.distributed.RelatedObjectMgr
    import direct.distributed.SampleObject
    import direct.distributed.ServerRepository
    import direct.distributed.StagedObject
    import direct.distributed.TimeManager
    import direct.distributed.TimeManagerAI
    import direct.extensions_native.extension_native_helpers
    import direct.filter.CommonFilters
    import direct.filter.FilterManager
    import direct.fsm.ClassicFSM
    import direct.fsm.FSM
    import direct.fsm.FourState
    import direct.fsm.FourStateAI
    import direct.fsm.SampleFSM
    import direct.fsm.State
    import direct.fsm.StateData
    import direct.fsm.StatePush
    import direct.gui.DirectButton
    import direct.gui.DirectCheckBox
    import direct.gui.DirectCheckButton
    import direct.gui.DirectDialog
    import direct.gui.DirectEntry
    import direct.gui.DirectEntryScroll
    import direct.gui.DirectFrame
    import direct.gui.DirectGui
    import direct.gui.DirectGuiBase
    import direct.gui.DirectGuiGlobals
    import direct.gui.DirectGuiTest
    import direct.gui.DirectLabel
    import direct.gui.DirectOptionMenu
    import direct.gui.DirectRadioButton
    import direct.gui.DirectScrollBar
    import direct.gui.DirectScrolledFrame
    import direct.gui.DirectScrolledList
    import direct.gui.DirectSlider
    import direct.gui.DirectWaitBar
    import direct.gui.OnscreenGeom
    import direct.gui.OnscreenImage
    import direct.gui.OnscreenText
    import direct.interval.ActorInterval
    import direct.interval.AnimControlInterval
    import direct.interval.FunctionInterval
    import direct.interval.IndirectInterval
    import direct.interval.Interval
    import direct.interval.IntervalGlobal
    import direct.interval.IntervalManager
    import direct.interval.IntervalTest
    import direct.interval.LerpBlendHelpers
    import direct.interval.LerpInterval
    import direct.interval.MetaInterval
    import direct.interval.MopathInterval
    import direct.interval.ParticleInterval
    import direct.interval.ProjectileInterval
    import direct.interval.ProjectileIntervalTest
    import direct.interval.SoundInterval
    import direct.interval.TestInterval
    import direct.motiontrail.MotionTrail
    import direct.particles.ForceGroup
    import direct.particles.GlobalForceGroup
    import direct.particles.ParticleEffect
    import direct.particles.ParticleFloorTest
    import direct.particles.ParticleManagerGlobal
    import direct.particles.ParticleTest
    import direct.particles.Particles
    import direct.particles.SpriteParticleRendererExt
    import direct.physics.FallTest
    import direct.physics.RotationTest
    import direct.showbase.Audio3DManager
    import direct.showbase.BufferViewer
    import direct.showbase.BulletinBoard
    import direct.showbase.BulletinBoardGlobal
    import direct.showbase.BulletinBoardWatcher
    import direct.showbase.ContainerLeakDetector
    import direct.showbase.ContainerReport
    import direct.showbase.CountedResource
    import direct.showbase.DirectObject
    import direct.showbase.DistancePhasedNode
    import direct.showbase.EventGroup
    import direct.showbase.EventManager
    import direct.showbase.EventManagerGlobal
    import direct.showbase.ExceptionVarDump
    import direct.showbase.Factory
    import direct.showbase.Finder
    import direct.showbase.GarbageReport
    import direct.showbase.GarbageReportScheduler
    import direct.showbase.InputStateGlobal
    import direct.showbase.Job
    import direct.showbase.JobManager
    import direct.showbase.JobManagerGlobal
    import direct.showbase.LeakDetectors
    import direct.showbase.Loader
    import direct.showbase.Messenger
    import direct.showbase.MessengerGlobal
    import direct.showbase.MessengerLeakDetector
    import direct.showbase.MirrorDemo
    import direct.showbase.ObjectPool
    import direct.showbase.ObjectReport
    import direct.showbase.OnScreenDebug
    import direct.showbase.PhasedObject
    import direct.showbase.PhysicsManagerGlobal
    import direct.showbase.Pool
    import direct.showbase.ProfileSession
    import direct.showbase.PythonUtil
    import direct.showbase.RandomNumGen
    import direct.showbase.ReferrerSearch
    import direct.showbase.SfxPlayer
    import direct.showbase.ShadowDemo
    import direct.showbase.ShadowPlacer
    import direct.showbase.ShowBase
    import direct.showbase.TaskThreaded
    import direct.showbase.ThreeUpShow
    import direct.showbase.Transitions
    import direct.showbase.VFSImporter
    import direct.showbase.WxGlobal
    import direct.showutil.BuildGeometry
    import direct.showutil.Effects
    import direct.showutil.Rope
    import direct.showutil.TexMemWatcher
    import direct.showutil.TexViewer
    import direct.stdpy.file
    import direct.stdpy.glob
    import direct.stdpy.pickle
    import direct.stdpy.thread
    import direct.stdpy.threading
    import direct.stdpy.threading2
    import direct.task.FrameProfiler
    import direct.task.MiniTask
    import direct.task.Task
    import direct.task.TaskManagerGlobal
    import direct.task.TaskProfiler
    import direct.task.TaskTester
    import direct.task.Timer


def test_imports_tk():
    Pmw = pytest.importorskip('Pmw')

    import direct.showbase.TkGlobal
    import direct.tkpanels.AnimPanel
    import direct.tkpanels.DirectSessionPanel
    import direct.tkpanels.FSMInspector
    import direct.tkpanels.Inspector
    import direct.tkpanels.MopathRecorder
    import direct.tkpanels.NotifyPanel
    import direct.tkpanels.ParticlePanel
    import direct.tkpanels.Placer
    import direct.tkpanels.TaskManagerPanel
    import direct.tkwidgets.AppShell
    import direct.tkwidgets.Dial
    import direct.tkwidgets.EntryScale
    import direct.tkwidgets.Floater
    import direct.tkwidgets.MemoryExplorer
    import direct.tkwidgets.ProgressBar
    import direct.tkwidgets.SceneGraphExplorer
    import direct.tkwidgets.Slider
    import direct.tkwidgets.Tree
    import direct.tkwidgets.Valuator
    import direct.tkwidgets.VectorWidgets
    import direct.tkwidgets.WidgetPropertiesDialog


def test_imports_wx():
    wx = pytest.importorskip('wx')

    import direct.wxwidgets.ViewPort
    import direct.wxwidgets.WxAppShell
    import direct.wxwidgets.WxPandaShell
    import direct.wxwidgets.WxPandaWindow
    import direct.wxwidgets.WxSlider

    import direct.leveleditor.ActionMgr
    import direct.leveleditor.AnimControlUI
    import direct.leveleditor.AnimGlobals
    import direct.leveleditor.AnimMgr
    import direct.leveleditor.AnimMgrBase
    import direct.leveleditor.CurveAnimUI
    import direct.leveleditor.CurveEditor
    import direct.leveleditor.FileMgr
    import direct.leveleditor.GraphEditorUI
    import direct.leveleditor.HotKeyUI
    import direct.leveleditor.LayerEditorUI
    import direct.leveleditor.LevelEditor
    import direct.leveleditor.LevelEditorBase
    import direct.leveleditor.LevelEditorStart
    import direct.leveleditor.LevelEditorUI
    import direct.leveleditor.LevelEditorUIBase
    import direct.leveleditor.LevelLoader
    import direct.leveleditor.LevelLoaderBase
    import direct.leveleditor.MayaConverter
    import direct.leveleditor.ObjectGlobals
    import direct.leveleditor.ObjectHandler
    import direct.leveleditor.ObjectMgr
    import direct.leveleditor.ObjectMgrBase
    import direct.leveleditor.ObjectPalette
    import direct.leveleditor.ObjectPaletteBase
    import direct.leveleditor.ObjectPaletteUI
    import direct.leveleditor.ObjectPropertyUI
    import direct.leveleditor.PaletteTreeCtrl
    import direct.leveleditor.ProtoObjs
    import direct.leveleditor.ProtoObjsUI
    import direct.leveleditor.ProtoPalette
    import direct.leveleditor.ProtoPaletteBase
    import direct.leveleditor.ProtoPaletteUI
    import direct.leveleditor.SceneGraphUI
    import direct.leveleditor.SceneGraphUIBase
