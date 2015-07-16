""" This script converts a file into a format that
doxygen can understand and process.  It can be used
as an INPUT_FILTER in doxygen. """

import sys, os

# Explicitly include these files.  Besides these, all
# files ending in _src will be explicitly included too.
INCLUDE_FILES = ["fltnames.h", "dblnames.h",
                 "flt2dblnames.h", "dbl2fltnames.h"]

def filter_file(infile):
    desc = ""
    reading_desc = False
    license = True
    indent = 0

    for line in infile.readlines():
        line = line.rstrip()

        if line.startswith("////"):
            if reading_desc:
                # Probably the end of a comment reading_desc.
                line = "*/"
                reading_desc = False
            else:
                line = ""

        elif line.startswith("//"):
            strline = line.lstrip('/ \t')
            if reading_desc:
                line = line[min(indent, len(line) - len(strline)):]
            else:
                # A "Description:" text starts the description.
                if strline.startswith("Description"):
                    strline = strline[11:].lstrip(': \t')
                    indent = len(line) - len(strline)
                    reading_desc = True
                    line = "/** " + strline

        else:
            license = False
            if reading_desc:
                line = "*/" + line
                reading_desc = False

            if line.startswith("#include"):
                fname = line.split(' ', 1)[1].strip().strip('"')
                if fname.rsplit('.', 1)[0].endswith("_src") or fname in INCLUDE_FILES:
                    # We handle these in a special way, because
                    # doxygen won't do this properly for us.
                    # This breaks line numbering, but beh.
                    if not os.path.isfile(fname):
                        fname = os.path.join(os.path.dirname(filename), fname)
                    if os.path.isfile(fname):
                        filter_file(open(fname, 'r'))
                        continue # Skip the #include

        if license:
            line = ""
        print(line)

if __name__ == "__main__":
    assert len(sys.argv) == 2, "please specify a filename"
    filename = sys.argv[1]
    infile = open(filename, 'r')
    filter_file(infile)
    infile.close()
