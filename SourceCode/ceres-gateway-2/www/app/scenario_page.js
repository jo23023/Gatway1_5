/**
 * Created by leoyu on 8/7/14.
 */
function scenario_page_sm_init() {
    sm.onEnter("scenario_page_starter", function (transition, eventName, next) {
        next();
        show_scenario_page();
    });
    sm.onLeave("scenario_page_starter", function (transition, eventName, next) {
        next();
    });

}

var scenario_model = {};

// chenjian 2014-10-28
scenario_model.need_update_linktable = true;

scenario_model.show_edit = false;
scenario_model.max_schedule_receiver_device = 8;
scenario_model.max_schedule_camera_device = 4;
function show_scenario_page() {
    // chenjian 2014-10-28
    if (true == scenario_model.need_update_linktable || typeof scenario_model.mLinkTable == 'undefined') {
        var actionid = new Date().getTime();

        console.log("getLinktable");
        $("#main_content").html('<div><div class="spinner circles"><div></div><div></div><div></div><div></div><div></div><div></div><div></div><div></div></div></div>');

        commit_command(actionid, "getLinktable", {}, {}, function (response) {
            console.log("getValueByKey response: ", response);
            try {
                if (typeof response == 'string') {
                    response = JSON.parse(response);
                }

                // chenjian 2014-10-28
                scenario_model.need_update_linktable = false;
                
                scenario_model.mLinkTable = response.result;

                scenario_model.sender_list = sender_list;
                scenario_model.camera_list = camera_list;
                var link_keys = Object.keys(scenario_model.mLinkTable);
                link_keys.forEach(function (key, index) {
                    for (var i = 0; i < scenario_model.sender_list.length; i++) {
                        if (key.indexOf('id_' + scenario_model.sender_list[i].id) != -1 && typeof scenario_model.mLinkTable[key].enable != 'undefined') {
                            if (scenario_model.mLinkTable[key].enable == 'yes') {
                                scenario_model.sender_list[i].enable = true;
                            } else {
                                scenario_model.sender_list[i].enable = false;
                            }
                        } else {
//                            if (typeof scenario_model.sender_list[i].enable != 'undefined') {
//                                scenario_model.sender_list[i].enable = true;
//                            }
                        }
                    }
                    for (var i = 0; i < scenario_model.camera_list.length; i++) {
                        if (key.indexOf('id_' + scenario_model.camera_list[i].id) != -1 && typeof scenario_model.mLinkTable[key].enable != 'undefined') {
                            if (scenario_model.mLinkTable[key].enable == 'yes') {
                                scenario_model.camera_list[i].enable = true;
                            } else {
                                scenario_model.camera_list[i].enable = false;
                            }
                        } else {
//                            if (typeof scenario_model.camera_list[i].enable != 'undefined') {
//                                scenario_model.camera_list[i].enable = true;
//                            }
                        }
                    }
                });

                var tpl = get_precompiled_template("scenario_list", null);
                if (tpl == null) {
                    $.get("./tpl/scenario_list.tpl", function (ret) {
                        ret = ret + footer_html;
                        var tpl = get_precompiled_template("scenario_list", ret);
                        var source = tpl(scenario_model);
                        console.log(JSON.stringify(scenario_model.camera_list[0]))
                        $("#main_content").html(source);
                    });
                } else {
                    var source = tpl(scenario_model);
                    $("#main_content").html(source);
                }
            } catch (ex) {
                console.log("warn read data error");
                toastr['remove']();
                toastr['error'](i18n('warn read data error'));
                logout_gateway();
            }
        });
    }
    else {
        scenario_model.sender_list = sender_list;
        scenario_model.camera_list = camera_list;

        var tpl = get_precompiled_template("scenario_list", null);
        if (tpl == null) {
            $.get("./tpl/scenario_list.tpl", function (ret) {
                ret = ret + footer_html;
                var tpl = get_precompiled_template("scenario_list", ret);
                var source = tpl(scenario_model);
                $("#main_content").html(source);
            });
        } else {
            var source = tpl(scenario_model);
            $("#main_content").html(source);
        }
    }


}

function clone(myObj) {
    if (typeof(myObj) != 'object') return myObj;
    if (myObj == null) return myObj;
    var myNewObj = new Object();
    for (var i in myObj)
        myNewObj[i] = clone(myObj[i]);
    return myNewObj;
}

function scenario(id, type) {
    scenario_model.selected_type = type;
    scenario_model.links = {};

    if (type == 'sender') {
        for (var i = 0; i < scenario_model.sender_list.length; i++) {
            if (scenario_model.sender_list[i].id == id) {
                scenario_model.selected_scenario = scenario_model.sender_list[i];
                scenario_model.selected_scenario.command[0].value1 = scenario_model.selected_scenario.command[0].value[0];
                scenario_model.selected_scenario.command[0].value2 = scenario_model.selected_scenario.command[0].value[1];
                console.log(scenario_model.selected_scenario);
            }
        }
    } else if (type == 'camera') {
        for (var i = 0; i < scenario_model.camera_list.length; i++) {
            if (scenario_model.camera_list[i].id == id) {
                scenario_model.selected_scenario = scenario_model.camera_list[i];
                scenario_model.selected_scenario.command[0].value1 = scenario_model.selected_scenario.command[0].value[0];
                scenario_model.selected_scenario.command[0].value2 = scenario_model.selected_scenario.command[0].value[1];
                console.log(scenario_model.selected_scenario);
            }
        }
    }

    if (typeof scenario_model.mLinkTable != 'undefined' && scenario_model.mLinkTable != '') {
        scenario_model.links.schedule = {};
        if (scenario_model.selected_scenario.command[0].value1 == 'Open') {
            scenario_model.selected_scenario.check = true;
            scenario_model.selected_scenario.value = "Open";
        } else {
            scenario_model.selected_scenario.check = true;
            scenario_model.selected_scenario.value = "On";
        }
        var link_keys = Object.keys(scenario_model.mLinkTable);
//                console.log("link_keys", link_keys);

        link_keys.forEach(function (key, index) {
//            var links = scenario_model.mLinkTable[key];

            var links = clone(scenario_model.mLinkTable[key]);
//                    console.log("links", links);
            var keys_id_source_target = Object.keys(links);

            keys_id_source_target.forEach(function (key2, index2) {
                if (key.indexOf('id_' + id) != -1) {
                    if (key2 != 'enable') {
                        scenario_model.links.key = key;
//                if (key2 === "schedule") {
                        if (links[key2].target.action == 'OnOff') {
                            links[key2].target.actions1 = 'ON';
                            links[key2].target.actions2 = 'OFF';
                        } else if (links[key2].target.action == 'OpenClose') {
                            links[key2].target.actions1 = 'Open';
                            links[key2].target.actions2 = 'Close';
                        } else if (links[key2].target.action == 'Trigger') {
                            links[key2].target.actions1 = 'ON';
                            links[key2].target.actions2 = 'OFF';
                        }
                        if (links[key2].source.value == 'Open') {
                            scenario_model.selected_scenario.check = true;
                            scenario_model.selected_scenario.value = "Open";
                        } else if (links[key2].source.value == 'Close') {
                            scenario_model.selected_scenario.check = false;
                            scenario_model.selected_scenario.value = "Close";
                        }
                        if (links[key2].source.value == 'On') {
                            scenario_model.selected_scenario.check = true;
                            scenario_model.selected_scenario.value = "On";
                        } else if (links[key2].source.value == 'Off') {
                            scenario_model.selected_scenario.check = false;
                            scenario_model.selected_scenario.value = "Off";
                        }
                        if (links[key2].target.value == 'Open' || links[key2].target.value == 'On') {
                            links[key2].target.check = true;
                        } else {
                            links[key2].target.check = false;
                        }
                        scenario_model.links.schedule = links;
                    } else {

                    }

//                }
                } else {

                }
            });
        });
    } else {
        if (scenario_model.selected_scenario.command[0].value1 == 'Open') {
            scenario_model.selected_scenario.check = true;
            scenario_model.selected_scenario.value = "Open";
        } else {
            scenario_model.selected_scenario.check = true;
            scenario_model.selected_scenario.value = "On";
        }
        scenario_model.mLinkTable = {};
        scenario_model.links.schedule = {};
    }

    var tpl = get_precompiled_template("scenario", null);
    if (tpl == null) {
        $.get("./tpl/scenario.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("scenario", ret);
            var source = tpl(scenario_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(scenario_model);
        $("#main_content").html(source);
    }
}

function select_device_page() {
    scenario_model.receiver_list = receiver_list;
    scenario_model.camera_list = camera_list;

    var tpl = get_precompiled_template("select_device_list", null);
    if (tpl == null) {
        $.get("./tpl/select_device_list.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("select_device_list", ret);
            var source = tpl(scenario_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(scenario_model);
        $("#main_content").html(source);
    }
}

function add_device_to_scenario(target_index, type) {
    var schedule_receiver_count = 0;
    var schedule_camera_count = 0;
    for (keys in scenario_model.links.schedule) {
        try {
            if (scenario_model.links.schedule[keys].target.fullmodelno == "JSW-Camera-0001") {
                schedule_camera_count++;
            } else {
                schedule_receiver_count++;
            }
        } catch (e) {

        }
    }
//    if (schedule_count >= scenario_model.max_schedule_device) {
//        toastr['error'](i18n('Up to 8 devices'));
//        return;
//    }
    if (type == 'camera') {
        if (schedule_camera_count >= scenario_model.max_schedule_camera_device) {
            toastr['remove']();
            toastr['error'](i18n('The camera limit 4 is reached'));
            return;
        } else {
            if (scenario_model.selected_scenario.id == scenario_model.camera_list[target_index].id) {
                //IPC不能关联自己
//                toastr['remove']();
//                toastr['error'](i18n('The camera can not link to itself'));
//                return;
                var target = scenario_model.camera_list[target_index];
            } else {
                var target = scenario_model.camera_list[target_index];
            }
        }
    } else {
        if (schedule_receiver_count >= scenario_model.max_schedule_receiver_device) {
            toastr['remove']();
            toastr['error'](i18n('The device limit 8 is reached'));
            return;
        } else {
            var target = scenario_model.receiver_list[target_index];
        }
    }
    if (models_dict[target.fullmodelno].command[0].type == 'OnOff') {
        target.action = "OnOff";
        target.value = "On";
        target.check = true;
        target.actions1 = 'ON';
        target.actions2 = 'OFF';
    } else if (models_dict[target.fullmodelno].command[0].type == 'OpenClose') {
        target.action = "OpenClose";
        target.value = "Open";
        target.check = true;
        target.actions1 = 'Open';
        target.actions2 = 'Close';
    } else if (models_dict[target.fullmodelno].command[0].type == 'Trigger') {
        target.action = "Trigger";
        target.value = "On";
        target.check = true;
        target.actions1 = 'ON';
        target.actions2 = 'OFF';
    } else if (models_dict[target.fullmodelno].command[0].type == 'OpenCamera') {
        target.action = "OpenCamera";
        target.view = true;
        target.record = true;
    }
    key = "id_" + scenario_model.selected_scenario.id + "_" + target.id;
    scenario_model.links.schedule[key] = {};
    scenario_model.links.schedule[key].source = scenario_model.selected_scenario;
    scenario_model.links.schedule[key].target = target;
    console.log("-----------", scenario_model.links.schedule[key].target);
//    scenario(scenario_model.selected_scenario.id);
    var tpl = get_precompiled_template("scenario", null);
    if (tpl == null) {
        $.get("./tpl/scenario.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("scenario", ret);
            var source = tpl(scenario_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(scenario_model);
        $("#main_content").html(source);
    }
}

function change_arm_status(target_id) {
    key = "id_" + scenario_model.selected_scenario.id + "_" + target_id;
    schedule = scenario_model.links.schedule[key];
//    target_id = schedule.target.id;
    if (schedule.target.value == "On") {
        scenario_model.links.schedule[key].target.value = "Off";
        scenario_model.links.schedule[key].target.check = false;
    } else if (schedule.target.value == "Off") {
        scenario_model.links.schedule[key].target.value = "On";
        scenario_model.links.schedule[key].target.check = true;
    } else if (schedule.target.value == "Open") {
        scenario_model.links.schedule[key].target.value = "Close";
        scenario_model.links.schedule[key].target.check = false;
    } else if (schedule.target.value == "Close") {
        scenario_model.links.schedule[key].target.value = "Open";
        scenario_model.links.schedule[key].target.check = true;
    }
}

function change_cam_status(target_id, type) {
    key = "id_" + scenario_model.selected_scenario.id + "_" + target_id;
    schedule = scenario_model.links.schedule[key];
    if (type == 'view') {
        if (schedule.target.view == true) {
            scenario_model.links.schedule[key].target.view = false;
            $('#view_' + target_id).attr('src', 'images/Confirm_N.png');
        } else {
            scenario_model.links.schedule[key].target.view = true;
            $('#view_' + target_id).attr('src', 'images/Confirm_S.png');
        }
    }
    if (type == 'record') {
        if (schedule.target.record == true) {
            scenario_model.links.schedule[key].target.record = false;
            $('#record_' + target_id).attr('src', 'images/Confirm_N.png');
        } else {
            scenario_model.links.schedule[key].target.record = true;
            $('#record_' + target_id).attr('src', 'images/Confirm_S.png');
        }
    }

}

function change_source_status() {
    if (scenario_model.selected_scenario.value == 'Open') {
        scenario_model.selected_scenario.check = false;
        scenario_model.selected_scenario.value = "Close";
    } else if (scenario_model.selected_scenario.value == 'Close') {
        scenario_model.selected_scenario.check = true;
        scenario_model.selected_scenario.value = "Open";
    } else if (scenario_model.selected_scenario.value == 'On') {
        scenario_model.selected_scenario.check = false;
        scenario_model.selected_scenario.value = "Off";
    } else if (scenario_model.selected_scenario.value == 'Off') {
        scenario_model.selected_scenario.check = true;
        scenario_model.selected_scenario.value = "On";
    }
}

deleted_device = [];
function save_scenario() {
    $("#main_content").html('<div><div class="spinner circles"><div></div><div></div><div></div><div></div><div></div><div></div><div></div><div></div></div></div>');
    var what = {};
    what.source = {};
    what.source.id = scenario_model.selected_scenario.id;
    what.source.bind = scenario_model.selected_scenario.bind;
    what.source.name = scenario_model.selected_scenario.name;
    what.source.action = scenario_model.selected_scenario.command[0].type;
    if (scenario_model.selected_type == 'sender') {
        what.source.value = scenario_model.selected_scenario.value;
    } else {
        what.source.value = 'detect';
    }

    var keys = Object.keys(scenario_model.links.schedule);
    if (keys != 'undefined' && keys.length != 0) {
        keys.forEach(function (key, index) {
            if (key != 'enable') {
                console.log(key)
//        var camera_model = $scope.model_dict[$scope.selected_camera_array[i].fullmodelno];
                what.target = {};
                what.target.id = scenario_model.links.schedule[key].target.id;
                what.target.fullmodelno = scenario_model.links.schedule[key].target.fullmodelno;
                what.target.seat = scenario_model.links.schedule[key].target.seat;
                what.target.bind = scenario_model.links.schedule[key].target.bind;
                what.target.name = scenario_model.links.schedule[key].target.name;
                what.target.action = scenario_model.links.schedule[key].target.action;
                scenario_model.links.schedule[key].source.value = scenario_model.selected_scenario.value;
                if (scenario_model.links.schedule[key].target.action == "OpenCamera") {
                    what.target.view = scenario_model.links.schedule[key].target.view;
                    what.target.record = scenario_model.links.schedule[key].target.record;
                } else {
                    what.target.value = scenario_model.links.schedule[key].target.value;
                }
                //console.log(what);return;
                var actionid = new Date().getTime();
                commit_command(actionid, "addLink", what, {}, function () {
                    console.log("send addLink Sensor command");
                    if (typeof scenario_model.links.key == 'undefined') {
                        var mLinkTable_key = "id_" + scenario_model.selected_scenario.id;
                        scenario_model.mLinkTable[mLinkTable_key] = scenario_model.links.schedule;
                    } else {
                        var mLinkTable_keys = Object.keys(scenario_model.mLinkTable);
                        mLinkTable_keys.forEach(function (k, i) {
                            if (k == scenario_model.links.key) {
                                scenario_model.mLinkTable[k] = scenario_model.links.schedule;
                            }
                        });
                    }
                });
            }else{
                //add by wanghua 2014/11/4 10:52
                if(keys.length == 1){
                    var mLinkTable_keys = Object.keys(scenario_model.mLinkTable);
                    mLinkTable_keys.forEach(function (k, i) {
                        if (k == scenario_model.links.key) {
                            console.log(JSON.stringify(scenario_model.links.schedule));
                            scenario_model.mLinkTable[k] = scenario_model.links.schedule;
                        }
                    });
                }
            }
        });
    } else {
        var mLinkTable_keys = Object.keys(scenario_model.mLinkTable);
        mLinkTable_keys.forEach(function (k, i) {
            if (k == scenario_model.links.key) {
                console.log(JSON.stringify(scenario_model.links.schedule));
                scenario_model.mLinkTable[k] = scenario_model.links.schedule;
            }
        });
    }

    for (var i = 0; i < deleted_device.length; i++) {
        var actionid = new Date().getTime();
        commit_command(actionid, "removeLink", deleted_device[i], {}, function () {
            console.log("send removeLink command");
        });
    }
    deleted_device = [];

    show_scenario_page();
}

function show_edit() {
    if (scenario_model.show_edit == false) {
        scenario_model.show_edit = true;
        $('.noedit').hide();
        $('.edit').show();
    } else {
        scenario_model.show_edit = false;
        $('.edit').hide();
        $('.noedit').show();
    }
}

function del_scenario_device(target_id) {
    key = "id_" + scenario_model.selected_scenario.id + "_" + target_id;
    schedule = scenario_model.links.schedule[key];
//    schedule.key = "id_" + scenario_model.selected_scenario.id + "_action_" + scenario_model.selected_scenario.command[0].type + "_value_" + scenario_model.selected_scenario.value;

    if (scenario_model.selected_type == 'sender') {
        schedule.key = "id_" + scenario_model.selected_scenario.id + "_action_" + scenario_model.selected_scenario.command[0].type + "_value_" + scenario_model.selected_scenario.value;
    } else {
        schedule.key = "id_" + scenario_model.selected_scenario.id + "_action_" + scenario_model.selected_scenario.command[0].type + "_value_detect";
    }

    deleted_device.push(schedule);
    delete scenario_model.links.schedule[key];
    $('#tr_' + target_id).remove();

//    var what = schedule;
//    var actionid = new Date().getTime();
//    commit_command(actionid, "removeLink", what, {}, function () {
//        console.log("send removeLink command");
//        delete scenario_model.links.schedule[key];
//        $('#tr_' + target_id).remove();
//    });
}

function change_scenario_status(id) {
    var scenario_check = $('.scenario_' + id).is(':checked');
    var what = {};
    if (scenario_check == true) {
        what.enable = 'no';
    } else {
        what.enable = 'yes';
    }
    for (var i = 0; i < scenario_model.sender_list.length; i++) {
        if (scenario_model.sender_list[i].id == id) {
            what.source = scenario_model.sender_list[i];
            scenario_model.sender_list[i].enable = what.enable;

            what.source.action = what.source.command[0].type;
            if (what.source.action == 'OnOff') {
                what.source.value = 'On';
            } else if (what.source.action == 'OpenClose') {
                what.source.value = 'Open';
            }
        }
    }
    for (var i = 0; i < scenario_model.camera_list.length; i++) {
        if (scenario_model.camera_list[i].id == id) {
            what.source = scenario_model.camera_list[i];
            scenario_model.camera_list[i].enable = what.enable;

            what.source.action = what.source.command[0].type;
            what.source.value = 'detect';
        }
    }

    var actionid = new Date().getTime();
    commit_command(actionid, "setLinkToggle", what, {}, function () {
        console.log("send setLinkToggle command");
    });
}