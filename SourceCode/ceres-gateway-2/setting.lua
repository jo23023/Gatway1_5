--
-- Created by IntelliJ IDEA.
-- User: yulin
-- Date: 14-1-25
-- Time: 上午11:31
--

local require = require
local json_module = require("dkjson")

require("persistence")
require("util")


package.path = './?.lua;./install/share/lua5.1/?.lua;./install/?.lua' .. package.path
package.cpath = './install/lib/lua5.1/?.so'





debug = true

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

local shellcmd = {
    ["GET WLAN MAC"] = "Group All";
    ["GET MAC0"] = "JSW-Group-0001";
    ["GET WAN MAC"] = "1";
    ["GET date"] = "itemrepo=1";
    ["GET DATEZONE"] = "itemrepo=1";
}
function get_wlan_mac()

    local cmdline = "reg c r 0"

    return os.capture(cmdline, true)
end

function get_mac0()

    local cmdline = "reg c r 1"
    return os.capture(cmdline, true)
end

function get_wan_mac()

    local cmdline = "reg c r 2"
    return os.capture(cmdline, true)
end

function get_date()
    local cmdline = "date  +\"%Y-%m-%d %H:%M\""
    return os.capture(cmdline, true)
end

function get_timezone()

    local cmdline = "nvram_get TZ"
    return os.capture(cmdline, true)
end


function get_ntp_time()

    local cmdline = "nvram_get NTPSync"
    return os.capture(cmdline, true)
end

function get_lan_ip()

    local cmdline = "nvram_get lan_ipaddr"
    return os.capture(cmdline, true)
end

function get_lan_netmask()

    local cmdline = "nvram_get lan_netmask"
    return os.capture(cmdline, true)
end

function get_lan_gateway()

    local cmdline = "nvram_get dhcpGateway"
    return os.capture(cmdline, true)
end

function get_lan_dns()

    local cmdline = "nvram_get dhcpPnDns"
    return os.capture(cmdline, true)
end

-- GW196_20150106_victorwu: for static ip
function get_wan_connection_mode()
    local cmdline = "nvram_get wanConnectionMode"
    return os.capture(cmdline, true)
end

function get_wan_ip()

    local cmdline = "nvram_get wan_ipaddr"
    return os.capture(cmdline, true)
end

function get_wan_netmask()

    local cmdline = "nvram_get wan_netmask"
    return os.capture(cmdline, true)
end

function get_wan_gateway()

    local cmdline = "nvram_get wan_gateway"
    return os.capture(cmdline, true)
end

function get_wan_dns()

    local cmdline = "nvram_get wan_primary_dns"
    return os.capture(cmdline, true)
end



function get_operationmode()
    local cmdline = "nvram_get OperationMode"
    return os.capture(cmdline, true)
end


function get_wlan_ssid()

    local cmdline = "nvram_get SSID1"
    return os.capture(cmdline, true)
end

function get_wlan_channel()

    local cmdline = "nvram_get Channel"
    return os.capture(cmdline, true)
end


function default_machine()

    local cmdline = "user_cmd l"
    return os.capture(cmdline, true)
end


function get_version()

    local cmdline = "user_cmd v"
    return os.capture(cmdline, true)
end

function get_diskinfo()

    local cmdline = "df"
    return os.capture(cmdline, true)
end

local setting = {
    ["get_wlan_mac"] = function() return get_wlan_mac() end,
    ["get_mac0"] = function() return get_mac0() end,
    ["get_wan_mac"] = function() return get_wan_mac() end,
    ["get_date"] = function() return get_date() end,
    ["get_timezone"] = function() return get_timezone() end,
    ["get_ntp_time"] = function() return get_ntp_time() end,
    ["get_ntp_serverip"] = function(x) return get_ntp_serverip(x) end,
    ["get_lan_ip"] = function() return get_lan_ip() end,
    ["get_lan_netmask"] = function() return get_lan_netmask() end,
    ["get_lan_gateway"] = function() return get_lan_gateway() end,
    ["get_lan_dns"] = function() return get_lan_dns() end,
    ["get_wan_connection_mode"] = function() return get_wan_connection_mode() end,
    ["get_wan_ip"] = function() return get_wan_ip() end,
    ["get_wan_netmask"] = function() return get_wan_netmask() end,
    ["get_wan_gateway"] = function() return get_wan_gateway() end,
    ["get_wan_dns"] = function() return get_wan_dns() end,
    ["get_wlan_ssid"] = function() return get_wlan_ssid() end,
    ["get_wlan_channel"] = function() return get_wlan_channel() end,
    ["get_operationmode"] = function() return get_operationmode() end,
    ["default_machine"] = function() return default_machine() end,
    ["get_version"] = function() return get_version() end,
    ["get_diskinfo"] = function() return get_diskinfo() end,
    ["set_wlan_mac"] = function(x) return set_wlan_mac(x) end,
    ["set_mac0"] = function(x) return set_mac0(x) end,
    ["set_wan_mac"] = function(x) return set_wan_mac(x) end,
    ["set_date"] = function(x) return set_date(x) end,
    ["set_timezone"] = function(x) return set_timezone(x) end,
    ["set_ntp_serverip"] = function(x) return set_ntp_serverip(x) end,
    ["set_ntp_time"] = function(x) return set_ntp_time(x) end,
    ["set_lan_ip"] = function(x) return set_lan_ip(x) end,
    ["set_lan_netmask"] = function(x) return set_lan_netmask(x) end,
    ["set_lan_gateway"] = function(x) return set_lan_gateway(x) end,
    ["set_lan_dns"] = function(x) return set_lan_dns(x) end,
    ["set_wan_connection_mode"] = function(x) return set_wan_connection_mode(x) end,
    ["set_wan_ip"] = function(x) return set_wan_ip(x) end,
    ["set_wan_netmask"] = function(x) return set_wan_netmask(x) end,
    ["set_wan_gateway"] = function(x) return set_wan_gateway(x) end,
    ["set_wan_dns"] = function(x) return set_wan_dns(x) end,
    ["set_wlan_ssid"] = function(x) return set_wlan_ssid(x) end,
    ["set_wlan_channel"] = function(x) return set_wlan_channel(x) end,
    ["set_operationmode"] = function(x) return set_operationmode(x) end,
	["enable_wifi"] = function(x) return enable_wifi(x) end,
}

function enable_wifi(x)
	if x == true then
		local cmdline = "ifconfig ra0 up"
		os.capture(cmdline, true)
	else
		local cmdline = "ifconfig ra0 down"
		os.capture(cmdline, true)
	end
end

function set_wlan_mac(x)

    local cmdline = "reg c w 0"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_mac0(x)

    local cmdline = "reg c w 1"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_wan_mac(x)

    local cmdline = "reg c w 2"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_date(x)

    local cmdline = "date -s"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_timezone(x)

    local cmdline = "nvram_set TZ"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_ntp_serverip(x)
    local cmdline = "nvram_set NTPServerIP"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_ntp_time(x)

    local cmdline = "nvram_set NTPSync"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x .. "; ntp.sh&"
    os.capture(cmdline, true)
    return "ok"
end

function set_lan_ip(x)

    local cmdline = "nvram_set lan_ipaddr"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_lan_netmask(x)

    local cmdline = "nvram_set lan_netmask"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_lan_gateway(x)

    local cmdline = "nvram_set dhcpGateway"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_lan_dns(x)

    local cmdline = "nvram_set dhcpPnDns"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

-- GW196_20150106_victorwu: for static ip
function set_wan_connection_mode(x)
    local cmdline = "nvram_set wanConnectionMode "

    if x == "0" then
        cmdline = cmdline .. "STATIC"
    else
        cmdline = cmdline .. "DHCP"
    end

    os.capture(cmdline, true)
    return "ok"
end

function set_wan_ip(x)
    local cmdline = "nvram_set wan_ipaddr "
	cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_wan_netmask(x)

    local cmdline = "nvram_set wan_netmask"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_wan_gateway(x)

    local cmdline = "nvram_set wan_gateway"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end

function set_wan_dns(x)

    local cmdline = "nvram_set wan_primary_dns"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    return os.capture(cmdline, true)
end



function set_operationmode(x)
    local cmdline = "nvram_set OperationMode"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end


function set_wlan_ssid(x)

    local cmdline = "nvram_set SSID1"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    return os.capture(cmdline, true)
end

function set_wlan_channel(x)

    local cmdline = "nvram_set Channel"
    cmdline = cmdline .. " "
    cmdline = cmdline .. x
    os.capture(cmdline, true)
    local cmdline = "iwpriv ra0 set Channel="

    cmdline = cmdline .. x
    os.capture(cmdline, true)
    return "ok"
end


function excut_cmd(cmdline, param)
    if (param == nil) then
        return setting[cmdline]()
    else
        return setting[cmdline](param)
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


 
