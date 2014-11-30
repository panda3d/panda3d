-- Lua Syntax Test File
-- Some comments about this file

-- Hello World Program
print "Hello World"
print "An Open String

-- Factorial Calculator
function factorial(n)
  if n == 0 then
    return 1
  end
  return n * factorial(n - 1)
end

-- Fibonacci Numbers
fibs = { 1, 1 }
setmetatable(fibs, {
  __index = function(name, n)
    name[n] = name[n - 1] + name[n - 2] 
    return name[n]
  end
})

-- string buffer implementation
function newbuf ()
  local buf = {
    _buf = {},
    clear =   function (self) self._buf = {}; return self end,
    content = function (self) return table.concat(self._buf) end,
    append =  function (self, s)
      self._buf[#(self._buf) + 1] = s
      return self
    end,
    set =     function (self, s) self._buf = {s}; return self end,
  }
  return buf
end