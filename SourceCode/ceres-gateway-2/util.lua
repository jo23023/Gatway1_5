--
-- Created by IntelliJ IDEA.
-- User: yulin
-- Date: 14-1-25
-- Time: 上午11:39
-- To change this template use File | Settings | File Templates.
--
local io = require"io"

function readAll(file)
    local f = io.open(file, "rb")
    local content = f:read("*all")
    f:close()
    return content
end

function scandir(directory)
    local i, t, popen = 0, {}, io.popen
    for filename in io.popen('ls -a "' .. directory .. '"'):lines() do
        i = i + 1
        t[i] = filename
    end
    return t
end

function file_exists(name)
    local f = io.open(name, "r")
    if f ~= nil then io.close(f) return true else return false end
end

function Set(list)
    local set = {}
    for _, l in pairs(list) do set[l] = true end
    return set
end

function array_concat(...)
    local t = {}

    for i=1, arg.n do
        local array = arg[i]
        if (type(array) == "table") then
            for j = 1, #array do
                t[#t + 1] = array[j]
            end
        else
            t[#t + 1] = array
        end
    end

    return t
end

function tableMerge(t1, t2)
    if(t2 == nil) then
        return t1
    end
    for k,v in pairs(t2) do
        if type(v) == "table" then
            if type(t1[k] or false) == "table" then
                tableMerge(t1[k] or {}, t2[k] or {})
            else
                t1[k] = v
            end
        else
            t1[k] = v
        end
    end
    return t1
end

function tableCat(t1, t2)
  if (t2 == nil) then
    return t1
  end
  
  for k, v in pairs(t2) do
    table.insert(t1, v)
  end
  
  return t1
end


function id_compare(a,b)
  a_id = tonumber(a.id)
  b_id = tonumber(b.id)

  print(a_id, b_id)
  return a_id<b_id
end


function time_compare(a,b)
  a_t = tonumber(a.t)
  b_t = tonumber(b.t)
  return a_t<b_t
end
