/**
 * Created by yulin on 14-7-30.
 */

var MySandbox = null;
var core = null;
var sm = null;

var i18n_data = null;

//预编译模板的缓存
var templates = {};

var footer_html = null;

// 操作等待时间计数
var operate_permmit = true;
function ifCanOperate(){
    if (false == operate_permmit){
        toastr['remove']();toastr['error'](i18n('waiting'));
        return false;
    }

    return true;
}

function forbidOperate(){
    operate_permmit = false;
    setTimeout(function(){
        operate_permmit = true;
    }, 1500);
}

function ceres_init() {
    core = new scaleApp.Core(null);
    scaleApp.plugins.state(core);

    i18n_data = myLocalization["en"];

    $.get("tpl/footer.tpl", function (html) {
        footer_html = html;
    });

    statemachine_init();

    initListener();

    initWatcher();
}

function initListener() {
    core.on("somethingHappened", messageHandler);
}

var messageHandler = function (data, topic) {
    switch (topic) {
        case "somethingHappened":
            console.log("somethingHappened");
            break;
        case "aNiceTopic":
//            justProcess( data );
            break;
    }
};

function initWatcher() {
    watch(add_device_model, "camera_list_changed", function () {
        console.log("add_device_model.camera_list_changed changed!");
    });
}

function isInSimpleMode() {
    if (typeof setting_model.simple_mode === "undefined") {
        return false;
    } else {
        return setting_model.simple_mode;
    }
}

function footer_goto_1_status_page() {
    if (false == isInSimpleMode()) {
        sm_force_to('status_page');
    } else if (sm.current != "remote_controller_page") {
        sm_force_to('remote_controller_page');
    }
}

function footer_goto_2_control_page() {
    if (false == isInSimpleMode()) {
        sm_force_to('control_page_starter');
    } else if (sm.current != "remote_controller_page") {
        sm_force_to('remote_controller_page');
    }
}

function footer_goto_3_scenario_page() {
    if (false == isInSimpleMode()) {
        sm_force_to('scenario_page_starter');
    } else if (sm.current != "remote_controller_page") {
        sm_force_to('remote_controller_page');
    }
}


function footer_goto_4_event_page() {
    if (false == isInSimpleMode()) {
        sm_force_to('event_page');
    } else if (sm.current != "remote_controller_page") {
        sm_force_to('remote_controller_page');
    }
}

function footer_goto_5_setting_page() {
    sm_force_to('setting_page_starter');
}

// 这个函数执行后，确保 setting_model.simple_mode 和 setting_model.simple_mode_did 一定有效
function checkSimpleModeState() {
    console.log("checkSimpleModeState");
    if (typeof window.echo === "undefined") {
        if (typeof setting_model.simple_mode === "undefined") {
            setting_model.simple_mode = false;
            setting_model.simple_mode_did = "empty";
        }
    } else {
        console.log("read from native");
        var reply = window.native.getSimpleMode();
        console.log("simple mode reply = " + reply);
        if ("empty" == reply) {
            setting_model.simple_mode = false;
            setting_model.simple_mode_did = "empty";
        } else {
            setting_model.simple_mode = true;
            setting_model.simple_mode_did = reply;
        }
    }

    console.log("setting_model.simple_mode = " + setting_model.simple_mode);
    console.log("setting_model.simple_mode_did = " + setting_model.simple_mode_did);
}

$(document).ready(function () {
    ceres_init();
    console.log("current = " + current);

    checkSimpleModeState();

    toastr.options = {
        closeButton: $('#closeButton').prop('checked'),
        debug: $('#debugInfo').prop('checked'),
        positionClass: $('#positionGroup input:radio:checked').val() || 'toast-top-full-width',
        onclick: null
    };

    // 首先从 native 读取 Simple Mode，并保存在 setting_model 中
    if (current == "gateway_wizard") {
        sm.fire("to_gateway_wizard_step1");
    } else if (current == "starter") {
        sm.fire("to_status_page");
    } else if (current == "add_device") {
        sm.fire("to_add_device_starter");
    } else if (current == "gateway_list_page") {
        sm.fire("to_gateway_list_page");
    } else if (current == "remote_controller_page") {
        sm.fire("to_remote_controller_page");
    } else if (current == "control_page_starter") {
        sm.fire("to_control_page_starter");
    }
});


