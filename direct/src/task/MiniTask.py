"""This module implements a minimum task manager.  It is similar in
principle to the full-featured task manager implemented in Task.py,
but it has a sharply reduced feature set--completely bare-bones, in
fact--and it is designed to be a pure-python implementation that does
not require any C++ Panda support, so that it can be loaded before
Panda has been fully downloaded. """

__all__ = ['MiniTask', 'MiniTaskManager']

class MiniTask:
    done = 0
    cont = 1

    def __init__(self, callback):
        self.__call__ = callback

class MiniTaskManager:

    def __init__(self):
        self.taskList = []
        self.running = 0

    def add(self, task, name):
        assert isinstance(task, MiniTask)
        task.name = name
        self.taskList.append(task)

    def remove(self, task):
        try:
            self.taskList.remove(task)
        except ValueError:
            pass

    def __executeTask(self, task):
        return task(task)

    def step(self):
        i = 0
        while (i < len(self.taskList)):
            task = self.taskList[i]
            ret = task(task)

            # See if the task is done
            if (ret == task.cont):
                # Leave it for next frame, its not done yet
                pass

            else:
                # Remove the task
                try:
                    self.taskList.remove(task)
                except ValueError:
                    pass
                # Do not increment the iterator
                continue

            # Move to the next element
            i += 1

    def run(self):
        self.running = 1
        while self.running:
            self.step()

    def stop(self):
        # Set a flag so we will stop before beginning next frame
        self.running = 0

