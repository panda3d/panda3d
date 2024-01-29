# Functions for dealing with dates and times.

import re
import string

def timestringtoseconds(text, separator = ':'):
    # to Py3
    #inputList = string.split(string.strip(text), separator)
    inputList = text.strip().split(separator)

    if len(inputList) != 3:
        raise ValueError('invalid value: ' + text)

    sign = 1
    if len(inputList[0]) > 0 and inputList[0][0] in ('+', '-'):
        if inputList[0][0] == '-':
            sign = -1
        inputList[0] = inputList[0][1:]

    #Py3 if re.search('[^0-9]', string.join(inputList, '')) is not None:
    if re.search('[^0-9]', ''.join(inputList)) is not None:    
        raise ValueError('invalid value: ' + text)

    hour = int(inputList[0])
    minute = int(inputList[1])
    second = int(inputList[2])
    
    if minute >= 60 or second >= 60:
        raise ValueError('invalid value: ' + text)
    return sign * (hour * 60 * 60 + minute * 60 + second)

_year_pivot = 50
_century = 2000

def setyearpivot(pivot, century = None):
    global _year_pivot
    global _century
    oldvalues = (_year_pivot, _century)
    _year_pivot = pivot
    if century is not None:
        _century = century
    return oldvalues

def datestringtojdn(text, fmt = 'ymd', separator = '/'):
    #Py3 inputList = string.split(string.strip(text), separator)
    inputList = text.strip().split(separator)
    if len(inputList) != 3:
        raise ValueError('invalid value: ' + text)

    #Py3 if re.search('[^0-9]', string.join(inputList, '')) is not None:
    if re.search('[^0-9]', ''.join(inputList)) is not None:
        raise ValueError('invalid value: ' + text)
    formatList = list(fmt)
    day = int(inputList[formatList.index('d')])
    month = int(inputList[formatList.index('m')])
    year = int(inputList[formatList.index('y')])

    if _year_pivot is not None:
        if year >= 0 and year < 100:
            if year <= _year_pivot:
                year = year + _century
            else:
                year = year + _century - 100

    jdn = ymdtojdn(year, month, day)

    if jdntoymd(jdn) != (year, month, day):
        raise ValueError('invalid value: ' + text)
    return jdn

def _cdiv(a, b):
        # Return a / b as calculated by most C language implementations,
        # assuming both a and b are integers.

    if a * b > 0:
        return int(a / b)
    else:
        return -int(abs(a) / abs(b))

def ymdtojdn(year, month, day, julian = -1, papal = 1):

    # set Julian flag if auto set
    if julian < 0:
        if papal:                          # Pope Gregory XIII's decree
            lastJulianDate = 15821004     # last day to use Julian calendar
        else:                              # British-American usage
            lastJulianDate = 17520902     # last day to use Julian calendar

        julian = ((year * 100) + month) * 100 + day  <=  lastJulianDate

    if year < 0:
        # Adjust BC year
        year = year + 1

    if julian:
        return 367 * year - _cdiv(7 * (year + 5001 + _cdiv((month - 9), 7)), 4) + \
            _cdiv(275 * month, 9) + day + 1729777
    else:
        return (day - 32076) + \
            _cdiv(1461 * (year + 4800 + _cdiv((month - 14), 12)), 4) + \
            _cdiv(367 * (month - 2 - _cdiv((month - 14), 12) * 12), 12) - \
            _cdiv((3 * _cdiv((year + 4900 + _cdiv((month - 14), 12)), 100)), 4) + \
            1            # correction by rdg

def jdntoymd(jdn, julian = -1, papal = 1):

    # set Julian flag if auto set
    if julian < 0:
        if papal:                          # Pope Gregory XIII's decree
            lastJulianJdn = 2299160       # last jdn to use Julian calendar
        else:                              # British-American usage
            lastJulianJdn = 2361221       # last jdn to use Julian calendar

        julian = (jdn <= lastJulianJdn);

    x = jdn + 68569
    if julian:
        x = x + 38
        daysPer400Years = 146100
        fudgedDaysPer4000Years = 1461000 + 1
    else:
        daysPer400Years = 146097
        fudgedDaysPer4000Years = 1460970 + 31

    z = _cdiv(4 * x, daysPer400Years)
    x = x - _cdiv((daysPer400Years * z + 3), 4)
    y = _cdiv(4000 * (x + 1), fudgedDaysPer4000Years)
    x = x - _cdiv(1461 * y, 4) + 31
    m = _cdiv(80 * x, 2447)
    d = x - _cdiv(2447 * m, 80)
    x = _cdiv(m, 11)
    m = m + 2 - 12 * x
    y = 100 * (z - 49) + y + x

    # Convert from longs to integers.
    yy = int(y)
    mm = int(m)
    dd = int(d)

    if yy <= 0:
        # Adjust BC years.
        yy = yy - 1

    return (yy, mm, dd)

def stringtoreal(text, separator = '.'):
    if separator != '.':
        #Py3 if string.find(text, '.') >= 0:
        if text.find('.') >= 0:
            raise ValueError('invalid value: ' + text)
        #Py3 index = string.find(text, separator)
        index = text.find(separator)
        if index >= 0:
            text = text[:index] + '.' + text[index + 1:]
    return float(text)
