This is a sample of a COM object ('server') implemented in Python.

This builds the pywin32 sample COM object 'interp' - see the win32com\servers 
directory.

Execute:

  setup.py py2exe

And in the dist directory you will find interp.exe and inter.dll.

You can register the objects with 'interp.exe /regserver' or 
'regsvr32 interp.dll'.  'interp.exe /unregister' and 'regsvr32 /u interp.dll'
will unregister the objects.

Once registered, test the object using 'test_interp.py' or 'test_interp.vbs'