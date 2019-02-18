/**
 * Created by yulin on 14-7-31.
 */


function status_page_sm_init() {
    sm.onEnter("status_page", function (transition, eventName, next) {
        next();
        console.log("enter status_page. status_model.data_update_needed = " + status_model.data_update_needed);
        if (true == status_model.data_update_needed) {
            get_devicejsontree();
        }
        else {
            show_status_page();
        }
    });
    sm.onLeave("status_page", function (transition, eventName, next) {
        next();
    });
}

var status_model = {};
status_model.data_update_needed = true;

function show_status_page() {
    console.log("show_status_page");
    status_model.in_edit = false;
    status_model.camera_list = camera_list;
    status_model.sender_list = sender_list;
    status_model.remote_list = remote_list;
    status_model.receiver_list = receiver_list;
    status_model.lang = i18n_data;

    var tpl = get_precompiled_template("status_page", null);
    if (tpl == null) {
        $.get("./tpl/status_page.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("status_page", ret);
            var source = tpl(status_model);
            $("#main_content").html(source);

            updateItemsStatus();
        });
    } else {
        var source = tpl(status_model);
        $("#main_content").html(source);

        updateItemsStatus();
    }

}

// 获取所有receiver item的当前 status
function updateItemsStatus() {
    console.log("updateItemsStatus()");
    var what = {};
    what.statusName = "status";
    what.ids = [];

    var len = receiver_list.length;
    for (var i = 0; i < len; i++) {
        what.ids.push(receiver_list[i].id)
    }

    console.log("what = " + JSON.stringify(what));
    var actionid = new Date().getTime();
    commit_command(actionid, "getMultiItemOneStatus", what, {}, function (ret) {
        console.log("getMultiItemOneStatus ret : " + ret);

        try {
            // update receiver item status
            if (typeof(ret) === "string") {
                ret = JSON.parse(ret);
            }

            var varname = null;
            var result = ret.result;
            var keys = Object.keys(result);
            keys.forEach(function (key, index) {
                varname = "sensor_" + key + "_state";
                eval("status_model." + varname + "='" + result[key] + "'");
            });
        } catch (ex) {
            console.log("warn read data error");
            toastr['remove']();
            toastr['error'](i18n('warn read data error'));
            logout_gateway();
        }
    });
}

var camera_list = [];
var sender_list = [];
var remote_list = [];
var receiver_list = [];

var devicejsontree = null;
var items_dict = {};
var items_list = [];
var rf433_nodeid_array = {};
var groups_dict = {};
var models_dict = {}; //dict

function clear_status_page_model_data() {
    console.log("clear_status_page_model_data");
    camera_list = [];
    sender_list = [];
    remote_list = [];
    receiver_list = [];

    devicejsontree = null;
    items_dict = {};
    items_list = [];
    rf433_nodeid_array = {};
    groups_dict = {};
    models_dict = {};
}

var readCount = 0;
function get_devicejsontree() {
    console.log("get_devicejsontree");
    $("#main_content").html('<div><div class="spinner circles"><div></div><div></div><div></div><div></div><div></div><div></div><div></div><div></div></div></div>');

    var actionid = new Date().getTime();
    commit_command(actionid, "devicejsontree", {}, null, function (data) {
        if (typeof window.echo == "undefined" &&
            (DATA_SOURCE_LAN == data_from || DATA_SOURCE_CHROME_DEBUG == data_from)) {
            data = JSON.parse(data);
        }

        try {
            clear_status_page_model_data();

            console.log("devicejsontree reply = " + data);
            devicejsontree = data.result;
            console.log(devicejsontree);

            models_dict = devicejsontree.model;

            // chenjian 2014-10-28
            scenario_model.need_update_linktable = true;

            var keys = Object.keys(devicejsontree.item);
            keys.forEach(function (key, index) {

                var item = devicejsontree.item[key.toString()];
                item.icon = "images/model/" + models_dict[item.fullmodelno].icon + ".png";
                if (item.fullmodelno.indexOf("Group") > -1) {
                    if (key.toString() === "1") {//1->Group_All, ignore it!
                    } else {
                        groups_dict[key.toString()] = item;
                    }
                } else {
                    rf433_nodeid_array[item.nodeid] = true;
                    items_dict[key.toString()] = item;
                    items_list.push(item);
                }
            });

            //将数据放入不同的设备array中去
            for (var i = 0; i < items_list.length; i++) {
                var item = items_list[i];
                item.command = models_dict[item.fullmodelno].command;
                if (gateway.isCamera(item)) {
                    camera_list.push(item);
                    init_camera_watcher(item.id);
                } else if (gateway.isReceiver(item)) {
                    receiver_list.push(item);
                    init_receiver_watcher(item.id);
                } else if (gateway.isSender(item)) {
                    sender_list.push(item);
                    init_sender_watcher(item.id);
                } else if (gateway.isRemoteKey(item)) {
                    remote_list.push(item);
                    init_remote_watcher(item.id);
                }
            }

            console.log("sm.current = " + sm.current);
            if (sm.current == "status_page") {
                // show RemoteKey UI if SimpleMode enabled
                if (true == setting_model.simple_mode) {
                    sm.fire("to_remote_controller_page");
                } else {
                    show_status_page();
                }
            }

            readCount = 0;
            status_model.data_update_needed = false;
        } catch (ex) {
            console.log("warn read data error");
            toastr['remove']();
            toastr['error'](i18n('warn read data error'));


            readCount++;
            if (readCount < 3) {
                setTimeout(function () {
                    get_devicejsontree();
                }, 3000);
            } else {
                readCount = 0;
                logout_gateway();
            }

        }
    });
}

function getCameraFromId(itemid) {
    var item = null;

    var len = camera_list.length;
    for (var i = 0; i < len; i++) {
        var cam = camera_list[i];
        if (cam.id == itemid) {
            console.log("find camera: " + itemid);
            item = cam;
            break;
        }
    }

    return item;
}

// 摄像头上报状态改变信息
// "{\"id\":\"%s\",\"did\":\"%s\",\"state\":\"%s\",\"code\":\"%s\"}"
function native_ipc_state_change(key, result) {
    console.log("native_ipc_state_change key = " + key + ", result = " + result);
    var cam = JSON.parse(result);
    var varname = "camera_" + cam.id + "_status";
    eval("status_model." + varname + "='" + cam.state + "';");
}

// gateway and sensor 上报状态信息：防拆报警，电量报警
// "{"id":"%s","status":"%s","value":"%s", "from":"%s"}"
function native_sensor_status(key, result) {
    console.log("native_sensor_status. key = " + key + ", result = " + result);

    var trigger = JSON.parse(result);
    var source = trigger.source;
    var status = source.status;
    var value = source.value;
    var from = source.from;

    if ("gateway" == from) {
        // gateway alarm
        if ("Tamper" == status) {
            toastr['remove']();
            toastr['error'](i18n('warn gateway tamper'));
        } else if ("Battery" == status) {
            var isLow = false;
            if (typeof(value) == "string" && "low" == value) {
                isLow = true;
            } else if (typeof(value) == "number" && value < 20) {
                isLow = true;
            }

            if (true == isLow) {
                toastr['remove']();
                toastr['error'](i18n('warn gateway battery low'));
            }
        } else if ("ARMState" == status) {
            if ("Lock" == value) {
                toastr['remove']();
                toastr['error'](i18n('arm lock'));
                if ($("#remote_lock_icon") != 'undefined' && $("#remote_lock_icon").length != 0) {
                    $("#remote_lock_icon").attr("src", "images/Arm_S.png");
                    $("#remote_unlock_icon").attr("src", "images/Disarm_N.png");
                }
                if (typeof control_page_model.arm_group != 'undefined') {
                    control_page_model.arm_group.remotekey = 'Lock';
                    if ($("#icon_" + control_page_model.arm_group.id) != 'undefined' && $("#icon_" + control_page_model.arm_group.id).length != 0) {
                        $("#icon_" + control_page_model.arm_group.id).attr("src", "images/Arm_N.png");
                    }
                }
            } else {
                toastr['remove']();
                toastr['error'](i18n('arm unlock'));
                if ($("#remote_lock_icon") != 'undefined' && $("#remote_lock_icon").length != 0) {
                    $("#remote_lock_icon").attr("src", "images/Arm_N.png");
                    $("#remote_unlock_icon").attr("src", "images/Disarm_S.png");
                }
                if (typeof control_page_model.arm_group != 'undefined') {
                    control_page_model.arm_group.remotekey = 'Unlock';
                    if ($("#icon_" + control_page_model.arm_group.id) != 'undefined' && $("#icon_" + control_page_model.arm_group.id).length != 0) {
                        $("#icon_" + control_page_model.arm_group.id).attr("src", "images/Disarm_N.png");
                    }
                }
            }
        }
    } else {
        var itemid = source.id;

        var varname = null;
        if ("Tamper" == status) {
            // 防拆报警
            varname = "alarm_" + itemid + "_tamper";
            eval("status_model." + varname + "='tamper'");
        }
        else if ("Battery" == status) {
            // 低电报警
            var isLow = false;
            if (typeof(value) == "string" && "low" == value) {
                isLow = true;
            } else if (typeof(value) == "number" && value < 20) {
                isLow = true;
            }

            if (true == isLow) {
                varname = "alarm_" + itemid + "_battery";
                if (value > 20) {    // 百分比
                    eval("status_model." + varname + "='high'");
                } else {
                    eval("status_model." + varname + "='low'");
                }
            }
        }
    }
}

// sensor 上报报警信息：门磁打开，PIR触发
function native_sensor_next_action(key, result) {
    console.log("native_sensor_next_action. key = " + key + ", result = " + result);

    // 判断 trigger 类型
    var trigger = JSON.parse(result);
    var source = trigger.source;
    var itemid = source.id;
    var action = source.action;
    var value = source.value;

    if ("OnOff" == action) {
        // 设备状态
        varname = "sensor_" + itemid + "_state";
        eval("status_model." + varname + "='" + value + "'");
    } else if ("OpenClose" == action) {
        // show item alarm icon
        var varname = "alarm_" + itemid + "_state";
        eval("status_model." + varname + "='alarmed'");
    }

    // test: show item battery low icon
    // var varname = "alarm_" + itemid + "_battery";
    // eval("status_model." + varname + "='low'");

    // who send this nextaction: sender? arm? scenario?
    console.log("from = " + trigger.from);
    if ("sensor" == trigger.from) {
        console.log("nothing todo");
    } else if ("arm" == trigger.from) {
        var targets = trigger.result;
        var keys = Object.keys(targets);
        keys.forEach(function (key, index) {
            var target = targets[key]
            if (target.action === "OpenCamera") {
                var camera = getCameraFromId(target.id);
                if (null != camera) {
                    var what = {};
                    what.did = camera.did;
                    what.password = camera.password;

                    if (typeof(window.echo) === "undefined") {
                        console.log("opencamera " + JSON.stringify(what));
                    } else {
                        window.echo("opencamera", JSON.stringify(what), function (r) {
                        })
                    }
                }
            }else if (target.action === "OnOff") {
                // chenjian 2014-10-28
                varname = "sensor_" + target.id + "_state";
                eval("status_model." + varname + "='" + target.value + "'");
            }
        });
    } else if ("scenario" == trigger.from) {
        var targets = trigger.result;
        var keys = Object.keys(targets);

        keys.forEach(function (key, index) {
            var target = targets[key].target;
            var source = targets[key].source;

            if (target.action === "OpenCamera") {
                var camera = getCameraFromId(target.id);
                if (null != camera) {
                    var what = {};
                    what.did = camera.did;
                    what.password = camera.password;

                    if (typeof(window.echo) === "undefined") {
                        console.log("opencamera " + JSON.stringify(what));
                    } else {
                        window.echo("opencamera", JSON.stringify(what), function (r) {
                        })
                    }
                }
            } else if ("OnOff" === target.action) {
                // 设备状态
                varname = "sensor_" + target.id + "_state";
                eval("status_model." + varname + "='" + target.value + "'");
            } else if ("OpenClose" == target.action) {
                // show item alarm icon
                var varname = "alarm_" + target.id + "_state";
                eval("status_model." + varname + "='alarmed'");
            }
        });
    }
}

/*function native_sensor_next_action(key, result){
 console.log("native_sensor_next_action. key = " + key + ", result = " + result);

 // 判断 trigger 类型
 var trigger = JSON.parse(result);
 if ("SenderTrigger" == trigger.method){
 // 仅仅是 Sender 触发信息

 // trigger = {"method":"SenderTrigger","result":{"name":"门磁","fullmodelno":"JSW-GateMaglock-0001","id":"25",
 // "value":"Open","action":"OpenClose"}}
 var itemid = trigger.result.id;
 var action = trigger.result.action;
 var value  = trigger.result.value;

 var varname = "alarm_" + itemid + "_state";
 eval("status_model." + varname + "='alarmed'");
 }else if ("SenderTriggerTarget" == trigger.method){
 // Sender  触发了其他 Target

 // trigger = {"method":"SenderTriggerTarget","result":
 // {"id_22_6":
 //   {"target":{"seat":"xiaomen","bind":"ttyS0=868/2357206976/0","fullmodelno":"JSW-Slot-B084","id":"6","action":"OnOff","name":"插座","value":"On"},
 //  "source":{"name":"ganying2","id":"22","value":"Open","action":"OpenClose","bind":"ttyS0=868/613/0"},"key":"id_22_action_OpenClose_value_Open"}}}
 var result = trigger.result;
 for (var key in result){
 console.log("key = " + key);
 var onerule = trigger[key];
 var source = result.source;

 var itemid = source.id;
 var action = source.action;
 var value = source.value;

 var varname = "alarm_" + itemid + "_state";
 eval("status_model." + varname + "='alarmed'");
 break;
 }
 }
 }*/

/*
 每个sensor上的图标位置
 1-5 空
 2-5 当前状态        On/Off          camera/receiver
 3-5 报警图标        alarm           sender
 4-5 低电图标        battery low     sender/receiver/remote
 5-5 防拆图标        tamper          暂时还没有明确哪些设备有防拆报警
 */
function init_camera_watcher(itemid) {
    console.log("init_camera_watcher. itemid = " + itemid);

    var varname = "camera_" + itemid + "_status";

    if (typeof status_model.varname == undefined) {
        eval("status_model." + varname + "=null;");
    }

    watch(status_model, varname, function () {
//        console.log("status_model." + varname + " changed!");
        if (status_model[varname] == "connected") {
            $("#camera_" + itemid + "_state").attr("src", "images/Power_On.png");
        } else {
            $("#camera_" + itemid + "_state").attr("src", "images/Power_Off.png");
        }
    });
}

function init_sender_watcher(itemid) {
    console.log("init_sender_watcher. itemid = " + itemid);

    // 设备触发报警信息
    var varname = "alarm_" + itemid + "_state";

    if (typeof status_model.varname == undefined) {
        eval("status_model." + varname + "=null;");
    }

    watch(status_model, varname, function () {
        console.log("status_model." + varname + " changed!");
        if (status_model[varname] == "alarmed") {
//            $("#alarm_" + itemid + "_state").show();
            $("#alarm_" + itemid + "_state").css({'visibility': 'visible'});
        } else {
//            $("#alarm_" + itemid + "_state").hide();
            $("#alarm_" + itemid + "_state").css({'visibility': 'hidden'});
        }
    });

    // 设备低电报警信息
    var name2 = "alarm_" + itemid + "_battery";
    if (typeof status_model.name2 == undefined) {
        eval("status_model." + name2 + "=null;");
    }

    watch(status_model, name2, function () {
        console.log("status_model." + name2 + " changed!");
        if (status_model[name2] == "low") {
//            $("#alarm_" + itemid + "_battery").show();
            $("#alarm_" + itemid + "_battery").css({'visibility': 'visible'});
        } else {
//            $("#alarm_" + itemid + "_battery").hide();
            $("#alarm_" + itemid + "_battery").css({'visibility': 'hidden'});
        }
    });

    // 设备防拆报警信息
    var name3 = "alarm_" + itemid + "_tamper";
    if (typeof status_model.name3 == undefined) {
        eval("status_model." + name3 + "=null;");
    }

    watch(status_model, name3, function () {
        console.log("status_model." + name3 + " changed!");
        if (status_model[name3] == "tamper") {
//            $("#alarm_" + itemid + "_tamper").show();
            $("#alarm_" + itemid + "_tamper").css({'visibility': 'visible'});
        } else {
//            $("#alarm_" + itemid + "_tamper").hide();
            $("#alarm_" + itemid + "_tamper").css({'visibility': 'hidden'});
        }
    });
}

function init_receiver_watcher(itemid) {
    console.log("init_receiver_watcher. itemid = " + itemid);

    // 设备 current state 信息
    var varname = "sensor_" + itemid + "_state";

    if (typeof status_model.varname == undefined) {
        eval("status_model." + varname + "=null;");
    }

    watch(status_model, varname, function () {
        console.log("status_model." + varname + " changed!");
        if (status_model[varname] == "On") {
            $("#sensor_" + itemid + "_state").attr("src", "images/Power_On.png");
        } else {
            $("#sensor_" + itemid + "_state").attr("src", "images/Power_Off.png");
        }
    });

    // 设备低电报警信息
    var name2 = "alarm_" + itemid + "_battery";
    if (typeof status_model.name2 == undefined) {
        eval("status_model." + name2 + "=null;");
    }

    watch(status_model, name2, function () {
        console.log("status_model." + name2 + " changed!");
        if (status_model[name2] == "low") {
//            $("#alarm_" + itemid + "_battery").show();
            $("#alarm_" + itemid + "_battery").css({'visibility': 'visible'});
        } else {
//            $("#alarm_" + itemid + "_battery").hide();
            $("#alarm_" + itemid + "_battery").css({'visibility': 'hidden'});
        }
    });

    // 设备防拆报警信息
    var name3 = "alarm_" + itemid + "_tamper";
    if (typeof status_model.name3 == undefined) {
        eval("status_model." + name3 + "=null;");
    }

    watch(status_model, name3, function () {
        console.log("status_model." + name3 + " changed!");
        if (status_model[name3] == "tamper") {
//            $("#alarm_" + itemid + "_tamper").show();
            $("#alarm_" + itemid + "_tamper").css({'visibility': 'visible'});
        } else {
//            $("#alarm_" + itemid + "_tamper").hide();
            $("#alarm_" + itemid + "_tamper").css({'visibility': 'hidden'});
        }
    });
}


function update_remote_status(jsonstring) {
    // TODO
}

function init_remote_watcher(itemid) {
    // TODO
}

function clear_alarm_status(itemid) {
    // clear alarm icon
    var varname = "alarm_" + itemid + "_state";
    eval("status_model." + varname + "='disalarmed'");

    // clear battery low icon
    var varname2 = "alarm_" + itemid + "_battery";
    eval("status_model." + varname2 + "='high'");

    // clear tamper icon
    var varname3 = "alarm_" + itemid + "_tamper";
    eval("status_model." + varname3 + "='notamper'");
}

function update_alarm_status123(jsonstring) {
    var v = JSON.parse(jsonstring);
    var itemid = v.result[Object.keys(v.result)[0]].source.id;
    var varname = "alarm_" + itemid + "_state";
    eval("status_model." + varname + "='alarmed'");
}

function status_page_enter_edit_mode() {
    status_model.in_edit = !status_model.in_edit;
    if (status_model.in_edit == true) {
        $(".edit-button").show();
    } else {
        $(".edit-button").hide();
    }
}

function db_remove_item(itemid) {
    var item = items_dict[itemid];
    item.child = item.id;

    var actionid = new Date().getTime();
    commit_command(actionid, "removeItem", item, {
        "tosaveintodb": true
    }, function () {
        console.log("remove succeed.");

        $("#item_" + itemid + "_block").remove();
        get_devicejsontree();
    });
}

function open_camera(itemid) {
    console.log("open_camera: " + itemid);

    if (typeof window.echo === "undefined") {
        toastr['remove']();
        toastr['error'](i18n('warn wrong operate in broswer'));
    } else {
        var varname = "camera_" + itemid + "_status";
        if (status_model[varname] == null || status_model[varname] == undefined) {
            console.log("has not get camera state");

            // 触发连接
            var item = items_dict[itemid];

            var what = {};
            what.did = item.did;
            what.password = item.password;
            window.echo("conncamera", JSON.stringify(what), function (r) {
            })
        }
        else if (status_model[varname] == "connected") {
            var item = items_dict[itemid];

            var what = {};
            what.did = item.did;
            what.password = item.password;

            window.echo("opencamera", JSON.stringify(what), function (r) {
            });
        }
        else {
            console.log("camera state = " + status_model[varname]);

            // 触发连接
            var item = items_dict[itemid];

            var what = {};
            what.did = item.did;
            what.password = item.password;
            window.echo("conncamera", JSON.stringify(what), function (r) {
            })
        }
    }
}

function configure_item(itemid) {
    console.log("configure_item: " + itemid);
    var item = items_dict[itemid];

    if ("JSW-Camera-0001" == item.fullmodelno) {
        show_configure_camera(item);
    }
    else {
        show_configure_sensor(item);
    }
}

function show_configure_camera(item) {
    status_model.confiure_item = item;

    var tpl = get_precompiled_template("configure_camera", null);
    if (tpl == null) {
        $.get("./tpl/configure_camera.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("configure_camera", ret);
            var source = tpl(status_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(status_model);
        $("#main_content").html(source);
    }
}


function update_ipc() {
    var name = $("#camera-name").val();
    if (name.trim() == "") {
//        alert(i18n("please input name"));
        toastr['remove']();
        toastr['error'](i18n('warn input name'));
        return;
    }

    var did = $("#camera-did").val();
    if (did.trim() == "") {
//        alert(i18n("please input did"));
        toastr['remove']();
        toastr['error'](i18n('warn input did'));
        return;
    }

    var passwd = $("#camera-password").val();
    if (passwd.trim() == "") {
//        alert(i18n("please input password"));
        toastr['remove']();
        toastr['error'](i18n('warn input password'));
        return;
    }

    var location = $("#camera-location").val();
    if (location.trim() == "") {
//        alert(i18n("please input location"));
        toastr['remove']();
        toastr['error'](i18n('warn input location'));
        return;
    }

    var what = {};
    what.name = name;
    what.did = did;
    what.password = passwd;
    what.seat = location;

    what.child = status_model.confiure_item.id;
    what.parent = "2";
    what.tosaveintodb = true;

    var actionid = new Date().getTime();
    commit_command(actionid, "updateItem", what, {"tosaveintodb": true}, function (ret) {
        console.log("update_ipc ret : " + ret);

        get_devicejsontree();
    });
}

function show_configure_sensor(item) {
    status_model.confiure_item = item;

    var tpl = get_precompiled_template("configure_sensor", null);
    if (tpl == null) {
        $.get("./tpl/configure_sensor.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("configure_sensor", ret);
            var source = tpl(status_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(status_model);
        $("#main_content").html(source);
    }
}

function update_sensor() {
    var actionid = new Date().getTime();

    var what = {};

    what.name = $("#sensor-name").val();
    if (what.name.trim() == "") {
//        alert(i18n("please input name"));
        toastr['remove']();
        toastr['error'](i18n('warn input name'));
        return;
    }

    what.seat = $("#sensor-seat").val();
    if (what.seat.trim() == "") {
//        alert(i18n("please input location"));
        toastr['remove']();
        toastr['error'](i18n('warn input location'));
        return;
    }

    console.log("update_sensor: " + what.name + ", " + what.seat);

    what.child = status_model.confiure_item.id;
    what.parent = "2";
    what.tosaveintodb = true;

    commit_command(actionid, "updateItem", what, {"tosaveintodb": true}, function (ret) {
        console.log("updateItem ret = " + ret);
        get_devicejsontree();
    });
}

function show_remote_controller_page_receiver(itemid) {

    // clear battery low and tamper icon
    var varname2 = "alarm_" + itemid + "_battery";
    eval("status_model." + varname2 + "='high'");

    var varname3 = "alarm_" + itemid + "_tamper";
    eval("status_model." + varname3 + "='notamper'");

    status_model.control_item = items_dict[itemid];
    var control_item_status_key = "sensor_" + itemid + "_state";
    var control_item_status = status_model[control_item_status_key];
    if (control_item_status == 'On') {
        var control_item_icon_on = 'images/remote_controller/ON_S.png';
        var control_item_icon_off = 'images/remote_controller/OFF_N.png';
    } else if (control_item_status == 'Off') {
        var control_item_icon_on = 'images/remote_controller/ON_N.png';
        var control_item_icon_off = 'images/remote_controller/OFF_S.png';
    } else {
        var control_item_icon_on = 'images/remote_controller/ON_N.png';
        var control_item_icon_off = 'images/remote_controller/OFF_N.png';
    }

    var tpl = get_precompiled_template("control_item_page", null);
    if (tpl == null) {
        $.get("./tpl/control_item_page.tpl", function (ret) {
            var tpl = get_precompiled_template("control_item_page", ret);
            var source = tpl(status_model);
            $("#main_content").html(source);
            $("#item_ctl_on").attr("src", control_item_icon_on);
            $("#item_ctl_off").attr("src", control_item_icon_off);
        });
    } else {
        var source = tpl(status_model);
        $("#main_content").html(source);
        $("#item_ctl_on").attr("src", control_item_icon_on);
        $("#item_ctl_off").attr("src", control_item_icon_off);
    }
}

function item_on(itemid) {
    if (false == ifCanOperate()) {
        return;
    }

    var item = items_dict[itemid];

    var what = {};
    what.id = item.id;
    what.action = "OnOff";
    what.value = "On";
    what.bind = item.bind;
    what.busname = "ttyS0";
    what.fullmodelno = item.fullmodelno;

    var actionid = new Date().getTime();
    commit_command(actionid, "controlItem", what, {}, function () {
        forbidOperate();
        console.log("send OnOff_On command");

        // change control icon image
        $("#item_ctl_on").attr("src", "images/remote_controller/ON_S.png");
        $("#item_ctl_off").attr("src", "images/remote_controller/OFF_N.png");

        // change sensor status icon in status_page.tpl
        var varname = "sensor_" + itemid + "_state";
        eval("status_model." + varname + "='On'");
    });
}

function item_off(itemid) {
    if (false == ifCanOperate()) {
        return;
    }

    var item = items_dict[itemid];

    var what = {};
    what.id = item.id;
    what.action = "OnOff";
    what.value = "Off";
    what.bind = item.bind;
    what.busname = "ttyS0";
    what.fullmodelno = item.fullmodelno;

    var actionid = new Date().getTime();
    commit_command(actionid, "controlItem", what, {}, function () {
        forbidOperate();
        console.log("send OnOff_Off command");

        // change icon
        $("#item_ctl_on").attr("src", "images/remote_controller/ON_N.png");
        $("#item_ctl_off").attr("src", "images/remote_controller/OFF_S.png");

        // change sensor status icon in status_page.tpl
        var varname = "sensor_" + itemid + "_state";
        eval("status_model." + varname + "='Off'");
    });
}

function goto_remotekey_sm(itemid) {
    // remote_controller_page_model.itemid = itemid;
    console.log("goto_remotekey_sm. itemid = " + itemid);
    sm.fire('to_remote_controller_page');
}

function i18n(key) {
    if (typeof i18n_data[key] == "undefined") {
        return key;
    } else {
        return i18n_data[key];
    }
}

function logout_gateway() {
    console.log("logout_gateway");

    if (typeof window.echo === "undefined") {
//        sm.fire("to_gateway_list_page");
        window.location.reload();
    } else {
        var param = {};
        param.did = gateway_list_page_model.connected_did;

        // test android to disconnect gateway
        window.echo("logout", JSON.stringify(param), function (r) {
        });
//        sm.fire("to_gateway_list_page");
        window.location.reload();
    }
}
