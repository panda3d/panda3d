import os, sys

EXTENSIONS = ['.i', '.I', '.cxx', '.yxx', '.lxx', '.h', '.cpp', '.mm', '.java', '.hh', '.C.template']
#TAGS = ['Description', 'Function', 'Function name', 'Enum', 'Date', 'Updated by', 'Created by', 'Filename', 'Access', 'Return type', 'Argument', 'Class']

INCLUDE_FILES = ['soft2Egg.c']
EXCLUDE_FILES = []
#EXCLUDE_FILES = ['stb_image.h', 'frustum_src.I', 'directd.h', 'newheader.cxx', 'socket_ip.h', 'geoMipTerrain.cxx']

def warn(fn, ln, msg):
    print("%s:%s: warning: %s" % (fn, ln, msg))

license_lines = [
    # lol... these occur
    "PANDA 3D SOFTWARE",
    "PANDA 2D SOFTWARE",
    "PANDA 4D SOFTWARE",

    "Copyright (c) Carnegie Mellon University.  All rights reserved.",
    "All use of this software is subject to the terms of the revised BSD",
    "license.  You should have received a copy of this license along",
    "with this source code in a file named \"LICENSE.\"",
]

new_license = ''' * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
'''

months = ["jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"]

author_tags = ["Adapted by", "Changes by", "Created by", "Extended by", "Heavily Modified", "Modified by", "Rewrite [for new Version of FMOD-EX] by", "Updated by"]

def convert_date(value):
    value = value.lower()
    if ')' in value:
        value = value.split(')')[0]

    for mo in months:
        if mo in value:
            day, year = value.split(mo)
            day = day.strip()
            year = int(year.strip())
            month = (months.index(mo) + 1)

            if year < 50:
                year = 2000 + year
            elif year < 100:
                year = 1900 + year

            if not day:
                return "%04d-%02d" % (year, month)
            else:
                return "%04d-%02d-%02d" % (year, month, int(day))
    return value


def transform_file(fn):
    #if '_composite' in fn:
    #    return

    fp = open(fn, 'r')

    start_file = True
    is_header = False
    file_info = []

    line = fp.readline()
    if line.startswith('// Filename') or line.startswith('/* Filename'):
        is_header = True
        file_info.append('@file ' + line.split(':', 1)[1].strip())
    else:
        return

    for line in fp:
        if line.startswith('///////////////') or line.startswith(' * * * * * * *'):
            break

        if not line.strip():
            # End of comments.
            break

        if not line.startswith('//') and not line.startswith(' *'):
            print fn
            return

        stripped = line.strip('/* \n')

        if ':' in stripped:
            tag, value = stripped.split(':', 1)
            if tag in author_tags:
                value = value.strip()
                date = None
                if value == 'skyler 2002.04.08':
                    author = 'skyler'
                    date = '2002-04-08'
                elif value == 'schuyler 2003-03-13':
                    author = 'schuyler'
                    date = '2003-03-13'
                elif '(' in value:
                    author, date = value.split('(', 1)
                    author = author.strip()
                    date = date.strip(') \t\n').lower()
                    date = date.replace('june', 'jun')
                    date = date.replace('sept', 'sep')
                    date = convert_date(date)
                else:
                    author = value

                if author == 'pro-rsoft':
                    author = 'rdb'

                if author:
                    file_info.append('@author ' + author)
                if date:
                    file_info.append('@date ' + date)
                #print date
            else:
                file_info.append(line[3:].rstrip())
                #print line[3:].rstrip()
        elif not stripped:
            if len(file_info) > 0 and file_info[-1]:
                file_info.append('')
        else:
            file_info.append(line[3:].rstrip())

    # Remove trailing newlines
    while file_info and not file_info[-1]:
        file_info.pop()

    have_license = False
    for line in fp:
        if line.startswith('///////////////') or line.startswith(' * * * * * * *'):
            have_extra_stuff = False

            for line in fp:
                if line.startswith('///////////////') or line.startswith(' * * * * * * *'):
                    # End of comment block.
                    break
                elif line.startswith('//'):
                    stuff = line.rstrip()[3:]
                    if stuff.strip() or (have_extra_stuff and file_info[-1]):
                        if not have_extra_stuff:
                            file_info.append('')
                        have_extra_stuff = True
                        file_info.append(stuff)
                else:
                    break
            break

        if not line.startswith('//') and not line.startswith(' *'):
            # End of comments.
            if line.strip():
                print fn, line
            return

        stripped = line.strip('/* \n')
        if stripped in license_lines:
            have_license = True
        elif stripped:
            print fn, line

    # Remove trailing newlines
    while file_info and not file_info[-1]:
        file_info.pop()

    assert file_info

    other_lines = []
    for line in fp:
        stripped = line.strip()

        if not len(other_lines) and not stripped:
            continue

        other_lines.append(line.rstrip('\n'))
    fp.close()

    # Write modified file
    out = open(fn, 'w')
    out.write('/**\n')

    if have_license:
        out.write(new_license)
        out.write(' *\n')

    for iline in file_info:
        out.write(' * %s\n' % (iline))

    out.write(' */\n\n')

    for line in other_lines:
        out.write(line + '\n')

    out.close()

    #print fn, have_license

    #if not have_license:
    #    print fn


if len(sys.argv) > 1:
    for arg in sys.argv:
        transform_file(arg)
else:
    for dirpath, dirnames, filenames in os.walk('.'):
        for fn in filenames:
            ext = os.path.splitext(fn)[1]
            if fn in INCLUDE_FILES or (ext in EXTENSIONS and fn not in EXCLUDE_FILES):
                path = os.path.join(dirpath, fn)
                transform_file(path)
