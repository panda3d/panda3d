"""Undocumented Module"""

__all__ = ['TaskManagerPanel', 'TaskManagerWidget']

from direct.tkwidgets.AppShell import *
from Tkinter import *
from direct.showbase.DirectObject import DirectObject
import Pmw

class TaskManagerPanel(AppShell):
    # Override class variables here
    appname = 'TaskManager Panel'
    frameWidth      = 300
    frameHeight     = 400
    usecommandarea = 0
    usestatusarea  = 0

    def __init__(self, taskMgr, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        self.taskMgr = taskMgr
        
        # Call superclass initialization function
        AppShell.__init__(self, parent = parent)

        self.initialiseoptions(TaskManagerPanel)

    def createInterface(self):
        # FILE MENU
        # Get a handle on the file menu so commands can be inserted
        # before quit item
        self.taskMgrWidget = TaskManagerWidget(
            self.interior(), self.taskMgr)


    def onDestroy(self, event):
        self.taskMgrWidget.onDestroy()

class TaskManagerWidget(DirectObject):
    """
    TaskManagerWidget class: this class contains methods for creating
    a panel to control taskManager tasks.
    """

    def __init__(self, parent, taskMgr):
        """
        TaskManagerWidget class pops up a control panel to view/delete
        tasks managed by the taskManager.
        """
        # Make sure TK mainloop is running
        from direct.showbase import TkGlobal
        # Record parent (used by ok cancel dialog boxes)
        self.parent = parent
        # Record taskManager
        self.taskMgr = taskMgr
        # Init current task
        self.currentTask = None
        self.__taskDict = {}

        # Create widgets
        # Create a listbox
        self.taskListBox = Pmw.ScrolledListBox(
            parent,
            labelpos = NW, label_text = 'Tasks:',
            label_font=('MSSansSerif', 10, 'bold'),
            listbox_takefocus = 1,
            items = [],
            selectioncommand = self.setCurrentTask)
        self.taskListBox.pack(expand = 1, fill = BOTH)

        self._popupMenu = Menu(self.taskListBox.component('listbox'),
                               tearoff = 0)
        self._popupMenu.add_command(
            label = 'Remove Task',
            command = self.removeCurrentTask)
        self._popupMenu.add_command(
            label = 'Remove Matching Tasks',
            command = self.removeMatchingTasks)
                                           
        # Controls Frame
        controlsFrame = Frame(parent)
        self.removeButton = Button(controlsFrame, text = 'Remove Task',
                                   command = self.removeCurrentTask)
        #self.removeButton.pack(expand = 1, fill = X, side = LEFT)
        self.removeButton.grid(row = 0, column = 0, sticky = EW)
        self.removeMatchingButton = Button(controlsFrame,
                                           text = 'Remove Matching Tasks',
                                           command = self.removeMatchingTasks)
        #self.removeMatchingButton.pack(expand = 1, fill = X, side = LEFT)
        self.removeMatchingButton.grid(row = 0, column = 1, sticky = EW)

        self.taskMgrVerbose = IntVar()
        self.taskMgrVerbose.set(0)
        self.update = Button(
            controlsFrame,
            text = 'Update',
            command = self.updateTaskListBox)
        #self.update.pack(expand = 1, fill = X, side = LEFT)
        self.update.grid(row = 1, column = 0, sticky = EW)
        self.dynamicUpdate = Checkbutton(
            controlsFrame,
            text = 'Dynamic Update',
            variable = self.taskMgrVerbose,
            command = self.toggleTaskMgrVerbose)
        #self.dynamicUpdate.pack(expand = 1, fill = X, side = LEFT)
        self.dynamicUpdate.grid(row = 1, column = 1, sticky = EW)
        # Pack frames
        controlsFrame.pack(fill = X)
        controlsFrame.grid_columnconfigure(0, weight = 1)
        controlsFrame.grid_columnconfigure(1, weight = 1)
        
        # Add hook to spawnTaskEvents
        self.accept('TaskManager-spawnTask', self.spawnTaskHook)
        self.accept('TaskManager-removeTask', self.removeTaskHook)
        # Get listbox
        listbox = self.taskListBox.component('listbox')
        # Bind updates to arrow buttons
        listbox.bind('<KeyRelease-Up>', self.setCurrentTask)
        listbox.bind('<KeyRelease-Down>', self.setCurrentTask)
        listbox.bind('<ButtonPress-3>', self.popupMenu)
        # And grab focus (to allow keyboard navigation)
        listbox.focus_set()
        # Update listbox values
        self.updateTaskListBox()

    def popupMenu(self, event):
        """
        listbox = self.taskListBox.component('listbox')
        index = listbox.nearest(event.y)
        listbox.selection_clear(0)
        listbox.activate(index)
        self.taskListBox.select_set(index)
        self.setCurrentTask()
        """
        self._popupMenu.post(event.widget.winfo_pointerx(),
                             event.widget.winfo_pointery())
        return "break"

    def setCurrentTask(self, event = None):
        if len(self.taskListBox.curselection()) > 0: # [gjeon] to avoid crash when nothing is selected
            index = int(self.taskListBox.curselection()[0])
            self.currentTask = self.__taskDict[index]
        else:
            self.currentTask = None

    def updateTaskListBox(self):
        # Get a list of task names
        taskNames = []
        self.__taskDict = {}
        tasks = self.taskMgr.getTasks()
        tasks.sort(key = lambda t: t.getName())
        count = 0
        for task in tasks:
            taskNames.append(task.getName())
            self.__taskDict[count] = task
            count += 1
        
        if taskNames:
            self.taskListBox.setlist(taskNames)
            # And set current index (so keypresses will start with index 0)
            self.taskListBox.component('listbox').activate(0)
            # Select first item
            #self.taskListBox.select_set(0) # [gjeon] commented out to avoid focus problem with other lists
            self.setCurrentTask()

    def toggleTaskMgrVerbose(self):
        if self.taskMgrVerbose.get():
            self.updateTaskListBox()

    def spawnTaskHook(self, task):
        if self.taskMgrVerbose.get():
            self.updateTaskListBox()

    def removeTaskHook(self, task):
        if self.taskMgrVerbose.get():
            self.updateTaskListBox()

    def removeCurrentTask(self):
        if self.currentTask:
            name = self.currentTask.name
            ok = 1
            if ((name == 'dataLoop') or
                (name == 'resetPrevTransform') or
                (name == 'tkLoop') or
                (name == 'eventManager') or
                (name == 'igLoop')):
                from tkMessageBox import askokcancel
                ok = askokcancel('TaskManagerControls',
                                 'Remove: %s?' % name,
                                 parent = self.parent,
                                 default = 'cancel')
            if ok:
                self.taskMgr.remove(self.currentTask)
                self.updateTaskListBox()

    def removeMatchingTasks(self):
        name = self.taskListBox.getcurselection()[0]
        ok = 1
        if ((name == 'dataLoop') or
            (name == 'resetPrevTransform') or
            (name == 'tkLoop') or
            (name == 'eventManager') or
            (name == 'igLoop')):
            from tkMessageBox import askokcancel
            ok = askokcancel('TaskManagerControls',
                             'Remove tasks named: %s?' % name,
                             parent = self.parent,
                             default = 'cancel')
        if ok:
            self.taskMgr.remove(name)
            self.updateTaskListBox()

    def onDestroy(self):
        self.ignore('TaskManager-spawnTask')
        self.ignore('TaskManager-removeTask')
        

