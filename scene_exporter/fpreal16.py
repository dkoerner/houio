'''
Decode a 16-bit floating point number and return a Python float

For example:
    # Decode the integer from the bytestream
    i = struct.unpack('H', '\x00\x3c')

    # Then interpret the 16-bit integer as a float
    f = fpreal16.ItoF16(i)
'''

import struct

def ItoF16(h16):
    '''
        The integer (h16) passed in is expected to be the 16-bit integer
        representation of the float.
    '''
    s = int((h16 >> 15) & 0x00000001)     # sign
    e = int((h16 >> 10) & 0x0000001f)     # exponent
    m = int(h16 &         0x000003ff)     # mantissa

    if e == 0:
        if m == 0:
            iv = int(s << 31)
        else:
            while not (m & 0x00000400):
                m <<= 1
                e -= 1
            e += 1
            m &= ~0x00000400
            e = e + (127 -15)
            m = m << 13
            iv = int((s << 31) | (e << 23) | m)
    elif e == 31:
        if m == 0:
            iv = int((s << 31) | 0x7f800000)
        else:
            iv = int((s << 31) | 0x7f800000 | (m << 13))
    else:
        e = e + (127 -15)
        m = m << 13
        iv = int((s << 31) | (e << 23) | m)

    # iv now stores the bit-wise representation of a 32-bit IEEE float.
    # Pack up the integer into a 32 bit unsigned integer, then unpack it
    # into a IEEE float.
    return struct.unpack('f', struct.pack('I', iv))[0]
