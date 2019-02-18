/**
 * Created by yulin on 14-7-31.
 */


function remote_controller_page_sm_init() {
    sm.onEnter("remote_controller_page", function (transition, eventName, next) {
        next();
        show_remote_controller_page();
        getRemotekeyValue();
    });
    sm.onEnter("remote_controller_page_receiver", function (transition, eventName, next) {
        next();
        show_remote_controller_page_receiver();
    });
}

var remote_controller_page_model = {};

function show_remote_controller_page() {
    // console.log("show_remote_controller_page. id = " + remote_controller_page_model.itemid);
    // remote_controller_page_model.item = items_dict[remote_controller_page_model.itemid];

    if (setting_model.simple_mode == true) {
        remote_controller_page_model.title = 'Simple Mode';
        remote_controller_page_model.simple_mode = true;
    } else {
        remote_controller_page_model.title = 'Remote';
        remote_controller_page_model.simple_mode = false;
    }

    var tpl = get_precompiled_template("remote_controller_page", null);
    if (tpl == null) {
        $.get("./tpl/remote_controller_page.tpl", function (ret) {

            if (true == setting_model.simple_mode) {
                ret = ret + footer_html;
            }

            var tpl = get_precompiled_template("remote_controller_page", ret);
            var source = tpl(remote_controller_page_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(remote_controller_page_model);
        $("#main_content").html(source);
    }

}

function getRemotekeyValue() {
    console.log("getRemotekeyValue");
    var actionid = new Date().getTime();

    commit_command(actionid, "getRemotekeyValue", {}, {}, function (ret) {
        console.log("getRemotekeyValue ret = " + ret);
        try {
            var result = null;
            if (typeof ret == "string") {
                result = JSON.parse(ret).result;
            } else {
                result = ret.result;
            }

            console.log("remotekey = " + result.remotekey);
            if ("Lock" == result.remotekey) {
                $("#remote_lock_icon").attr("src", "images/Arm_S.png");
                $("#remote_unlock_icon").attr("src", "images/Disarm_N.png");
            } else {
                $("#remote_lock_icon").attr("src", "images/Arm_N.png");
                $("#remote_unlock_icon").attr("src", "images/Disarm_S.png");
            }
        } catch (ex) {
            console.log("warn read data error");
            toastr['remove']();
            toastr['error'](i18n('warn read data error'));
            logout_gateway();
        }
    });
}

function remotekey_control(value) {
    var what = {};

    what.action = "RemoteKey";
    what.value = value;

    var actionid = new Date().getTime();
    commit_command(actionid, "remotekeyControl", what, {"tosaveintodb": true}, function (ret) {
        console.log("remotekeyControl ret = " + ret);
        forbidOperate();
    });
}

function remotekey_lock() {
    if (false == ifCanOperate()) {
        return;
    }

    remotekey_control("Lock");
    $("#remote_lock_icon").attr("src", "images/Arm_S.png");
    $("#remote_unlock_icon").attr("src", "images/Disarm_N.png");
    if (typeof control_page_model.arm_group != 'undefined') {
        control_page_model.arm_group.remotekey = 'Lock';
    }
}

function remotekey_unlock() {
    if (false == ifCanOperate()) {
        return;
    }

    remotekey_control("Unlock");
    $("#remote_lock_icon").attr("src", "images/Arm_N.png");
    $("#remote_unlock_icon").attr("src", "images/Disarm_S.png");
    if (typeof control_page_model.arm_group != 'undefined') {
        control_page_model.arm_group.remotekey = 'Unlock';
    }
}

function remotekey_ipc_recording() {
    if (false == ifCanOperate()) {
        return;
    }

    remotekey_control("Recording");
    $("#remote_record_icon").attr("src", "images/CameraRecording_S.png");
    setTimeout(function () {
        $("#remote_record_icon").attr("src", "images/CameraRecording_N.png");
    }, 500);
}

function remotekey_siren() {
    if (false == ifCanOperate()) {
        return;
    }

    remotekey_control("Siren");
    $("#remote_siren_icon").attr("src", "images/Panic_S.png");
    setTimeout(function () {
        $("#remote_siren_icon").attr("src", "images/Panic_N.png");
    }, 500);
}

var panic_timer = null;
function remotekey_siren_down() {
    console.log("remotekey_siren_down");
    var i = 0;
    if (null == panic_timer) {
        panic_timer = setInterval(function () {
            i += 1;
            if (i == 3) {
                console.log("3 secondes pass");
                remotekey_siren();
                clearTimeout(panic_timer);
                panic_timer = null;
            } else {
                console.log("i = " + i);
            }
        }, 1000);
    }
}

function remotekey_siren_up() {
    console.log("remotekey_siren_up");
    clearTimeout(panic_timer);
    panic_timer = null;
}
