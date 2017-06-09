#   This document is licensed under CC-3.0 Attribution-Share Alike 3.0
#         http://creativecommons.org/licenses/by-sa/3.0/
#   Attribution:  There is no requirement to attribute the author.
'''
    A Python JSON parser (http://www.json.org)

    This is a pure Python implementation of a JSON parser (http://www.json.org)
    It supports the Houdini (http://www.sidefx.com) extensions for a binary
    JSON format.

    This is intended as a reference implementation for Houdini's binary JSON
    extension.

    It is not very efficient.  Please use the hjson module for efficient
    parsing/writing.

    HDK classes:  UT_JSONParser UT_JSONWriter
    HOM Module:   hjson
'''

import struct
import fpreal16
import re

''' Parsing states '''
_STATE_START             = 0
_STATE_COMPLETE          = 1
_STATE_MAP_START         = 2
_STATE_MAP_SEP           = 3
_STATE_MAP_NEED_VALUE    = 4
_STATE_MAP_GOT_VALUE     = 5
_STATE_MAP_NEED_KEY      = 6
_STATE_ARRAY_START       = 7
_STATE_ARRAY_NEED_VALUE  = 8
_STATE_ARRAY_GOT_VALUE   = 9

''' JSON identifiers used in binary encoding and parsing '''
JID_NULL                = 0x00
JID_MAP_BEGIN           = 0x7b
JID_MAP_END             = 0x7d
JID_ARRAY_BEGIN         = 0x5b
JID_ARRAY_END           = 0x5d
JID_BOOL                = 0x10
JID_INT8                = 0x11
JID_INT16               = 0x12
JID_INT32               = 0x13
JID_INT64               = 0x14
JID_REAL16              = 0x18
JID_REAL32              = 0x19
JID_REAL64              = 0x1a
JID_UINT8               = 0x21
JID_UINT16              = 0x22
JID_STRING              = 0x27
JID_FALSE               = 0x30
JID_TRUE                = 0x31
JID_TOKENDEF            = 0x2b
JID_TOKENREF            = 0x26
JID_TOKENUNDEF          = 0x2d
JID_UNIFORM_ARRAY       = 0x40
JID_KEY_SEPARATOR       = 0x3a
JID_VALUE_SEPARATOR     = 0x2c
JID_MAGIC               = 0x7f

''' Magic number (and swapped version) for binary JSON files '''
BINARY_MAGIC            = 0x624a534e
BINARY_MAGIC_SWAP       = 0x4e534a62

_ASCII_VALUE_SEPARATOR  = ','
_ASCII_KEY_SEPARATOR    = '\t:'

class Handle:
    '''
        The Handle class is used to handle events in a SAX style when parsing
        JSON files (see the Parser class).  As the file is parsed, different
        callbacks are invoked.  A subclass should override methods to process
        the JSON file.

        The ua* (Uniform Array) methods are an extension to binary JSON files.
        These methods, by default, will just invoke jsonBeginArray(), json*(),
        jsonEndArray().  However, it may be possible for some implementations
        of binary JSON to take advantage of these methods to pre-allocate data
        or batch process the load data.

        The ua* methods do not have to be implemented as their default
        implementation just calls the json* methods.

        Houdini HDK Equivalent:  UT_JSONHandle
    '''
    def jsonNull(self, parser):
        ''' Invoked on <null> token'''
        return True
    def jsonBool(self, parser, value):
        ''' Invoked on either <true> or <false> tokens.'''
        return True
    def jsonInt(self, parser, value):
        '''
            binary JSON separates int/float values.  This event is called when
            an integer value was parsed.
        '''
        return True
    def jsonReal(self, parser, value):
        '''
            binary JSON separates int/float values.  This event is called when
            an integer value was parsed.
        '''
        return True
    def jsonString(self, parser, value):
        ''' Invoked when a <string> is encountered (see jsonKey()) '''
        return True
    def jsonKey(self, parser, value):
        '''
            When processing a map, the jsonKey() method will be invoked when
            the key is read).
        '''
        return True
    def jsonBeginMap(self, parser):
        ''' Invoked at beginning of a new map/object '''
        return True
    def jsonEndMap(self, parser):
        ''' Invoked at the end of a map/object '''
        return True
    def jsonBeginArray(self, parser):
        ''' Invoked at the beginning of a new array '''
        return True
    def jsonEndArray(self, parser):
        ''' Invoked at the end of an array '''
        return True

    def uaBool(self, parser, length):
        '''
            In binary JSON files, uniform bool arrays are stored as bit
            streams.  This method decodes the bit-stream, calling jsonBool()
            for each element of the bit array.
        '''
        if not self.jsonBeingArray(parser):
            return False
        while length > 0:
            x = parser.readU32()
            if x == None:
                return False
            nbits = min(length, 32)
            length -= nbits
            for i in range(nbits):
                if not self.jsonBool(parser, word & (1 << i) != 0):
                    return False
        return self.jsonEndArray(parser)

    def uniformArray(self, parser, length, method, callback):
        ''' Generic method to handle a uniform array '''
        if not self.jsonBeginArray(parser):
            return False
        while length > 0:
            length -= 1
            value = method()
            if value == None:
                return False
            if not callback(parser, value):
                return False
        return self.jsonEndArray(parser)


    def uaInt8(self, parser, length):
        ''' Read the next <length> bytes as singed 8-bit integer values '''
        return self.uniformArray(parser, length, parser.readI8, self.jsonInt)
    def uaInt16(self, parser, length):
        ''' Read the next <length*2> bytes as singed 16-bit integer values '''
        return self.uniformArray(parser, length, parser.readI16, self.jsonInt)
    def uaInt32(self, parser, length):
        ''' Read the next <length*4> bytes as singed 32-bit integer values '''
        return self.uniformArray(parser, length, parser.readI32, self.jsonInt)
    def uaInt64(self, parser, length):
        ''' Read the next <length*8> bytes as singed 64-bit integer values '''
        return self.uniformArray(parser, length, parser.readI64, self.jsonInt)
    def uaReal16(self, parser, length):
        '''
            Read the next <length*2> bytes as singed 16-bit float values
            (see fpreal16.py)
        '''
        return self.uniformArray(parser, length, parser.readF16, self.jsonReal)
    def uaReal32(self, parser, length):
        ''' Read the next <length*4> bytes as singed 32-bit float values '''
        return self.uniformArray(parser, length, parser.readF32, self.jsonReal)
    def uaReal64(self, parser, length):
        ''' Read the next <length*8> bytes as singed 64-bit float values '''
        return self.uniformArray(parser, length, parser.readF64, self.jsonReal)
    def uaUInt8(self, parser, length):
        ''' Read the next <length> bytes as unsinged 8-bit integer values '''
        return self.uniformArray(parser, length, parser.readU8, self.jsonInt)
    def uaUInt16(self, parser, length):
        ''' Read the next <length*2> bytes as unsinged 16-bit integer values '''
        return self.uniformArray(parser, length, parser.readU16, self.jsonInt)
    def uaString(self, parser, length):
        ''' Read the next <N> bytes as binary encoded string values '''
        return self.uniformArray(parser, length,
                                parser.binaryString, self.jsonString)
    def uaString(self, parser, length):
        ''' Read the next <N> bytes as binary encoded string token values '''
        return self.uniformArray(parser, length,
                                parser.binaryStringToken, self.jsonString)


#
#  This is a list of the struct sizes and format strings used by binary loading
#
def _getIntFormat(size, swap=''):
    # Find the format string for 64 bit ints
    for f in [ 'b', 'h', 'i', 'l', 'q', 'qq' ]:
        if struct.calcsize(f) == size:
            return swap+f
    raise RuntimeError("No struct size for %d-bit integers found" % size*8)

def _getUIntFormat(size, swap=''):
    return _getIntFormat(size, swap).upper()

def _getRealFormat(size, swap=''):
    for f in [ 'h', 'f', 'd']:
        if struct.calcsize(f) == size:
            return f
    raise RuntimeError("No struct size for %d-bit floats found" % size*8)

def _getByteSwap():
    x = 'AB'
    h = struct.pack( '<h', struct.unpack('=h', x)[0] )
    if h == 'AB':
        return '>'      # < was the correct byte order, so return swapped order
    return '<'          # > is byte swapped

_INT_FORMATS = {
    JID_INT8    : (1, 0),       # (bytes, index in format array)
    JID_INT16   : (2, 1),
    JID_INT32   : (4, 2),
    JID_INT64   : (8, 3),
}
_UINT_FORMATS = {
    JID_UINT8   : (1, 0),       # (bytes, index in format array)
    JID_UINT16  : (2, 1),
}
_REAL_FORMATS = {
    JID_REAL16  : (2, 0),       # (bytes, index in format array)
    JID_REAL32  : (4, 1),
    JID_REAL64  : (8, 2),
}
_NAN = {
    # Possible values for not-a-number or infinity
    'nan'       : True,
    'inf'       : True,
    'infinity'  : True,
    'Nan'       : True,
    'Inf'       : True,
    'Infinity'  : True,
    '1.#ind'    : True,
    '1.#inf'    : True,
    'NAN'       : True,
    'INF'       : True,
    'INFINITY'  : True,
    '1.#IND'    : True,
    '1.#INF'    : True,
}

class _jValue:
    '''
        This class holds a JSON value which might be a map or array of
        additional JSON values.  The class knows how to send events to the
        handle class.
    '''
    def __init__(self, token, value):
        ''' Initialize _jValue '''
        self.Token = token
        self.Value = value
        self.Type = None

    def event(self, parser, key=False):
        '''
            Method invoked to send an event to a Handle.  This invokes the
            appropriate callback on the handle.
        '''
        handle = parser.Handle
        if self.Token == JID_STRING:
            if key:
                return handle.jsonKey(parser, self.Value)
            return handle.jsonString(parser, self.Value)
        if self.Token == JID_BOOL:
            return handle.jsonBool(parser, self.Value)
        if {JID_INT8:1,JID_INT16:1,JID_INT32:1,JID_INT64:1}.has_key(self.Token):
            return handle.jsonInt(parser, self.Value)
        if {JID_UINT8:1,JID_UINT16:1}.has_key(self.Token):
            return handle.jsonInt(parser, self.Value)
        if {JID_REAL16:1,JID_REAL32:1,JID_REAL64:1}.has_key(self.Token):
            return handle.jsonReal(parser, self.Value)
        if self.Token == JID_MAP_BEGIN:
            return handle.jsonBeginMap(parser)
        if self.Token == JID_MAP_END:
            return handle.jsonEndMap(parser)
        if self.Token == JID_ARRAY_BEGIN:
            return handle.jsonBeginArray(parser)
        if self.Token == JID_ARRAY_END:
            return handle.jsonEndArray(parser)
        if self.Token == JID_NULL:
            return handle.jsonNull(parser)
        if self.Token == JID_UNIFORM_ARRAY:
            if self.Type == JID_BOOL:
                return handle.uaBool(parser, self.Value)
            if self.Type == JID_INT8:
                return handle.uaInt8(parser, self.Value)
            if self.Type == JID_INT16:
                return handle.uaInt16(parser, self.Value)
            if self.Type == JID_INT32:
                return handle.uaInt32(parser, self.Value)
            if self.Type == JID_INT64:
                return handle.uaInt64(parser, self.Value)
            if self.Type == JID_REAL16:
                return handle.uaReal16(parser, self.Value)
            if self.Type == JID_REAL32:
                return handle.uaReal32(parser, self.Value)
            if self.Type == JID_REAL64:
                return handle.uaReal64(parser, self.Value)
            if self.Type == JID_UINT8:
                return handle.uaUInt8(parser, self.Value)
            if self.Type == JID_UINT16:
                return handle.uaUInt16(parser, self.Value)
            if self.Type == JID_STRING:
                return handle.uaString(parser, self.Value)
            if self.Type == JID_TokenRef:
                return handle.uaStringToken(parser, self.Value)
        return False

class Parser:
    '''
        The Parser class will parse an ASCII or binary JSON stream.  The
        parse() method should be called with a "file" type object (one which
        has the read(bytes) method implemented).

        The second argument should be a Handle (which implements the
        appropriate json* methods.

        Houdini HDK Equivalent:  UT_JSONParser
    '''
    def __init__(self):
        ''' Initializer '''
        self.Handle = None
        self.ParseError = False
        self.Stream = None
        self.Errors = []
        self.Stack = []
        self.Tokens = {}
        self.State = _STATE_START
        self.Binary = False
        self.ReadCount = 0
        self.IFormats = map(_getIntFormat,  [1,2,4,8])
        self.UFormats = map(_getUIntFormat, [1,2,4,8])
        self.FFormats = map(_getRealFormat, [2,4,8])
        self.Peek = None

    def parse(self, fp, h):
        '''
        Parse a single object out of a JSON stream.
          @param fp This is a file object which has the \c read(bytes) method
          @param h This is a Handle object which implements the json* methods
        '''
        saveStream = self.Stream
        saveHandle = self.Handle
        saveParseError = self.ParseError
        if h:
            self.Handle = h
        if fp:
            self.Stream = fp

        result = self._parseStream()
        if self.ParseError:
            result = False

        self.Handle = saveHandle
        self.Stream = saveStream
        self.ParseError = saveParseError
        return result

    def _parseStream(self):
        ''' Parse the JSON state stream '''
        stack = 0
        while True:
            if self.State == _STATE_COMPLETE:
                return True
            push = None
            popped = False
            t = self._readToken()
            if not t:
                return False
            if self.State in [_STATE_START, _STATE_MAP_NEED_VALUE,
                                _STATE_ARRAY_NEED_VALUE, _STATE_ARRAY_START]:
                if t.Token in [JID_STRING, JID_BOOL, JID_NULL, JID_INT64,
                                JID_REAL64, JID_UNIFORM_ARRAY]:
                    pass
                elif t.Token == JID_MAP_BEGIN:
                    push = _STATE_MAP_START
                elif t.Token == JID_ARRAY_BEGIN:
                    push = _STATE_ARRAY_START
                elif t.Token == JID_ARRAY_END:
                    if self.State not in [_STATE_ARRAY_START, _STATE_ARRAY_NEED_VALUE]:
                        self.error('Unexpected array end token')
                        return False
                    if not self._popState():
                        return False
                    stack -= 1
                    popped = True
                elif t.Token == JID_MAP_END:
                    if self.State != _STATE_MAP_START:
                        self.error('Unexpected map end token')
                        return False
                    if not self._popState():
                        return False
                    stack -= 1
                    popped = True
                elif t.Token in [JID_KEY_SEPARATOR, JID_VALUE_SEPARATOR]:
                    self.error('Invalid separator token in parsing')
                    return False
                else:
                    self.error('Invalid parsing token')
                    return False
                if push:
                    self.Stack.append(push)
                    self._setState(push)
                    stack += 1
                elif not popped:
                    if self.State == _STATE_START:
                        self._setState(_STATE_COMPLETE)
                    elif self.State == _STATE_MAP_NEED_VALUE:
                        self._setState(_STATE_MAP_GOT_VALUE)
                    else:
                        self._setState(_STATE_ARRAY_GOT_VALUE)
                if not t.event(self):
                    return False
                if stack == 0:
                    return True
            elif self.State in [_STATE_MAP_START, _STATE_MAP_NEED_KEY]:
                if t.Token == JID_STRING:
                    self._setState(_STATE_MAP_SEP)
                    if not t.event(self, key=True):
                        return False
                    if stack == 0:
                        return True
                elif t.Token == JID_MAP_END:
                    if self.State in [_STATE_MAP_START, _STATE_MAP_NEED_KEY]:
                        if not self._popState():
                            return False
                        if not t.event(self):
                            return False
                        stack -= 1
                        if stack == 0:
                            return True
                    else:
                        self.error("Missing map key (need string)")
            elif self.State == _STATE_MAP_SEP:
                if t.Token == JID_KEY_SEPARATOR:
                    self._setState(_STATE_MAP_NEED_VALUE)
                else:
                    self.error("Key/value must be separated by a colon(':')")
            elif self.State == _STATE_MAP_GOT_VALUE:
                if t.Token == JID_MAP_END:
                    if not self._popState():
                        return False
                    if not t.event(self):
                        return False
                    stack -= 1
                    if stack == 0:
                        return True
                elif t.Token == JID_VALUE_SEPARATOR:
                    self._setState(_STATE_MAP_NEED_KEY)
                else:
                    self.error("Expected comma(',') or close brace('}') in map")
                    return False
            elif self.State == _STATE_ARRAY_GOT_VALUE:
                if t.Token == JID_ARRAY_END:
                    if not self._popState():
                        return False
                    if not t.event(self):
                        return False
                    stack -= 1
                    if stack == 0:
                        return True
                elif t.Token == JID_VALUE_SEPARATOR:
                    self._setState(_STATE_ARRAY_NEED_VALUE)
                else:
                    self.error(
                        "Expected comma(',') or close bracket(']') in array")
                    return False
            else:
                self.error("Unexpected parsing state")
                return False
                    
    def _setState(self, state):
        ''' Change the state of the parser '''
        if self.Binary:
            if state == _STATE_MAP_GOT_VALUE:
                state = _STATE_MAP_NEED_KEY
            elif state == _STATE_MAP_SEP:
                state = _STATE_MAP_NEED_VALUE
            elif state == _STATE_ARRAY_GOT_VALUE:
                state = _STATE_ARRAY_NEED_VALUE
        self.State = state
        try:
            self.Stack[ len(self.Stack)-1 ] = state
        except:
            self.error('Stack error processing stream')
            self.ParseError = True

    def _popState(self):
        ''' Restore the state '''
        n = len(self.Stack)
        if n < 1:
            return False
        self.Stack.pop()
        if n == 1:
            self.State = _STATE_COMPLETE
        else:
            self.State = self.Stack[n-2]
        if self.State in [_STATE_MAP_NEED_VALUE, _STATE_MAP_START]:
            self._setState(_STATE_MAP_GOT_VALUE)
        elif self.State in [_STATE_ARRAY_NEED_VALUE, _STATE_ARRAY_START]:
            self._setState(_STATE_ARRAY_GOT_VALUE)
        return True

    def _swapFormats(self):
        ''' Make binary struct format strings '''
        swap = _getByteSwap()
        self.IFormats = map(_getIntFormat,  [1,2,4,8], [swap]*4)
        self.UFormats = map(_getUIntFormat, [1,2,4,8], [swap]*4)
        self.FFormats = map(_getRealFormat, [2,4,8],   [swap]*3)

    def error(self, msg):
        ''' Simple method to append an error/warning message to the list '''
        self.Errors.append('JSON Parsing[%d]: %s' % (self.ReadCount, msg))

    def _putBack(self, c):
        ''' Put back a byte into the stream '''
        self.Peek = c

    def _readBytes(self, n):
        ''' Read n bytes from the stream '''
        if self.Peek:
            v = self.Peek
            self.Peek = None
            v1 = self._readBytes(n-1)
            if v1 == None:
                return None
            return ''.join([v, v1])
        else:
            self.ReadCount += n
            v = self.Stream.read(n)
            if len(v) != n:
                self.error('Read %d bytes past end of stream'%(len(v)-n))
                return None
        return v

    def _readUnpacked(self, size, format):
        ''' Read a string of size bytes and unpack it using the format given '''
        s = self._readBytes(size)
        if s != None:
            s = struct.unpack(format, s)[0]
        return s

    def readU8(self):
        ''' Read next byte as a binary unsigned 8-bit integer '''
        return self._readUnpacked(1, self.UFormats[0])
    def readU16(self):
        '''
            Read next 2 bytes as a binary unsigned 16-bit integer.
            Byte swapping is performed depending on endianness of the file.'''
        return self._readUnpacked(2, self.UFormats[1])
    def readU32(self):
        '''
            Read next 4 bytes as a binary unsigned 32-bit integer.
            Byte swapping is performed depending on endianness of the file.'''
        return self._readUnpacked(4, self.UFormats[2])
    def readI8(self):
        ''' Read next byte as a binary signed 8-bit integer '''
        return self._readUnpacked(1, self.IFormats[0])
    def readI16(self):
        '''
            Read next 2 bytes as a binary signed 16-bit integer.
            Byte swapping is performed depending on endianness of the file.'''
        return self._readUnpacked(2, self.IFormats[1])
    def readI32(self):
        '''
            Read next 4 bytes as a binary signed 32-bit integer.
            Byte swapping is performed depending on endianness of the file.'''
        return self._readUnpacked(4, self.IFormats[2])
    def readI64(self):
        '''
            Read next 8 bytes as a binary signed 64-bit integer.
            Byte swapping is performed depending on endianness of the file.'''
        return self._readUnpacked(8, self.IFormats[3])
    def readF32(self):
        '''
            Read next 4 bytes as a binary 32-bit float.
            Byte swapping is performed depending on endianness of the file.'''
        return self._readUnpacked(4, self.FFormats[1])
    def readF64(self):
        '''
            Read next 8 bytes as a binary 64-bit float.
            Byte swapping is performed depending on endianness of the file.'''
        return self._readUnpacked(8, self.FFormats[2])
    def readF16(self):
        '''
            Read next 2 bytes as a binary 32-bit float (see fpreal16.py)
            Byte swapping is performed depending on endianness of the file.
        '''
        # 16-bit floats require additional conversion
        f = self._readUnpacked(2, self.FFormats[0])
        if f != None:
            f = fpreal16.ItoF16(f)
        return f

    def binaryString(self):
        '''
            A binary string is encoded by storing it's length (encoded)
            followed by the string data.  The trailing <null> character is not
            stored in the file.
        '''
        l = self.readLength()
        if l == None:
            return None
        return self._readBytes(l)

    def binaryStringToken(self):
        '''
            Common strings may be encoded using a shared string table.  Each
            string token defined (defineToken()) is given an integer "handle"
            which can be used to reference the token.  Reating a token involves
            reading the integer handle and looking up the value in the Tokens
            map.
        '''
        id = self.readLength()
        if id == None:
            return None
        return self.Tokens.get(id, None)

    def readLength(self):
        '''
            In the binary format, length is encoded in a multi-byte format.
            For lenthgs < 0xf1 (240) the lenths are stored as a single byte.
            If the length is longer than 240 bytes, the value of the first byte
            determines the number of bytes that follow used to store the
            length.  Currently, the only supported values for the binary byte
            are:
                0xf2 = 2 bytes (16 bit unsigned length)
                0xf4 = 4 bytes (32 bit unsigned length)
                0xf8 = 8 bytes (64 bit signed length)
            Other values (i.e. 0xf1 or 0xfa) are considered errors.
        '''
        n = self.readU8()
        if n != None:
            if n < 0xf1:    return n
            if n == 0xf2:   return self.readU16()
            if n == 0xf4:   return self.readU32()
            if n == 0xf8:   return self.readI64()       # Signed 64 bit int
        self.error('Invalid binary length encoding: 0x%02x' % n)
        return None

    def defineToken(self):
        '''
            Read an id followed by an encoded string.  There is no handle
            callback, but rather, the string is stored in the shared string
            Token map.
        '''
        id = self.readLength()
        if id != None:
            s = self.binaryString()
            if s != None:
                self.Tokens[id] = s
                return True
        return False

    def undefineToken(self):
        ''' Remove a string from the shared string Token map. '''
        id = self.readLength()
        if id != None and self.Tokens.has_key(id):
            del self.Tokens[id]

    def _binaryToken(self, token):
        ''' Read a binary token (returns a _jValue or None) '''
        while token == JID_TOKENDEF or token == JID_TOKENUNDEF:
            if token == JID_TOKENDEF:
                if not self.defineToken():
                    return None
            else:
                if not self.undefineToken():
                    return None
            token = self.readI8()
        if { JID_NULL:1,
             JID_MAP_BEGIN:1,
             JID_MAP_END:1,
             JID_ARRAY_BEGIN:1,
             JID_ARRAY_END:1 }.has_key(token):
            return _jValue(token, None)

        if token == JID_BOOL:
            byte = self.readI8()
            if byte == None:
                return None
            return _jValue(token, byte != 0)
        if token == JID_FALSE:
            return _jValue(JID_BOOL, False)
        if token == JID_TRUE:
            return _jValue(JID_BOOL, True)
        if _INT_FORMATS.has_key(token):
            f = _INT_FORMATS[token]
            v = self._readUnpacked(f[0], self.IFormats[f[1]])
            if v == None:
                return None
            return _jValue(JID_INT64, v)
        if _UINT_FORMATS.has_key(token):
            f = _UINT_FORMATS[token]
            v = self._readUnpacked(f[0], self.UFormats[f[1]])
            if v == None:
                return None
            return _jValue(JID_INT64, v)
        if _REAL_FORMATS.has_key(token):
            f = _REAL_FORMATS[token]
            v = self._readUnpacked(f[0], self.FFormats[f[1]])
            if v == None:
                return None
            if token == JID_REAL16:
                v = fpreal16.ItoF16(v)
            return _jValue(JID_REAL64, v)
        if token == JID_STRING:
            v = self.binaryString()
            if v == None:
                return None
            return _jValue(JID_STRING, v)
        if token == JID_TOKENREF:
            v = self.binaryStringToken()
            if v == None:
                return None
            return _jValue(JID_STRING, v)
        if token == JID_UNIFORM_ARRAY:
            byte = self.readI8()        # Read the type information
            if byte == None:
                return None
            length = self.readLength()
            if length == None:
                return None
            v = _jValue(JID_UNIFORM_ARRAY, length)
            v.Type = byte
            return v
        self.error("Invalid binary token: 0x%02x" % token)
        return None

    def _readQuotedString(self):
        ''' Read a quoted string one character at a time '''
        word = []
        while True:
            c = self._readBytes(1)
            if c == None:
                self.error('Missing end-quote for string')
                return None
            if c == '\\':
                c = self._readBytes(1)
                if c in '"\\/': word.append(c)
                elif c == 'b':  word.append('\b')
                elif c == 'f':  word.append('\f')
                elif c == 'n':  word.append('\n')
                elif c == 'r':  word.append('\r')
                elif c == 't':  word.append('\t')
                elif c == 'u':
                    if self._readBytes(4) == None:
                        return False
                    self.error('UNICODE string escape not supported')
                else:
                    word.append('\\')
                    word.append(c)
            elif c == '"':
                return ''.join(word)
            else:
                word.append(c)

    def _readNumber(self, char):
        '''
            Catch-all parsing method which handles
                - null
                - true
                - false
                - numbers
            Any unquoted string which doesn't match the null, true, false
            tokens is considered to be a number.  We throw warnings on
            non-number values, but set the value to 0.  This is not ideal since
            a NAN or infinity is not preserved.

            Other implementations may choose to handle this differently.
        '''
        word = []
        real = False
        while True:
            word.append(char)
            char = self._readBytes(1)
            if char == None:
                break
            if char in ' \r\n\t/{}[],:"':
                self._putBack(char)
                break
            if char in '.eE':
                real = True
        if len(word) == 0:
            return None
        word = ''.join(word)
        word = word.lower()
        if word == 'null':
            return _jValue(JID_NULL, None)
        if word == 'false':
            return _jValue(JID_BOOL, False)
        if word == 'true':
            return _jValue(JID_BOOL, True)
        if _NAN.has_key(word):
            # This is a special nan/infinity token.
            # 1e1000 turns into an # infinity in Python (2.5 at least).
            return _jValue(JID_REAL64, 1e1000)
        if real:
            try:
                n = float(word)
            except:
                n = 0
                self.error('Error converting real: "%s"' % word)
                self.ParseError = True
            return _jValue(JID_REAL64, n)
        try:
            n = int(word)
        except:
            n = 0
            self.error('Error converting int: "%s"' % word)
            self.ParseError = True
        return _jValue(JID_INT64, n)

    def _asciiToken(self, char):
        ''' Read an ASCII token (returns a _jValue or None) '''
        while char in ' \r\n\t':
            char = self._readBytes(1)
            if char == None:
                return None
        if char == '/':
            char = self._readBytes(1)
            if char == '/':     # We support // style comments
                while char != None:
                    char = self._readBytes(1)
                    if char == '\n' or char == '\r':
                        return self._readToken()
                return None
        if char == '{': return _jValue(JID_MAP_BEGIN, None)
        if char == '}': return _jValue(JID_MAP_END, None)
        if char == '[': return _jValue(JID_ARRAY_BEGIN, None)
        if char == ']': return _jValue(JID_ARRAY_END, None)
        if char == ',': return _jValue(JID_VALUE_SEPARATOR, None)
        if char == ':': return _jValue(JID_KEY_SEPARATOR, None)
        if char == '"':
            v = self._readQuotedString()
            if v == None:
                return None
            return _jValue(JID_STRING, v)
        return self._readNumber(char)

    def _readToken(self):
        ''' Read a single token from the JSON Stream '''
        char = self._readBytes(1)
        if char == None:
            return False;
        byte = struct.unpack(self.UFormats[0], char)[0]
        if byte == JID_MAGIC:
            magic = self.readU32()
            if magic == None:
                return False
            if magic == BINARY_MAGIC:
                self.Binary = True
            elif magic == BINARY_MAGIC_SWAP:
                self.Binary = True
                self._swapFormats()
            else:
                self.error('Bad magic number 0x%08x' % magic)
                return False
            return self._readToken()
        if self.Binary:
            return self._binaryToken(byte)
        else:
            return self._asciiToken(char)

class _bWriter:
    ''' Binary JSON Writer '''
    def __init__(self, fp, options):
        self.Fp = fp
        self.UseTokens = options.get('usetokens', True)
        self.Tokens = {}
        self.TokenCount = 0
        self.I8  = _getIntFormat(1)
        self.I16 = _getIntFormat(2)
        self.I32 = _getIntFormat(4)
        self.I64 = _getIntFormat(8)
        self.U8  = _getUIntFormat(1)    # For packed bit array/lengths
        self.U16 = _getUIntFormat(2)    # For packed bit array/lengths
        self.U32 = _getUIntFormat(4)    # For packed bit array/lengths
        self.U64 = _getUIntFormat(8)    # For packed bit array/lengths
        self.F16 = _getRealFormat(2)
        self.F32 = _getRealFormat(4)
        self.F64 = _getRealFormat(8)
        self.WriteSize = 0
        self.WriteCount = 0
        self.BitCount = 0
        self.BitValue = 0
        self.UType = JID_NULL

        self.jsonKey = self.jsonString
        self.jsonKeyToken = self.jsonStringToken

    def jsonMagic(self):
        if self._writeId(JID_MAGIC):
            return self._writeData( struct.pack( self.U32, BINARY_MAGIC ) )
        return False

    def jsonNull(self):
        return self._writeId(JID_NULL)
    def jsonBool(self, value):
        return self._writeId([JID_TRUE, JID_FALSE][not value])
    def jsonInt(self, value):
        if value >= -0x80 and value < 0x80:
            id = JID_INT8
            data = struct.pack(self.I8, value)
        elif value >= -0x8000 and value < 0x8000:
            id = JID_INT16
            data = struct.pack(self.I16, value)
        elif value >= -0x80000000 and value < 0x80000000:
            id = JID_INT32
            data = struct.pack(self.I32, value)
        else:
            id = JID_INT64
            data = struct.pack(self.I64, value)
        if self._writeId(id):
            return self._writeData(data)
        return False
    def jsonReal16(self, value):
        ''' This implementation does not handle generation of 16 bit reals '''
        return self.jsonReal32(self, value)
    def jsonReal32(self, value):
        if self._writeId(JID_REAL32):
            return self._writeData( struct.pack(self.F32, value ) )
        return False
    def jsonReal64(self, value):
        if self._writeId(JID_REAL64):
            return self._writeData( struct.pack(self.F64, value ) )
        return False
    def jsonString(self, value):
        if self._writeId(JID_STRING):
            if self._writeLength(len(value)):
                return self._writeData(value)
        return false
    def jsonStringToken(self, value):
        if not self.UseTokens:
            return self.jsonString(value)
        id = self.Tokens.get(value, -1)
        if id < 0:
            id = self.TokenCount
            self.Tokens[value] = id
            self.TokenCount += 1
            if not self._writeId(JID_TOKENDEF):
                return False
            if not self._writeLength(id):
                return False
            if not self._writeLength(len(value)):
                return False
            if not self._writeData(value):
                return False
        if self._writeId(JID_TOKENREF):
            return self._writeLength(id)
        return False
    def jsonNewLine(self):
        pass

    def jsonBeginMap(self):
        return self._writeId(JID_MAP_BEGIN)
    def jsonEndMap(self):
        return self._writeId(JID_MAP_END)
    def jsonBeginArray(self):
        return self._writeId(JID_ARRAY_BEGIN)
    def jsonEndArray(self):
        return self._writeId(JID_ARRAY_END)
    def beginUniformArray(self, length, jid_type):
        if jid_type == JID_REAL16:
            # Writing of 16-bit floats is not supported in this version
            jid_type = JID_REAL32
        if not self._startArray(JID_UNIFORM_ARRAY, jid_type):
            return False
        self.BitCount = 0
        self.BitValue = 0
        self.WriteSize = length
        self.WriteCount = 0
        self.UType = jid_type
        return self._writeLength(length)

    def uniformWriteBool(self, value):
        self.BitValue |= (1 << self.BitCount)
        self.BitCount += 1
        if self.BitCount == 32:
            return self._flushBits()
        return self.Writer.uniformWriteBool(value)
    def uniformWriteInt8(self, value):
        return self._uniformData( struct.pack(self.I8, value) )
    def uniformWriteInt16(self, value):
        return self._uniformData( struct.pack(self.I16, value) )
    def uniformWriteInt32(self, value):
        return self._uniformData( struct.pack(self.I32, value) )
    def uniformWriteInt64(self, value):
        return self._uniformData( struct.pack(self.I64, value) )
    def uniformWriteReal16(self, value):
        # 16-bit real not supported in this version
        return self.uniformWriteReal32(value)
    def uniformWriteReal32(self, value):
        return self._uniformData( struct.pack(self.F32, value) )
    def uniformWriteReal64(self, value):
        return self._uniformData( struct.pack(self.F64, value) )
    def endUniformArray(self):
        nwritten = self.WriteCount
        while self.WriteCount < self.WriteSize:
            if self.UType == JID_BOOL:          self.uniformWriteBool(False)
            elif self.UType == JID_INT8:        self.uniformWriteInt8(0)
            elif self.UType == JID_INT16:       self.uniformWriteInt16(0)
            elif self.UType == JID_INT32:       self.uniformWriteInt32(0)
            elif self.UType == JID_INT64:       self.uniformWriteInt64(0)
            elif self.UType == JID_REAL32:      self.uniformWriteReal32(0)
            elif self.UType == JID_REAL64:      self.uniformWriteReal64(0)
            else:
                return False
        if self.UType == JID_BOOL and self.BitCount:
            if not self._flushBits():
                return False
        if not self._finishArray(self.UType):
            return False
        self.UType = JID_NULL
        self.WriteCount = 0
        self.WriteSize = 0
        return True

    def _startArray(self, id, uniform_type=JID_NULL):
        if self._writeId(id):
            if id == JID_UNIFORM_ARRAY:
                return self._writeId(uniform_type)
            return True
        return False

    def _finishArray(self, id):
        if id == JID_ARRAY_BEGIN:
            # Only emit the end-array when non-uniform arrays
            return self._writeId(JID_ARRAY_END)
        return True

    def _uniformData(self, data):
        self.WriteCount += 1
        if self.WriteCount > self.WriteSize:
            return True
        return self._writeData( data )

    def _flushBits(self):
        data = struct.pack( self.U32, self.BitValue)
        if not _writeData( data ):
            return False
        self.BitCount = 0
        self.BitValue = 0
        return True

    def _writeId(self, id):
        return self._writeData( struct.pack(self.U8, id) )

    def _writeLength(self, l):
        if l < 0xf1:
            return self._writeData( struct.pack(self.U8, l) )
        if l < 0xffff:
            if self._writeData( struct.pack(self.U8, 0xf2) ):
                return self._writeData( struct.pack(self.U16, l) )
            return False
        if l < 0xffffffff:
            if self._writeData( struct.pack(self.U8, 0xf4) ):
                return self._writeData( struct.pack(self.U32, l) )
            return False
        if self._writeData( struct.pack(self.U8, 0xf8) ):
            return self._writeData( struct.pack(self.U64, l) )
        return False

    def _writeData(self, data):
        self.Fp.write(data)
        return True

_ESCAPE_DICT = {
    '\\'        : '\\\\',
    '"'         : '\\"',
    '\b'        : '\\b',
    '\f'        : '\\f',
    '\n'        : '\\n',
    '\r'        : '\\r',
    '\t'        : '\\t',
}
#_ESCAPE = re.compile(r'[\x00-\x1f\\"\b\f\n\r\t'])
_ESCAPE = re.compile(r'[\x00-\x1f\\"\b\f\n\r\t]')
for _i in range(0x20):
    _ESCAPE_DICT[chr(_i)] = '\\u%04x' % _i

def _quoteString(s):
    ''' Quote a string using JSON rules '''
    def replace(m):     return _ESCAPE_DICT[m.group(0)]
    return ''.join([ '"', _ESCAPE.sub(replace, s), '"' ])

class _aWriter:
    ''' ASCII JSON Writer '''
    def __init__(self, fp, options):
        self.Fp = fp
        self.Precision = options.get('precision', 12)
        self.IndentStep = options.get('indentstep', 8)
        self.FirstMap = False

        self.Indent = 0
        self.IndentString = ''
        self.Prefix = None
        self.Stack = []
        self.NewLine = False

        self.jsonReal16 = self.jsonReal
        self.jsonReal32 = self.jsonReal
        self.jsonReal64 = self.jsonReal
        self.jsonStringToken = self.jsonString
        self.jsonKeyToken = self.jsonKey
        self.uniformWriteBool = self.jsonBool
        self.uniformWriteInt8 = self.jsonInt
        self.uniformWriteInt16 = self.jsonInt
        self.uniformWriteInt32 = self.jsonInt
        self.uniformWriteInt64 = self.jsonInt
        self.uniformWriteReal16 = self.jsonReal
        self.uniformWriteReal32 = self.jsonReal
        self.uniformWriteReal64 = self.jsonReal
        self.endUniformArray = self.jsonEndArray

    def jsonNull(self):
        self._prefix()
        return self._writeString('null')
    def jsonBool(self, value):
        self._prefix()
        return self._writeString(['true', 'false'][not value])
    def jsonInt(self, value):
        self._prefix()
        return self._writeString('%d' % value)
    def jsonReal(self, value):
        self._prefix()
        return self._writeString('%.*g' % (self.Precision, value))
    def jsonString(self, value):
        self._prefix()
        return self._quotedString(value)
    def jsonKey(self, value):
        if self.FirstMap:
            self._writeString('\n')
            self.FirstMap = False
        self._prefix()
        self._indent()
        self.Prefix = None
        if not self._quotedString(value):
            return False
        return self._writeString(_ASCII_KEY_SEPARATOR)

    def jsonNewLine(self):
        self.NewLine = True

    def jsonBeginMap(self):
        self.FirstMap = True
        self._prefix()
        self._pushState(JID_MAP_BEGIN)
        return self._writeString('{')
    def jsonEndMap(self):
        self._popState()
        if not self.FirstMap:
            if not self._writeString('\n'):
                return False
            self._indent()
        self.FirstMap = False
        return self._writeString(['}', '}\n'][len(self.Stack) == 0])

    def jsonBeginArray(self):
        self._prefix()
        self._pushState(JID_ARRAY_BEGIN)
        return self._writeString('[')

    def jsonEndArray(self):
        self._popState()
        if self._writeString(']'):
            if len(self.Stack) == 0:
                return self._writeString('\n')
            return True
        return False

    def beginUniformArray(self, len, type):
        return self.jsonBeginArray()

    def _setPrefix(self):
        n = len(self.Stack) - 1
        if n >= 0:
            id = self.Stack[n]
            self.Prefix = [ ',', ',\n'][id == JID_MAP_BEGIN]

    def _indent(self):
        return self._writeString(self.IndentString)

    def _prefix(self):
        if not self.Prefix:
            self._setPrefix()
        else:
            self._writeString(self.Prefix)
            if self.NewLine:
                if self.Prefix.find('\n') < 0:
                    self._writeString('\n\t')
                    self._indent()
                    self.NewLine = False
        return True

    def _writeString(self, data):
        return self.Fp.write( data ) != len(data)

    def _quotedString(self, data):
        return self._writeString( _quoteString(data) )

    def _pushState(self, id):
        self.Indent += self.IndentStep
        self.IndentString = '\t'*(self.Indent//8) + ' '*(self.Indent%8)
        self.Stack.append(id)
        self.Prefix = None

    def _popState(self):
        if len(self.Stack) < 1:
            print 'Stack underflow'
            return False
        self.Stack.pop()
        self.Indent -= self.IndentStep
        self.IndentString = '\t'*(self.Indent//8) + ' '*(self.Indent%8)
        self._setPrefix()

class Writer:
    def __init__(self, fp,
                    binary=False,
                    precision=12,
                    indentstep=8,
                    usetokens=True
                ):
        '''
            A class to write JSON (ASCII or binary).  Very minimal error
            detection is performed.

            @param fp         An object that has the write() method defined
            @param binary     Write stream in binary as opposed to ASCII
            @param precision  ASCII format floating point precision
            @param indentstep ASCII format indenting on map objects
            @param usetokens  Binary format optimization to use string tokens

            Houdini HDK Equivalent:  UT_JSONWriter
        '''
        self.Fp = fp
        self.Options = {}
        self.Options['precision'] = precision
        self.Options['indentstep'] = indentstep
        self.Options['usetokens'] = usetokens
        if binary:
            self.Binary = True
            self.Writer = _bWriter(self.Fp, self.Options)
            self.Writer.jsonMagic()
        else:
            self.Binary = False
            self.Writer = _aWriter(self.Fp, self.Options)

    def jsonNull(self):
        ''' Emit a null token '''
        return self.Writer.jsonNull()
    def jsonBool(self, value):
        ''' Emit a true/false token '''
        return self.Writer.jsonBool(value)
    def jsonInt(self, value):
        ''' Emit an integer value '''
        return self.Writer.jsonInt(value)
    def jsonReal16(self, value):
        ''' Emit a 16 bit real value '''
        return self.Writer.jsonReal16(value)
    def jsonReal32(self, value):
        ''' Emit a 32 bit real value '''
        return self.Writer.jsonReal32(value)
    def jsonReal64(self, value):
        ''' Emit a 64 bit real value '''
        return self.Writer.jsonReal64(value)
    def jsonReal(self, value):
        ''' Emit a real value '''
        return self.jsonReal64(value)
    def jsonString(self, value):
        ''' Emit string value.  Please use jsonStringToken() instead '''
        return self.Writer.jsonString(value)
    def jsonStringToken(self, value):
        '''
            Emit a string token (or string value depending on whether tokens
            are enabled)
        '''
        return self.Writer.jsonStringToken(value)
    def jsonKey(self, value):
        ''' Emit string as a map key.  Please use jsonKeyToken() instead '''
        return self.Writer.jsonKey(value)
    def jsonKeyToken(self, value):
        '''
            Emit a string token as a map key (may output a string value
            depending on whether tokens are enabled)
        '''
        return self.Writer.jsonKeyToken(value)

    def jsonNewLine(self):
        ''' Emit a new-line or separator '''
        return self.Writer.jsonNewLine()

    def jsonBeginMap(self):
        ''' Start a map/object '''
        return self.Writer.jsonBeginMap()
    def jsonEndMap(self):
        ''' End a map/object '''
        return self.Writer.jsonEndMap()
    def jsonBeginArray(self):
        ''' Start an array '''
        return self.Writer.jsonBeginArray()
    def jsonEndArray(self):
        ''' End an array '''
        return self.Writer.jsonEndArray()
    def beginUniformArray(self, length, jid_type):
        '''
            Begin an array of uniform values.
            @param length The length of the uniform array
            @param jid_type The type of items (i.e. JID_INT16)
        '''
        return self.Writer.beginUniformArray(length, jid_type)
    def uniformWriteBool(self, value):
        ''' Write a bool to the array of uniform values '''
        return self.Writer.uniformWriteBool(value)
    def uniformWriteInt8(self, value):
        ''' Write an 8-bit integer to the uniform array '''
        return self.Writer.uniformWriteInt8(value)
    def uniformWriteInt16(self, value):
        ''' Write a 16-bit integer to the uniform array '''
        return self.Writer.uniformWriteInt16(value)
    def uniformWriteInt32(self, value):
        ''' Write a 32-bit integer to the uniform array '''
        return self.Writer.uniformWriteInt32(value)
    def uniformWriteInt64(self, value):
        ''' Write a 64-bit integer to the uniform array '''
        return self.Writer.uniformWriteInt64(value)
    def uniformWriteReal16(self, value):
        ''' Write a 16-bit real to the uniform array '''
        return self.Writer.uniformWriteReal16(value)
    def uniformWriteReal32(self, value):
        ''' Write a 32-bit real to the uniform array '''
        return self.Writer.uniformWriteReal32(value)
    def uniformWriteReal64(self, value):
        ''' Write a 64-bit real to the uniform array '''
        return self.Writer.uniformWriteReal64(value)
    def endUniformArray(self):
        ''' End the uniform array '''
        return self.Writer.endUniformArray()

    def _jsonUniformArray(self, jid_type, value, method):
        ''' Internal convenience function to handle uniform arrays '''
        if not self.beginUniformArray(len(value), jid_type):
            return False
        for v in value:
            if not method(v):
                return False
        return self.endUniformArray()

    def jsonUniformArrayBool(self, value):
        ''' Convenience method to output an entire array of bools '''
        return self._jsonUniformArray(JID_INT8, value, self.uniformWriteBool)
    def jsonUniformArrayInt8(self, value):
        ''' Method to output an entire array as 8-bit signed integers '''
        return self._jsonUniformArray(JID_INT8, value, self.uniformWriteInt8)
    def jsonUniformArrayInt16(self, value):
        ''' Method to output an entire array as 16-bit signed integers '''
        return self._jsonUniformArray(JID_INT16, value, self.uniformWriteInt16)
    def jsonUniformArrayInt32(self, value):
        ''' Method to output an entire array as 32-bit signed integers '''
        return self._jsonUniformArray(JID_INT32, value, self.uniformWriteInt32)
    def jsonUniformArrayInt64(self, value):
        ''' Method to output an entire array as 64-bit signed integers '''
        return self._jsonUniformArray(JID_INT64, value, self.uniformWriteInt64)
    def jsonUniformArrayReal16(self, value):
        ''' Method to output an entire array as 16-bit floats '''
        return self._jsonUniformArray(JID_REAL16, value,self.uniformWriteReal16)
    def jsonUniformArrayReal32(self, value):
        ''' Method to output an entire array as 32-bit floats '''
        return self._jsonUniformArray(JID_REAL32, value,self.uniformWriteReal32)
    def jsonUniformArrayReal64(self, value):
        ''' Method to output an entire array as 64-bit floats '''
        return self._jsonUniformArray(JID_REAL64, value,self.uniformWriteReal64)

    def getBinary(self):
        ''' Returns whether the writer is binary or ASCII '''
        return self.Binary
