function add_device_page_sm_init() {
    console.log("add_device_page_init");

    sm.onEnter("add_device_starter", function (transition, eventName, next) {
        next();
        console.log("enter add_device_starter");
        sm.fire("to_add_device_step1");
    });
    sm.onEnter("add_device_step1", function (transition, eventName, next) {
//        show_add_device_step1();
        show_add_device_step2();
        next();
    });
    sm.onLeave("add_device_step1", function (transition, eventName, next) {
        next();
    });

    sm.onEnter("add_device_step2_sensor", function (transition, eventName, next) {
        next();
        show_add_device_step2_sensor();
    });
    sm.onLeave("add_device_step2_sensor", function (transition, eventName, next) {
        next();
    });

    sm.onEnter("add_device_step2_camera", function (transition, eventName, next) {
        next();
        show_add_device_step2_camera();
    });
    sm.onLeave("add_device_step2_camera", function (transition, eventName, next) {
        next();
    });
}

var add_device_model = {};
//add_device_model.camera_list_changed = 0;

function show_add_device_step1() {
    console.log("show_add_device_step1");

    add_device_model.in_edit = false;
//    add_device_model.camera_list = camera_list;
    add_device_model.sender_list = sender_list;
    add_device_model.receiver_list = receiver_list;
    add_device_model.lang = i18n_data;

    var tpl = get_precompiled_template("add_device_step1", null);

    if (tpl == null) {
        $.get("./tpl/add_device_step1.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("add_device_step1", ret);
            var source = tpl(add_device_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(add_device_model);
        $("#main_content").html(source);
    }
}

/*function update_ipc_search_camera_list(arr) {
    add_device_model.camera_list = arr;
    add_device_model.camera_list_changed = 1;
}*/

function show_add_device_step2_camera() {
    console.log("show_add_device_step2_camera");

    // 判断是否已经添加到 4 个 IPCAM
    console.log("camera_list.length = " + camera_list.length);
    if (camera_list.length >= 4){
        toastr['remove']();toastr['error'](i18n('warn max ipc'));
        return;
    }

    var tpl = get_precompiled_template("add_device_step2_camera", null);

    if (tpl == null) {
        $.get("./tpl/add_device_step2_camera.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("add_device_step2_camera", ret);
            var source = tpl(add_device_model);
            $("#main_content").html(source);

            if (typeof window.echo != "undefined"){
                search_ipc_wizard();
            }
        });
    } else {
        var source = tpl(add_device_model);
        $("#main_content").html(source);

        if (typeof window.echo != "undefined"){
                search_ipc_wizard();
        }
    }
}

function show_add_device_step2_sensor() {
    console.log("show_add_device_step2_sensor");

    if (typeof add_device_model.models_list != "undefined" && add_device_model.models_list.length > 0) {

    } else {
        add_device_model.models_list = [];
        var keys = Object.keys(models_dict);
        keys.forEach(function (key, index) {
            var model = models_dict[key];
            if (typeof(model.mode) != "undefined") { //只包含定义了mode的设备，例如Sender, Receiver
                model.iconS = "images/model/" + model.icon + ".png";
                model.iconN = "images/model/" + model.icon + "_N.png";
                add_device_model.models_list.push(model);

            }
        });
    }
    console.log(add_device_model.models_list);

    var tpl = get_precompiled_template("add_device_step2_sensor", null);

    if (tpl == null) {
        $.get("./tpl/add_device_step2_sensor.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("add_device_step2_sensor", ret);
            var source = tpl(add_device_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(add_device_model);
        $("#main_content").html(source);
    }
}

function show_add_device_step2() {
    console.log("show_add_device_step2_sensor");

    if (typeof add_device_model.models_list != "undefined" && add_device_model.models_list.length > 0) {

    } else {
        add_device_model.models_list = [];
        var keys = Object.keys(models_dict);
        keys.forEach(function (key, index) {
            var model = models_dict[key];
            if (typeof(model.mode) != "undefined") { //只包含定义了mode的设备，例如Sender, Receiver
                model.iconS = "images/model/" + model.icon + ".png";
                model.iconN = "images/model/" + model.icon + "_N.png";
                add_device_model.models_list.push(model);

            }
        });
    }
    console.log(add_device_model.models_list);

    var tpl = get_precompiled_template("add_device_step2", null);

    if (tpl == null) {
        $.get("./tpl/add_device_step2.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("add_device_step2", ret);
            var source = tpl(add_device_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(add_device_model);
        $("#main_content").html(source);
    }
}

function match_sensor(fullmodelno) {
    console.log(fullmodelno);

    // 判断是否已经添加到 24 个设备
    console.log("sender_list.length = " + sender_list.length);
    console.log("receiver_list.length = " + receiver_list.length);

    if (sender_list.length + receiver_list.length >= 24){
        toastr['remove']();toastr['error'](i18n('warn max device'));
        return;
    }

    show_add_device_step3_sensor_matching(fullmodelno);
}

function show_add_device_step3_sensor_matching(fullmodelno) {

    add_device_model.fullmodelno = fullmodelno;

    var tpl = get_precompiled_template("add_device_step3_sensor_matching", null);

    if (tpl == null) {
        $.get("./tpl/add_device_step3_sensor_matching.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("add_device_step3_sensor_matching", ret);
            var source = tpl(add_device_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(add_device_model);
        $("#main_content").html(source);
    }
}


//Camera部分
var ipc_search_camera_list_html = [
    {
        "did": "CHXX-000338-TBZRJ"
    },
    {
        "did": "CHXX-000007-TEST"
    }
];

function search_ipc_wizard(){
    console.log("search_ipc_wizard");
    var reply = window.native.searchCameraWizard();
    if (null != reply){
        console.log(reply);
    }
}

function search_ipc_lan(){
    console.log("search_ipc_lan");
    if (typeof window.echo === "undefined") {
        add_device_model.ipc_search_list = ipc_search_camera_list_html;

        show_ipc_search_result_dialog();
    }
    else{
        var reply = window.native.searchCameraLan();
        if (null != reply){
            console.log(reply);
        }
    }
}

function setting_ipc_adv(){
    console.log("setting_ipc_adv");

    if (typeof window.echo === "undefined") {
        toastr['remove']();toastr['error'](i18n('warn wrong operate in broswer'));
    }
    else{
        var param = {};
        param.did = $("#camera-did").val();

        window.echo("enter_advanced_setting", JSON.stringify(param),
            function (r) {
                console.log("enter_advanced_setting reply = " + r);
        });
    }
}

function native_search_camera_wizard_result(key, result){
    console.log("native_search_camera_wizard_result: " + key + " , " + result);
    if ("CameraSearchWizard" == key){
        add_device_model.ipc_search_list = JSON.parse(result);
        
        if (null != add_device_model.ipc_search_list && add_device_model.ipc_search_list.length > 0){
            // if only on IPC searched, auto select it, or show select dialog
            if (1 == add_device_model.ipc_search_list.length){
                var did = add_device_model.ipc_search_list[0].did;
                console.log("did = " + did);
                $("#camera-did").val(did);
            }else{
                show_ipc_search_result_dialog();
            }
        }
    }
}

function native_search_camera_lan_result(key, result){
    console.log("native_search_camera_lan_result: " + key + " , " + result);
    if ("CameraSearchLan" == key){
        add_device_model.ipc_search_list = JSON.parse(result);
        if (null != add_device_model.ipc_search_list && add_device_model.ipc_search_list.length > 0){
            // if only on IPC searched, auto select it, or show select dialog
            if (1 == add_device_model.ipc_search_list.length){
                var did = add_device_model.ipc_search_list[0].did;
                console.log("did = " + did);
                $("#camera-did").val(did);
            }else{
                show_ipc_search_result_dialog();
            }
        }
    }
}

function native_search_camera_wizard_finish_result(key, result){
    console.log("native_search_camera_wizard_finish_result: " + key + " , " + result);
    if ("CameraSearchLan" == key){
        var cameraObj = JSON.parse(result);
        var name = cameraObj.cameraName;
        var did = cameraObj.cameraDID;
        var pw = cameraObj.cameraPassword;
        $("#camera-name").val(name);
        $("#camera-did").val(did);
        $("#camera-password").val(pw);
    }
}



function native_ipc_security_code(key, result){
    console.log("native_ipc_security_code: " + key + " , " + result);
    if ("CameraSecurityCode" == key){
        var pwJson = JSON.parse(result);
        var pw = pwJson.password;
        $("#camera-password").val(pw);
    }
}

function save_ipc(){
    var name = $("#camera-name").val();
    if (name.trim() == ""){
//        alert(i18n('please input name'));
        toastr['remove']();toastr['error'](i18n('warn input name'));
        return;
    }

    var did = $("#camera-did").val();
    if (did.trim() == ""){
//        alert(i18n('please input did'));
        toastr['remove']();toastr['error'](i18n('warn input did'));
        return;
    }

    var passwd = $("#camera-password").val();
    if (passwd.trim() == ""){
//        alert(i18n('please input password'));
        toastr['remove']();toastr['error'](i18n('warn input password'));
        return;
    }

    var location = $("#camera-location").val();
    if (location.trim() == ""){
//        alert(i18n('please input location'));
        toastr['remove']();toastr['error'](i18n('warn input location'));
        return;
    }


    console.log("save_ipc: " + name + "," + did + "," + passwd + "," + location);

    var actionid = new Date().getTime();
    var what = {};
    what.name = name;
    what.did = did;
    what.password = passwd;
    what.seat = location;

    what.busname = "p2p";
    what.fullmodelno = "JSW-Camera-0001";
    what.bind = what.busname + "=" + what.did;
    what.tosaveintodb = true;
    what.parent = "2";
    what.group = "";

    commit_command(actionid, "addItem", what, {"tosaveintodb": true}, function (ret) {
        console.log("add ipc ret : " + ret);
        status_model.data_update_needed = true;
        show_add_device_step2();
    });
}

function show_ipc_search_result_dialog() {
    console.log("show_ipc_search_result_dialog");

    var tpl = get_precompiled_template("add_device_step2_camera_search_result_dialog", null);

    if (tpl == null) {
        $.get("./tpl/add_device_step2_camera_search_result_dialog.tpl", function (ret) {
            ret = ret;
            var tpl = get_precompiled_template("add_device_step2_camera_search_result_dialog", ret);
            var source = tpl(add_device_model);
            $("#dialog").html(source);

            var h = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
            $("#dialog").height(h);
            $("#dialog").show();
        });
    } else {
        var source = tpl(add_device_model);
        $("#dialog").html(source);
        var h = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
        $("#dialog").height(h);
        $("#dialog").show();
    }
}

function add_device_camera_choose(did) {
    $("#camera-did").val(did);
    $("#dialog").hide();
}

function start_pair(fullmodelno) {
    console.log("start_pair " + fullmodelno);

    var model = models_dict[fullmodelno];

    var actionid = new Date().getTime();
    commit_command(actionid, "controlItem",
        {'action': 'match', 'busname': "ttyS0", 'actionid': actionid.toString(),
            'result_varname': 'result:match:' + actionid, "mode": model.mode,
            "nodeid": "0", "name": model.name, "fullmodelno": model.fullmodelno},
        {}, function () {
        });

    // 禁止 match icon
    $("#add_device_pair").css("display", "none");

    // 改变title
    document.getElementById("add_sensor_title").innerText = i18n("step 2 of 3");

    //获取配对结果
    add_device_model.resultkey = 'result:match:' + actionid;
    add_device_model.user = {};
    add_device_model.user.timer_count = 30;
    add_device_model.timer_getKeyValue = setTimeout(getKeyValue, 1000);
    add_device_model.pair_disable = "disabled";
}

function getKeyValue() {
    add_device_model.user.timer_count = add_device_model.user.timer_count - 1;

    var el = document.getElementById("timeout_text");
    if (el == null){
        console.log("user break matching");
        return;
    }

    el.innerText = i18n("matching wait") + add_device_model.user.timer_count + i18n("second");

    if (add_device_model.user.timer_count == 1) {
        add_device_model.paired = false;
        var el = document.getElementById("timeout_text");

        //disable Pair Button
        $("#add_device_pair").css("display", "block");

        document.getElementById("timeout_text").innerText = i18n("matching failed");

        return;
    }

    var command = {};
    command.method = "getValueByKey";
    command.param = {};
    command.param.key = add_device_model.resultkey;

    var data = JSON.stringify(command);
    console.info(data);

    if (typeof window.echo === "undefined") {
        var p2 = $.post('http://10.0.0.118:8888/command', {"command": data}, function (reply) {
            reply = JSON.parse(reply);
            console.log(add_device_model.user.timer_count + " ==== webui get reply " + JSON.stringify(reply));

            if (typeof(reply.error) != "undefined") {
                //get repeatly
                setTimeout(getKeyValue, 1000);
                return;
            }

            reply = reply.result.result;
            add_device_model.matchreply = reply;

            if (typeof(reply) != "undefined") { //说明获取到反馈了，进行判断
                if (reply === "failed") {
//                    alert("Match Failed.");
                    toastr['remove']();toastr['error'](i18n('warn match failed'));
                    return;
                } else {
//                    alert("Match Succeed.");
                }

                show_add_device_step4_sensor_save(reply);
            }
        });

    } else {
        window.echo("command", JSON.stringify(command), function (reply) {
            clearInterval(add_device_model.timer_getKeyValue);
            console.log("==== native get reply " + JSON.stringify(reply));

            if (typeof(reply.error) != "undefined") {
                //get repeatly
                setTimeout(getKeyValue, 1000);
                return;
            }

            reply = reply.result.result;
            add_device_model.matchreply = reply;

            if (typeof(reply) != "undefined") { //说明获取到反馈了，进行判断
                if (reply === "failed") {
//                    alert("Match Failed.");
                    toastr['remove']();toastr['error'](i18n('warn match failed'));
                    return;
                }

                show_add_device_step4_sensor_save(reply);
            }
            //没有反馈的时候，将会继续在下一秒钟检查返回值，很SB的方式。。。
        });
    }
}


function show_add_device_step4_sensor_save(data) {

    var tpl = get_precompiled_template("add_device_step4_sensor_save", null);

    if (tpl == null) {
        $.get("./tpl/add_device_step4_sensor_save.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("add_device_step4_sensor_save", ret);
            var source = tpl(add_device_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(add_device_model);
        $("#main_content").html(source);
    }
}

function save_item() {
    var actionid = new Date().getTime();

    var what = {};

    what.name = $("#sensor-name").val();
    if (what.name.trim() == ""){
//        alert(i18n("please input name"));
        toastr['remove']();toastr['error'](i18n('warn input name'));
        return;
    }

    what.seat = $("#sensor-seat").val();
    if (what.seat.trim() == ""){
//        alert(i18n("please input location"));
        toastr['remove']();toastr['error'](i18n('warn input location'));
        return;
    }

    console.log("save_item: " + what.name + ", " + what.seat);

    what.busname = add_device_model.matchreply.busname;
    what.fullmodelno = add_device_model.matchreply.fullmodelno;
    what.bind = add_device_model.matchreply.bind;
    what.nodeid = add_device_model.matchreply.nodeid;
    what.tosaveintodb = true;
    what.parent = "2";

    rf433_nodeid_array[what.nodeid] = true;

    commit_command(actionid, "addItem", what, {"tosaveintodb": true}, function (ret) {
        console.log("addItem ret = " + ret);

        status_model.data_update_needed = true;
        show_add_device_step2();
    });
}

function i18n(key) {
    if (typeof i18n_data[key] == "undefined") {
        return key;
    } else {
        return i18n_data[key];
    }
}