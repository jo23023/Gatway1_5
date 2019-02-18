--
-- Created by IntelliJ IDEA.
-- User: yulin
-- Date: 14-1-25
-- Time: 上午11:31
--

require("persistence")
require("util")
require("eventlist")
require("link")
require("setting")

-- json 解析库
local json_module = require("dkjson")

local lfs = require("lfs")
assert(lfs ~= nil, "LFS library missing!")

-- 全局变量定义
g_item_pool = {}
g_model_pool = {}
g_bind_id_map = {} --bind参数与id参数之间的映射
g_status_poll = {}
g_item_repo = {} --主要用于保存一些临时变量
g_item_repo_pre = {} -- Store previous status of item
g_bind_fullmodelno = {}

g_aeskey_pool = {}   -- 保存所有AES key

g_max_id = 0

g_device_tree = {}
g_export = {} --用于导出方法符号

debug = false
SD_path = nil

-- chenjian add 2014-10-08
g_path_aeskey_db = nil
g_path_armlinktable_db = nil
g_path_partialarmlinktable_db = nil
g_path_config_db = nil
g_path_item_db = nil
g_path_linktable_db = nil
g_path_schedule_db = nil
g_path_virtualdevice_db = nil

g_arm_delay_status = "off"

g_free_cache_cmd = "echo 3 > /proc/sys/vm/drop_caches&"

function msleep(n)
  os.execute("sleep " .. 1)
end

function logDebug(msg)
  print("[ CORE LUA   ] " .. msg)
end

function logWarn(msg)
  print("[ CORE LUA   ] " .. msg)
end

function set_db_path()
  g_path_aeskey_db = "/mnt/aeskey.db"
  g_path_armlinktable_db = "/mnt/armlinktable.db"
  g_path_partialarmlinktable_db = "/mnt/partialarmlinktable.db"
  g_path_config_db = "/mnt/config.db"
  g_path_item_db = "/mnt/item.db"
  g_path_linktable_db = "/mnt/linktable.db"
  g_path_schedule_db = "/mnt/schedule.db"
  g_path_virtualdevice_db = "/mnt/virtualdevice.db"
  g_path_item_status_db = "/mnt/item_status.db"
end

-- 获取SD卡的路径
function get_SD_path()
  local sda1Cmd = "cd /media/mmcblk0/"
  local sdaCmd = "cd /media/mmcblk0p1/"

  if os.execute(sda1Cmd) == 0 then
    SD_path = "/media/mmcblk0/"
  elseif os.execute(sdaCmd) == 0 then
    SD_path = "/media/mmcblk0p1/"
  end
  
  if nil == SD_path then
    logWarn("Attention: get SD Card path fail!!")
  else
    -- 创建 EventList 目录
--    if os.execute("cd " .. SD_path .. "EventList >nul 2>nul") == 0 then
--      print("EventList exist")
--    else
--      os.execute("mkdir " .. SD_path .. "EventList -p")
--    end
    
    os.execute("cd -")
    logDebug("SD_path = " .. SD_path)
  end
end

function printMemorySize(where)
  --logDebug(string.rep("*", 80))
  local mSize = collectgarbage("count")
  logDebug(where .. " memory used(KB): " .. tostring(mSize))  
  logDebug(string.rep("*", 80))
end

function get_cameras_state()
  logDebug("get_cameras_state")
  cams = {}
  for k, v in pairs(g_item_pool) do
    if ("JSW-Camera-0001" == v.fullmodelno) then
      cam = {["id"] = v.id, ["did"] = v.did, ["password"] = v.password}
      table.insert(cams, cam)
    end
  end

  local r = json_module.encode(cams)
  return r
end

function item_id_compare(a, b)
  a_id = tonumber(a[2].id)
  b_id = tonumber(b[2].id)

  return a_id < b_id
end

function load_sensor_model()
  --logDebug("load_sensor_model")
  -- models = { "JSW-Camera-0001", "JSW-DIMMER-B091", "JSW-DoorBell-B099", "JSW-Group-0001", "JSW-Light1-B082", "JSW-PIR-B090", "JSW-Room-0001", "JSW-Slot1-B084", "JSW-GateMaglock-0001"}
  models = {}

  -- TODO models 应该从目录中提取
  for entry in lfs.dir("./sensor/model") do
    if (entry == ".." or entry == ".") then
    else
      table.insert(models, entry)
    end
  end

  for i = 1, #models do
    local fullmodelno = models[i]
    if (not file_exists("./sensor/model/" .. fullmodelno)) then
      return nil
    end

    local json = readAll("./sensor/model/" .. fullmodelno)
    if (json == nil) then
      return
    end

    local obj, pos, err = json_module.decode(json, 1, nil)
    if err then
      logDebug("Error:" .. err)
      return nil
    end

    g_model_pool[models[i]] = obj
  end

  --logDebug("---------------------")
end

--根据id获取item
function getItemById(id)
  local k = "" .. id
  return g_item_pool[k]
end

--根据fullmodelno获取Model
function getModelByNo(fullmodelno)
  return g_item_pool[fullmodelno]
end

--从数据库中加载已经添加的sensor
function load_sensor_from_db()
  --logDebug("load_sensor_from_db ------------------------------------------")
  g_item_pool = persistence.load(g_path_item_db)
  if (g_item_pool == nil) then
    g_item_pool = {}
  end
  table.sort(g_item_pool, item_id_compare)

  local temp_exist = false

  for k, v in pairs(g_item_pool) do

    if (tonumber(v.id) > g_max_id) then
      g_max_id = tonumber(v.id)
    end

    -- local json = json_module.encode(v, { indent = true })
    -- logDebug(json)
    g_item_pool[k] = v
    g_bind_id_map["bind:" .. v.bind] = v.id
    g_bind_fullmodelno["bind"] = v.bind
    g_bind_fullmodelno["fullmodelno"] = v.fullmodelno

    if v.fullmodelno == "JSW-GateTemp-0001" then
      temp_exist = true
    end
  end

  if temp_exist == false then
    local new_temp = {}

    new_temp.param = {}
    new_temp.param.name = "temperature"
    new_temp.param.seat = "gateway"
    new_temp.param.busname = "ttyS0"
    new_temp.param.fullmodelno = "JSW-GateTemp-0001"
    new_temp.param.bind = "ttyS0=868/123456789/0"
    new_temp.param.nodeid = "0"
    new_temp.param.tosaveintodb = true
    new_temp.param.parent = "2"

    new_temp.option = {}
    new_temp.option.tosaveintodb = true

    addItem(new_temp.param, new_temp.option)
  end

  --TODO: Item 排序问题
end

function load_sensor_status_from_db()
  --logDebug("load_sensor_status_from_db")
  g_item_repo = persistence.load(g_path_item_status_db)
  if (g_item_repo == nil) then
    g_item_repo = {}
  end

  g_item_repo_pre = persistence.load(g_path_item_status_db)
  if (g_item_repo_pre == nil) then
    g_item_repo_pre = {}
  end

  table.sort(g_item_repo, item_id_compare)
  table.sort(g_item_repo_pre, item_id_compare)
  --logDebug("sensor_status : " .. getjson(g_item_repo))
  --logDebug("sensor_status : " .. getjson(g_item_repo_pre))
end

--取得下一个可用的item id
function next_id()
  --TODO add MUTEX
  g_max_id = g_max_id + 1
  return g_max_id
end


--将item添加到数据库中
function _addItem(parent, child, toSaveDB)
  if not (parent ~= nil and child ~= nil) then
    logDebug("addItem: parameters error!")
    return nil
  end

  local update_item_status = false

  --如果child还没有id说明是全新的item，需要获取新的id
  if (child.id == nil) then
    logDebug("child has no id, allocate one ID for it")
    child.id = "" .. next_id()
    update_item_status = true

    --将全新的item加入g_item_pool
    g_item_pool[child.id] = child

    --添加child-》group
    child.parent = parent.id

    --添加bind到id的映射
    g_bind_id_map["bind:" .. child.bind] = child.id
  end

  --检查parent.child是否存在
  local child_dict
  if (parent.child ~= nil) then
    -- existed
    logDebug("parent has child, use old dict")
    child_dict = parent.child
  else
    -- no exist
    logDebug("parent has no child before, create a new dict for parent")
    child_dict = Set{}
    parent.child = child_dict
  end

  --  为item创建status pool
  if (update_item_status) then
    g_item_repo[child.id] = { ["status"] = {} };
    g_item_repo_pre[child.id] = { ["status"] = {} };
  end

  --  child节点中的子节点
  parent.child[child.id] = {} --child
  logDebug("parent item--->" .. json_module.encode(parent, { indent = true }))

  if (toSaveDB) then
    --commit child into db
    table.sort(g_item_pool, compare_id)
    persistence.store(g_path_item_db, g_item_pool)

    if (update_item_status) then
      persistence.store(g_path_item_status_db, g_item_repo)
    end
  end
  --logDebug("addItem complete ------------------------------------------")

  return child
end


--将item从数据库中删除
function _removeItem(parent, child, toSaveDB)

  if not (parent ~= nil and child ~= nil) then
    logDebug("_removeItem: parameters error!")
    return nil
  end

  if (parent.child == nil) then
    logDebug("_removeItem: parent.child == nil!")
    return nil
  end

  -- remove item
  parent.child[child.id] = nil
  g_item_pool[child.id] = nil
  --删除bind到id的映射
  g_bind_id_map["bind:" .. child.bind] = nil  --20150209_victorwu: fix remote still control gateway after removing it

  --  如果删除的child是group的话，需要移动他的child到default group
  if (child.child ~= nil) then
    local newParent = getItemById("2")
    for k, _ in pairs(child.child) do
      local nestChild = getItemById(k)
      nestChild.parent = "2"
      _addItem(newParent, nestChild, false)
    end
  else
    --该item不包含其他子item，删除它的status库
    g_item_repo[child.id] = nil;
    g_item_repo_pre[child.id] = nil;
  end

  logDebug(json_module.encode(parent, { indent = false }))

  if (toSaveDB) then
    --commit child into db
    table.sort(g_item_pool, compare_id)
    persistence.store(g_path_item_db, g_item_pool)
    persistence.store(g_path_item_status_db, g_item_repo)
  end
  --logDebug("_removeItem ------------------------------------------")

  return child
end

--将item从数据库中删除
function _updateItem(parent, child, changes, toSaveDB)

  logDebug("_updateItem")
  if not (parent ~= nil and child ~= nil) then
    logDebug("_updateItem: parameters error!")
    return nil
  end

  --更新parent-child关系
  _addItem(parent, child, toSaveDB)
  changes.child = nil
  tableMerge(child, changes)

  logDebug(json_module.encode(child, { indent = true }))

  if (toSaveDB) then
    --commit child into db
    table.sort(g_item_pool, compare_id)

    persistence.store(g_path_item_db, g_item_pool)
  end
  logDebug("_updateItem ------------------------------------------")

  return child
end

--根据key值从status中获取状态信息
function getStatusValue(key)
  logDebug(key)
  status = g_status_poll[key]
  if (status ~= nil) then
    return json_module.encode(status, { intent = true })
  else
    return nil
  end
end

function addItem(param, option) --param is table
  local parent = getItemById(param.parent)
  --  新添加Item，没有指定id
  if (parent ~= nil and param.name ~= nil and param.bind ~= nil and param.fullmodelno ~= nil) then
    --  参数OK
    param.group = nil
    local item = param --Item:new(json_module.encode(param))
    item = _addItem(parent, item, option.tosaveintodb)
    if (item ~= nil) then
      logDebug("addItem success")
      
      if "JSW-Camera-0001" == item.fullmodelno and nil ~= c_start_camera_add then
        c_start_camera_add(item.did .. ";" .. item.password)
      end
      
      return item
    else
      logDebug("!!!!ERROR in _addItem")
    end
    
    return item
  else
  --参数检查没有通过
  end
end

function removeItem(param, option)
  local parent = getItemById(param.parent)
  local child = getItemById(param.child)

  --  removeItem
  if (parent ~= nil and child ~= nil) then
    child = _removeItem(parent, child, option.tosaveintodb)
    
    -- delete this item info in linktable.db
    filterLinkItem(child.id);
    
    -- delete this item info in arm_linktable.db
    filterARMLinkItem(child.id)
    
    -- delete this item info in schedule.db
    filterScheduleItem(child.id)
    
    -- delete event list of this item
    removeEventDB(child.id)
    
    if (child ~= nil) then
      logDebug("removeItem success")

      -- send unmatch command
      child.action = "unmatch"
      child.method = "unmatch"
      child.result_varname = "unmatch_result"
      -- logDebug("unmatch " .. getjson(child))
      controlItem(child)

      if "JSW-Camera-0001" == child.fullmodelno and nil ~= c_start_camera_remove then
        c_start_camera_remove(child.did .. ";" .. child.password)
      end
      return child
    else
      return nil
    end
  end

  --  here，说明有地方出错了，直接返回nil
  return nil
end

function updateItem(param, option)
  if (param.parent == nil or param.child == nil) then
    local rt = { ["result"] = nil, ["error"] = "key params, parent or child, is not specified.", ["callid"] = -1 }
    return rt
  end

  parent = getItemById(param.parent)
  child = getItemById(param.child)
  --  更新Item
  if (parent ~= nil and child ~= nil) then
    --  parent/child 都是存在的
    local oldDID = nil
    local oldPW = nil
    if "JSW-Camera-0001" == child.fullmodelno then
      oldDID = child.did
      oldPW = child.password
    end

    child = _updateItem(parent, child, param, option.tosaveintodb)
    if (child ~= nil) then
      logDebug("updateItem success")
      
      if nil ~= oldDID and nil ~= oldPW then
        -- 如果DID, password 都没改变,则不需要更新
        if oldDID == child.did and oldPW == child.password then
          logWarn("IPCamera did and password is same.")
        else
          logWarn("IPCamera did or password is changed.")
        
          if nil ~= c_start_camera_remove then
            c_start_camera_remove(oldDID .. ";" .. oldPW)
          end
          
          os.execute("sleep 1")
          
          if nil ~= c_start_camera_add then
            c_start_camera_add(child.did .. ";" .. child.password)
          end
        end
      end
      
      return child
    else
      return nil
    end
  end

  --  here，说明有地方出错了，直接返回nil
  return nil
end

function checkItemBind(id, bind)
  logDebug("begin checkItemBind")
  if nil == id or nil == bind then
    logDebug("id or bind is nil")
  else
    local item = getItemById(id)
    if nil == item then
      logDebug(id .. ": this sensor is not exist in item.db")
    elseif item.bind == nil then
      logDebug(id .. ": this sensor has no bind")
    elseif (item.bind == bind) then
      logDebug("this sensor is VALID in my system")
    else
      logDebug("this sensor is IN-VALID in my system")
    end
  end
  
  logDebug("end checkItemBind")
end

function controlMultiItem(param)
  for k, v in pairs(param) do
    if string.find(k, "control") ~= nil then
      controlItem(v)
      os.execute("usleep 100000")
    end
  end
end

function controlItem(param)
  --如果直接执行lua脚本，该函数将是nil
  if (c_mqtt_publish_message == nil) then
    local rt = { ["result"] = nil, ["error"] = "no c_mqtt_publish_message, you execute the lua script directly, please define it in your c code", ["callid"] = -1 }
    return rt
  end

  local j = json_module.encode(param)
  if (param.bind ~= nil) then
    local bind = param.bind
    local b, e = string.find(bind, "=")
    local busname = string.sub(bind, 1, b - 1)

    param.busname = busname;
    -- checkItemBind(param.id, param.bind)
    c_mqtt_publish_message(param.busname, j)
    
    if "match" ~= param.action and param.value ~= nil then
      if "OnOff" == param.action then
        -- update item status
        setItemOneStatus(param.id, "status", param.value)
      elseif "VolumeLevel" == param.action then
        -- update volume status
        setItemOneStatus(param.id, "volume", param.value)
      elseif "LedOnOff" == param.action then
        -- update led status
        setItemOneStatus(param.id, "led", param.value)
      elseif "AlarmTime" == param.action then
        -- update alarm time
        setItemOneStatus(param.id, "alarm_time", param.value)
      end
    end
    
    local rt = { ["result"] = nil, ["error"] = "please get return value with correct get method.", ["callid"] = -1 }
    return rt
  end

  if (param.busname ~= nil) then
    -- checkItemBind(param.id, param.bind)
    c_mqtt_publish_message(param.busname, j)
    
    if "match" ~= param.action and param.value ~= nil then
      if "OnOff" == param.action then
        -- update item status
        setItemOneStatus(param.id, "status", param.value)
      elseif "VolumeLevel" == param.action then
        -- update volume status
        setItemOneStatus(param.id, "volume", param.value)
      elseif "LedOnOff" == param.action then
        -- update led status
        setItemOneStatus(param.id, "led", param.value)
      elseif "AlarmTime" == param.action then
        -- update alarm time
        setItemOneStatus(param.id, "alarm_time", param.value)
      end
    end
    
    local rt = { ["result"] = nil, ["error"] = "please get return value with correct get method.", ["callid"] = -1 }
    return rt
  end
end

function updateKeyValue(t)
  --{"method":"updateKeyValue","key":"result:match:1390810483101","result":{"type":"Light","pid":"xxxx-yyyy-zzzz","fullmodelno":"JSW-Light1-B082","busname":"ttyS0"}}
  g_status_poll[t.key] = t.result
  -- logDebug("g_status_poll----> ", json_module.encode(g_status_poll))
  -- logDebug(t.key, "----> ", json_module.encode(g_status_poll[t.key]))
end

function getValueByKey(t)
  local rt = g_status_poll[t.key]
  --  g_status_poll[t.param.key] = nil --清理掉，非常重要，防止无限制增长
  if (rt == nil) then
    local rt = { ["result"] = nil, ["error"] = "key does no exist!!!", ["callid"] = -1 }
    local r = json_module.encode(rt)
    logDebug(r)
    return r
  end

  local rt_rpc = {}
  rt_rpc.result = rt
  --logDebug(getjson(rt_rpc))
  return rt_rpc
end

function lua_dispatch(payload)
 -- logWarn("lua_dispatch payload = " .. payload)
  --json参数校验
  local t, pos, err = json_module.decode(payload, 1, nil)
  if err then
    --    logDebug(err)
    local rt = { ["result"] = nil, ["error"] = err, ["callid"] = -1 }
    local r = json_module.encode(rt)
    return r
  end

  if (t.method == nil) then
    local rt = { ["result"] = nil, ["error"] = "method is nil", ["callid"] = -1 }
    local r = json_module.encode(rt)
    return r
  end

  if t.method == "getEventListMix" then
    t.method = "getEventListSeparate"
  end

  method = g_export[t.method]
  if (method == nil) then
    local rt = { ["result"] = nil, ["error"] = "method " .. t.method .. " is invalid", ["callid"] = t.callid }
    local r = json_module.encode(rt)
    return r
  end

  --参数的基本校验完成
  if (t.method ~= "getNextActionList") then
    --logWarn("lua_dispatch, method:" .. t.method .. " param:" .. getjson(t.param))
  end

  if (t.option == nil) then
    t.option = {}
  end

  --printMemorySize("before call method: " .. t.method);

  if t.method == "getEventListSeparate" then
    t.param.callid = t.callid
  end

  if t.method == "getEventListSeparate" then
    local event_total = method(t.param, t.option)
    collectgarbage("collect")

    local rt = { ["total"] = tostring(event_total), ["status"] = "finish", ["result"] = {}, ["error"] = nil, ["callid"] = t.callid, ["method"] = "getEventListMix" }
    local r = json_module.encode(rt)
    return r
  else
    local r = method(t.param, t.option) --调用method，并以t为参数
    collectgarbage("collect")
    --printMemorySize("after call method: " .. t.method);

    if (r == nil) then
      local rt = { ["result"] = nil, ["error"] = t.method .. " return value is nil", ["callid"] = t.callid, ["method"] = t.method }
      local r = json_module.encode(rt)
      return r
    end
    if (type(r) ~= "table") then
      --  返回参数的类型不正确
      local rt = { ["result"] = nil, ["error"] = "return value is invalid", ["callid"] = t.callid, ["method"] = t.method }
      local r = json_module.encode(rt)
      return r
    else
      local rt = { ["result"] = r, ["error"] = nil, ["callid"] = t.callid, ["method"] = t.method }
      local r = json_module.encode(rt)
      return r
    end
  end
end

function ex_devicejsontree()
  -- logDebug(getjson(g_device_tree))
  return g_device_tree

end

function getjson(t)
  return json_module.encode(t)
end

g_linktable = {}
g_nextaction_pool = {}

function loadLink()
  g_linktable = persistence.load(g_path_linktable_db)
  if (g_linktable == nil) then
    g_linktable = {}
  end
end

g_arm_linktable = {}
function loadArmLink()
  g_arm_linktable = persistence.load(g_path_armlinktable_db)
  if (g_arm_linktable == nil) then
    g_arm_linktable = {}
  end
end

g_partialarm_linktable = {}
function loadPartialArmLink()
  g_partialarm_linktable = persistence.load(g_path_partialarmlinktable_db)
  if (g_partialarm_linktable == nil) then
    g_partialarm_linktable = {}
  end
end

-- {"enable":"no","source":{"nodeid":"0","id":"4","tosaveintodb":true,"busname":"ttyS0","parent":"2","bind":"ttyS0=868/3387458107/0","fullmodelno":"JSW-GateMaglock-0001","command":[{"value":["Open","Close"],"type":"OpenClose","name":"OpenClose"},{"value":["Open2","Close2"],"type":"OpenClose","name":"OpenClose2"}],"name":"门磁","icon":"images/model/GateMaglock.png","seat":"哦咯"}}
function setLinkToggle(t)
  logDebug("setLinkToggle " .. getjson(t))
  local rt = {}
  
  if (t.enable ~= nil and t.source ~= nil and t.source.id ~= nil and t.source.action ~= nil) then
    local k
    if (t.source.value == nil) then
      k = "id_" .. t.source.id .. "_action_" .. t.source.action
    else
      k = "id_" .. t.source.id .. "_action_" .. t.source.action .. "_value_" .. t.source.value
    end
    
    -- 首先精确匹配
    local exist = g_linktable[k]
    
    -- 查看是否仅仅是最后的value部分不同(Open-->Close)    
    if (exist == nil) then
      local pre = "id_" .. t.source.id .. "_action_" .. t.source.action .. "_value_"
      for gk, gv in pairs(g_linktable) do
        local _, e = string.find(gk, pre)
        if e ~= nil then
          g_linktable[gk] = nil
        end
      end
      
      exist = {}
    end
    
    exist.enable = t.enable

    g_linktable[k] = exist
    persistence.store(g_path_linktable_db, g_linktable)
    
    rt.result = "success"
  else
    rt.result = "params format error"
  end
  
  return rt
end

function addLink(t)
  logDebug("addLink " .. getjson(t))

  local rt = {}

  -- '{"method":"addLink","param":{"source":{"id":"3", "action":"OnOff", "value":"On"}, "target":{"id":"4", "action":"Record"}}}'
  if (t.source ~= nil and t.source.id ~= nil and t.source.action ~= nil) then
    local k
    if (t.source.value == nil) then
      k = "id_" .. t.source.id .. "_action_" .. t.source.action
    else
      k = "id_" .. t.source.id .. "_action_" .. t.source.action .. "_value_" .. t.source.value
    end
    t.key = k

    -- 先精确匹配
    local exist = g_linktable[k]

    -- 查看是否仅仅是最后的value部分不同(Open-->Close)    
    if (exist == nil) then
      local pre = "id_" .. t.source.id .. "_action_" .. t.source.action .. "_value_"
      for gk, gv in pairs(g_linktable) do
        local _, e = string.find(gk, pre)
        if e ~= nil then
          g_linktable[gk] = nil
        end
      end
      
      exist = {}
    end

    exist["id_" .. t.source.id .. "_" .. t.target.id] = t
    --        table.insert(exist, t)
    g_linktable[k] = exist

    persistence.store(g_path_linktable_db, g_linktable)

    --logDebug(getjson(g_linktable))
    rt.result = "success"
  else
    rt.result = "fail, invalid params"
  end

  return rt
end

function removeLink(t)
  logDebug("removeLink " .. getjson(t))
  --logDebug(getjson(g_linktable))

  local rt = {}

  if (g_linktable[t.key] ~= nil) then
    for k, v in pairs(g_linktable[t.key]) do
      if (k ~= "enable") then
        if ("id_" .. v.source.id .. "_" .. t.target.id == k) then
           logDebug("found link to delete, " .. k .. getjson(g_linktable[t.key][k]))
  
           g_linktable[t.key][k] = nil
  
           persistence.store(g_path_linktable_db, g_linktable)
           break;
        end
      end
    end
  end

  rt.result = "success"
  return rt
end

function filterLinkItem(itemId)
  logDebug("filterLinkItem. itemId = " .. itemId)
  
  -- filter sender device
  logDebug("check sender device")
  for k, v in pairs(g_linktable) do
    s,e = string.find(k, "id_" .. itemId .. "_")
    if (s ~= nil) then
      logDebug("   delete " .. k)
      g_linktable[k] = nil
    end
  end
  
  -- filter receiver device
  logDebug("check receiver device")
  for k, v in pairs(g_linktable) do
    --logDebug("== in key:" .. k)
    s1, e1 = string.find(k, "id_%d+_")
    if (s1 ~= nil) then
      pre = string.sub(k, s1, e1)
      if (v[pre .. itemId] ~= nil) then
        logDebug("==delete:" .. pre .. itemId)
        v[pre .. itemId] = nil
      end
    end
  end
  
  persistence.store(g_path_linktable_db, g_linktable)
end

function getRemotekeyValue()
  logDebug("getRemotekeyValue: " .. g_config.setting.remotekey)
  local ret = {}
  ret["remotekey"] = g_config.setting.remotekey
  return ret
end

-- from APP Control page ARM group button
function setRemotekeyValue(param, option)
  logDebug("setRemotekeyValue. param = " .. getjson(param))
  local res = {}
  if (param.remotekey ~= nil) then
    local prepare_state = g_config.setting.prepareState
    local remote_key = g_config.setting.remotekey
    if "Lock" == param.remotekey then
      if "PartialLock" == prepare_state or "PartialLock" == remote_key then
        DoARMUnlockOperations(false, false)
        os.execute("sleep 1")
        ARMLockDelayStart(param.remotekey)
      else
        ARMLockDelayStart(param.remotekey)
      end
    elseif "PartialLock" == param.remotekey then
      if "Lock" == prepare_state or "Lock" == remote_key then
        DoARMUnlockOperations(false, false)
        os.execute("sleep 1")
        ARMLockDelayStart(param.remotekey)
      else
        ARMLockDelayStart(param.remotekey)
      end
    elseif "Unlock" == param.remotekey then
      if "Unlock" == remote_key then
        DoARMUnlockOperations(true, false)
      else
        DoARMUnlockOperations(true, true)
      end
    end
    
    res.result = "success"
  else
    res.result = "param format error"
  end
  
  return res;
end

function setArmLinktable(param, option)
  logDebug("setArmLinktable param = " .. getjson(param))
  local res = ""
  
  if (param.name ~= nil and param.enable ~= nil and param.source ~= nil and param.target) ~= nil then
    g_arm_linktable = param
    persistence.store(g_path_armlinktable_db, g_arm_linktable)
    res = {["result"] = "success"}
  else
    res = {["result"] = "error", ["reason"] = "format error"}
  end
  
  return res
end

function setPartialArmLinktable(param, option)
  logDebug("setPartialArmLinktable param = " .. getjson(param))
  local res = ""
  
  if (param.name ~= nil and param.enable ~= nil and param.source ~= nil and param.target) ~= nil then
    g_partialarm_linktable = param
    persistence.store(g_path_partialarmlinktable_db, g_partialarm_linktable)
    res = {["result"] = "success"}
  else
    res = {["result"] = "error", ["reason"] = "format error"}
  end
  
  return res
end

function filterARMLinkItem(itemId)
  logDebug("filterARMLinkItem itemId = " .. itemId)
  
  for k, v in pairs(g_arm_linktable) do
    -- find in target
    if k == "target" then
      for kt, vt in pairs(v) do
        if kt == itemId then
          v[kt] = nil
          persistence.store(g_path_armlinktable_db, g_arm_linktable)
          logDebug("==delete sensor in arm_linktable-target")
          break
        end
      end
    end  
    
    -- find in source
    if k == "source" then
      for ks, vs in pairs(v) do
        if ks == itemId then
          v[ks] = nil
          persistence.store(g_path_armlinktable_db, g_arm_linktable)
          logDebug("==delete sensor in arm_linktable-source")
          break
        end
      end
    end
  end
end

function filterPartialARMLinkItem(itemId)
  logDebug("filterPartialARMLinkItem itemId = " .. itemId)
  
  for k, v in pairs(g_partialarm_linktable) do
    -- find in target
    if k == "target" then
      for kt, vt in pairs(v) do
        if kt == itemId then
          v[kt] = nil
          persistence.store(g_path_partialarmlinktable_db, g_partialarm_linktable)
          logDebug("==delete sensor in partialarm_linktable-target")
          break
        end
      end
    end  
    
    -- find in source
    if k == "source" then
      for ks, vs in pairs(v) do
        if ks == itemId then
          v[ks] = nil
          persistence.store(g_path_partialarmlinktable_db, g_partialarm_linktable)
          logDebug("==delete sensor in partialarm_linktable-source")
          break
        end
      end
    end
  end
end

function getArmLinktable()
--  logDebug("getArmLinktable " .. getjson(g_arm_linktable))
  local ret = g_arm_linktable
  ret["remotekey"] = g_config.setting.remotekey
  return ret
end

function getPartialArmLinktable()
--  logDebug("getPartialArmLinktable " .. getjson(g_partialarm_linktable))
  local ret = g_partialarm_linktable
  ret["remotekey"] = g_config.setting.remotekey
  return ret
end

g_schedule = {}
function loadSchedule()
  g_schedule = persistence.load(g_path_schedule_db)
  if (g_schedule == nil) then
    g_schedule = {}
  end
end

g_VirtualDevice = {}
function loadVirtualDevice()
  g_VirtualDevice = persistence.load(g_path_virtualdevice_db)
  if (nil == g_VirtualDevice) then
    g_VirtualDevice = {}
  end
end

function in_array(b, list)
  if not list then
    return false
  end
  if list then
    for k, v in pairs(list) do
      if v.tableName == b then
        return true
      end
    end
  end
end

function Split(szFullString, szSeparator)
  local nFindStartIndex = 1
  local nSplitIndex = 1
  local nSplitArray = {}
  while true do
    local nFindLastIndex = string.find(szFullString, szSeparator, nFindStartIndex)
    if not nFindLastIndex then
      nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, string.len(szFullString))
      break
    end
    nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, nFindLastIndex - 1)
    nFindStartIndex = nFindLastIndex + string.len(szSeparator)
    nSplitIndex = nSplitIndex + 1
  end
  return nSplitArray
end

-- trigger sirens in group
function remotekeyCameraRecord(groupID)
  logDebug("remotekeyCameraRecord. groupID = " .. groupID)
  
  -- check group exist or not
  if nil == g_VirtualDevice[groupID] then 
    logWarn("group: " .. groupID .. " not exist!")
    return
  end
  
  local items = g_VirtualDevice[groupID]["items"]
  
  if nil ~= c_start_camera_record then
    for k, v in pairs(items) do
      local item = getItemById(k)
      if (item ~= nil and "JSW-Camera-0001" == item.fullmodelno) then
        logDebug("===> Remotekey trigger to camera: " .. item.name)
        c_start_camera_record(item.did .. ";" .. item.password)
      end
    end
  end
end

-- trigger all siren on
function remotekeySirenOn(groupID)
  logDebug("remotekeySirenOn. groupID = " .. groupID)
  
   -- check group exist or not
  if nil == g_VirtualDevice[groupID] then 
    logWarn("group: " .. groupID .. " not exist!")
    return
  end
  
  local items = g_VirtualDevice[groupID]["items"]
  
  if nil ~= c_mqtt_publish_message then
    for k, v in pairs(items) do
      local item = getItemById(k)
      if (item ~= nil and item.busname == "ttyS0" and item.fullmodelno == "JSW-Siren-B001") then
          local msg = {}
          msg.busname = item.busname
          msg.bind = item.bind
          msg.action = "OnOff"
          msg.value = "On"
          msg.id = item.id
          msg.fullmodelno = item.fullmodelno
        
          -- checkItemBind(item.id, msg.bind)
          logDebug("===> Remotekey trigger to: " .. item.name)
          c_mqtt_publish_message(msg.busname, getjson(msg))
          -- msleep(20)
          
          -- 记录状态
          setItemOneStatus(item.id , "status", "On")
          
          -- chenjian 2014-10-28
          sendSensorStatus2App(item.id, "OnOff", "On");
      end
    end
  end
end

function remotekeySirenOnAll()
  logDebug("remotekeySirenOnAll")
  
  if nil ~= c_mqtt_publish_message then
    for k, v in pairs(g_item_pool) do
      local item = getItemById(k)
      if (item ~= nil and item.busname == "ttyS0" and item.fullmodelno == "JSW-Siren-B001") then
          local msg = {}
          msg.busname = item.busname
          msg.bind = item.bind
          msg.action = "OnOff"
          msg.value = "On"
          msg.id = item.id
          msg.fullmodelno = item.fullmodelno
        
          -- checkItemBind(item.id, msg.bind)
          logDebug("===> Remotekey trigger to: " .. item.name)
          c_mqtt_publish_message(msg.busname, getjson(msg))
          -- msleep(20)
          
          -- 记录状态
          setItemOneStatus(item.id , "status", "On")
          
          -- chenjian 2014-10-28
          sendSensorStatus2App(item.id, "OnOff", "On");
      end
    end
  end
end

function remotekeySirenOff()
  logDebug("remotekeySirenOff")
  
  if nil ~= c_mqtt_publish_message then
    for k, v in pairs(g_item_pool) do
      local item = getItemById(k)
      if (item ~= nil and item.busname == "ttyS0" and item.fullmodelno == "JSW-Siren-B001") then
        local item_status = g_item_repo
        if (item_status[item.id].status == "On") then --20150317_victorwu: fix disarm cannot turn off scenario siren
          local msg = {}
          msg.busname = item.busname
          msg.bind = item.bind
          msg.action = "OnOff"
          msg.value = "Off"
          msg.id = item.id
          msg.fullmodelno = item.fullmodelno
        
          -- checkItemBind(item.id, msg.bind)
          logDebug("===> Remotekey trigger to: " .. item.name)
          c_mqtt_publish_message(msg.busname, getjson(msg))
          -- msleep(20)
          -- 记录状态
          setItemOneStatus(item.id , "status", "Off")
          
          -- chenjian 2014-10-28
          sendSensorStatus2App(item.id, "OnOff", "Off");
        end
      end
    end
  end
end

--20150205_victorwu: add slot to arm and panic - begin
function remotekeySlotOn(groupID)
  logDebug("remotekeySlotOn. groupID = " .. groupID)
  
   -- check group exist or not
  if nil == g_VirtualDevice[groupID] then 
    logWarn("group: " .. groupID .. " not exist!")
    return
  end
  
  local items = g_VirtualDevice[groupID]["items"]
  
  if nil ~= c_mqtt_publish_message then
    for k, v in pairs(items) do
      local item = getItemById(k)
      if (item ~= nil and item.busname == "ttyS0" and item.fullmodelno == "JSW-Slot-B084") then
          local msg = {}
          msg.busname = item.busname
          msg.bind = item.bind
          msg.action = "OnOff"
          msg.value = "On"
          msg.id = item.id
          msg.fullmodelno = item.fullmodelno
        
          -- checkItemBind(item.id, msg.bind)
          logDebug("===> Remotekey trigger to: " .. item.name)
          c_mqtt_publish_message(msg.busname, getjson(msg))
          -- msleep(20)
          
          -- 记录状态
          setItemOneStatus(item.id , "status", "On")
          
          -- chenjian 2014-10-28
          sendSensorStatus2App(item.id, "OnOff", "On");
      end
    end
  end
end

function remotekeySlotOff()
  logDebug("remotekeySlotOff")
  
  if nil ~= c_mqtt_publish_message then
    for k, v in pairs(g_item_pool) do
      local item = getItemById(k)
      if (item ~= nil and item.busname == "ttyS0" and item.fullmodelno == "JSW-Slot-B084") then
        local item_status = g_item_repo
        if (item_status[item.id].status == "On") then --20150317_victorwu: fix disarm cannot turn off scenario siren
          local msg = {}
          msg.busname = item.busname
          msg.bind = item.bind
          msg.action = "OnOff"
          msg.value = "Off"
          msg.id = item.id
          msg.fullmodelno = item.fullmodelno
        
          -- checkItemBind(item.id, msg.bind)
          logDebug("===> Remotekey trigger to: " .. item.name)
          c_mqtt_publish_message(msg.busname, getjson(msg))
          -- msleep(20)
          -- 记录状态
          setItemOneStatus(item.id , "status", "Off")
          
          -- chenjian 2014-10-28
          sendSensorStatus2App(item.id, "OnOff", "Off");
        end
      end
    end
  end
end
--20150205_victorwu: add slot to arm and panic - begin

-- chenjian 2014-10-28 发送 siren 的状态到 APP
function sendSensorStatus2App(itemId, action, value)
 -- logDebug("sendSensorStatus2App ")
  if nil == itemId or nil == action or nil == value then
    logDebug("param error")
    return
  end
  
  if nil == c_p2p_publish_nextaction then
    logDebug("c function is nil");
    return
  end
  
  
  local item = getItemById(itemId)
  if nil == item then
    logDebug("item not exist");
    return
  end
  
  local src = {}
  src.id = itemId
  src.name = item.name
  src.fullmodelno = item.fullmodelno
  src.action = action
  src.value = value
  
  local rt = { ["result"] = nil, ["error"] = nil, ["callid"] = "123", ["source"] = src, ["from"] = "sensor"}
  local r = json_module.encode(rt)

  logDebug("send SensorStatus to App c_p2p_publish_nextaction r = " .. r)
  c_p2p_publish_nextaction(r)
end

-- from APP UI Remote key 4 buttons
function remotekeyControl(param, option)
  logDebug("remotekeyControl " .. getjson(param))
  if (param ~= nil and param.action ~= nil and param.value ~= nil) then
    local prepare_state = g_config.setting.prepareState
    local remote_key = g_config.setting.remotekey
    if "Lock" == param.value then
      if "PartialLock" == prepare_state or "PartialLock" == remote_key then
        DoARMUnlockOperations(false, false)
        os.execute("sleep 1")
        ARMLockDelayStart(param.value)
      else
        ARMLockDelayStart(param.value)
      end
    elseif "PartialLock" == param.value then
      if "Lock" == prepare_state or "Lock" == remote_key then
        DoARMUnlockOperations(false, false)
        os.execute("sleep 1")
        ARMLockDelayStart(param.value)
      else
        ARMLockDelayStart(param.value)
      end
    elseif "Unlock" == param.value then
      if "Unlock" == remote_key then
        DoARMUnlockOperations(true, false)
      else
        DoARMUnlockOperations(true, true)
      end
    elseif "Recording" == param.value then
      DoARMIPCamOperations()
    elseif "Siren" == param.value then
      DoARMPanicOperations()
    end
  else
    logDebug("param format error!!")
  end
end

function getItemStatus(param)
  local id = param.id;
  local statusname_array = param.statusname;
  local item_repo = g_item_repo[id];
  if (item_repo == nil) then
    return nil;
  end

  local rt_rpc = {}
  local i = 0
  for i = 1, #statusname_array do
    local statusname = statusname_array[i]
    if (g_item_repo[id].status[statusname] ~= nil) then
      rt_rpc[statusname] = g_item_repo[id].status[statusname];
    end
  end
  logDebug(json_module.encode(rt_rpc))

  return rt_rpc
end

function getItemMultiStatus(param)
  logDebug("getItemMultiStatus " .. getjson(param))
  local ret = {}
  
  local id = param.id
  local item_repo = g_item_repo[id]
  if item_repo ~= nil then
    ret["id"] = id

    for k, v in pairs(param) do
      if string.find(k, "statusName") ~= nil then
        ret[v] = item_repo[v]
      end
    end
  end

  return ret
end

-- 获取多个设备的某个状态
-- param = {"ids":["1", "2", "3"], "statusName":"status"}
function getMultiItemOneStatus(param)
  logDebug("getMultiItemOneStatus " .. getjson(param))
  local ret = {}
  
  local idsArray = param.ids
  local i = 0
  for i = 1, #idsArray do
    local item_repo = g_item_repo[idsArray[i]];
    if (item_repo ~= nil and item_repo[param.statusName] ~= nil) then
      ret[idsArray[i]] = item_repo[param.statusName]
    end
  end
  
--  logDebug("ret = " .. getjson(ret))  
  return ret
end

-- 获取一个设备的某个状态
function getItemOneStatus(itemId, statusName)
  local item_repo = g_item_repo[itemId]
  if nil == item_repo then
    return nil
  end
  
  return item_repo[statusName]
end

-- 设置一个设备的某个状态
function setItemOneStatus(itemId, statusName, value)
  local item_repo = g_item_repo[itemId];
  if nil ~= item_repo then
    -- update previous status
    g_item_repo_pre[itemId] = item_repo

    --item_repo = { [statusName] = value }
    item_repo[statusName] = value
    g_item_repo[itemId] = item_repo
    persistence.store(g_path_item_status_db, g_item_repo)
  else
    logDebug("Invalid item id : " .. itemId)
  end
end

-- 接收到 gateway_app 发来的设备报警信息（低电报警，防拆报警），设备状态信息（Slot状态）
-- sensor 防拆报警
-- "param":{"bind":"ttyS0=868/1109465933/0", “status”:"Tamper", “value”:“”}

-- sensor 低电报警-百分比
-- "param":{"bind":"ttyS0=868/1109465933/0", “status”:"Battery", “value”: 30} 

-- sensor 低电报警-low
-- "param":{"bind":"ttyS0=868/1109465933/0", “status”:"Battery", “value”: "low"}

-- gateway低电报警
-- "param":{"bind":"myself", “status”:"Battery", “value”: "low"}

-- gateway 防拆报警
-- "param":{"bind":"myself", “status”:"Tamper", “value”: “”}

function updateItemStatus(t)
  --logDebug("updateItemStatus " .. getjson(t))
  
  local ret
  if "myself" == t.bind or "ttyS0=868/123456789/0" == t.bind then
    ret = updateItemStatusGateway(t)
  else
    ret = updateItemStatusSensor(t)
  end
  
  return ret
end

function updateItemStatusGateway(t)
  --logDebug("updateItemStatusGateway ")
  
  -- step1: 发送信息通知 APP
  if c_p2p_publish_status ~= nil then
    local src = {}
    src.status = t.status
    src.value = t.value
    src.from = "gateway"
    
    local rt = { ["callid"] = t.callid, ["source"] = src}
    local r = json_module.encode(rt)
  
   -- logDebug("send gateway status to apk: " .. r)
    c_p2p_publish_status(r)
  end

  local gcmMsg = nil
  local title = nil
  local body = nil
  
  if "Tamper" == t.status then
    --20150319_victorwu: notify gateway siren if Tamper
    if g_config.setting.gatewaySiren == true then     
      notifyGatewayARMTrigger()
    end
    
    os.execute(". jsw_alarm.sh 180&")
    -- GCM
    gcmMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System has been tampered."
    sendGCMMessage(gcmMsg)

    -- SMS
    smsMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System has been tampered."
    sendSMSMessage(smsMsg)
    
    -- send email
    title = "Your System has been tampered."
    body = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System has been tampered."
    --sendEmailMessage(title, body)
    sendEmailNotification(title, body)

    --20150324_victorwu: add eventlist if Tamper - begin
    --logDebug("add eventlist")
  
    -- get standard timestamp
    local modSeconds = 0
    local tzStr = g_config.setting.timesetup.timezone
    if nil == tzStr then
        tzStr = "GMT_000"
    end
    
    local s, e = string.find(tzStr, "%a%a%a_.%d%d")
    if nil ~= s and nil ~= e then
       local dirc = string.sub(tzStr, 5, 5)
       local hourStr = string.sub(tzStr, 6)
       modSeconds = tonumber(hourStr) * 3600
       
       if "0" == dirc then
          modSeconds = 0 - modSeconds
       end
    end
    logDebug("modSeconds=" .. modSeconds)
    
    local what = {}
    what.t = os.time() + modSeconds;
    what.source = {}
    what.source.id = itemid
    what.source.name = item.name
    what.source.fullmodelno = item.fullmodelno
    what.source.seat = item.seat
    if item.fullmodelno == "JSW-PIR-B090" then
    what.source.action = "Trigger"
    what.source.value = "Open"
    else
    what.source.action = "OpenClose"
    what.source.value = "Open"
    end
    
    what.source.status = t.status
 --   what.source.humidity = g_config.information.humidity
 --   what.source.temperature = g_config.information.temperature
    addEvent(what)
    --20150324_victorwu: add eventlist if Tamper - end
  elseif "Battery" == t.status then
    local isLow = false
    if type(t.value) == "string" then
      if "low" == t.value then
        isLow = true
      end
    elseif type(t.value) == "number" then
      if t.value < 20 then
        isLow = true
      end
    end
    
    if true == isLow then
      -- GCM
      gcmMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is battery low."
      sendGCMMessage(gcmMsg)

      -- SMS
      smsMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is battery low."
      sendSMSMessage(smsMsg)
    
      -- send email
      title = "Your System has been battery low."
      body = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is battery low."
      --sendEmailMessage(title, body)
      sendEmailNotification(title, body)
    end
  elseif "humidity" == t.status then
    if g_config.information == nil then
      g_config.information = {}
    end

    g_config.information.humidity = tostring(t.value)
    persistence.store(g_path_config_db, g_config)

    if c_humd_value ~= nil then
        c_humd_value(t.value)
    end




  elseif "Temperature" == t.status then
    if g_config.information == nil then
      g_config.information = {}
    end

    g_config.information.temperature = tostring(t.value)
    persistence.store(g_path_config_db, g_config)

    if c_temp_value ~= nil then
        c_temp_value(t.value)
    end
  end
  
  -- 处理结束
  local rt = { ["result"] = "OK", ["error"] = nil, ["callid"] = -1 }
  return rt
end

function updateItemStatusSensor(t)
  logDebug("updateItemStatusSensor ")

  -- 检查该触发设备是否存在
  local bind = t.bind
  local itemid = g_bind_id_map["bind:" .. t.bind]
  if (itemid == nil) then
    logDebug("===>Exit: no itemid")
    local rt = { ["result"] = "error", ["error"] = "no itemid", ["callid"] = -1 }
    return rt
  end
  
  local item = getItemById(itemid)
  if nil == item then
    logDebug("===>Exit: item is nil")
    local rt = { ["result"] = "error", ["error"] = "item is nil", ["callid"] = -1 }
    return rt
  end

  -- step1: 发送信息通知 APP
  if c_p2p_publish_status ~= nil then
    local src = {}
    src.id = itemid
    src.name = item.name
    src.fullmodelno = item.fullmodelno
    src.status = t.status
    src.value = t.value
    src.from = "sensor"
    
    local rt = { ["result"] = nil, ["error"] = nil, ["callid"] = t.callid, ["source"] = src}
    local r = json_module.encode(rt)
  
  --  logDebug("send sensor status to apk: " .. r)
    c_p2p_publish_status(r)
  end
  
  -- step2: 低电报警，防拆报警, Handle RF return status
  local gcmMsg = nil
  local title = nil
  local body = nil
  local emailMsg = nil
  if "Tamper" == t.status then
    --20150319_victorwu: notify gateway siren if Tamper
    if g_config.setting.gatewaySiren == true then
      notifyGatewayARMTrigger()
    end
    os.execute(". jsw_alarm.sh 180&")
    -- Siron On
    remotekeySirenOnAll();
  
    -- GCM
    gcmMsg = item.name .. " at your " .. item.seat .. " has been tampered."
    sendGCMMessage(gcmMsg)

    -- SMS
    smsMsg = item.name .. " at your " .. item.seat .. " has been tampered."
    sendSMSMessage(smsMsg)
    
    -- send email
    title = "Your sensor has been tampered."
    body = item.name .. " at your " .. item.seat .. " has been tampered."
    --sendEmailMessage(title, body)
    sendEmailNotification(title, body)

    --20150324_victorwu: add eventlist if Tamper - begin
    logDebug("add eventlist")
  
    -- get standard timestamp
    local modSeconds = 0
    local tzStr = g_config.setting.timesetup.timezone
    if nil == tzStr then
        tzStr = "GMT_000"
    end
    
    local s, e = string.find(tzStr, "%a%a%a_.%d%d")
    if nil ~= s and nil ~= e then
       local dirc = string.sub(tzStr, 5, 5)
       local hourStr = string.sub(tzStr, 6)
       modSeconds = tonumber(hourStr) * 3600
       
       if "0" == dirc then
          modSeconds = 0 - modSeconds
       end
    end
    logDebug("modSeconds=" .. modSeconds)
    
    local what = {}
    what.t = os.time() + modSeconds;
    what.source = {}
    what.source.id = itemid
    what.source.name = item.name
    what.source.fullmodelno = item.fullmodelno
    what.source.seat = item.seat    
    if item.fullmodelno == "JSW-PIR-B090" then
    what.source.action = "Trigger"
    what.source.value = "Open"
    else
    what.source.action = "OpenClose"
    what.source.value = "Open"
    end
    what.source.status = t.status
    --what.source.humidity = g_config.information.humidity
    --what.source.temperature = g_config.information.temperature
    addEvent(what)
    --20150324_victorwu: add eventlist if Tamper - end
  elseif "Battery" == t.status then
    local isLow = false
    if type(t.value) == "string" then
      if "low" == t.value then
        isLow = true
      end
    elseif type(t.value) == "number" then
      if t.value < 20 then
        isLow = true
      end
    end
    
    if true == isLow then
      -- GCM
      gcmMsg = item.name .. " at your " .. item.seat .. " device battery low"
      sendGCMMessage(gcmMsg)

      -- SMS
      smsMsg = item.name .. " at your " .. item.seat .. " device battery low"
      sendSMSMessage(smsMsg)
    
      -- send email
      title = "Your sensor has been battery low."
      body = item.name .. " at your " .. item.seat .. " device battery low."
      --sendEmailMessage(title, body)
      sendEmailNotification(title, body)
    end
  elseif "RFLinkstaus" == t.status then
    -- Set status back if return NG status
    if t.value == "NG" then --20150317_victorwu: set status to off if RFLinkstaus result is NG
      logDebug("RFLinkstaus NG. Set default : Off")

      local item_repo = { ["status"] = "Off" }
      g_item_repo[itemid] = item_repo
      g_item_repo_pre[itemid] = item_repo

      -- Save to DB
      persistence.store(g_path_item_status_db, g_item_repo)
    elseif t.value == "yes" then
      logDebug("RFLinkstaus yes. save feedback time")
      g_item_repo[itemid].feedback_time = os.time()
      persistence.store(g_path_item_status_db, g_item_repo)
    end
  end

  -- 处理结束
  local rt = { ["result"] = "OK", ["error"] = nil, ["callid"] = -1 }
  return rt
end

-- 接收到 gateway_app 发来的 sender 触发消息
function updateItemTrigger(t)
  logDebug("updateItemTrigger " .. getjson(t))
  -- 检查该触发设备是否存在
  local bind = t.bind
  local itemid = g_bind_id_map["bind:" .. t.bind]
  if (itemid == nil) then
    logDebug("===>Exit: no itemid")
    return
  end
  
  -- 检查该触发设备是否为 RemoteKey(Lock, Unlock, ARM_Off, ARM_On)
  if t.action == "RemoteKey" then
    local prepare_state = g_config.setting.prepareState
    local remote_key = g_config.setting.remotekey
    if "Lock" == t.value then
      if "PartialLock" == prepare_state or "PartialLock" == remote_key then
        DoARMUnlockOperations(false, false)
        os.execute("sleep 1")
        ARMLockDelayStart(t.value)
      else
        ARMLockDelayStart(t.value)
      end
    elseif "PartialLock" == t.value then
      if "Lock" == prepare_state or "Lock" == remote_key then
        DoARMUnlockOperations(false, false)
        os.execute("sleep 1")
        ARMLockDelayStart(t.value)
      else
        ARMLockDelayStart(t.value)
      end
    elseif "Unlock" == t.value then
      if "Unlock" == remote_key then
        DoARMUnlockOperations(true, false)
      else
        DoARMUnlockOperations(true, true)
      end
    elseif "ARM_On" == t.value then
      DoARMIPCamOperations()
    elseif "ARM_Off" == t.value then
      DoARMPanicOperations()      
    end
    
    logDebug("===>Exit: save RemoteKey setting: " .. t.value)
    return
  end
  
  local item = getItemById(itemid)
  
  -- step1: check ARM or Scenario
  local armRet = false
  local partialArmRet = false
  local scenarioRet = false
  
--  logDebug("remotekey = " .. g_config.setting.remotekey)
  
  -- Both do Scenario and ARM action when sensor trigger
  armRet = updateItemTrigger4ARMControl(t)
  partialArmRet = updateItemTrigger4PartialARMControl(t)
  scenarioRet = updateItemTrigger4Scenrio(t)
  
  -- step2: 检查是否需要发送信息通知 APP
  -- 在 step1 中，可能已经把联动规则发送给APP，否则这里需要发送一次只包含该 tirgger 的消息给APP
  if c_p2p_publish_nextaction ~= nil and item ~= nil then
    if false == armRet and false == partialArmRet and false == scenarioRet then
      local src = {}
      src.id = itemid
      src.name = item.name
      src.fullmodelno = item.fullmodelno
      src.action = t.action
      src.value = t.value
      
      local rt = { ["result"] = nil, ["error"] = nil, ["callid"] = t.callid, ["source"] = src, ["from"] = "sensor"}
      local r = json_module.encode(rt)
    
      logDebug("send sender_trigger or receiver_status message to apk: " .. r)
      c_p2p_publish_nextaction(r)
    end
  end
  
  -- step5: 如果是设备状态信息需要保存
  if "OnOff" == t.action or "OpenClose" == t.action then
    local item_repo = g_item_repo[itemid]; --对应id该有item的repo
    if item_repo == nil then
      item_repo = { ["status"] = t.value }
      g_item_repo_pre[itemid] = g_item_repo[itemid]
      g_item_repo[itemid] = item_repo
    else
      item_repo["status"] = t.value
    end
    persistence.store(g_path_item_status_db, g_item_repo)
    
    logDebug("g_item_repo = " .. json_module.encode(g_item_repo[itemid]))
  end
  
  -- step6: save to eventlist db if item is not IPCam
  if "OpenCamera" ~= t.action then
    logDebug("add eventlist")
  
    -- get standard timestamp
    local modSeconds = 0
    local tzStr = g_config.setting.timesetup.timezone
    if nil == tzStr then
        tzStr = "GMT_000"
    end
    
    local s, e = string.find(tzStr, "%a%a%a_.%d%d")
    if nil ~= s and nil ~= e then
       local dirc = string.sub(tzStr, 5, 5)
       local hourStr = string.sub(tzStr, 6)
       modSeconds = tonumber(hourStr) * 3600
       
       if "0" == dirc then
          modSeconds = 0 - modSeconds
       end
    end
    logDebug("modSeconds=" .. modSeconds)
    
    local what = {}
    what.t = os.time() + modSeconds;
    what.source = {}
    what.source.id = itemid
    what.source.name = item.name
    what.source.fullmodelno = item.fullmodelno
    what.source.seat = item.seat  --20150311_victorwu: add eventlist triggering item seat
    
    if item.fullmodelno == "JSW-PIR-B090" then
		what.source.action = "Trigger"
	 what.source.value = "Open"
    else
    what.source.action = t.action
    what.source.value = t.value
    end
    

    if (item.fullmodelno=="JSW-GateTemp-0001") then
		what.source.humidity = g_config.information.humidity
		what.source.temperature = g_config.information.temperature
    end 

    local trigger_lock_status = g_config.setting.remotekey --20150306_victorwu: add eventlist triggering arm status

    if trigger_lock_status ~= nil then
      if trigger_lock_status == "Lock" then
        what.source.status = "Arm"
      elseif trigger_lock_status == "PartialLock" then
        what.source.status = "PartialArm"
      elseif trigger_lock_status == "Unlock" then
        what.source.status = "Disarm"
      end
    end
 
    addEvent(what)
  end
  
  -- 处理结束
  local rt = { ["result"] = "OK", ["error"] = nil, ["callid"] = -1 }
  return rt
end

function temperatureTrigger(t)
  logDebug("temperatureTrigger " .. getjson(t))
  -- 检查该触发设备是否存在
  local bind = t.bind
  local temperature_id = g_bind_id_map["bind:" .. t.bind]
  local temperature_item = getItemById(temperature_id)

  if (temperature_id == nil) then
    logDebug("===>Exit: no temperature_id")
    return
  end
  
  if g_config.setting.gatewaySiren == true then
    os.execute(". jsw_alarm.sh 180&")
    notifyGatewayARMTrigger()
  end
    
  for tk, tv in pairs(g_item_pool) do
    local item = getItemById(tk)
    
    -- 发送控制指令
    if (item ~= nil and item.fullmodelno == "JSW-Siren-B001") then
      local msg = {}
      msg.busname = item.busname
      msg.action = "OnOff"
      msg.value = "On"
      msg.bind = item.bind
      msg.fullmodelno = item.fullmodelno
        
      -- checkItemBind(item.id, msg.bind)
      logDebug("===>Temperature trigger device: " .. msg.bind)
      c_mqtt_publish_message(msg.busname, getjson(msg))
      
      -- 记录状态
      setItemOneStatus(item.id , "status", tv.value)
    end
  end
  
  -- gcm
  local gcmMsg
  if true == sirenExit then
    gcmMsg = t.action .. " at your " .. temperature_item.seat .. " exceeds bound and your siren is going off"
  else
    gcmMsg = t.action .. " at your " .. temperature_item.seat .. " exceeds bound"
  end
  sendGCMMessage(gcmMsg)

  -- SMS
  local smsMsg 
  if true == sirenExit then
    smsMsg = t.action .. " at your " .. temperature_item.seat .. " exceeds bound and your siren is going off"
  else
    smsMsg = t.action .. " at your " .. temperature_item.seat .. " exceeds bound"
  end
  sendSMSMessage(smsMsg)
  
  -- step4: handle email
  local title = "Your system has been triggered!"
  local body
  if true == sirenExit then
    body = t.action .. " at your " .. temperature_item.seat .. " exceeds bound and your siren is going off!"
  else
    body = t.action .. " at your " .. temperature_item.seat .. " exceeds bound!"
  end
  --sendEmailMessage(title, body)
  sendEmailNotification(title, body)
  
  -- send alarm message to apk
  if c_p2p_publish_nextaction ~= nil then
    local src = {}
    src.id = temperature_id
    src.name = temperature_item.name
    src.fullmodelno = temperature_item.fullmodelno
    src.action = t.action
    src.value = t.value
    
    local rt = { ["result"] = arm_link, ["error"] = nil, ["callid"] = t.callid, ["source"] = src, ["from"] = "arm"}
    local r = json_module.encode(rt)
  
    logDebug("send sensor trigger to apk: " .. r)
    c_p2p_publish_nextaction(r)

    -- get standard timestamp
    local modSeconds = 0
    local tzStr = g_config.setting.timesetup.timezone
    if nil == tzStr then
        tzStr = "GMT_000"
    end
    
    local s, e = string.find(tzStr, "%a%a%a_.%d%d")
    if nil ~= s and nil ~= e then
       local dirc = string.sub(tzStr, 5, 5)
       local hourStr = string.sub(tzStr, 6)
       modSeconds = tonumber(hourStr) * 3600
       
       if "0" == dirc then
          modSeconds = 0 - modSeconds
       end
    end
    logDebug("modSeconds=" .. modSeconds)
    
    local what = {}
    what.t = os.time() + modSeconds;
    what.source = {}
    what.source.id = temperature_id
    what.source.name = temperature_item.name
    what.source.fullmodelno = temperature_item.fullmodelno
    what.source.seat = temperature_item.seat
    
    what.source.action = t.action
    what.source.value = t.value
    what.source.humidity = g_config.information.humidity
    what.source.temperature = g_config.information.temperature

    local trigger_lock_status = g_config.setting.remotekey

    if trigger_lock_status ~= nil then
      if trigger_lock_status == "Lock" then
        what.source.status = "Arm"
      elseif trigger_lock_status == "PartialLock" then
        what.source.status = "PartialArm"
      elseif trigger_lock_status == "Unlock" then
        what.source.status = "Disarm"
      end
    end
 
    addEvent(what)
  end
end

function sendGCMMessage(message)
  logDebug("sendGCMMessage " .. message)
  
  if (g_config.setting.message ~= nil) then
    if (g_config.setting.message.notification_enable == true) then
      if (c_gcm_send_event_list ~= nil) then
        c_gcm_send_event_list(message)
      end
    end
  end
end

--20150324_victorwu: send sms message - begin
function sendSMSMessage(message)
  logDebug("sendSMSMessage " .. message)
  
  if (g_config.setting.message ~= nil) then
    local sms_setting = g_config.setting.message.sms
    if (sms_setting ~= nil) then
      if (sms_setting.sms_enable == true and sms_setting.sms_key ~= nil and sms_setting.sms_from ~= nil and sms_setting.sms_to ~= nil) then
        if (c_sms_send_event_list ~= nil) then
          local sms_data = sms_setting.sms_key .. " " .. sms_setting.sms_from .. " " .. sms_setting.sms_to .. " " .. message
          c_sms_send_event_list(sms_data)
        end
      end
    end
  end
end
--20150324_victorwu: send sms message - end

function sendEmailNotification(title, message)
  logDebug("sendEmailNotification " .. message)
  
  if g_config.setting.message ~= nil then
    if g_config.setting.message.email_enable == true then
      local mail_group = nil

      for mail_receiver, mail_addr in pairs(g_config.setting.message) do
        if string.sub(mail_receiver, 1, 8) == "receiver" then
          if (mail_group == nil) then
            mail_group = mail_addr
          else
            mail_group = mail_group .. "," .. mail_addr
          end
        end
      end

      if c_send_email_notification ~= nil and mail_group ~= nil then
        local to_address = mail_group
        local from_address = "donot-reply@omguard.com"
        local subject = title
        local content = message
        local email_notification_command = to_address .. ";" .. from_address .. ";" .. subject .. ";" .. content .. ";"
        logDebug("email_notification_command: " .. email_notification_command)
        c_send_email_notification(email_notification_command)
      end
    end
  end
end

-- new mailSend 
function sendEmailMessage(title, message)
  logDebug("sendEmailMessage " .. message)
  
  if (g_config.setting.message ~= nil) then
      if g_config.setting.message.email_enable == true then --20150305_victorwu: multi-receiver
        local mail_group = nil

        for mail_receiver, mail_addr in pairs(g_config.setting.message) do
          if string.sub(mail_receiver, 1, 8) == "receiver" then
            if (mail_group == nil) then
              mail_group = mail_addr
            else
              mail_group = mail_group .. "," .. mail_addr
            end
          end
        end

        if (c_email_send_alarm ~= nil and mail_group ~= nil) then --20150317_victorwu: fix email cannot delete if email_enable is true
          os.execute("date   +\"%Y-%m-%d %H:%M:%S\" > /tmp/email.msg")
          os.execute("echo '" .. message .. "' >> /tmp/email.msg");
          logDebug("receiver = " .. g_config.setting.message.receiver)
          local email_command = "./mailsend -smtp email-smtp.us-east-1.amazonaws.com -port 587 -auth -starttls -user AKIAJQKEJBLIVQHNNLNQ -pass AhIFYFhUmvcmTjpyjSFEfNprmaP2EV8Dq0oQhptyZEas -f donot-reply@omguard.com -t " .. mail_group .. " -sub " .. "'" .. title .. "'" .. " -v -M " .. "'" .. message .. "'"
          logDebug("email_command = " .. email_command)
          c_email_send_alarm(email_command)
        end
      end
    end
end

function notifyGatewayARMTrigger()
  logDebug("notifyGatewayARMTrigger")
  
  local msg = {}
  msg.busname = "ttyS0"
  msg.action = "ARMTrigger"
  
  -- below NO use, but fan need
  msg.fullmodelno = "JSW-Slot-B084"
  msg.value = "On"
  msg.id = "0"
  msg.bind = "ttyS0=868/857323752/0"
  c_mqtt_publish_message("ttyS0", getjson(msg))
end

-- notify gateway_app: ARM Lock or UnLock
function lua_notifyGatewayArmState()
  logDebug("lua_notifyGatewayArmState")
  notifyGatewayArmState()
end

-- notify gateway_app: ARM Lock or UnLock
function notifyGatewayArmState()
--  logDebug("notifyGatewayArmState")
  
  local remote_key = g_config.setting.remotekey
  local msg = {}
  msg.busname = "ttyS0"
  
  if "Lock" == remote_key then
    msg.action = "SetARMLockOk"
  elseif "Unlock" == remote_key then
    msg.action = "SetARMUnlock"
  else
    return
  end
  
  -- below NO use, but fan need
  msg.fullmodelno = "JSW-Slot-B084"
  msg.value = "On"
  msg.id = "0"
  msg.bind = "ttyS0=868/857323752/0"
  c_mqtt_publish_message("ttyS0", getjson(msg))
end

-- 当ARM规则中被触发时,通知gateway
function notifyGatewayARMTrigger()
  logDebug("notifyGatewayARMTrigger")
  
  local msg = {}
  msg.busname = "ttyS0"
  msg.action = "ARMTrigger"
  
  -- below NO use, but fan need
  msg.fullmodelno = "JSW-Slot-B084"
  msg.value = "On"
  msg.id = "0"
  msg.bind = "ttyS0=868/857323752/0"
  c_mqtt_publish_message("ttyS0", getjson(msg))
end

-- notify gateway_app: ARM Lock or UnLock
function notifyGatewayArmStatePre(state)
  logDebug("notifyGatewayArmStatePre state = " .. state)
  
  local msg = {}
  msg.busname = "ttyS0"
  
  if "Lock" == state then
    msg.action = "SetARMLock"
  elseif "Unlock" == state then
    msg.action = "SetARMUnlock"
  else
    return
  end
  
  -- below NO use, but fan need
  msg.fullmodelno = "JSW-Slot-B084"
  msg.value = "On"
  msg.id = "0"
  msg.bind = "ttyS0=868/857323752/0"
  c_mqtt_publish_message("ttyS0", getjson(msg))
end

-- 通知 gateway(本应该新建一个指令,但是范工坚持使用P2PConectOK时发送的指令,也是嘀嘀2声)
function notifyGatewayArmStateOk()
  if nil ~= c_mqtt_publish_message then
    local msg = {}
    msg.busname = "ttyS0"
    msg.action = "SetARMLockOk"
    
    -- below NO use, but fan need
    msg.fullmodelno = "JSW-Slot-B084"
    msg.value = "On"
    msg.id = "0"
    msg.bind = "ttyS0=868/857323752/0"
    c_mqtt_publish_message("ttyS0", getjson(msg))
  end
end

-- notify App : ARM Lock or Unlock
function notifyAppArmState(state)
  logDebug("notifyAppArmState")
  if c_p2p_publish_status ~= nil then
    local src = {}
    src.status = "ARMState"
    src.value = state
    src.from = "gateway"
    
    local rt = { ["callid"] = "1", ["source"] = src}
    local r = json_module.encode(rt)
  
    logDebug("send gateway status to app: " .. r)
    c_p2p_publish_status(r)
  end
end

--20150325_victorwu: for led lightness - begin
function setGatewayLedLightness(t)
  logDebug("setGatewayLedLightness")
  g_config.setting.gatewayLedLight = t.gatewayLedLight

  if t.gatewayLedLight == "Low" then
    os.execute(". jsw_control_led.sh 6 -v 1")
    os.execute(". jsw_control_led.sh 7 -v 0")
  elseif t.gatewayLedLight == "High" then
    os.execute(". jsw_control_led.sh 6 -v 1")
    os.execute(". jsw_control_led.sh 7 -v 1")
  else
    os.execute(". jsw_control_led.sh 6 -v 0")
  end

  persistence.store(g_path_config_db, g_config)
end

function getGatewayLedLightness()
  logDebug("getGatewayLedLightness")
  local ret = {}
  ret["gatewayLedLight"] = g_config.setting.gatewayLedLight
  return ret
end
--20150325_victorwu: for led lightness - end

--20150324_victorwu: for arm delay control - begin
function setArmDelayTime(t)
  logDebug("setArmDelayTime")
  g_config.setting.armDelayTime = t.armDelayTime
  persistence.store(g_path_config_db, g_config)
end

function getArmDelayTime()
  logDebug("getArmDelayTime")
  local ret = {}
  ret["armDelayTime"] = g_config.setting.armDelayTime
  return ret
end
--20150324_victorwu: for arm delay control - end

-- ARM Lock 需要延时 30 秒
function ARMLockDelayStart(state)
  logDebug("ARMLockDelayStart")
  local remote_key = g_config.setting.remotekey
  local prepare_state = g_config.setting.prepareState

  if (state == remote_key and g_arm_delay_status == "off") or (state == prepare_state and g_arm_delay_status == "on") then
    logDebug("Already in lock process, skip lock delay")
  else
    g_arm_delay_status = "on"
    notifyGatewayArmStatePre("Lock")
    g_config.setting.prepareState = state
    persistence.store(g_path_config_db, g_config)
    
    -- 开启线程,延时30秒钟,延时结束后,调用 ARMLockDelayEnd()
    if nil ~= c_delay_operate then
      local armDelayTime = g_config.setting.armDelayTime
      if tonumber(armDelayTime) ~= 0 then
        os.execute(". jsw_arm.sh&")
      end

      if armDelayTime == nil then
        c_delay_operate(30)
      else
        c_delay_operate(tonumber(armDelayTime))
      end
    end
  end
end

function ARMLockDelayEnd()
  logDebug("ARMLockDelayEnd")
  DoARMLockOperations()
  g_arm_delay_status = "off"
end

-- ARM Lock operation
function DoARMLockOperations()
  logDebug("DoARMLockOperations")
  
  -- 保存到配置文件
  g_config.setting.remotekey = g_config.setting.prepareState
  persistence.store(g_path_config_db, g_config)
  -- notify gateway
  os.execute(". jsw_arm_ok.sh&")
  notifyGatewayArmStateOk()
  
  if g_config.setting.remotekey == "Lock" then
    -- GCM
    local gcmMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now armed."
    sendGCMMessage(gcmMsg)

    -- SMS
    local smsMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now armed."
    sendSMSMessage(smsMsg)
    
    -- send email
    local title = "System is now armed."
    local body = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now armed."
    --sendEmailMessage(title, body)
    sendEmailNotification(title, body)
  elseif g_config.setting.remotekey == "PartialLock" then
    -- GCM
    local gcmMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now partial-armed."
    sendGCMMessage(gcmMsg)

    -- SMS
    local smsMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now partial-armed."
    sendSMSMessage(smsMsg)
    
    -- send email
    local title = "System is now partial-armed."
    local body = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now partial-armed."
    --sendEmailMessage(title, body)
    sendEmailNotification(title, body)
  else
    print("Unknown state: " .. g_config.setting.remotekey)
  end
  
  -- send message to App
  notifyAppArmState(g_config.setting.remotekey)
--  os.execute(g_free_cache_cmd)
end

function DoARMUnlockOperations(sendToApp, turnOffSlot)
  logDebug("DoARMUnlockOperations")

  -- 保存到配置文件
  g_config.setting.prepareState = "Unlock"
  g_config.setting.remotekey = "Unlock"
  persistence.store(g_path_config_db, g_config)

  -- notify gateway
  notifyGatewayArmStatePre("Unlock")

  -- 如果之前设置过Lock操作,需要取消倒计时定时器
  if nil ~= c_delay_operate_cancel then
    c_delay_operate_cancel()
  end
  remotekeySirenOff()

  if turnOffSlot == true then
    remotekeySlotOff()
  end
  
  if sendToApp == true then
    -- GCM
    local gcmMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now disarmed."
    sendGCMMessage(gcmMsg)

    -- SMS
    local smsMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now disarmed."
    sendSMSMessage(smsMsg)
    
    -- send email
    local title = "System is now disarmed."
    local body = "Your SHC_pro System [" .. g_config.gateway_did .. "] : System is now disarmed."
    --sendEmailMessage(title, body)
    sendEmailNotification(title, body)
    
    -- send message to App
    notifyAppArmState("Unlock")
    g_arm_delay_status = "off"
    os.execute(". jsw_disarm.sh&")
  end

  --os.execute(g_free_cache_cmd)
end

function DoARMIPCamOperations()
  logDebug("DoARMIPCamOperations")
  
  -- 触发所有 IPCamera 录像
  remotekeyCameraRecord("98")
  
  -- GCM
  local gcmMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : Camera is now recording."
  sendGCMMessage(gcmMsg)

  -- SMS
  local smsMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : Camera is now recording."
  sendSMSMessage(smsMsg)
  
  -- send email
  local title = "Your camera is now recording."
  local body = "Your camera is now recording."
  --sendEmailMessage(title, body)
  sendEmailNotification(title, body)
end

function DoARMPanicOperations()
  logDebug("DoARMPanicOperations")
  
  -- 触发所有 Siren 动作
  remotekeyCameraRecord("99")
  remotekeySirenOn("99")
  remotekeySlotOn("99") --20150205_victorwu: add slot to arm and panic
  
  -- GCM
  local gcmMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : Siren has been manually activated."
  sendGCMMessage(gcmMsg)

  -- SMS
  local smsMsg = "Your SHC_pro System [" .. g_config.gateway_did .. "] : Siren has been manually activated."
  sendSMSMessage(smsMsg)
  
  -- send email
  local title = "Your siren has been manually activated."
  local body = "Your siren has been manually activated."
  --sendEmailMessage(title, body)
  sendEmailNotification(title, body)
end

-- old email
function sendEmailMessage2(message)
  logDebug("sendEmailMessage2 " .. message)
  if (g_config.setting.message ~= nil) then
      if g_config.setting.message.email_enable == true then
        if (c_email_send_alarm ~= nil) then
          os.execute("date   +\"%Y-%m-%d %H:%M:%S\" > /tmp/email.msg")
          os.execute("echo '" .. message .. "' >> /tmp/email.msg");
          local email_command = ""
          if g_config.setting.message.ssl_enable ~= true then
            email_command = "./email -f'" .. g_config.setting.message.smtpusername .. "' -n'" .. g_config.setting.message.smtpusername .. "' -s'" .. "Device " .. " Alarm! " ..  "' -r'" .. g_config.setting.message.smtpserver .. "' -p " .. g_config.setting.message.smtpport .. " -mlogin -u'" .. g_config.setting.message.smtpusername .. "' -i'" .. g_config.setting.message.smtppassword .. "' -V '" .. g_config.setting.message.receiver ..  "' < /tmp/email.msg"
          else
            email_command = "./email -f'" .. g_config.setting.message.smtpusername .. "' -n'" .. g_config.setting.message.smtpusername .. "' -s'" .. "Device " .. " Alarm! " ..  "' -r'" .. g_config.setting.message.smtpserver .. "' -p " .. g_config.setting.message.smtpport .. " -mlogin -u'" .. g_config.setting.message.smtpusername .. "' -i'" .. g_config.setting.message.smtppassword .. "' -tls -V '" .. g_config.setting.message.receiver ..  "' < /tmp/email.msg"
          end

          c_email_send_alarm(email_command)
        end
      end
    end
end

function updateItemTrigger4ARMControl(t)
  logDebug("updateItemTrigger4ARMControl")
  local ret = false

  if g_config.setting.remotekey ~= "Lock" then
    logDebug("Not in Arm state!, exit")
    return ret
  end

  if g_arm_linktable["source"] == nil then
    logDebug("g_arm_linktable.source is nil")
    return ret
  end
  
  local itemid = g_bind_id_map["bind:" .. t.bind]
  local item = getItemById(itemid)
  
  -- 检查该 trigger 是否存在于 g_arm_linktable 的 source 中
  local sourceHit = false
  for sk, sv in pairs(g_arm_linktable["source"]) do
    if (sk == itemid and sv.value == t.value) then
      sourceHit = true
    end
  end
  -- Then, check ipcam in target of g_arm_linktable
  for sk, sv in pairs(g_arm_linktable["target"]) do
    if (sk == itemid) then
      sourceHit = true
    end
  end
  
  if false == sourceHit then
    logDebug("===>Exit: sourceHit is false")
    return ret
  end
  
  if "Off" ~= t.value then -- GW-181_20150108_victorwu: fix re-alarm issue
    -- ARM 规则中如果有设备被触发,通知 gateway
  --  if triggerSensor then
  --    notifyGatewayARMTrigger()
  --  end
    -- chenjian changed 2014-10-14
    -- Consider gateway siren setting to trigger siren on or off 2014-12-02
    if g_config.setting.gatewaySiren == true then
      os.execute(". jsw_alarm.sh 180&")
      notifyGatewayARMTrigger()
    end
    local triggerSensor = false   -- 记录 ARM 规则中是否有设备被触发
    local sirenExit = false       -- 记录是否有Siren
    local arm_link = {}
    
    for tk, tv in pairs(g_arm_linktable["target"]) do
      local item = getItemById(tv.id)
      
      -- 组装发送到APK的信息
      arm_link[tk] = tv
      
      -- 记录是否有 Siren
      if "JSW-Siren-B001" == item.fullmodelno then
        sirenExit = true
      end
      
      -- 发送控制指令
      if (item ~= nil and item.busname == "ttyS0") then
        local msg = {}
        msg.busname = item.busname
        msg.action = tv.action
        msg.value = tv.value
        msg.bind = item.bind
        msg.fullmodelno = item.fullmodelno
          
        -- checkItemBind(item.id, msg.bind)
        logDebug("===>ARM Conrol trigger device: " .. msg.bind)
        c_mqtt_publish_message(msg.busname, getjson(msg))
        
        -- 记录状态
        setItemOneStatus(item.id , "status", tv.value)
        
        -- msleep(20)
        triggerSensor = true
      elseif (item ~= nil) then
         if(tv.record == true and c_start_camera_record ~= nil) then 
            logDebug("===>ARM Conrol trigger camera: " .. item.did)
            c_start_camera_record(item.did .. ";" .. item.password)
            triggerSensor = true
         end
      end
    end
    
    -- gcm
    local gcmMsg
    if true == sirenExit then
      gcmMsg = item.name .. " at your " .. item.seat .. " detects motion and your siren is going off"
    else
      gcmMsg = item.name .. " at your " .. item.seat .. " detects motion"
    end
    sendGCMMessage(gcmMsg)

    -- SMS
    local smsMsg 
    if true == sirenExit then
      smsMsg = item.name .. " at your " .. item.seat .. " detects motion and your siren is going off"
    else
      smsMsg = item.name .. " at your " .. item.seat .. " detects motion"
    end
    sendSMSMessage(smsMsg)
    
    -- step4: handle email
    local title = "Your system has been triggered!"
    local body
    if true == sirenExit then
      body = item.name .. " at your " .. item.seat .. " detects motion and your siren is going off!"
    else
    body = item.name .. " at your " .. item.seat .. " detects motion!"
    end
    --sendEmailMessage(title, body)
    sendEmailNotification(title, body)
    
    -- send alrm message to apk
    if c_p2p_publish_nextaction ~= nil then
      local src = {}
      src.id = itemid
      src.name = item.name
      src.fullmodelno = item.fullmodelno
      src.action = t.action
      src.value = t.value
      
      local rt = { ["result"] = arm_link, ["error"] = nil, ["callid"] = t.callid, ["source"] = src, ["from"] = "arm"}
      local r = json_module.encode(rt)
    
      logDebug("send sensor trigger to apk: " .. r)
      c_p2p_publish_nextaction(r)
      
      ret = true
    end
  end
  
  return ret
end

function updateItemTrigger4PartialARMControl(t)
  logDebug("updateItemTrigger4PartialARMControl")
  local ret = false

  if g_config.setting.remotekey ~= "PartialLock" then
    logDebug("Not in Partial Arm state!, exit")
    return ret
  end

  if g_partialarm_linktable["source"] == nil then
    logDebug("g_partialarm_linktable.source is nil")
    return ret
  end
  
  local itemid = g_bind_id_map["bind:" .. t.bind]
  local item = getItemById(itemid)
  
  -- 检查该 trigger 是否存在于 g_partialarm_linktable 的 source 中
  local sourceHit = false
  for sk, sv in pairs(g_partialarm_linktable["source"]) do
    if (sk == itemid and sv.value == t.value) then
      sourceHit = true
    end
  end
  -- Then, check ipcam in target of g_partialarm_linktable
  for sk, sv in pairs(g_partialarm_linktable["target"]) do
    if (sk == itemid) then
      sourceHit = true
    end
  end
  
  if false == sourceHit then
    logDebug("===>Exit: sourceHit is false")
    return ret
  end
  
  if "Off" ~= t.value then -- GW-181_20150108_victorwu: fix re-alarm issue
    -- ARM 规则中如果有设备被触发,通知 gateway
  --  if triggerSensor then
  --    notifyGatewayARMTrigger()
  --  end
    -- chenjian changed 2014-10-14
    -- Consider gateway siren setting to trigger siren on or off 2014-12-02
    if g_config.setting.gatewaySiren == true then
      os.execute(". jsw_alarm.sh&")
      notifyGatewayARMTrigger()
    end
    local triggerSensor = false   -- 记录 ARM 规则中是否有设备被触发
    local sirenExit = false       -- 记录是否有Siren
    local arm_link = {}
    
    for tk, tv in pairs(g_partialarm_linktable["target"]) do
      local item = getItemById(tv.id)
      
      -- 组装发送到APK的信息
      arm_link[tk] = tv
      
      -- 记录是否有 Siren
      if "JSW-Siren-B001" == item.fullmodelno then
        sirenExit = true
      end
      
      -- 发送控制指令
      if (item ~= nil and item.busname == "ttyS0") then
        local msg = {}
        msg.busname = item.busname
        msg.action = tv.action
        msg.value = tv.value
        msg.bind = item.bind
        msg.fullmodelno = item.fullmodelno
          
        -- checkItemBind(item.id, msg.bind)
        logDebug("===>Partial ARM Conrol trigger device: " .. msg.bind)
        c_mqtt_publish_message(msg.busname, getjson(msg))
        
        -- 记录状态
        setItemOneStatus(item.id , "status", tv.value)
        
        -- msleep(20)
        triggerSensor = true
      elseif (item ~= nil) then
         if(tv.record == true and c_start_camera_record ~= nil) then 
            logDebug("===>Partial ARM Conrol trigger camera: " .. item.did)
            c_start_camera_record(item.did .. ";" .. item.password)
            triggerSensor = true
         end
      end
    end
    
    -- gcm
    local gcmMsg
    if true == sirenExit then
      gcmMsg = item.name .. " at your " .. item.seat .. " detects motion and your siren is going off"
    else
      gcmMsg = item.name .. " at your " .. item.seat .. " detects motion"
    end
    sendGCMMessage(gcmMsg)

    -- SMS
    local smsMsg 
    if true == sirenExit then
      smsMsg = item.name .. " at your " .. item.seat .. " detects motion and your siren is going off"
    else
      smsMsg = item.name .. " at your " .. item.seat .. " detects motion"
    end
    sendSMSMessage(smsMsg)
    
    -- step4: handle email
    local title = "Your system has been triggered!"
    local body
    if true == sirenExit then
      body = item.name .. " at your " .. item.seat .. " detects motion and your siren is going off!"
    else
    body = item.name .. " at your " .. item.seat .. " detects motion!"
    end
    --sendEmailMessage(title, body)
    sendEmailNotification(title, body)
    
    -- send alrm message to apk
    if c_p2p_publish_nextaction ~= nil then
      local src = {}
      src.id = itemid
      src.name = item.name
      src.fullmodelno = item.fullmodelno
      src.action = t.action
      src.value = t.value
      
      local rt = { ["result"] = partial_arm_link, ["error"] = nil, ["callid"] = t.callid, ["source"] = src, ["from"] = "arm"}
      local r = json_module.encode(rt)
    
      logDebug("send sensor trigger to apk: " .. r)
      c_p2p_publish_nextaction(r)
      
      ret = true
    end
  end
  
  return ret
end

function updateItemTrigger4Scenrio(t)
  logDebug("updateItemTrigger4Scenrio " .. getjson(t))
  local ret = false
  
  -- 查看该sensor是否在 linktable 中设置了联动规则
  local itemid = g_bind_id_map["bind:" .. t.bind]
  local item = getItemById(itemid)
  
  local k = "id_" .. itemid .. "_action_" .. t.action .. "_value_" .. t.value
  local nextaction = g_linktable[k];
  --logDebug("NextAction ".. getjson(nextaction))
  if (nextaction == nil) then
    -- 没有找到该 sensor 的联动规则
    logDebug("===>Exit: nextaction is nil")
    return ret
  end

  -- 判断该 sender 的 enable 状态
  if "yes" ~= nextaction["enable"] then
    logDebug("===>Exit: This sender enable is not yes")
    return ret
  end
  
  local effective_link = {}
  for k1, v in pairs(g_linktable[k]) do
    if (k1 ~= "enable") then
      effective_link[k1] = v
    end
  end
  g_nextaction_pool = effective_link
  if (g_nextaction_pool == nil) then
    logDebug("===>Exit: g_nextaction_pool is null.")
    return ret
  end

  logDebug("trigger receiver device action")
  if (c_p2p_publish_nextaction ~= nil) then
    local src = {}
    src.id = itemid
    src.name = item.name
    src.fullmodelno = item.fullmodelno
    src.action = t.action
    src.value = t.value
    
    local rt = { ["result"] = g_nextaction_pool, ["error"] = nil, ["callid"] = t.callid, ["source"] = src, ["from"] = "scenario" }
    local r = json_module.encode(rt)

    -- call send message to gateway_app
    --{"id_11_7":{"target":{"name":"Camera1","id":"7","value":"OpenCamera","action":"OpenCamera","bind":"ttyS0=433/0/1"},"source":{"name":"fake pir","id":"11","value":"Open","action":"OpenClose","bind":"ttyS0=433/1523359744/0"},"key":"id_11_action_OpenClose_value_Open"}}

    for k, v in pairs(g_nextaction_pool) do
      local target = v.target
      local item = getItemById(target.id)
      local preset_point = "-1"

      if item.fullmodelno == "JSW-Camera-0001" and target.preset_point ~= nil then
          preset_point = target.preset_point
      end

      if (item ~= nil and item.busname == "ttyS0") then
        local msg = {}
        msg.busname = item.busname
        msg.action = target.action
        msg.value = target.value
        msg.bind = item.bind
        msg.fullmodelno = item.fullmodelno
        
        -- checkItemBind(item.id, msg.bind)
        logDebug("===>Scenario trigger device: " .. msg.bind)
        c_mqtt_publish_message(msg.busname, getjson(msg))
        -- msleep(20)
        -- 记录状态
        setItemOneStatus(target.id , "status", target.value)

        if (item.fullmodelno == "JSW-Slot-B084" and item.autoofftime ~= "0" and g_item_repo[target.id].status == "On") then --20150326_victorwu: for scenario autooff
          g_item_repo[target.id].scenario_trigger_time = os.time()
          persistence.store(g_path_item_status_db, g_item_repo)
        end
      elseif (item ~= nil) then
         if(target.record == true and c_start_camera_record ~= nil) then 
            logDebug("===>Scenario trigger camera: " .. item.did)
            c_start_camera_record(item.did .. ";" .. item.password .. ";" .. preset_point)
         end
      end
    end
    
    logDebug("===>send nextaction to Apk by P2P")
    c_p2p_publish_nextaction(r)
    ret = true
  end
  
  return ret
end

function getNextActionList()
  local r = g_nextaction_pool
  g_nextaction_pool = nil
  --    logDebug("getNextActionList", r)
  return r
end


function addSchedule(t)
  logDebug("addSchedule" .. getjson(t))

  local rt = {}

  if (t.key ~= nil) then
    table.foreachi(t.schedule, function(i, v)
      t.schedule[i]["$$hashKey"] = nil
    end)
    
    g_schedule[t.key] = t.schedule
    persistence.store(g_path_schedule_db, g_schedule)
    logDebug("schedule: " .. getjson(g_schedule))

    rt.result = "success"
  else
    rt.result = "fail, nil params"
  end
  return rt
end

function removeSchedule(t)
  logDebug("removeSchedule" .. getjson(t))
  
  local rt = {}

  if (t.key ~= nil) then
    table.remove(g_schedule[t.key], t.index + 1)

    persistence.store(g_path_schedule_db, g_schedule)
    logDebug("schedule: " .. getjson(g_schedule))

    rt.result = "success"
  else
    rt.result = "fail, nil params"
  end
  return rt
end

function filterScheduleItem(itemId)
  logDebug("filterScheduleItem itemId = " .. itemId)
  
  local idKey = "id_" .. itemId
  for k, v in pairs(g_schedule) do
    if k == idKey then
      g_schedule[k] = nil
      persistence.store(g_path_schedule_db, g_schedule)
      logDebug("==delete sensor in schedule")
      break
    end
  end
end

function getLinktable()
  --logDebug("getLinktable: " .. getjson(g_linktable))
  return g_linktable
end

function getSchedule(t)
  logDebug("getSchedule: " .. getjson(g_schedule))
  return g_schedule
end

-- TODO
function getVirtualDevice()
  -- logDebug("g_VirtualDevice: " .. getjson(g_VirtualDevice))
  return g_VirtualDevice
end

function addVirtualDevice(param, option)
  --logDebug("addVirtualDevice")
  local res = ""
  
  if (param.id~=nil and param.name~=nil and param.bind~=nil and param.fullmodelno~=nil and param.value~=nil and param.items~=nil) then
     local device = param
     g_VirtualDevice[param.id] = device
     res = {["result"] = "success"}
  else
    res = {["result"] = "error", ["reason"] = "format error"}
  end
  
  persistence.store(g_path_virtualdevice_db, g_VirtualDevice)
  --logDebug("VirtualDevice: " .. getjson(g_VirtualDevice))
    
  return res
end

function removeVirtualDevice(param, option)
  --logDebug("removeVirtualDevice")
  local res = ""
  
  if (param.id~=nil and param.name~=nil and param.bind~=nil and param.fullmodelno~=nil and param.value~=nil and param.items~=nil) then
     g_VirtualDevice[param.id] = nil
     res = {["result"] = "success"}
  else
    res = {["result"] = "error", ["reason"] = "format error"}
  end
  
  persistence.store(g_path_virtualdevice_db, g_VirtualDevice)
  --logDebug("VirtualDevice: " .. getjson(g_VirtualDevice))
  
  return res
end

function updateVirtualDevice(param, option)
  --logDebug("updateVirtualDevice")
  local res = ""
  
  if (param.id~=nil and param.name~=nil and param.bind~=nil and param.fullmodelno~=nil and param.value~=nil and param.items~=nil) then
     local device = param
     g_VirtualDevice[param.id] = device
     res = {["result"] = "success"}
  else
    res = {["result"] = "error", ["reason"] = "format error"}
  end
  
  persistence.store(g_path_virtualdevice_db, g_VirtualDevice)
  --logDebug("VirtualDevice: " .. getjson(g_VirtualDevice))
  
  return res
end

function virtualDeviceTrigger(param, option)
  --logDebug("virtualDeviceTrigger. param = " .. getjson(param))
  
  local res = ""
  
  if (param.id~=nil and param.name~=nil and param.value~=nil) then
     local items = g_VirtualDevice[param.id]["items"]
     
     g_VirtualDevice[param.id].value = param.value
     --logDebug("VirtualDevice: " .. getjson(g_VirtualDevice))
     
     for k, v in pairs(items) do
        local item = getItemById(k)
        if (item ~= nil and item.busname == "ttyS0") then
          local msg = {}
          msg.busname = item.busname
          msg.bind = item.bind
          msg.fullmodelno = item.fullmodelno
          
          msg.action = v[3]
            
          if ("On" == param.value) then
            msg.value = v[1]
          else
            msg.value = v[2]
          end
          
          -- checkItemBind(item.id, msg.bind)
          logDebug("===> ActiveArm trigger to: " .. msg.bind)
          c_mqtt_publish_message(msg.busname, getjson(msg))
          -- msleep(20)
          -- 记录状态
          setItemOneStatus(item.id , "status", msg.value)
        elseif (item ~= nil and "JSW-Camera-0001" == item.fullmodelno) then
          logDebug("===> ActiveArm trigger to camera: " .. item.did)
          c_start_camera_record(item.did .. ";" .. item.password)
        end
     end
     
     res = {["result"] = "success"}
  else
    res = {["result"] = "error", ["reason"] = "format error"}
  end
  
  return res
end

function modifyPassword(t)
  local rt = {}

  if (t.admin_password ~= nil) then
    g_device_tree["gateway"].admin_password = t.admin_password
    g_config.admin_password = t.admin_password

    rt.modify_addmin_password = "success"
  end
  if (t.gateway_password ~= nil) then
    g_device_tree["gateway"].gateway_password = t.gateway_password
    g_config.gateway_password = t.gateway_password

    -- modify password in ceres.c
    if nil ~= c_modify_gateway_password then
      c_modify_gateway_password(g_config.gateway_password)
    end

    rt.modify_gateway_password = "success"
  end
  persistence.store(g_path_config_db, g_config)

  return rt
end

function getIpValue()

  local wan_info = {}
  if g_config.setting.ipsetup == nil then
    wan_info = {}
  else
    wan_info = g_config.setting.ipsetup

    wan_info["dhcp_enable"] = g_config.setting.ipsetup.dhcp_enable
    wan_info["wan_ip"] = g_config.setting.ipsetup.wan_ip
    wan_info["wan_netmask"] = g_config.setting.ipsetup.wan_netmask
    wan_info["wan_gateway"] = g_config.setting.ipsetup.wan_gateway
    wan_info["wan_dns"] = g_config.setting.ipsetup.wan_dns

  end
 

  return wan_info
end

function setIpValue(t)
  g_config.setting.ipsetup = t
  
  g_config.setting.ipsetup.dhcp_enable  = t.dhcp_enable
  g_config.setting.ipsetup.wan_ip		= t.wan_ip
  g_config.setting.ipsetup.wan_netmask	= t.wan_netmask
  g_config.setting.ipsetup.wan_gateway	= t.wan_gateway
  g_config.setting.ipsetup.wan_dns		= t.wan_dns

  persistence.store(g_path_config_db, g_config)
  
  os.execute("./switch_ip_config.sh")

end

function getWifiValue()
  local wifi_info = {}
  if g_config.setting.wifiapsetup == nil then
    g_config.setting.wifiapsetup = {}
  end

  if g_config.setting.wifiapsetup.ssid == nil then
    g_config.setting.wifiapsetup.ssid = excut_cmd("get_wlan_ssid")
  end
  wifi_info = g_config.setting.wifiapsetup

  return wifi_info
end

function setWifiValue(t)
  excut_cmd("enable_wifi", t.wifi_enable)

  if t.ssid and t.wifi_enable then
    excut_cmd("set_wlan_ssid", t.ssid)
  end
  g_config.setting.wifiapsetup = t
  persistence.store(g_path_config_db, g_config)

  os.execute("reboot")
end

function getTime()
  local time_info = {}
  time_info = g_config.setting.timesetup
  if (time_info ~= nil) then
    time_info["date"] = excut_cmd("get_date")
  end

  return time_info
end

function setTime(t)
  logDebug("setTime" .. getjson(t))
  if nil ~= t.machineformat then
    excut_cmd("set_date", t.machineformat)
  end
  
  if nil ~= t.timezone then
    excut_cmd("set_timezone", t.timezone)
  end
  
  g_config.setting.timesetup = t
  -- g_config.setting.ntpserver = t.ntpserver
  -- g_config.setting.daylight = t.daylight

  if t.ntpserver == true then
    excut_cmd("set_ntp_serverip", "time-a.nist.gov")
    excut_cmd("set_ntp_time", "0")
  else
    excut_cmd("set_ntp_time", "")
  end

  persistence.store(g_path_config_db, g_config)
end

function getSensorType()
  --    {"callid":2, "method":"updateitemtrigger", "param":{"bind":"ttyS0=433/2318049024/0", "action":"OpenClose", "value":"Open"} }
  return getjson(g_bind_fullmodelno)
end

function saveAESKey(param, option)
  if param.aeskey ~= nil then
    --  参数OK
    -- TOTO: save to local db
    find = false
    for _, v in pairs(g_aeskey_pool) do
      if (v == param.aeskey) then
        find = true
      end
    end

    if find == false then
      table.insert(g_aeskey_pool, param.aeskey)
      persistence.store(g_path_aeskey_db, g_aeskey_pool)
      --logDebug("save new aes key")

      updateCAesKey()
    end

    local item = {["saved"] = "yes", ["aeskey"] = param.aeskey}
    return item
  else
  --参数检查没有通过
  end
end

function updateCAesKey()
  --logDebug("updateCAesKey")
  if 0 == #g_aeskey_pool then
    --logDebug("g_aeskey_pool length is 0")
    return
  end

  if (c_update_aes_key == nil) then
    --logDebug("c_update_aes_key is null!")
    return
  end

  --    logDebug("call C code: c_update_aes_key()")
  c_update_aes_key(table.concat(g_aeskey_pool, ";"))
end

function getMessageValue()
  local message_info = g_config.setting.message

  return message_info
end

function setMessageValue(t)
  logDebug("setMessageValue: " .. getjson(t))
  local ret = {}

  g_config.setting.message = t
  persistence.store(g_path_config_db, g_config)
  ret.result = "success"

  return ret
end

function getGatewaySiren()
  logDebug("getGatewaySiren: " .. getjson(g_config.setting.gatewaySiren))
  local message_info = {}

  message_info.gatewaySiren = g_config.setting.gatewaySiren

  return message_info
end

function setGatewaySiren(t)
  logDebug("setGatewaySiren: " .. getjson(t))
  local ret = {}

  if t.gatewaySiren == nil then
    ret.result = "fail, nil parameter"
  else
    g_config.setting.gatewaySiren = t.gatewaySiren
    persistence.store(g_path_config_db, g_config)
    ret.result = "success"
  end

  return ret
end

function sendScheduleCommand(itemid, action, value)
  logDebug("sendScheduleCommand()")
  logDebug("action = " .. action .. ", itemid = " .. itemid .. ", value = " .. value)
  
  if nil == action or nil == value then
    logDebug("params error")
    return
  end
  
  if (nil == c_mqtt_publish_message) then
    return
  end
  
  local item = getItemById(itemid)
  if (item ~= nil and item.busname == "ttyS0") then
    -- schedule 中不可以添加siren， 因此不需要判断是否禁止siren动作
    local msg = {}
    msg.busname = item.busname
    msg.action = action
    msg.value = value
    msg.bind = item.bind
    msg.fullmodelno = item.fullmodelno
    
    -- checkItemBind(item.id, msg.bind)
    c_mqtt_publish_message(msg.busname, getjson(msg))
    -- msleep(20)

    -- Send status change, support device : slot
    if item.fullmodelno == "JSW-Slot-B084" or item.fullmodelno == "JSW-Siren-B001" then
      local src = {}

      src.id = itemid
      src.name = item.name
      src.fullmodelno = item.fullmodelno
      src.action = action
      src.value = value

      local rt = { ["result"] = nil, ["error"] = nil, ["callid"] = 2, ["source"] = src, ["from"] = "schedule"}
      local r = json_module.encode(rt)

      logDebug("send " .. item.fullmodelno .. " status message to apk.")
      logDebug("Message " .. getjson(rt))
      c_p2p_publish_nextaction(r)
    end
  elseif (item ~= nil) then
     if(c_start_camera_record ~= nil) then 
        c_start_camera_record(item.did .. ";" .. item.password)
     end
  end
end

function lua_schedule()
  -- logDebug("lua_schedule " .. os.date("%c", os.time()))
  
  if (g_schedule == nil) then
    logWarn("lua_schedule is nil")
    return
  end
  
  local curWeek = string.lower(os.date("%a"))
  local onTimeHit = false
  local offTimeHit = false
  local action = nil
  local value0 = nil
  local value1 = nil
  
  for k, v in pairs(g_schedule) do
    -- logDebug("lua_schedule check: " .. k)
    
    -- 获取 sensor id
    local s, e, itemid = string.find(k, "%a+_(%d+)")
    if nil ~= itemid then   
      -- 获取 sensor item
      local item = getItemById(itemid)
      if nil ~= item then     
        -- 恢复初始值
        onTimeHit = false
        action = nil
        value0 = nil
        --value1 = nil
        for k1, v1 in pairs(v) do
          if false == onTimeHit and "yes" == v1["enable"] and true == v1["week"][curWeek] then
            if nil == action then
              action = v1["action"]
              value0 = v1["value0"]   -- On
              value1 = v1["value1"]   -- Off
            end
            
            -- 当前时间是否处于该 sensor 的任一 shedule 时间范围内
            local duration = v1["duration"]
            local on = Split(duration.On, ":")
            local start_time = on[1] * 60 + on[2]
            local off = Split(duration.Off, ":")
            local end_time = off[1] * 60 + off[2]
            
            local now = os.date("%H") * 60 + os.date("%M")
            -- 加上时区偏移量
            now = now - v1["offset"]
            
            if (now >= start_time and now < end_time) then
              onTimeHit = true
            elseif (now == end_time) then
              offTimeHit = true
            end
          end
        end -- for
        
        local curValue = getItemOneStatus(itemid, "status")
        if true == onTimeHit and nil ~= value0 then
          if curValue == nil or curValue ~= value0 then
            sendScheduleCommand(itemid, action, value0)
            setItemOneStatus(itemid , "status", value0)
          end
        elseif true == offTimeHit and nil ~= value1 then
          if curValue == nil or curValue ~= value1 then
            sendScheduleCommand(itemid, action, value1)
            setItemOneStatus(itemid , "status", value1)
          end
        end
      end -- if nil ~= item
    end -- if nil ~= itemid
  end -- for
end

--20150326_victorwu: for scenario autooff - begin
function lua_autooff_schedule()
  --logDebug("lua_autooff_schedule " .. os.time())

  for k, v in pairs(g_item_pool) do
    if g_item_pool[k].fullmodelno == "JSW-Slot-B084" then
      if g_item_repo[k].scenario_trigger_time ~= 0 then
        for k1, v1 in pairs(v) do
          if tostring(k1) == "autoofftime" then
            if tonumber(v1) == 0 or g_item_repo[k].status == "Off" then
              g_item_repo[k].scenario_trigger_time = 0
              persistence.store(g_path_item_status_db, g_item_repo)
            else
              local diff = os.difftime(os.time(), tonumber(g_item_repo[k].scenario_trigger_time))
              if diff >= tonumber(v1) or diff < 0 then
                local item = getItemById(k)
                if (item ~= nil and item.busname == "ttyS0") then
                  local msg = {}
                  msg.busname = item.busname
                  msg.action = "OnOff"
                  msg.value = "Off"
                  msg.bind = item.bind
                  msg.fullmodelno = item.fullmodelno

                  logDebug("===>Turn off scenario trigger device: " .. msg.bind)
                  c_mqtt_publish_message(msg.busname, getjson(msg))
                end
                setItemOneStatus(k, "status", "Off")
                g_item_repo[k].scenario_trigger_time = 0
                persistence.store(g_path_item_status_db, g_item_repo)
              end
            end
          end
        end
      end
    end
  end
end

function getAutoOffTimeValue(t)
  local autoofftime_table = {}
  local item = getItemById(t.id)
  autoofftime_table.id = t.id
  autoofftime_table.autoofftime = item.autoofftime
  return autoofftime_table
end
--20150326_victorwu: for scenario autooff - end

function lua_sensor_alive_schedule()
  --logDebug("lua_sensor_alive_schedule " .. os.time())

  for k, v in pairs(g_item_pool) do
    if g_item_pool[k].fullmodelno == "JSW-GateMaglock-0001" or g_item_pool[k].fullmodelno == "JSW-PIR-B090" or g_item_pool[k].fullmodelno == "JSW-Siren-B001" then
      if g_item_repo[k].previous_feedback_time == nil then
        g_item_repo[k].previous_feedback_time = 0
      end

      if g_item_repo[k].feedback_time == nil then
        g_item_repo[k].feedback_time = 0
      end

      if g_item_repo[k].no_alive_count == nil then
        g_item_repo[k].no_alive_count = 0
      end
      
      if g_item_repo[k].previous_feedback_time == g_item_repo[k].feedback_time then
        g_item_repo[k].no_alive_count = tonumber(g_item_repo[k].no_alive_count)+1
      else
        g_item_repo[k].previous_feedback_time = g_item_repo[k].feedback_time
        g_item_repo[k].no_alive_count = 0
      end
      
      persistence.store(g_path_item_status_db, g_item_repo)

      if g_item_repo[k].no_alive_count >= 2 then
        if c_p2p_publish_status ~= nil then
          local src = {}
          src.id = tostring(k)
          src.name = g_item_pool[k].name
          src.fullmodelno = g_item_pool[k].fullmodelno
          src.status = "RFLinkstaus"
          src.value = "Unknown"
          src.from = "sensor"
          
          local rt = {["result"] = nil, ["error"] = nil, ["source"] = src}
          local r = json_module.encode(rt)
        
          logDebug("send sensor alive status to apk: " .. r)
          c_p2p_publish_status(r)
        end
      end
    end
  end
end

function getGatewayTemperature()
  --logDebug("getGatewayTemperature")
  lua_gateway_temperature_schedule()
 
   --if c_p2p_publish_status ~= nil then
    --local rt = {}
   --local rt =  {"callid":2, "method":"getGatewayTemperature", "param":{"Temparature":tostring(g_config.information.temperature), "Humidity":tostring(g_config.information.humidity)} }
	--local rt =  {"callid":2, "method":"getGatewayTemperature", "param":{"Temparature":"55", "Humidity":"99" } }   
   --local r = json_module.encode(rt)  
  -- c_p2p_publish_status(r)
--  end 

	--if c_p2p_publish_status ~= nil then
		--local src = {}
	--	src.status = "Temparature"
	--	src.value = 55
	--	src.from = "sensor"

	--	local rt = { ["result"] = nil, ["error"] = nil, ["callid"] = -1, ["source"] = src}
	--	local r = json_module.encode(rt)
	--	c_p2p_publish_status(rt)
  -- end
    
end

function lua_gateway_temperature_schedule()
  --logDebug("lua_gateway_temperature_schedule " .. os.time())
  -- send fake data to gateway_app
  local msg = {}
  msg.busname = "ttyS0"
  msg.action = "Temperature"
  
  -- below NO use, but fan need
  msg.fullmodelno = "JSW-GateTemp-0001"
  msg.value = "On"
  msg.id = "0"
  msg.bind = "ttyS0=868/123456789/0"
  c_mqtt_publish_message("ttyS0", getjson(msg))

  local temperature_id = g_bind_id_map["bind:" .. "ttyS0=868/123456789/0"]
  local check_temperature = g_item_repo[temperature_id].check_temperature

  if check_temperature == "On" then
    local temperature = tonumber(g_config.information.temperature)
    local temperature_bound = tonumber(g_item_repo[temperature_id].temperature)

    if temperature_bound == nil then
      temperature_bound = 100
    end

    if temperature >= temperature_bound then
      local temperature_trigger = {}

      temperature_trigger.bind = "ttyS0=868/123456789/0"
      temperature_trigger.action = "Temperature"
      temperature_trigger.value = tostring(temperature_bound)
      temperatureTrigger(temperature_trigger)
    end
  end

  local check_humidity = g_item_repo[temperature_id].check_humidity

  if check_humidity == "On" then
    local humidity = tonumber(g_config.information.humidity)
    local humidity_bound = tonumber(g_item_repo[temperature_id].humidity)

    if humidity_bound == nil then
      humidity_bound = 100
    end

    if humidity >= humidity_bound then
      local humidity_trigger = {}

      humidity_trigger = {}
      humidity_trigger.bind = "ttyS0=868/123456789/0"
      humidity_trigger.action = "Humidity"
      humidity_trigger.value = tostring(humidity_bound)
      temperatureTrigger(humidity_trigger)
    end
  end
end

function setTemperatureAlarm(param, option)
  logDebug("setTemperatureAlarm")

  local temperature_id = g_bind_id_map["bind:" .. "ttyS0=868/123456789/0"]

  if param.action == "Temperature" then
    setItemOneStatus(temperature_id, "check_temperature", param.check_temperature)
    setItemOneStatus(temperature_id, "temperature", param.temperature)
  elseif param.action == "Humidity" then
    setItemOneStatus(temperature_id, "check_humidity", param.check_humidity)
    setItemOneStatus(temperature_id, "humidity", param.humidity)
  end
  
 -- return temperature_id;
end

function getTemperatureAlarm(param, option)
  logDebug("getTemperatureAlarm")

  local ret = {}

  if param.action == "Temperature" then
    ret.id = g_bind_id_map["bind:" .. "ttyS0=868/123456789/0"]
    ret.bind = "ttyS0=868/123456789/0"
    ret.check_temperature = g_item_repo[ret.id].check_temperature
    ret.temperature = g_item_repo[ret.id].temperature
  elseif param.action == "Humidity" then
    ret.id = g_bind_id_map["bind:" .. "ttyS0=868/123456789/0"]
    ret.bind = "ttyS0=868/123456789/0"
    ret.check_humidity = g_item_repo[ret.id].check_humidity
    ret.humidity = g_item_repo[ret.id].humidity
  end

  return ret
end

function setTemperatureFrequency(param, option)
  logDebug("setTemperatureFrequency")

  if c_set_temperature_frequency ~= nil then
    g_config.information.frequency = param.period
    persistence.store(g_path_config_db, g_config)
    c_set_temperature_frequency(tonumber(param.period))
  end
end

function getTemperatureFrequency()
  logDebug("getTemperatureFrequency")

  local ret = {}

  ret.id = g_bind_id_map["bind:" .. "ttyS0=868/123456789/0"]
  ret.bind = "ttyS0=868/123456789/0"
  ret.period = g_config.information.frequency

  return ret
end

function lua_getTemperatureFrequency()
  logDebug("lua_getTemperatureFrequency")

  local period = 180
  
  if g_config.information.frequency ~= nil then
    period = tonumber(g_config.information.frequency)
  end

  return period
end

function getFile(file_name)
  local f = assert(io.open(file_name, 'r'))
  local string = f:read("*all")
  f:close()
  return string
end

-- 厂测代码中, gateway_app 返回的数据在这里
-- "param":{"bind":"ttyS0=868/857323752/0", "action":"GetMCUVersion", "value":"V1.2.3"}}
-- "param":{"bind":"ttyS0=868/857323752/0", "action":"Test868RF", "value":"Pass"}}
-- "param":{"bind":"ttyS0=868/857323752/0", "action":"TestBatteryLow", "value":"Pass"}}

-- 获取 OS 版本命令:user_cmd v
-- 得到结果:
-- Linux kernel version: 2.6.21
-- SDK version: 4.1.2.0
function updateFactoryTest(t)
  logDebug("updateFactoryTest " .. getjson(t))
  
  if c_p2p_publish_status ~= nil then
    local reply = {}
  
    reply.method = t.action
    reply.callid = "123123"
    reply.result = {}
    
    if "GetMCUVersion" == reply.method then
      reply.result.MCU_version = t.value
      reply.result.FW_version = g_config.gateway_app_version
      
      local os_ver = nil
      
      if nil ~= SD_path then
        local fPath = SD_path .. "os.ver"
        os_ver = getFile(fPath)
      end
      
      if nil == os_ver then
        reply.result.OS_version = "unknown"
      else
        reply.result.OS_version = os_ver
      end
    else
      reply.result.value = t.value
    end
    
    local r = json_module.encode(reply)
  
    logDebug("send factory test reply to PC exe: " .. r)
    c_p2p_publish_status(r)
  end
end

function GetMCUVersion()
  logDebug("GetMCUVersion")
  
  if nil ~= SD_path then
    local cmd = "cat /etc_ro/VERSION > " .. SD_path .. "os.ver"
    os.execute(cmd)
    os.execute("sync")
  end
  
  -- send command to gateway get mcu version
  local msg = {}
  msg.busname = "ttyS0"
  msg.action = "GetMCUVersion"
  msg.fullmodelno = "JSW-Slot-B084"
  msg.value = "On"
  msg.id = "0"
  msg.bind = "ttyS0=868/857323752/0"
  c_mqtt_publish_message("ttyS0", getjson(msg))
  
  local ret = {}
  ret.value = "get version please waiting"
  return ret
end

function Test868RF()
  logDebug("Test868RF")
  
  -- send command to gateway get mcu version
  local msg = {}
  msg.busname = "ttyS0"
  msg.action = "Test868RF"
  msg.fullmodelno = "JSW-Slot-B084"
  msg.value = "On"
  msg.id = "0"
  msg.bind = "ttyS0=868/857323752/0"
  c_mqtt_publish_message("ttyS0", getjson(msg))
  
  local ret = {}
  ret.value = "test 868 RF please waiting"
  return ret
end

function TestBatteryLow()
  logDebug("TestBatteryLow")
  
  -- send command to gateway get mcu version
  local msg = {}
  msg.busname = "ttyS0"
  msg.action = "TestBatteryLow"
  msg.fullmodelno = "JSW-Slot-B084"
  msg.value = "On"
  msg.id = "0"
  msg.bind = "ttyS0=868/857323752/0"
  c_mqtt_publish_message("ttyS0", getjson(msg))
  
  local ret = {}
  ret.value = "test battery low please waiting"
  return ret
end

function TestLedMode(param, option)
  if param.led_mode == "arm" then
    notifyGatewayArmStatePre("Lock")
  elseif param.led_mode == "arm_ok" then
    DoARMLockOperations()
  elseif param.led_mode == "disarm" then
    DoARMUnlockOperations(true, false)
  elseif param.led_mode == "p2p_ok" then
    if c_mqtt_publish_message ~= nil then
      local msg = {}
      msg.busname = "ttyS0"
      msg.bind = "ttyS0=868/857323752/0"
      msg.action = "P2PConnectOk"
      msg.value = "On"
      msg.id = "0"
      msg.fullmodelno = "JSW-Slot-B084"
      c_mqtt_publish_message(msg.busname, getjson(msg))
    end

    os.execute(". jsw_control_led.sh 12 -b 0; . jsw_control_led.sh 14 -b 0; . jsw_control_led.sh 14 -v 0")
  elseif param.led_mode == "p2p_ng" then
    if c_mqtt_publish_message ~= nil then
      local msg = {}
      msg.busname = "ttyS0"
      msg.bind = "ttyS0=868/857323752/0"
      msg.action = "P2PConnectFail"
      msg.value = "On"
      msg.id = "0"
      msg.fullmodelno = "JSW-Slot-B084"
      c_mqtt_publish_message(msg.busname, getjson(msg))
    end
    
    os.execute(". jsw_control_led.sh 12 -b 0; . jsw_control_led.sh 14 -b 1&")
  elseif param.led_mode == "upgrade" then
    os.execute(". jsw_control_led.sh 14 -b 0; . jsw_control_led.sh 12 -b 1&")
  end

  local ret = {}
  ret.value = "test led please waiting"
  return ret
end

function TestGatewayTemperature()
  --getGatewayTemperature()

  local ret = {}
  ret.value = "test temperature and humidity please waiting"
  return ret
end

function writeRandomString2SDCard()
  logDebug("writeRandomString2SDCard")
  
  -- write
  if nil == SD_path then
    sendReply2PCExe("Fail")
    return
  end
  
  local fPath = SD_path .. "sdtest"
  os.execute("echo \"hello,JSW\" > " .. fPath)
  os.execute("sync;sync")
  
  -- read
  local read = getFile(fPath)
  if nil == read then
    sendReply2PCExe("Fail")
  else
    logDebug("read = " .. read)
    local pre = string.sub(read,1,9)
    if nil == pre then
      sendReply2PCExe("Fail")
    elseif "hello,JSW" == pre then
      logDebug("pre = " .. pre)
      sendReply2PCExe("Pass")
    else
      logDebug("pre = " .. pre)
      sendReply2PCExe("Fail")
    end
  end
end

function sendReply2PCExe(value)
  logDebug("sendReply2PCExe  value = " .. value)
  if c_p2p_publish_status ~= nil then
    local reply = {}
  
    reply.method = "TestSDCard"
    reply.callid = "123123"
    reply.result = {}
    reply.result.value = value
    
    local r = json_module.encode(reply)
    logDebug("send factory test reply to PC exe: " .. r)
    c_p2p_publish_status(r)
  end
end

function TestSDCard()
  logDebug("TestSDCard")
  writeRandomString2SDCard()
  
  local ret = {}
  ret.value = "test SD card please waiting";
  return ret
end

function runResetScript(type)
  logDebug("runResetScript type = " .. type)
  local cmd = nil
  if (1 == type) then
    cmd = "./resetfactory.sh"
  else
    cmd = "./resetuser.sh"
  end
  
  if nil ~= cmd then
    os.execute(cmd)
  end
end

function resetfactory()
  logDebug("resetfactory")
  
  -- 延时 5 秒钟
  if nil ~= c_delay_reset then
    c_delay_reset(5, 1)
  end
  
  
  local ret = {["result"] = "success"}
  return ret
end

function resetuser()
  logDebug("resetuser")
  
  local ret = {["result"] = "success"}
  return ret
end

function getAllIPCams()
  local did
  local pw
  local num = 0
  local IPCams = ""

  for k, v in pairs(g_item_pool) do
    if v["fullmodelno"] == "JSW-Camera-0001" then
      if num ~= 0 then
        IPCams = IPCams .. ","
      end
      
      IPCams = IPCams .. v["did"] .. ";" .. v["password"]
      num = num + 1
    end
  end
  
  logDebug("getAllIPCams. num = " .. num)
  return IPCams
end

-- 工厂测试添加
g_export["GetMCUVersion"] = GetMCUVersion
g_export["Test868RF"] = Test868RF
g_export["TestBatteryLow"] = TestBatteryLow
g_export["TestSDCard"] = TestSDCard
g_export["TestLedMode"] = TestLedMode
g_export["TestGatewayTemperature"] = TestGatewayTemperature
g_export["updateFactoryTest"] = updateFactoryTest

g_export["addItem"] = addItem
g_export["removeItem"] = removeItem
g_export["updateItem"] = updateItem
g_export["controlMultiItem"] = controlMultiItem
g_export["controlItem"] = controlItem
g_export["devicejsontree"] = ex_devicejsontree
g_export["updateKeyValue"] = updateKeyValue
g_export["getValueByKey"] = getValueByKey

g_export["getItemStatus"] = getItemStatus
g_export["updateItemStatus"] = updateItemStatus

g_export["updateitemtrigger"] = updateItemTrigger
g_export["temperatureTrigger"] = temperatureTrigger

g_export["removeEvent"] = removeEvent
g_export["getEventList"] = getEventList

g_export["removeEventMix"] = removeEventMix
g_export["getEventListMix"] = getEventListMix
g_export["getEventListSeparate"] = getEventListSeparate

g_export["getNextActionList"] = getNextActionList

g_export["setLinkToggle"] = setLinkToggle
g_export["addLink"] = addLink
g_export["removeLink"] = removeLink

g_export["getRemotekeyValue"] = getRemotekeyValue
g_export["setRemotekeyValue"] = setRemotekeyValue

g_export["setArmLinktable"] = setArmLinktable
g_export["getArmLinktable"] = getArmLinktable

g_export["addSchedule"] = addSchedule
g_export["removeSchedule"] = removeSchedule
g_export["getSchedule"] = getSchedule

g_export["addVirtualDevice"] = addVirtualDevice
g_export["removeVirtualDevice"] = removeVirtualDevice
g_export["updateVirtualDevice"] = updateVirtualDevice
g_export["virtualDeviceTrigger"] = virtualDeviceTrigger
g_export["getVirtualDevice"] = getVirtualDevice

g_export["modifyPassword"] = modifyPassword
g_export["getIpValue"] = getIpValue
g_export["setIpValue"] = setIpValue
g_export["getWifiValue"] = getWifiValue
g_export["setWifiValue"] = setWifiValue
g_export["getTime"] = getTime
g_export["setTime"] = setTime
g_export["getMessageValue"] = getMessageValue
g_export["setMessageValue"] = setMessageValue
g_export["getGatewaySiren"] = getGatewaySiren
g_export["setGatewaySiren"] = setGatewaySiren

g_export["getSensorType"] = getSensorType
g_export["saveAESKey"] = saveAESKey

g_export["remotekeyControl"] = remotekeyControl
g_export["getLinktable"] = getLinktable

g_export["getItemMultiStatus"] = getItemMultiStatus
g_export["getMultiItemOneStatus"] = getMultiItemOneStatus

g_export["resetfactory"] = resetfactory
g_export["resetuser"] = resetuser

g_export["setArmDelayTime"] = setArmDelayTime
g_export["getArmDelayTime"] = getArmDelayTime

g_export["setGatewayLedLightness"] = setGatewayLedLightness
g_export["getGatewayLedLightness"] = getGatewayLedLightness

g_export["setPartialArmLinktable"] = setPartialArmLinktable
g_export["getPartialArmLinktable"] = getPartialArmLinktable

g_export["getAutoOffTimeValue"] = getAutoOffTimeValue

g_export["setTemperatureAlarm"] = setTemperatureAlarm
g_export["getTemperatureAlarm"] = getTemperatureAlarm
g_export["setTemperatureFrequency"] = setTemperatureFrequency
g_export["getTemperatureFrequency"] = getTemperatureFrequency
g_export["lua_getTemperatureFrequency"] = lua_getTemperatureFrequency

g_export["getGatewayTemperature"] = getGatewayTemperature

function start_repo()
 -- logDebug("start_repo")
  --logDebug("---------------------")

  --    key step 1
  load_sensor_model()

  load_sensor_from_db()
  load_sensor_status_from_db()

  g_config = persistence.load(g_path_config_db)



  g_device_tree["gateway"] = { ["did"] = g_config.gateway_did, ["initstring"] = g_config.initstring, ["gateway_password"] = g_config.gateway_password, ["api_checksum"] = g_config.api_checksum, ["luascript"] = g_config.luascript, ["admin_password"] = g_config.admin_password, ["gateway_app_version"] = g_config.gateway_app_version }
  g_device_tree["item"] = g_item_pool
  g_device_tree["model"] = g_model_pool
  g_status_poll["devicejsontree"] = g_device_tree

  -- chenjian 20140411
  g_aeskey_pool = persistence.load(g_path_aeskey_db)
  if (g_aeskey_pool == nil) then
    g_aeskey_pool = {}
    persistence.store(g_path_aeskey_db, g_aeskey_pool)
  end
  updateCAesKey()

  --printAESKey(g_aeskey_pool)
end

function printAESKey(t)
 -- for k, v in pairs(t) do
 --   logDebug("k = " .. k .. ", v = " .. v)
 -- end
end

function isFile(name)
  local f = io.open(name)

  if f then
    f:close()
    return true
  end

  return false
end

function notifyGatewayUpdateOk()  --20150305_victorwu: notify mcu and app for update
  if (isFile("/mnt/fw_update") == true and nil ~= c_mqtt_publish_message) then
    local msg = {}
    msg.busname = "ttyS0"
    msg.action = "updateend"
    
    -- below NO use, but fan need
    msg.fullmodelno = "JSW-Slot-B084"
    msg.value = "On"
    msg.id = "0"
    msg.bind = "ttyS0=868/857323752/0"
    c_mqtt_publish_message("ttyS0", getjson(msg))
    os.execute("rm /mnt/fw_update")
  end
end

notifyGatewayUpdateOk()

-- 把 aeskey.db, config.db, item.db, linktable.db, schedule.db, virtualdevice.db 放到 /mnt/ 目录下面
set_db_path()

-- 获取SD卡的路径
get_SD_path() 

start_repo()
loadLink()
loadArmLink()
loadPartialArmLink()
loadVirtualDevice()
loadSchedule()
notifyGatewayArmState()

g_status_poll["linktable"] = g_linktable
g_status_poll["schedule"] = g_schedule

--g_bind_id_map["bind:" .. "ttyS0=433/4073500160/9"] = "14"
--
--local nextaction = json_module.decode('{"id":"5", "action":"OpenCamera"}')
--- - local k = "id_" .. "14" .. "_action_" .. "OpenClose" .. "_value_" .. "Open"
---- g_linktable[k] = {}
---- tableMerge(nextaction, getItemById(nextaction.id))
---- table.insert(g_linktable[k], nextaction)
--
---- updateItemTrigger(json_module.decode('{"bind":"ttyS0=433/4073500160/9", "action":"OpenClose", "value":"Open"}'))
--
-- r = lua_dispatch('{"method":"removeLink", "param":{"key":"id_4_action_OpenClose_value_Open", "target":{"id":"7", "action":"Trigger", "value":"On"}}}')
-- logDebug(r)
