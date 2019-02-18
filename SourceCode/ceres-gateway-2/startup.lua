require("persistence")
require("util")
local json_module = require("dkjson")

local gateway_db_path = "/mnt/gatewayinfo.db"
local config_db_path = "/mnt/config.db"
local item_db_path = "/mnt/item.db"

function copyfile(source,destination)
  local fSrc = io.open(source, "r")
  local fDst = io.open(destination, "w")

  local content = fSrc:read("*all")
  fDst:write(content)

  fSrc:close()
  fDst:close()
end

-- 判断 /mnt/config.db 是否存在,如果不存在就拷贝 ./config.db 过去
local file1, err1=io.open(config_db_path)
if (nil == file1) then
  copyfile("./config.db", config_db_path)
else
  print(config_db_path .. " exist!")
end

-- 判断 /mnt/item.db 是否存在,如果不存在就拷贝 ./item.db 过去
local file2, err2=io.open(item_db_path)
if (nil == file2) then
  copyfile("./item.db", item_db_path)
else
  print(item_db_path .. " exist!")
end

gatewayinfo = persistence.load(gateway_db_path)
--print(gatewayinfo.initstring)
if (gatewayinfo == nil) then
	gatewayinfo = {}
	gatewayinfo.did = ""
	gatewayinfo.license = ""
	gatewayinfo.password = ""
	gatewayinfo.apichecksum = ""
	gatewayinfo.initstring = ""
	gatewayinfo.admin_password = ""
	persistence.store(gateway_db_path, gatewayinfo)
end

g_config = persistence.load(config_db_path)
if (g_config == nil) then
    g_config = {}
    g_config.initstring = gatewayinfo.initstring
    g_config.gateway_did = gatewayinfo.did
    g_config.gateway_license = gatewayinfo.license
    g_config.gateway_password = gatewayinfo.password
    g_config.api_checksum = gatewayinfo.apichecksum
    
    g_config.admin_password = "123456"
    g_config.luascript = "core.lua"
    g_config.gateway_app_version = "V1.0.3.AG"
    
    g_config.setting = {}
    g_config.setting.ntpserver = true
    g_config.setting.staticip = true
    g_config.setting.daylight = true
    g_config.setting.wifiPassword = ""
    g_config.setting.receiver = ""
    g_config.setting.wifiapsetup = {}
    g_config.setting.timesetup = {}
    g_config.setting.message = {}
    
    g_config.setting.remotekey = ""
    g_config.setting.gatewaySiren = true

    persistence.store(config_db_path, g_config)
    g_config = persistence.load(config_db_path)
elseif g_config.initstring ~= gatewayinfo.initstring or g_config.gateway_did ~= gatewayinfo.did or g_config.api_checksum ~= gatewayinfo.apichecksum then
    g_config.initstring = gatewayinfo.initstring
    g_config.gateway_did = gatewayinfo.did
    g_config.gateway_license = gatewayinfo.license
    g_config.api_checksum = gatewayinfo.apichecksum
    g_config.gateway_password = gatewayinfo.password
    
    persistence.store(config_db_path, g_config)
    g_config = persistence.load(config_db_path)
end

-- Sync new parameter to config file
if g_config.setting.gatewaySiren == nil then
    -- gatewaySiren
    g_config.setting.gatewaySiren = true

    -- write back
    persistence.store(config_db_path, g_config)
end

--20150325_victorwu: for led lightness - begin
-- Set gateway led lightness
if g_config.setting.gatewayLedLight == "Low" then
  os.execute(". jsw_control_led.sh 6 -v 1")
  os.execute(". jsw_control_led.sh 7 -v 0")
elseif g_config.setting.gatewayLedLight == "High" then
  os.execute(". jsw_control_led.sh 6 -v 1")
  os.execute(". jsw_control_led.sh 7 -v 1")
else
  os.execute(". jsw_control_led.sh 6 -v 0")
end
--20150325_victorwu: for led lightness - end

local cmd = "./ceres " .. g_config.initstring .. " " .. g_config.gateway_did .. " " .. g_config.gateway_license .. " " .. g_config.gateway_password .. " " .. g_config.api_checksum .. " " .. g_config.luascript .. " " .. g_config.gateway_app_version
print("cmd:", cmd)
os.execute(cmd)
