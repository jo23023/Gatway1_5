local require = require
local json_module = require("dkjson")

require("util")
local persistence_module = require("persistence")

package.cpath = 'lib/?.so;lib/x86_32/?.so'

local lfs = require("lfs")
assert(lfs ~= nil, "LFS library missing!")

EVENTLIST_DATA_DIR = "./EventList/"

function os.capture(cmd, raw)
	print(cmd)
	local f = assert(io.popen(cmd, 'r'))
	local s = assert(f:read('*a'))
	f:close()
	if raw then return s end
	s = string.gsub(s, '^%s+', '')
	s = string.gsub(s, '%s+$', '')
	s = string.gsub(s, '[\n\r]+', ' ')
	return s
end

function getOldestEventDB(param)
  local event = param
  local eventTime = {}
  local data

  for index = 1, 5, 1 do
    local dataFilePath = EVENTLIST_DATA_DIR .. event.source.id .. "-" .. index .. ".db"

    if index == 1 then
      dataFilePath = EVENTLIST_DATA_DIR .. event.source.id .. ".db"
    end

    if (file_exists(dataFilePath)) then
      data = persistence.load(dataFilePath)
    end

    if data ~= nil then
      eventTime[index] = data[1].t
    end
  end

  local oldestDB = 1
  local oldestTime = eventTime[1]

  for index = 2, 5, 1 do
    if eventTime[index] < oldestTime then
      oldestTime = eventTime[index]
      oldestDB = index
    end
  end

  return oldestDB
end

function addEvent(param)
  local eventid = param.t
  local event = param
  -- event.id = eventid

  local maxSingleEventDBSize = 200
  local maxEventDBNumber = 5
  local data

  for index = 1, maxEventDBNumber, 1 do
    local dataFilePath = EVENTLIST_DATA_DIR .. event.source.id .. "-" .. index .. ".db"

    if index == 1 then
      dataFilePath = EVENTLIST_DATA_DIR .. event.source.id .. ".db"
    end

    if (file_exists(dataFilePath)) then
        data = persistence.load(dataFilePath)
    else
        data = {}
    end

    if (nil == data) then
        data = {}
    end

    if #data >= maxSingleEventDBSize then
      if index == maxEventDBNumber then
        local removeDB = getOldestEventDB(param)
        local removeDataFilePath = EVENTLIST_DATA_DIR .. event.source.id .. "-" .. removeDB .. ".db"

        if removeDB == 1 then
          removeDataFilePath = EVENTLIST_DATA_DIR .. event.source.id .. ".db"
        end

        local cmd_line = "rm " .. removeDataFilePath
        print("database is full, remove the oldest one : " .. removeDataFilePath)
        os.execute(cmd_line)
        addEvent(param)
        break
      else
        if #data > maxSingleEventDBSize then
          local removeStart = maxSingleEventDBSize + 1
          print("database is over max size, trim " .. dataFilePath)

          while #data > maxSingleEventDBSize do
            table.remove(data, 1)
          end

          persistence.store(dataFilePath, data)
        end
      end
    else
      table.insert(data, event)
      persistence.store(dataFilePath, data)
      break
    end
  end
end

function removeEvent(param)
    local data
    local dataFilePath = EVENTLIST_DATA_DIR .. param.itemid .. ".db"

    if (file_exists(dataFilePath)) then
        data = persistence.load(dataFilePath)
    else
        data = {}
    end

    if (nil == data) then
        data = {}
    end
    
    for i = 1, #param.eventid do
        local et = param.eventid[i]
        for j = 1, #data do
            if (et == data[j].t) then
                table.remove(data, j)
                break
            end
        end
    end

    persistence.store(dataFilePath, data)
end

function getEventList(param)
    local data
    local dataFilePath = EVENTLIST_DATA_DIR .. param.id .. ".db"

    if (file_exists(dataFilePath)) then
        data = persistence.load(dataFilePath)
        table.sort(data, id_compare)
    else
        data = {}
    end

    if (table.getn(data) > 200) then
        table_len = table.getn(data)
        for i = table_len, 100, -1 do
            table.remove(data, i)
        end
        persistence.store(dataFilePath, data)
    end

    print(json_module.encode(data, { indent = true }))
    return data
end

function getFileTable(path)
  local files = {}
  local i, j
  for f in lfs.dir(path) do
    if f~="." and f~=".." then
      i, j = string.find(f, "^%d+%.db$")
      if i ~= nil and j ~= nil then
        table.insert(files, path .. f)
      end

      i, j = string.find(f, "^%d+%-%d+%.db$")
      if i ~= nil and j ~= nil then
        table.insert(files, path .. f)
      end
    end
  end
  
  return files
end

-- {"event_del":[{"id":"9","t":946684915},{"id":"8","t":946684943}]}
function removeEventMix(param)
--  print("removeEventMix() param = " .. json_module.encode(param))
  
  if (nil == param.event_del) then
    return
  end
  
  local data
  local ftable = getFileTable(EVENTLIST_DATA_DIR)
  
  for index, file in pairs(ftable) do
    print(index .. " = ".. file)
    data = persistence.load(file)
    if (nil ~= data and table.getn(data) > 0) then
      local srcId = data[1].source.id
      for k1, v1 in pairs(param.event_del) do
         for j = 1, #data do
            if (v1.t == data[j].t and v1.id == srcId) then
                table.remove(data, j)
                break
            end
        end
      end
      
      persistence.store(file, data)
    end
  end
  
  data = nil
  ftable = nil
end

function getEventListMix(param)
--  print("getEventListMix() param = " .. json_module.encode(param))
  local events = {}
  local data
  
  if (param.start ~= nil and param.stop ~= nil) then
    local ftable = getFileTable(EVENTLIST_DATA_DIR)
    
    for k, v in pairs(ftable) do
      print(k .. " = ".. v)
      data = persistence.load(v)
      if (nil ~= data) then
        for k, v in pairs(data) do
          --print("v = ".. json_module.encode(v))
          if (v.t > param.start and v.t < param.stop) then
            --print("========> insert one ")
            table.insert(events, v)
          end
        end
      end
    end
  
  -- table.sort(events, time_compare)
  -- print(json_module.encode(events, { indent = true }))
  end
  
  data = nil
  
  --print("events = " .. json_module.encode(events))
  return events
end

function getEventListSeparate(param)
--  print("getEventListSeparate() param = " .. json_module.encode(param))
  local event_total = 0
  local data
  
  if (param.start ~= nil and param.stop ~= nil) then
    local ftable = getFileTable(EVENTLIST_DATA_DIR)
    local event_order = 1
    
    for k, v in pairs(ftable) do
      local events = {}
      print(k .. " = ".. v)
      data = persistence.load(v)
      if (nil ~= data) then
        for k, v in pairs(data) do
          if (v.t > param.start and v.t < param.stop) then
            table.insert(events, v)

            if #events == 200 then
              if c_p2p_send_eventlist ~= nil then
                local rt = { ["order"] = tostring(event_order), ["result"] = events, ["error"] = nil, ["callid"] = param.callid, ["method"] = "getEventListMix"}
                local r = json_module.encode(rt)
                c_p2p_send_eventlist(r)
                event_total = event_order
                event_order = event_order + 1
              end

              events = {}
            end 
          end
        end
        
        if c_p2p_send_eventlist ~= nil and #events ~= 0 then
          local rt = { ["order"] = tostring(event_order), ["result"] = events, ["error"] = nil, ["callid"] = param.callid, ["method"] = "getEventListMix"}
          local r = json_module.encode(rt)
          c_p2p_send_eventlist(r)
          event_total = event_order
          event_order = event_order + 1
        end
      end
    end
  
  end
  
  data = nil

  return event_total
end

function removeEventDB(itemId)
	local dataFilePath = EVENTLIST_DATA_DIR .. itemId .. ".db"

	if (file_exists(dataFilePath)) then
		local cmdline = "rm -rf"

		cmdline = cmdline .. " "
		cmdline = cmdline .. dataFilePath
		os.capture(cmdline, true)
	else
		return
	end
end

function eventlist_test()
    t = '{"method":"addEvent","param":{"t":123, "source":{"id":"3", "action":"OnOff", "value":"On"}, "target":{"id":"4", "action":"Record"}}}'
    addEvent(json_module.decode(t).param)

    t = '{"method":"getEventList","param":{"id":"3"}}}'
    getEventList(json_module.decode(t).param)

    t = '{"method":"removeEvent", "param":{"itemid":"3", "eventid":[1392309490,1392309466]}}'
    removeEvent(json_module.decode(t).param)
end

--eventlist_test()
