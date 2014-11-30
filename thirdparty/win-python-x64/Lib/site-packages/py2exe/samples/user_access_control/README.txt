This is a sample for how to control Vista's user-access-control for your
py2exe created programs.

Execute 'python setup.py py2exe' to create various executables, all with
different manifest values.  Each of these may behave slightly differently
when executed under Vista.

Important:
----------

There is an important difference between Python 2.6 versus
earlier versions.

Python 2.5 and before created executables will not have a manifest
by default, meaning a backwards compatibility virtualization mode
may be used by default.  However, if you specify any value for 'uac_info'
this virtualization will be disabled as it will cause a manifest to be
created for the executable.

To demonstrate on Vista:

* Using an elevated process, create a temp directory under "C:\Program Files"
  and copy the generated 'dist' directory to that temp dir.

* From a non-elevated command-prompt, execute:
    c:\Program Files\temp\not_specified.exe -e foo

  You will see an error dialog due to 'foo' being written to stderr. The
  error dialog will tell you the log file was written to the 
  "C:\Program Files\temp" directory, but that log file will not exist
  in that directory - instead, it will be in the
  "C:\Users\{username}\AppData\Local\VirtualStore\Program Files (x86)\temp"
  directory - Windows has virtualized the file-system.  Similar things will
  be able to be demonstrated with the registry.

* From a non-elevated command-prompt, execute:
    c:\Program Files\temp\as_invoker.exe -e foo

  You will see an error dialog, but it will be slightly different - it
  will relate to a failure to open the log file in the "c:\Program Files\temp"
  directory.  In this case, Windows has *not* virtualized the file-system but
  your process does not have permission to write in that directory, so it 
  fails to write a log file at all.

* From a non-elevated command-prompt, execute:
    c:\Program Files\temp\admin_required.exe -e foo

  You will be prompted to elevate the process, then a log file will be
  written to "C:\Program Files\temp" as the permissions exists in the 
  elevated process.

NOTE: For Python 2.6 and later there is always a manifest, so virtualization
is always disabled.  In other words, in Python 2.6, not_specified.exe will 
work exactly like as_invoker.exe does for all versions.
