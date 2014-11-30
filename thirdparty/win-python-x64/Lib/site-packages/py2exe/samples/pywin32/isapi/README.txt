A pywin32 ISAPI sample.

This builds the pywin32 isapi 'redirector' sample (see 
site-packages\isapi\samples) into a py2exe distribution.  Execute:

  setup.py py2exe

and in the 'dist' directory you will find 'redirector.exe' (used to
register/unregister the ISAPI extension), and 'redirector.dll' (the ISAPI
filter and extension loaded by IIS.)

See the pywin32 sample for more details about how to use the sample.
