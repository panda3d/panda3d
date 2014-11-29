__all__ = []

import sys,time
importTime = time.clock()
PRmodules = [k for k in sys.modules.keys() if k.find(__name__)>-1]
PRmodulesTime = [sys.modules[m].importTime for m in PRmodules]
PRmodulesTime.sort()
PRmod1stImport = [sys.modules[m] for m in PRmodules if sys.modules[m].importTime==PRmodulesTime[0]][0]
# print PRmodules,PRmodulesTime
if len(PRmodules)>1:
   for k in PRmodules:
       if importTime==sys.modules[k].importTime:
          sys.modules[k] = PRmod1stImport
          print 'WARNING : PauseResume module was ALREADY IMPORTED,\n          using the 1st imported one.'
          break
else:
   import os
   from direct.task import Task
   directModulesDir=os.path.abspath(os.path.join(os.path.dirname(sys.modules[Task.__name__].__file__),os.pardir))

   from pandac.extension_native_helpers import Dtool_funcToMethod
   from pandac.PandaModules import AnimControl, AudioSound, MovieTexture, NodePath, PandaSystem, AsyncTaskManager
   from direct.interval.IntervalGlobal import ivalMgr
   from myMessenger import Messenger

   atLeast16=PandaSystem.getMajorVersion()*10+PandaSystem.getMinorVersion()>=16
   taskFunc=lambda t: t.getFunction() if atLeast16 else t.__call__
   taskFuncNameQuery=lambda t: 'getFunction' if atLeast16 else '__call__'
   taskXArgs=lambda t: t.getArgs() if atLeast16 else t.extraArgs
   taskXArgsName=lambda t: 'getArgs' if atLeast16 else 'extraArgs'
   taskWakeT=lambda t: t.getDelay() if atLeast16 else t.wakeTime

   PRmsg = Messenger()
   IDE_ivalsName = 'IDE_IVALS_'
   IDE_tasksName = 'IDE_TASKS_'
   PAUSED_TASKCHAIN_NAME = 'YNJH paused tasks'
   isPaused = 0
   resumeLocked = 0

   # keeps the original C++ functions
   AnimControl__origPlay=AnimControl.play
   AnimControl__origLoop=AnimControl.loop
   AnimControl__origPingpong=AnimControl.pingpong
   AnimControl__origStop=AnimControl.stop
   AudioSound__origPlay=AudioSound.play
   AudioSound__origStop=AudioSound.stop
   MovieTexture__origPlay=MovieTexture.play
   MovieTexture__origStop=MovieTexture.stop


   # defines the new method wrappers for intercepting messages

   # ANIMATIONS ################################################################
   def newAnimPlay(self):
       PRmsg.accept('pauseAllAnims',self,pauseAnim,[self,0])
       PRmsg.accept('pauseNotTaggedAnims',self,pauseAnim,[self])
       AnimControl__origPlay(self)
   Dtool_funcToMethod(newAnimPlay,AnimControl,'play')
   del newAnimPlay

   def newAnimLoop(self,restart=1,_from=None,_to=None):
       PRmsg.accept('pauseAllAnims',self,pauseAnim,[self,0])
       PRmsg.accept('pauseNotTaggedAnims',self,pauseAnim,[self])
       if _from is not None and _to is not None :
          AnimControl__origLoop(self,restart,_from,_to)
       else:
          AnimControl__origLoop(self,restart)
   Dtool_funcToMethod(newAnimLoop,AnimControl,'loop')
   del newAnimLoop

   def newAnimPingpong(self,restart=1,_from=None,_to=None):
       PRmsg.accept('pauseAllAnims',self,pauseAnim,[self,0])
       PRmsg.accept('pauseNotTaggedAnims',self,pauseAnim,[self])
       if _from is not None and _to is not None :
          AnimControl__origPingpong(self,restart,_from,_to)
       else:
          AnimControl__origPingpong(self,restart)
   Dtool_funcToMethod(newAnimPingpong,AnimControl,'pingpong')
   del newAnimPingpong

   def newAnimStop(self):
       for e in PRmsg.getAllAccepting(self):
           PRmsg.ignore(e,self)
       AnimControl__origStop(self)
   Dtool_funcToMethod(newAnimStop,AnimControl,'stop')
   del newAnimStop

   def pauseAnim(self,respectTag=1):
       if respectTag:
          part=self.getPart()
          for n in xrange(part.getNumNodes()):
              if NodePath(part.getNode(n)).hasNetTag('nopause'):
                 return
       PRmsg.ignore('pauseAllAnims',self)
       PRmsg.ignore('pauseNotTaggedAnims',self)
       PRmsg.accept('resumeAllAnims',self,resumeAnim,[self,self.getPlayRate()])
       self.setPlayRate(0)

   def resumeAnim(self,PR):
       PRmsg.ignore('resumeAllAnims',self)
       PRmsg.accept('pauseAllAnims',self,pauseAnim,[self,0])
       PRmsg.accept('pauseNotTaggedAnims',self,pauseAnim,[self])
       self.setPlayRate(PR)


   # AUDIO SOUNDS ##############################################################
   notPausableSounds = []
   invulnerableSounds = []

   def newAudioPlay(self):
       PRmsg.accept('pauseAllSounds',self,pauseAudio,[self,0])
       PRmsg.accept('pausePausableSounds',self,pauseAudio,[self])
       AudioSound__origPlay(self)
   newAudioPlay.__doc__=AudioSound__origPlay.__doc__
   Dtool_funcToMethod(newAudioPlay,AudioSound,'play')
   del newAudioPlay

   def newAudioStop(self):
       for e in PRmsg.getAllAccepting(self):
           PRmsg.ignore(e,self)
       AudioSound__origStop(self)
   newAudioStop.__doc__=AudioSound__origStop.__doc__
   Dtool_funcToMethod(newAudioStop,AudioSound,'stop')
   del newAudioStop

   def setPausable(self,status):
       if self in notPausableSounds:
          notPausableSounds.remove(self)
       if not status:
          notPausableSounds.append(self)
   Dtool_funcToMethod(setPausable,AudioSound)
   del setPausable

   def setInvulnerable(self,status):
       if self in invulnerableSounds:
          if status:
             return
          else:
             invulnerableSounds.remove(self)
       elif status:
          invulnerableSounds.append(self)
   Dtool_funcToMethod(setInvulnerable,AudioSound)
   del setInvulnerable

   def pauseAudio(self,respectTag=1):
       if self in invulnerableSounds:
          return
       if respectTag:
          if self in notPausableSounds:
             return
       PRmsg.ignore('pauseAllSounds',self)
       PRmsg.ignore('pausePausableSounds',self)
       PRmsg.accept('resumeAllSounds',self,resumeAudio,[self])
       AudioSound__origStop(self)
       self.setTime(self.getTime())

   def resumeAudio(self):
       PRmsg.ignore('resumeAllSounds',self)
       PRmsg.accept('pauseAllSounds',self,pauseAudio,[self,0])
       PRmsg.accept('pausePausableSounds',self,pauseAudio,[self])
       AudioSound__origPlay(self)


   # MOVIE TEXTURES ############################################################
   notPausableMovies = []

   def newMoviePlay(self):
       PRmsg.accept('pauseAllMovies',self,pauseMovie,[self,0])
       PRmsg.accept('pausePausableMovies',self,pauseMovie,[self])
       MovieTexture__origPlay(self)
   newMoviePlay.__doc__=MovieTexture__origPlay.__doc__
   Dtool_funcToMethod(newMoviePlay,MovieTexture,'play')
   del newMoviePlay

   def newMovieStop(self):
       for e in PRmsg.getAllAccepting(self):
           PRmsg.ignore(e,self)
       MovieTexture__origStop(self)
   newMovieStop.__doc__=MovieTexture__origStop.__doc__
   Dtool_funcToMethod(newMovieStop,MovieTexture,'stop')
   del newMovieStop

   def setPausable(self,status):
       if self in notPausableMovies:
          notPausableMovies.remove(self)
       if not status:
          notPausableMovies.append(self)
   Dtool_funcToMethod(setPausable,MovieTexture)
   del setPausable

   def pauseMovie(self,respectTag=1):
       if respectTag:
          if self in notPausableMovies:
             return
       PRmsg.ignore('pauseAllMovies',self)
       PRmsg.ignore('pausePausableMovies',self)
       PRmsg.accept('resumeAllMovies',self,resumeMovie,[self])
       MovieTexture__origStop(self)

   def resumeMovie(self):
       PRmsg.ignore('resumeAllMovies',self)
       PRmsg.accept('pauseAllMovies',self,pauseMovie,[self,0])
       PRmsg.accept('pausePausableMovies',self,pauseMovie,[self])
       self.restart()


   # INTERVALS #################################################################
   def pauseIvals(excludeNamePrefix=''):
       global pausedIvals
       pausedIvals=ivalMgr.getIntervalsMatching('*')
       excluded=[]
       for i in pausedIvals:
           if ( (excludeNamePrefix and i.getName().find(excludeNamePrefix)==0) or
                i.getName().find(IDE_ivalsName)==0
              ):
              excluded.append(i)
           else:
              #~ print 'PAUSED IVAL:',i.getName()
              i.pause()
       for e in excluded:
           pausedIvals.remove(e)

   def resumeIvals():
       for i in pausedIvals:
           i.resume()


   # TASKS #####################################################################
   def pauseTasks(excludedTaskNamePrefix,noCollision):
       global unneededTasks
       if not AsyncTaskManager.getGlobalPtr().findTaskChain(PAUSED_TASKCHAIN_NAME):
          taskMgr.setupTaskChain(PAUSED_TASKCHAIN_NAME,
             frameBudget=0) # frameBudget=0 doesn't allow any task to run
       unneededTasksName=[]
       # collects unneeded tasks
       if noCollision:
          unneededTasksName+=['collisionLoop','resetPrevTransform']
       unneededTasks=[ taskMgr.getTasksNamed(tn)[0] for tn in unneededTasksName]
       # collects all scene's tasks
       for t in taskMgr.getTasks(): # ordinary tasks
           if ( t and hasattr(t,taskFuncNameQuery(t)) and t.name.find(IDE_tasksName)!=0 and
                ( not excludedTaskNamePrefix or
                  (excludedTaskNamePrefix and t.name.find(excludedTaskNamePrefix))
                )
              ):
              func=taskFunc(t)
              mod=func.__module__
              # python-based intervals
              if mod.find('direct.interval')==0:
                 if not (func.im_class.__name__=='ActorInterval' and\
                         func.im_self.actor.hasNetTag('nopause')):
                    unneededTasks.append(t)
                    t.interval.pause()
              elif mod not in sys.modules or sys.modules[mod].__file__.find(directModulesDir)<0:
                 unneededTasks.append(t)
       currT=globalClock.getFrameTime()
       for t in taskMgr.getDoLaters(): # doLater tasks
           if ( t and hasattr(t,taskFuncNameQuery(t)) and t.name.find(IDE_tasksName)!=0 and
                ( not excludedTaskNamePrefix or
                  (excludedTaskNamePrefix and t.name.find(excludedTaskNamePrefix)) )
              ):
                 unneededTasks.append(t)
                 # I need to alter the wakeTime during task resume,
                 # so I have to save the remaining time.
                 # Just save it as its attribute, nobody would notice :D
                 t.remainingTime=t.wakeTime-currT
       # "pauses" tasks
       for t in unneededTasks:
           t.ORIG_extraArgs=taskXArgs(t) if hasattr(t,taskXArgsName(t)) else None
           if hasattr(t,taskFuncNameQuery(t)):
              t.ORIG_call=taskFunc(t)
           t.ORIG_priority=t._priority if hasattr(t,'_priority') else t.getSort()
           # only active tasks can be moved to other chain, so removes doLater
           # tasks since they are in sleeping state
           if hasattr(t,'remainingTime'): # doLater tasks
              t.remove()
           else: # ordinary tasks
              t.lastactivetime=-t.time if hasattr(t,'time') else 0
              try:
                  t.setTaskChain(PAUSED_TASKCHAIN_NAME)
              except:
                  pass

   def resumeTasks():
       # restarts tasks
       for t in unneededTasks:
           if hasattr(t,'interval'): # it must be python-based intervals
              t.interval.resume()
              if hasattr(t,'ORIG_call'):
                 if atLeast16:
                    t.setFunction(t.ORIG_call)
                 else:
                    t.__call__=t.ORIG_call
           else:
              if hasattr(t,'remainingTime'): # doLater tasks
                 tempDelay=t.remainingTime-(globalClock.getRealTime()-globalClock.getFrameTime())
                 if hasattr(t,'uponDeath'):
                    uponDeath=t.uponDeath
                 else:
                    uponDeath=None
                 # no need to pass appendTask, since if it's already true,
                 # the task is already appended to extraArgs
                 newTask=taskMgr.doMethodLater( tempDelay, t.ORIG_call,
                                                t.name, priority=t.ORIG_priority,
                                                extraArgs=t.ORIG_extraArgs,
                                                uponDeath=uponDeath)
                 # restore the original delayTime
                 if hasattr(t,'remainingTime'):
                    newTask.delayTime=t.delayTime
              else: # ordinary tasks
                 t.setDelay(t.lastactivetime)
                 t.setTaskChain('default')
                 # very important to avoid assertion error on resume
                 t.clearDelay()


   def pause( allAnims=0,allAudios=0,allMovies=0,collision=1,
              excludedTaskNamePrefix='',excludedIvalNamePrefix='',
              lowerLevelOperation=1
              ):
       '''
           allAnims  : pause all animations or only the not "nopause" tagged ones
           allAudios : pause all audio sounds or only the pausable ones
           allMovies : pause all movies or only the pausable ones
           collision : pause collision detection or not
           excludedTaskNamePrefix : do not pause tasks with this name prefix
           excludedIvalNamePrefix : do not pause intervals with this name prefix
           lowerLevelOperation : <DO NOT use this>
       '''
       global isPaused,resumeLocked
       if isPaused:
          print 'WARNING : SCENE IS ALREADY PAUSED !'
          return isPaused
       PRmsg.send('pauseAllAnims') if allAnims else PRmsg.send('pauseNotTaggedAnims')
       PRmsg.send('pauseAllSounds') if allAudios else PRmsg.send('pausePausableSounds')
       PRmsg.send('pauseAllMovies') if allMovies else PRmsg.send('pausePausableMovies')
       pauseIvals(excludedIvalNamePrefix)
       pauseTasks(excludedTaskNamePrefix,collision)
       base.disableParticles()
       isPaused=1
       resumeLocked=lowerLevelOperation
#        print 'PR:',isPaused
       return isPaused

   def resume(lowerLevelOperation=1):
       global isPaused,resumeLocked
       if resumeLocked and not lowerLevelOperation:
#           print 'WARNING : RESUME IS LOCKED'
          return 2
       if not isPaused:
          print 'WARNING : SCENE IS ALREADY RESUMED'
          return isPaused
       PRmsg.send('resumeAllAnims') # resume all animations
       PRmsg.send('resumeAllSounds') # resume all audio
       PRmsg.send('resumeAllMovies') # resume all movie
       resumeIvals()
       resumeTasks()
       base.enableParticles()
       isPaused=0
#        print 'PR:',isPaused
       return isPaused
