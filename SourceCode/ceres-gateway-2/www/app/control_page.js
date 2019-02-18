/**
 * Created by yulin on 14-8-1.
 */

function control_page_sm_init() {
    console.log("control page sm init...");
    sm.onEnter("control_page_starter", function (transition, eventName, next) {
        next();
        sm.fire("to_control_page_list");
    });
    sm.onLeave("control_page_starter", function (transition, eventName, next) {
        next();
    });

    sm.onEnter("control_page_list", function (transition, eventName, next) {
        next();
        console.log("begin show control page list");
        readDataThenShowControlList();
    });
}

var DEVICE_ID_BASE = 101;
var DEVICE_ID_MAX = 9;

var control_page_model = {};

// 是否需要读取数据数据
control_page_model.virtual_device_need_read = 1;
control_page_model.arm_linktable_need_read = 1;

var arm_control = {};

var all_control = {};
var camera_group = {};
var panic_group = {};

var general_group_arr = [
    { "index": "1", "name": "Group", "control": null },
    { "index": "2", "name": "Group", "control": null },
    { "index": "3", "name": "Group", "control": null },
    { "index": "4", "name": "Group", "control": null },
    { "index": "5", "name": "Group", "control": null },
    { "index": "6", "name": "Group", "control": null },
    { "index": "7", "name": "Group", "control": null },
    { "index": "8", "name": "Group", "control": null },
    { "index": "9", "name": "Group", "control": null }
];

function readDataThenShowControlList() {
    console.log("readDataThenShowControlList");

    if (control_page_model.virtual_device_need_read == 1) {
        get_virtualDevice();
    } else if (control_page_model.arm_linktable_need_read == 1) {
        get_armLinktable();
    } else {
        show_control_page_list();
    }
}

function clear_virtual_device_model_data() {
    console.log("clear_virtual_device_model_data");

    all_control = {};
    camera_group = {};
    panic_group = {};

    general_group_arr.forEach(function (key, index) {
        general_group_arr[index]["index"] = "" + (index + 1);
        general_group_arr[index]["name"] = "Group";
        general_group_arr[index]["control"] = null;
    });
}

function get_virtualDevice() {
    console.log("getVirtualDevice");
    $("#main_content").html('<div><div class="spinner circles"><div></div><div></div><div></div><div></div><div></div><div></div><div></div><div></div></div></div>');

    var actionid = new Date().getTime();
    commit_command(actionid, "getVirtualDevice", {}, null, function (data) {
        if (typeof window.echo == "undefined" &&
            (DATA_SOURCE_LAN == data_from || DATA_SOURCE_CHROME_DEBUG == data_from)) {
            data = JSON.parse(data);
        }

        try {
            console.log("getVirtualDevice data.result : " + JSON.stringify(data.result));

            if (all_control != null) {
                clear_virtual_device_model_data();

                all_control = data.result;
                control_page_model.virtual_device_need_read = 0;

                var keys = Object.keys(all_control);
                keys.forEach(function (key, index) {
                    var control = all_control[key];

                    if ("98" == key) {
                        camera_group = control;
                    }
                    else if ("99" == key) {
                        panic_group = control;
                    }
                    else {
                        if (typeof(control.index) != "undefined") {
                            var index = Number(control.index);
                            general_group_arr[index - 1].name = control.name;
                            general_group_arr[index - 1].control = control;
                        }
                    }
                });

                // 是否需要读取 ARM linktable
                if (1 == control_page_model.arm_linktable_need_read == 1) {
                    get_armLinktable();
                }
                else if (sm.current == "control_page_list") {
                    show_control_page_list();
                }
            }
        } catch (ex) {
            console.log("warn read data error");
            toastr['remove']();
            toastr['error'](i18n('warn read data error'));
            logout_gateway();
        }
    });
}

function get_armLinktable() {
    console.log("get_armLinktable");
    $("#main_content").html('<div><div class="spinner circles"><div></div><div></div><div></div><div></div><div></div><div></div><div></div><div></div></div></div>');

    var actionid = new Date().getTime();
    commit_command(actionid, "getArmLinktable", {}, null, function (data) {
        if (typeof window.echo == "undefined" &&
            (DATA_SOURCE_LAN == data_from || DATA_SOURCE_CHROME_DEBUG == data_from)) {
            data = JSON.parse(data);
        }

        try {
            console.log("getArmLinktable data.result : " + JSON.stringify(data.result));

            if (arm_control != null && typeof(data.result) != "undefined") {
                arm_control = data.result;
                control_page_model.arm_linktable_need_read = 0;
            }

            if (typeof(arm_control.id) == "undefined") {
                arm_control.id = "97";
            }

            if (typeof(arm_control.remotekey) == "undefined") {
                arm_control.id = "Unlock";
            }

            if (sm.current == "control_page_list") {
                show_control_page_list();
            }
        } catch (ex) {
            console.log("warn read data error");
            toastr['remove']();
            toastr['error'](i18n('warn read data error'));
            logout_gateway();
        }
    });
}

function show_control_page_list() {
    console.log("show_control_page_list");

    control_page_model.in_edit = false;

    control_page_model.arm_group = arm_control;

    control_page_model.camera_group = camera_group;
    control_page_model.panic_group = panic_group;
    control_page_model.general_group = general_group_arr;

    //获取模板
    var tpl = get_precompiled_template("control_page_list", null);

    if (tpl == null) {
        $.get("./tpl/control_page_list.tpl", function (ret) {

            ret = ret + footer_html;
            var tpl = get_precompiled_template("control_page_list", ret);
            var source = tpl(control_page_model);
            $("#main_content").html(source);

            $(".control-square").height((document.body.clientWidth - 12) / 3).width((document.body.clientWidth - 12) / 3);

            // 隐藏没有设置的 Control group
            hideEmptyControl();

            // 更新每个 Group 的状态图标
            updateControlGroupIcon();
        });
    } else {
        var source = tpl(control_page_model);
        $("#main_content").html(source);

        $(".control-square").height((document.body.clientWidth - 12) / 3).width((document.body.clientWidth - 12) / 3);

        // 隐藏没有设置的 Control group
        hideEmptyControl();

        // 更新每个 Group 的状态图标
        updateControlGroupIcon();
    }
}

function hideEmptyControl() {
    control_page_model.general_group.forEach(function (key, index) {
        var control = control_page_model.general_group[index];
        if (control.control == null) {
            var id = "#control_" + control.index;
            $(id).css("visibility", "hidden");
        }
    });
}

function updateControlGroupIcon() {
    // ARM
    if (typeof(control_page_model.arm_group.id) != "undefined") {
        if ("Lock" == control_page_model.arm_group.remotekey) {
            document.getElementById("icon_" + control_page_model.arm_group.id).src = "images/Arm_N.png";
        } else {
            document.getElementById("icon_" + control_page_model.arm_group.id).src = "images/Disarm_N.png";
        }
    }

}

function control_page_enter_edit_mode() {
    control_page_model.in_edit = !control_page_model.in_edit;
    if (control_page_model.in_edit == true) {
        $(".edit-button").show();
    } else {
        $(".edit-button").hide();
    }
}

function camera_select_changed(id) {
    console.log("camera_select_changed id = " + id);

    control_page_model.camera_list.forEach(function (camera, index) {
        if (id == camera.id) {
            control_page_model.camera_list[index].selected = !control_page_model.camera_list[index].selected;

            if (control_page_model.camera_list[index].selected) {
                document.getElementById("checkbox_" + id).src = "images/Confirm_N.png";
            } else {
                document.getElementById("checkbox_" + id).src = "images/Circle.png";
            }
        }
    });
}

function init_cameralist_seleted_icon() {
    console.log("init_cameralist_seleted_icon");

    control_page_model.camera_list.forEach(function (camera, index) {
        if (control_page_model.camera_list[index].selected) {
            document.getElementById("checkbox_" + camera.id).src = "images/Confirm_N.png";
        }
    });
}

function receiver_select_changed(id) {
    console.log("receiver_select_changed id = " + id);

    control_page_model.receiver_list.forEach(function (receiver, index) {
        if (id == receiver.id) {
            control_page_model.receiver_list[index].selected = !control_page_model.receiver_list[index].selected;

            if (control_page_model.receiver_list[index].selected) {
                document.getElementById("checkbox_" + id).src = "images/Confirm_N.png";
            } else {
                document.getElementById("checkbox_" + id).src = "images/Circle.png";
            }
        }
    });
}

function init_receiverlist_seleted_icon() {
    console.log("init_receiverlist_seleted_icon");

    control_page_model.receiver_list.forEach(function (receiver, index) {
        if (control_page_model.receiver_list[index].selected) {
            document.getElementById("checkbox_" + receiver.id).src = "images/Confirm_N.png";
        }
    });
}

function sender_select_changed(id) {
    console.log("sender_select_changed id = " + id);

    control_page_model.sender_list.forEach(function (sender, index) {
        if (id == sender.id) {
            control_page_model.sender_list[index].selected = !control_page_model.sender_list[index].selected;

            if (control_page_model.sender_list[index].selected) {
                document.getElementById("checkbox_" + id).src = "images/Confirm_N.png";
            } else {
                document.getElementById("checkbox_" + id).src = "images/Circle.png";
            }
        }
    });
}

function init_senderlist_seleted_icon() {
    console.log("init_senderlist_seleted_icon");

    control_page_model.sender_list.forEach(function (sender, index) {
        if (control_page_model.sender_list[index].selected) {
            document.getElementById("checkbox_" + sender.id).src = "images/Confirm_N.png";
        }
    });
}

function show_arm_control_page_setup(id) {
    console.log("show_arm_control_page_setup id = " + id);
    control_page_model.camera_list = [];
    control_page_model.receiver_list = [];
    control_page_model.sender_list = [];

    var control_type = "ARM";

    // set default name
    if (typeof(arm_control.name) == "undefined" || arm_control.name.length == 0) {
        arm_control.name = "ARM";
    }

    // add ip cam
    camera_list.forEach(function (cam, index) {
        cam.selected = false;
        if (-1 != models_dict[cam.fullmodelno].control.indexOf(control_type)) {
            control_page_model.camera_list.push(cam);
        }
    });

    // add receier
    receiver_list.forEach(function (receiver, index) {
        receiver.selected = false;
        if (-1 != models_dict[receiver.fullmodelno].control.indexOf(control_type)) {
            control_page_model.receiver_list.push(receiver);
        }
    });

    // add sender
    sender_list.forEach(function (sender, index) {
        sender.selected = false;
        if (-1 != models_dict[sender.fullmodelno].control.indexOf(control_type)) {
            control_page_model.sender_list.push(sender);
        }
    });

    // 判断哪些 sender 被选择过
    if (control_page_model.arm_group != null
        && control_page_model.arm_group.source != null
        && control_page_model.arm_group.target != null) {
        var source_keys = Object.keys(control_page_model.arm_group.source);
        control_page_model.sender_list.forEach(function (sender, index) {
            if (-1 != source_keys.indexOf(sender.id)) {
                control_page_model.sender_list[index].selected = true;
            } else {
                control_page_model.sender_list[index].selected = false;
            }
        });

        // 判断哪些 receiver 被选择过
        var target_keys = Object.keys(control_page_model.arm_group.target);
        control_page_model.receiver_list.forEach(function (receiver, index) {
            if (-1 != target_keys.indexOf(receiver.id)) {
                control_page_model.receiver_list[index].selected = true;
            } else {
                control_page_model.receiver_list[index].selected = false;
            }
        });

        // 判断哪些 camera 被选择过
        control_page_model.camera_list.forEach(function (camera, index) {
            if (-1 != target_keys.indexOf(camera.id)) {
                control_page_model.camera_list[index].selected = true;
            } else {
                control_page_model.camera_list[index].selected = false;
            }
        });
    }

    // show html UI
    var tpl = get_precompiled_template("arm_page_setup", null);
    if (tpl == null) {
        $.get("./tpl/arm_page_setup.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("arm_page_setup", ret);
            var source = tpl(control_page_model);
            $("#main_content").html(source);

            // 更新图片
            init_senderlist_seleted_icon();
            init_receiverlist_seleted_icon();
            init_cameralist_seleted_icon();
        });
    } else {
        var source = tpl(control_page_model);
        $("#main_content").html(source);

        // 更新图片
        init_senderlist_seleted_icon();
        init_receiverlist_seleted_icon();
        init_cameralist_seleted_icon();
    }
}

/*
 id:
 -1: add new group
 101--109: edit exist group

 control_page_model.setup_type 用来指示当前编辑的 group 类型
 1 - configure new   genera group
 2 - configure exist genera group

 3 - configure new   camera group
 4 - configure exist camera group

 5 - configure new   panic group
 6-  configure exist panic group
 */
function show_all_control_page_setup(id) {
    console.log("show_all_control_page_setup id = " + id);

    if ("-1" == id) {
        var new_id = -1;
        var id_keys = Object.keys(all_control);
        for (var i = DEVICE_ID_BASE; i < DEVICE_ID_BASE + DEVICE_ID_MAX; i++) {
            if (-1 == id_keys.indexOf("" + i)) {
                new_id = i;
                break;
            }
        }

        if (-1 == new_id) {
//            alert("Group has reached to max: 9");
            toastr['remove']();
            toastr['error'](i18n('warn max group reached'));
            return;
        }

        console.log("new_id = " + new_id);
        control_page_model.setup_id = "" + new_id;

        // new general group
        control_page_model.setup_type = 1;
    }
    else {
        control_page_model.setup_id = id;

        var group = all_control[id];
        if (typeof(group) === "undefined") {
            if ("98" == id) {
                // new camera group
                control_page_model.setup_type = 3;
            } else if ("99" == id) {
                // new panic group
                control_page_model.setup_type = 5;
            }
        } else {
            if ("98" == id) {
                // exist camera group
                control_page_model.setup_type = 4;
            } else if ("99" == id) {
                // exist panic group
                control_page_model.setup_type = 6;
            } else {
                // exist general group
                control_page_model.setup_type = 2;
            }
        }
    }


    // "control":["ARM","Panic","Record","Control"]
    var control_type = null;
    if ("98" == id) {
        control_type = "Record";
    } else if ("99" == id) {
        control_type = "Panic";
    } else {
        control_type = "Control";
    }
    console.log("control_type = " + control_type);

    control_page_model.control = {};
    control_page_model.camera_list = [];
    control_page_model.receiver_list = [];

    // add ip cam
    camera_list.forEach(function (cam, index) {
        cam.selected = false;
        if (-1 != models_dict[cam.fullmodelno].control.indexOf(control_type)) {
            control_page_model.camera_list.push(cam);
        }
    });

    // add receier
    receiver_list.forEach(function (receiver, index) {
        receiver.selected = false;
        if (-1 != models_dict[receiver.fullmodelno].control.indexOf(control_type)) {
            control_page_model.receiver_list.push(receiver);
        }
    });

    var control = all_control[id];

    if (1 == control_page_model.setup_type) {
        control = {};
        control.name = "Control " + (Number(control_page_model.setup_id) - 100);
    } else if (3 == control_page_model.setup_type) {
        control = {};
        control.name = "Camera";
    } else if (5 == control_page_model.setup_type) {
        control = {};
        control.name = "Panic";
    } else {
        // 该Group已经存在
        /*        //set default name
         if (typeof(control.name) == "undefined" || control.name.length == 0){
         if ("98" == id){
         control.name = "Camera";
         }else if ("99" == id){
         control.name = "Panic";
         }else{
         control.name = "Control " + (Number(id)-100);
         }
         }*/

        var item_keys = Object.keys(control.items); //已经selected的item id的数组

        control_page_model.camera_list.forEach(function (camera, index) {
            if (-1 != item_keys.indexOf(camera.id)) {
                control_page_model.camera_list[index].selected = true;
            } else {
                control_page_model.camera_list[index].selected = false;
            }
        });

        // set receiver selected or not
        control_page_model.receiver_list.forEach(function (receiver, index) {
            if (-1 != item_keys.indexOf(receiver.id)) {
                control_page_model.receiver_list[index].selected = true;
            } else {
                control_page_model.receiver_list[index].selected = false;
            }
        });
    }
    control_page_model.control = control;

    var tpl = get_precompiled_template("control_page_setup", null);
    if (tpl == null) {
        $.get("./tpl/control_page_setup.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("control_page_setup", ret);
            var source = tpl(control_page_model);
            $("#main_content").html(source);

            // 更新图片
            init_receiverlist_seleted_icon();
            init_cameralist_seleted_icon();
        });
    } else {
        var source = tpl(control_page_model);
        $("#main_content").html(source);

        // 更新图片
        init_receiverlist_seleted_icon();
        init_cameralist_seleted_icon();
    }
}

function save_arm_setup() {
    if (false == ifCanOperate()) {
        return;
    }
    console.log("save_arm_setup");

    // build new sender
    var new_source = {};
    control_page_model.sender_list.forEach(function (sender, index) {
        console.log("sender name: " + sender.name);

        if (true == control_page_model.sender_list[index].selected) {
            new_source[sender.id] = {};
            new_source[sender.id].name = sender.name;
            new_source[sender.id].id = sender.id;
            new_source[sender.id].value = models_dict[sender.fullmodelno].command[0].value[0];
            new_source[sender.id].action = models_dict[sender.fullmodelno].command[0].type;
            new_source[sender.id].bind = sender.bind;
        }
    });

    // build new receiver
    var new_target = {};
    control_page_model.receiver_list.forEach(function (receiver, index) {
        console.log("receiver name: " + receiver.name);

        if (true == control_page_model.receiver_list[index].selected) {
            new_target[receiver.id] = {};
            new_target[receiver.id].name = receiver.name;
            new_target[receiver.id].id = receiver.id;
            new_target[receiver.id].value = models_dict[receiver.fullmodelno].command[0].value[0];
            new_target[receiver.id].action = models_dict[receiver.fullmodelno].command[0].type;
            new_target[receiver.id].bind = receiver.bind;
        }
    });

    // build new camera
    control_page_model.camera_list.forEach(function (cam, index) {
        console.log("cam name: " + cam.name);

        if (true == control_page_model.camera_list[index].selected) {
            new_target[cam.id] = {};
            new_target[cam.id].name = cam.name;
            new_target[cam.id].id = cam.id;
            new_target[cam.id].action = models_dict[cam.fullmodelno].command[0].type;
            new_target[cam.id].bind = cam.bind;
            new_target[cam.id].view = true;
            new_target[cam.id].record = true;
        }
    });

    // 判断是否选择了设备
    var source_keys = Object.keys(new_source);
    console.log("source_keys.length = " + source_keys.length);

    var target_keys = Object.keys(new_target);
    console.log("target_keys.length = " + target_keys.length);

    // chenjian 2014-10-28
//    if (0 == source_keys.length + target_keys.length) {
//        toastr['remove']();
//        toastr['error'](i18n('warn select device'));
//        return;
//    }

    var new_arm_control = {};
    new_arm_control.name = $("#group_setup_name").val();
    new_arm_control.id = "97";

    // arm_linktable 不保存 remotekey
//    if (typeof arm_control.remotekey === "undefined"){
//        new_arm_control.remotekey = "Unlock";
//    }else{
//        new_arm_control.remotekey = arm_control.remotekey;
//    }

    new_arm_control.source = new_source;
    new_arm_control.target = new_target;
    control_page_model.new_arm_control = new_arm_control;

    // for debug
    console.log("new_arm_control = " + new_arm_control);

    // send to gateway
    var actionid = new Date().getTime();
    commit_command(actionid, "setArmLinktable", new_arm_control, {"tosaveintodb": true},
        function (rsp) {
            forbidOperate();
            console.log("setArmLinktable rsp : " + rsp);

            control_page_model.arm_linktable_need_read = 1;
            get_armLinktable();
        }
    );
}

function save_control_setup() {
    if (false == ifCanOperate()) {
        return;
    }
    console.log("save_control_setup setup_id = " + control_page_model.setup_id);

    // group name
    var group_name = $("#group_setup_name").val();

    var item = {};
    if (1 == control_page_model.setup_type) {
        console.log("configure new general group");
        // find new group id and index
        item.id = control_page_model.setup_id;
        item.name = group_name;
        item.bind = "virtualdevice=" + item.id;
        item.value = "Off";
        item.fullmodelno = 'JSW-VirtualDevice-001';
        item.index = "" + (Number(control_page_model.setup_id) - 100);
    } else if (2 == control_page_model.setup_type) {
        console.log("configure exist general group");
        item.id = control_page_model.setup_id;
        item.name = group_name;
        item.bind = control_page_model.control.bind;
        item.value = control_page_model.control.value;
        item.fullmodelno = control_page_model.control.fullmodelno;
        item.index = control_page_model.control.index;
    } else {
        console.log("configure new/exist camera/panic grup");
        item.id = control_page_model.setup_id;
        item.name = group_name;
        item.bind = "virtualdevice=" + item.id;
        item.value = "Off";
        item.fullmodelno = "JSW-VirtualDevice-001";
    }

    item.items = {};
    control_page_model.camera_list.forEach(function (cam, index) {
        console.log("cam name: " + cam.name);

        if (true == control_page_model.camera_list[index].selected) {
            item.items[cam.id] = [];
            var values = models_dict[cam.fullmodelno].command[0].value;
            values.forEach(function (key, index) {
                item.items[cam.id].push(values[index]);
            });

            item.items[cam.id].push(models_dict[cam.fullmodelno].command[0].type);
        }
    });

    control_page_model.receiver_list.forEach(function (receiver, index) {
        console.log("receiver name: " + receiver.name);

        if (true == control_page_model.receiver_list[index].selected) {
            item.items[receiver.id] = [];
            var values = models_dict[receiver.fullmodelno].command[0].value;
            values.forEach(function (key, index) {
                item.items[receiver.id].push(values[index]);
            });

            item.items[receiver.id].push(models_dict[receiver.fullmodelno].command[0].type);
        }
    });

    // 判断是否选择了设备
    var items_keys = Object.keys(item.items);
    console.log("items_keys.length = " + items_keys.length);

/*    if (0 == items_keys.length) {
        toastr['remove']();
        toastr['error'](i18n('warn select device'));
        return;
    }*/

    // for debug
//    control_page_model.item = item;

    // send to gateway
    var actionid = new Date().getTime();
    commit_command(actionid, "updateVirtualDevice", item, {"tosaveintodb": true},
        function (rsp) {
            forbidOperate();
            console.log("updateVirtualDevice rsp : " + rsp);

            control_page_model.virtual_device_need_read = 1;
            get_virtualDevice();
        }
    );
}

function delete_control_group(id) {
    if (false == ifCanOperate()) {
        return;
    }
    console.log("delete_control_group id = " + id);

    var control = all_control[id];

    var item = {};
    item.id = control.id;
    item.name = control.name;
    item.bind = control.bind;
    item.value = control.value;
    item.fullmodelno = control.fullmodelno;
    item.index = control.index;
    item.items = {};

    var actionid = new Date().getTime();
    commit_command(actionid, "removeVirtualDevice", item, { "tosaveintodb": true },
        function (rsp) {
            forbidOperate();
            console.log("removeVirtualDevice rsp : " + rsp);
            control_page_model.virtual_device_need_read = 1;
            get_virtualDevice();
        }
    );
}

// 改变 ARM 的 remotekey 状态，发送到 gateway
function send_arm_control_command(id) {
    if (false == ifCanOperate()) {
        return;
    }

    console.log("send_arm_control_command id = " + id);

    var what = {};
    if (typeof(arm_control.id) != "undefined") {
        // exist
        if ("Lock" == arm_control.remotekey) {
            arm_control.remotekey = "Unlock";
            what.remotekey = "Unlock";
        } else {
            arm_control.remotekey = "Lock";
            what.remotekey = "Lock";
        }
    } else {
        // ARM has NOT configured
        console.log("ARM has NOT configured!!!!!!");
        return;
    }

    // for debug
    console.log("what = " + what);

    // send to gateway
    var actionid = new Date().getTime();
    commit_command(actionid, "setRemotekeyValue", what, {"tosaveintodb": true},
        function (rsp) {
            forbidOperate();
            console.log("send_arm_control_command rsp : " + rsp);

//            control_page_model.arm_linktable_need_read = 1;
//            get_armLinktable();

            // change icon
            var imgPath = null;
            var iconId = "icon_" + arm_control.id;
            if ("Lock" == arm_control.remotekey) {
                imgPath = "images/Arm_N.png";
            } else {
                imgPath = "images/Disarm_N.png";
            }
            document.getElementById(iconId).src = imgPath;
        }
    );
}

// 发送控制指令到 gateway
function send_group_control_command(id) {
    if (false == ifCanOperate()) {
        return;
    }

    console.log("send_group_control_command id = " + id);

    var control = all_control[id];
    if (typeof(control) == "undefined") {
        console.log("Empty Group");
        return;
    }

    var item = {};
    item.id = control.id;
    item.name = control.name;

    if (control.value == 'On') {
        item.value = 'Off';
        control.value = 'Off';
    } else {
        item.value = 'On';
        control.value = 'On';
    }

    var actionid = new Date().getTime();
    commit_command(actionid, "virtualDeviceTrigger", item, { "tosaveintodb": false },
        function (rsp) {
            forbidOperate();
            console.log("virtualDeviceTrigger rsp : " + rsp);

            // 切换图标状态
            var iconId = null;
            var imgPath = null;
            if ("98" == id) {
                // camera
                iconId = "icon_98";
                if ("On" == control.value) {
                    imgPath = "images/CameraRecording_S.png";
                } else {
                    imgPath = "images/CameraRecording_N.png";
                }
            } else if ("99" == id) {
                // panic
                iconId = "icon_99";
                if ("On" == control.value) {
                    imgPath = "images/Panic_S.png";
                } else {
                    imgPath = "images/Panic_N.png";
                }
            } else {
                // general
                iconId = "icon_" + control.id;
                if ("On" == control.value) {
                    imgPath = "images/" + control.index + "_S.png";
                } else {
                    imgPath = "images/" + control.index + "_N.png";
                }
            }

            document.getElementById(iconId).src = imgPath;
        }
    );
}
