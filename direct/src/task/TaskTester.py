"""Undocumented Module"""

__all__ = []

from direct.task.TaskManagerGlobal import *
import direct.task.Task
import random

numTasks = 10000
maxDelay = 20
counter = 0

def spawnNewTask():
    global counter
    counter = (counter + 1) % 1000
    delay = random.random() * maxDelay
    taskMgr.doMethodLater(delay, taskCallback, ("taskTester-%s" % counter))

def taskCallback(task):
    randNum = int(round(random.random() * 1000))
    n = ("taskTester-%s" % randNum)
    taskMgr.remove(n)
    spawnNewTask()
    spawnNewTask()
    return Task.done

taskMgr.removeTasksMatching("taskTester*")

for i in range(numTasks):
    spawnNewTask()
