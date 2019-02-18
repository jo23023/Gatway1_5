/**
 * Created by yulin on 14-8-4.
 */

function gateway_list_page_sm_init() {
    console.log("gateway_list_page_sm_init");
    sm.onEnter("gateway_list_page", function (transition, eventName, next) {
        next();

        show_gateway_list_page();
    });
}

var gateway_list_page_model = {};
var gateway_list_html = [
    {
        "did": "CHXX-000405-WCLGJ",
        "name": "gateway405",
        "password": "123456"
    },
    {
        "did": "CHXX-000402-ABCDEF",
        "name": "gw002",
        "password": "123456"
    }
];

function show_gateway_list_page() {
    console.log("show_gateway_list_page");

    // get gateway list from native
    var gotoGatewayWiard = true;

    if (typeof window.echo === "undefined") {
        console.log("run in html");
        var gotoGatewayWiard = false;
        gateway_list_page_model.gateway_list = gateway_list_html;
    }
    else {
        console.log("run in android");
        var native_gw_list = window.native.getGatewayList();
        if (null != native_gw_list) {
            console.log(native_gw_list);
            gateway_list_page_model.gateway_list = JSON.parse(native_gw_list);
            if (null != gateway_list_page_model.gateway_list && gateway_list_page_model.gateway_list.length > 0) {
                gotoGatewayWiard = false;
            }
        }
    }

    if (gotoGatewayWiard == true) {
        sm.fire("to_gateway_wizard_step1");
    }
    else {
        var tpl = get_precompiled_template("gateway_list_page", null);
        if (tpl == null) {
            $.get("./tpl/gateway_list_page.tpl", function (ret) {
                var tpl = get_precompiled_template("gateway_list_page", ret);
                var source = tpl(gateway_list_page_model);
                $("#main_content").html(source);
                if (typeof window.echo != "undefined") {
                    var show_reset_btn = window.native.showResetButton();
                    if (show_reset_btn == true) {
                        $('#reset_btn').show();
                    } else {
                        $('#reset_btn').hide();
                    }
                }
            });
        } else {
            var source = tpl(gateway_list_page_model);
            $("#main_content").html(source);
            if (typeof window.echo != "undefined") {
                var show_reset_btn = window.native.showResetButton();
                if (show_reset_btn == true) {
                    $('#reset_btn').show();
                } else {
                    $('#reset_btn').hide();
                }
            }
        }


//        checkSimpleModeState();
        console.log("read from native");
        var reply = window.native.getSimpleMode();
        console.log("simple mode reply = " + reply);

        // 检查是否启用了 Simple Mode
        if (true == setting_model.simple_mode) {
            var conn_did = null;
            var conn_passwd = null;

            if (typeof window.echo === "undefined") {
                conn_did = gateway_list_page_model.gateway_list[0].did;
                conn_passwd = gateway_list_page_model.gateway_list[0].password;
            } else {
                // gateway list中，是否存在
                for (var i = 0; i < gateway_list_page_model.gateway_list.length; ++i) {
                    console.log(i + " = " + gateway_list_page_model.gateway_list[i]["did"]);
                    if (setting_model.simple_mode_did == gateway_list_page_model.gateway_list[i]["did"]) {
                        conn_did = gateway_list_page_model.gateway_list[i]["did"];
                        conn_passwd = gateway_list_page_model.gateway_list[i].password;
                        break;
                    }
                }
            }

            console.log("conn_did = " + conn_did + ", conn_passwd = " + conn_passwd);
            if (conn_did != null && conn_passwd != null) {
                // auto connect
                connect_to_gateway(conn_did, conn_passwd);
            }
        }
    }
}

function show_gateway_configure() {
    console.log("show_gateway_configure");

    var tpl = get_precompiled_template("gateway_configure", null);
    if (tpl == null) {
        $.get("./tpl/gateway_configure.tpl", function (ret) {
            var tpl = get_precompiled_template("gateway_configure", ret);
            var source = tpl(gateway_list_page_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(gateway_list_page_model);
        $("#main_content").html(source);
    }
}

function gateway_list_page_enter_edit_mode() {
    gateway_list_page_model.in_edit = !gateway_list_page_model.in_edit;
    if (gateway_list_page_model.in_edit == true) {
        $(".not-edit-button").hide();
        $(".edit-button").show();
    } else {
        $(".not-edit-button").show();
        $(".edit-button").hide();
    }
}

function connect_to_gateway(did, password) {
    console.log("connect_to_gateway. did = " + did);
    gateway_list_page_model.connected_did = did;

    if (typeof window.echo === "undefined") {
        // 检查是否启用了 Simple Mode 模式
        if (true == setting_model.simple_mode) {
            sm.fire("to_remote_controller_page");
        } else {
            sm.fire("to_status_page");
        }
    }
    else {
        var param = {};
        param.did = did;
        param.password = password;

        window.echo("connect_gateway", JSON.stringify(param),
            function (r) {
            });
    }
}

function configure_gateway(name, did, password) {
    var gateway_configure = {};

    gateway_configure.name = name;
    gateway_configure.did = did;
    gateway_configure.oldDid = did;
    gateway_configure.password = password;
    gateway_list_page_model.gateway_configure = gateway_configure;

    show_gateway_configure();
}

function save_configure_gateway() {
    var name = $("#gw_name").val();
    if (name.trim() == "") {
//        alert("Please input name");
        toastr['remove']();
        toastr['error'](i18n('warn input name'));
        return;
    }

    var did = $("#gw_did").val();
    if (did.trim() == "") {
//        alert("Please input did");
        toastr['remove']();
        toastr['error'](i18n('warn input did'));
        return;
    }

    var pw = $("#gw_password").val();
    if (pw.trim() == "") {
//        alert("Please input password");
        toastr['remove']();
        toastr['error'](i18n('warn input password'));
        return;
    }

    if (typeof window.echo === "undefined") {
//        alert("ignore edit gateway in html");
        toastr['remove']();
        toastr['error'](i18n('warn ignore edit geteway in broswer'));
    } else {
        var add_gw_rsp = window.native.modifyGateway(
            gateway_list_page_model.gateway_configure.oldDid,
            name, did, pw);
        console.log("add_gw_rsp = " + add_gw_rsp);
    }

    show_gateway_list_page();
}

function remove_gateway(did, password) {
    if (typeof window.echo === "undefined") {
//        alert("ignore remove gateway in html");
        toastr['remove']();
        toastr['error'](i18n('warn ignore remove gateway in broswer'));
    } else {
        var remove_gw_rsp = window.native.removeGateway(did);
        console.log("remove_gw_rsp = " + remove_gw_rsp);
    }

    show_gateway_list_page();
}

// gateway 连接结果
function native_p2p_connect_result(key, result) {
    console.log("native_p2p_connect_result: " + key + " , " + result);
    if ("Connecting" == key) {
        // toastr['remove']();toastr['error'](i18n('connecting to gateway'));
    } else if ("ConnectFailed" == key) {
        toastr['remove']();
        toastr['error'](i18n('connect failed to gateway'));
    } else if ("AuthOK" == key) {
        // 如果当前处于 添加摄像头 状态,保持不变； 否则进入 status_page
        console.log("sm.current = " + sm.current);
        if (sm.current == "add_device_step1")
            console.log("Reconnect ok when add IP Camera");
        else
            sm.fire("to_status_page");
    } else if ("AuthNotOK" == key) {
        toastr['remove']();
        toastr['error'](i18n('warn wrong password'));
    }
}

function native_p2p_gateway_broken(key, result) {
    console.log("native_p2p_gateway_broken. key = " + key);
    toastr['remove']();
    toastr['error'](i18n('connect failed to gateway'));
}

function native_show_gateway_list_page(key, result) {
    sm.fire("to_gateway_list_page");
}

// 给每一个gateway发送reset复位指令时,执行结果. key = did, result = "OK"或者"Fail"jdTf1Df35dOz3g2q63D
function native_gateway_reset_result(key, result) {
    console.log("native_gateway_reset_result. did = " + key + ", result = " + result);
    if(result == 'OK'){
        $('#reset_'+key+' div').css('visibility','visible');
    }else if(result == "Fail"){
        $('#reset_'+key+' div').html('FAILED');
        $('#reset_'+key+' div').attr('style','visibility: visible;height: 4em;line-height: 4em;overflow: hidden;color: red;');
//        $('#reset_'+key).css('visibility','visible');
    }else{
        $('#reset_'+key+' div').css('visibility','hidden');
    }
}

function reset_gateway() {
    if (typeof window.echo != "undefined") {
        window.native.resetAllGateway();
    }
}