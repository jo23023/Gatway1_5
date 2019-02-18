/**
 * Created by yulin on 14-7-31.
 */

function get_precompiled_template(tpl_name, tpl_source) {
    if (typeof(templates[tpl_name]) == "undefined") {
        if (tpl_source != null) {
            console.log("compiled template " + tpl_name);
            var tpl = Handlebars.compile(tpl_source);
            templates[tpl_name] = tpl;
            return tpl;
        } else {
            return null;
        }
    } else {
        console.log("use precompiled template " + tpl_name);
        return templates[tpl_name];
    }
}

Date.prototype.Format = function (fmt) { //author: meizz
    var o = {
        "M+": this.getMonth() + 1, //月份
        "d+": this.getDate(), //日
        "h+": this.getHours(), //小时
        "m+": this.getMinutes(), //分
        "s+": this.getSeconds(), //秒
        "q+": Math.floor((this.getMonth() + 3) / 3), //季度
        "S": this.getMilliseconds() //毫秒
    };
    if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    for (var k in o)
        if (new RegExp("(" + k + ")").test(fmt)) fmt = fmt.replace(RegExp.$1, (RegExp.$1.length == 1) ? (o[k]) : (("00" + o[k]).substr(("" + o[k]).length)));
    return fmt;
}


Handlebars.registerHelper('i18n', function (key) {
    if (typeof i18n_data[key] == "undefined") {
        return key;
    } else {
        return i18n_data[key];
    }
});

Handlebars.registerHelper('camera_state', function (id) {

    var varname = "camera_" + id + "_status";
    console.log("varname = " + varname);

    eval("var state = status_model." + varname + ";");
    console.log("state = " + state+" type = " + typeof(state));
    return state === "connected" ? "On":"Off";
});

Handlebars.registerHelper('show_alarm', function (id) {
    var varname = "alarm_" + id + "_state";
    // console.log("varname = " + varname);

    eval("var state = status_model." + varname + ";");
    // console.log("state = " + state);
    return state === "alarmed" ? "visible" : "hidden";
});

Handlebars.registerHelper('sensor_state', function (id) {
    var varname = "sensor_" + id + "_state";
    // console.log("varname = " + varname);

    eval("var state = status_model." + varname + ";");
    // console.log("state = " + state);
    return state === "On" ? "On" : "Off";
});


Handlebars.registerHelper('show_battery', function (id) {
    var varname = "alarm_" + id + "_battery";
    //console.log("varname = " + varname);

    eval("var state = status_model." + varname + ";");
    // console.log("state = " + state);
    return state === "low" ? "visible" : "hidden";
});

Handlebars.registerHelper('show_tamper', function (id) {
    var varname = "alarm_" + id + "_tamper";
    //console.log("varname = " + varname);

    eval("var state = status_model." + varname + ";");
    // console.log("state = " + state);
    return state === "tamper" ? "visible" : "hidden";
});


Handlebars.registerHelper('model_icon_url', function (fullmodelno) {
    if (typeof models_dict[fullmodelno] == "undefined") {
        return null;
    }
    return "images/model/" + models_dict[fullmodelno].icon + "_N.png";
});

Handlebars.registerHelper('event_date', function (d) {
    return new Date(d * 1000).Format("MM-dd-yyyy hh:mm:ss");
});

Handlebars.registerHelper('same', function (left, right) {
    return left === right;
});

Handlebars.registerHelper('checked', function (value) {
//    console.log("value: " + value);
    if(value == true ||value == 'yes') {
        return "checked";
    } else {
        return "";
    }
});

Handlebars.registerHelper('imgchecked', function (value) {
    console.log("value: " + value);
    if(value == true) {
        return "images/Confirm_S.png";
    } else {
        return "images/Confirm_N.png";
    }
});

Handlebars.registerHelper('selected', function (value1) {
    if(value1 == setting_model.timesetup.timezone) {
        return "selected";
    } else {
        return "";
    }
});

function sm_force_to(dest) {
    console.log(dest);
    var trans_name = sm.current + "_to_" + dest;
    console.log("trans_name: " + trans_name);
    sm.addTransition(trans_name, { from: sm.current, to: dest });
    console.log("smgoto: " + sm.transitions[trans_name].to);
    if (sm.current != sm.transitions[trans_name].to) {
        sm.fire(trans_name);
    }
}

// 用于浏览器调试
// PC浏览器加载本地HTML and JS文件，从本地读取模拟数据数据-调试用
var DATA_SOURCE_FAKE_DEBUG = 1;
// PC浏览器加载本地HTML and JS文件，从gateway上获取数据-调试用
var DATA_SOURCE_CHROME_DEBUG = 2;
// PC浏览器加载gateway上的HTML and JS文件-正式版本
var DATA_SOURCE_LAN = 3;


var data_from = DATA_SOURCE_LAN;


function commit_command(callid, method, params, option, fn) {
    var command = {};
    command.callid = callid.toString();
    command.method = method;
    command.param = params;
    command.option = option;

    if (typeof window.echo === "undefined") {
        // 非 android 平台
        var data = JSON.stringify(command);
        console.info("command = " + command);

        if (DATA_SOURCE_FAKE_DEBUG == data_from){
            var fileName = "";
            if (method == "devicejsontree"){
                fileName = "devicejsontree.json";
            }else if (method == "getVirtualDevice"){
                fileName = "virtualdevice.json"
            }else if (method == "getArmLinktable"){
                fileName = "armlinktable.json"
            }else if (method == "getEventList"){
                fileName = "eventlist.json";
            }else if (method == "getLinktable"){
                fileName = "linktable.json";
            }else if (method == "getSchedule"){
                fileName = "schedule.json";
            }

            $.get(fileName, function(response){
                fn(response);
            });
        }
        else if (DATA_SOURCE_CHROME_DEBUG == data_from){
            $.post("http://10.0.0.105:8888/command", {"command": data}, function (response) {
                    if (typeof fn === "undefined") {
                        return;
                    } else {
                        if (response.error == "authorize failed") {
                            window.location = "/login.html";
                        } else {
                            console.log("command ok");
                            fn(response);
                        }
                    }
                }
            );
        }
        else if (DATA_SOURCE_LAN == data_from){
            var data = "command=" + JSON.stringify(command);
//        console.info(data);
            $.post("/command", {"command": JSON.stringify(command)}, function (response) {
                    if (typeof fn === "undefined") {
                        return;
                    } else {
                        if (response.error == "authorize failed") {
                            window.location = "/login.html";
                        } else {
                            console.log("command ok");
                            fn(response);
                        }
                    }
                }
            );
        }
    } else {
        //defined window.echo - android 平台
        var d = JSON.stringify(command);
        //console.log("command:" + command.method);
        //alert(d);
        window.echo("command", d, function (r) {
            console.log("echo, response:", r);
            fn(r);
        });
    }
}

Object.extend = function(destination, source) {
    for (var property in source) {
        if (source.hasOwnProperty(property)) {
            destination[property] = source[property];
        }
    }
    return destination;
};
