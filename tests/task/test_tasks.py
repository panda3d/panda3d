from direct.task import Task
import random

taskMgr = Task.TaskManager()
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
    n = f"taskTester-{randNum}"
    taskMgr.remove(n)
    spawnNewTask()
    spawnNewTask()
    return Task.done

def test_tasks():
    taskMgr.removeTasksMatching("taskTester*")

    for i in range(numTasks):
        spawnNewTask()

    taskMgr.destroy()
