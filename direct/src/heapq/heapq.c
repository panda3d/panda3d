
/* Note: This module can probably go away when we upgrade to Python 2.4.
   Python 2.3 has a heapq implementation, but it is in Python. This is
   reported to be about 20x faster. In 2.4 they reimplemented heapq in C so
   it should be comparable to this. At this time though, Python 2.4 is
   still in alpha.
*/

#include <Python.h>

/* Prototypes */
static PyObject * heappush(PyObject *self, PyObject *args);
static PyObject * heappop(PyObject *self, PyObject *args);
static PyObject * heapreplace(PyObject *self, PyObject *args);
static PyObject * heapify(PyObject *self, PyObject *args);
static int _siftdown(PyObject *list, int startpos, int pos);
static int _siftup(PyObject *list, int pos);

#ifdef _WIN32
extern __declspec(dllexport) void initlibheapq(void);
#else
extern void initlibheapq();
#endif

static PyObject *
heappush(PyObject *self, PyObject *args) {
    int len;
    PyObject *list = NULL;
    PyObject *node = NULL;
    
    if (!PyArg_ParseTuple(args,"O!O",&PyList_Type,&list,&node))
        return NULL;

    len = PyList_Size(list);
    if (PyList_Append(list,node))
        return NULL;
    
    if (_siftdown(list,0,len))
        return NULL;
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
heappop(PyObject *self, PyObject *args) {
    PyObject *list = NULL;
    PyObject *node = NULL;
    PyObject *returnNode = NULL;
    int len;
    
    if (!PyArg_ParseTuple(args,"O!",&PyList_Type,&list))
        return NULL;

    len = PyList_Size(list);
    if (len == 0) {
        /* Special-case most common failure cause */
        PyErr_SetString(PyExc_IndexError, "pop from empty list");
        return NULL;
    }

    node = PySequence_GetItem(list,-1);
    PySequence_DelItem(list,-1);

    len -= 1;
    if (len > 0) {
        returnNode = PySequence_GetItem(list,0);
        PyList_SetItem(list,0,node);
        if (_siftup(list,0))
            return NULL;
    } else {
        returnNode = node;
    }
    
    return returnNode;
}

static PyObject * 
heapreplace(PyObject *self, PyObject *args) {
    PyObject *list = NULL;
    PyObject *node = NULL;
    PyObject *returnNode = NULL;
    int len;
    
    if (!PyArg_ParseTuple(args,"O!O",&PyList_Type,&list,&node))
        return NULL;

    len = PyList_Size(list);
    if (len == 0) {
        /* Special-case most common failure cause */
        PyErr_SetString(PyExc_IndexError, "replace on an empty list");
        return NULL;
    }

    returnNode = PySequence_GetItem(list,0);
    PySequence_SetItem(list,0,node);
    if (_siftup(list,0))
        return NULL;

    return returnNode;
}

static PyObject *
heapify(PyObject *self, PyObject *args) {
    int n, i;
    PyObject *list;

    if (!PyArg_ParseTuple(args,"O!",&PyList_Type,&list))
        return NULL;
    n = (PyList_Size(list)/2)-1;
    
    for (i=n;i>=0;i--) {
        if (_siftup(list,i))
            return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static int
_siftdown(PyObject *list, int startpos, int pos) {
    PyObject *newitem, *parent;
    int parentpos, cmp;

    newitem = PySequence_GetItem(list,pos);
    while (pos > startpos) {
        parentpos = (pos - 1) >> 1;
        parent = PyList_GetItem(list,parentpos);

        cmp = PyObject_RichCompareBool(parent,newitem,Py_LE);
        if (cmp > 0)
            break;
        else if (cmp < 0)
            return -1;
        Py_INCREF(parent);
        PyList_SetItem(list,pos,parent);
        pos = parentpos;
    }
    PyList_SetItem(list,pos,newitem);
    return 0;
}

static int
_siftup(PyObject *list, int pos) {
    PyObject *newitem, *right, *child;
    int endpos, rightpos, childpos;
    int startpos = pos;
    int cmp;
    
    endpos = PyList_Size(list);
    newitem = PySequence_GetItem(list,pos);
    
    childpos = (2*pos)+1;
    while (childpos < endpos) {
        rightpos = childpos + 1;
        child = PySequence_Fast_GET_ITEM(list,childpos);
        if (rightpos < endpos) {
            right = PySequence_Fast_GET_ITEM(list,rightpos);
            cmp = PyObject_RichCompareBool(right,child,Py_LE);
            if (cmp > 0)
                childpos = rightpos;
            else if (cmp < 0)
                return -1;
        }
        child = PySequence_GetItem(list,childpos);
        PyList_SetItem(list,pos,child);
        pos = childpos;
        childpos = (2*pos)+1;
    }
    PyList_SetItem(list,pos,newitem);

    return _siftdown(list,startpos,pos);
}

static PyMethodDef heapqcMethods[] = {
    {"heappush",heappush,METH_VARARGS},
    {"heappop",heappop,METH_VARARGS},
    {"heapreplace",heapreplace,METH_VARARGS},
    {"heapify",heapify,METH_VARARGS},
    {NULL, NULL} /* Sentinel */
};

void initlibheapq(void) {
    (void) Py_InitModule("libheapq", heapqcMethods);
};


