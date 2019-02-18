/**
 * Created by yulin on 14-7-31.
 */


function gateway_wizard_sm_init() {
    sm.onEnter("gateway_wizard_step1", function (transition, eventName, next) {
        next();
        show_gateway_wizard_step1();
    });

    sm.onEnter("gateway_wizard_step2", function (transition, eventName, next) {
        next();
        show_gateway_wizard_step2();
    });
}

var gateway_wizard_model = {};
gateway_wizard_model.step = 0;
gateway_wizard_model.scan_result = null;

function show_gateway_wizard_step1() {
    console.log("show_gateway_wizard_step1");
    gateway_wizard_model.step = 1;
    var tpl = get_precompiled_template("gateway_wizard_step1", null);
    if (tpl == null) {
        $.get("./tpl/gateway_wizard_step1.tpl", function (ret) {
            var tpl = get_precompiled_template("gateway_wizard_step1", ret);
            var source = tpl(gateway_wizard_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(gateway_wizard_model);
        $("#main_content").html(source);
    }
}

function show_gateway_wizard_step2() {
    gateway_wizard_model.step = 2;
    var tpl = get_precompiled_template("gateway_wizard_step2_scan", null);
    if (tpl == null) {
        $.get("./tpl/gateway_wizard_step2_scan.tpl", function (ret) {
            var tpl = get_precompiled_template("gateway_wizard_step2_scan", ret);
            var source = tpl(gateway_wizard_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(gateway_wizard_model);
        $("#main_content").html(source);
    }
}

function setSearchResultValue(){
    console.log("setSearchResultValue");
    if (typeof(gateway_wizard_model.scan_result) == undefined){
        return;
    }

    try{
        $("#did").val(gateway_wizard_model.scan_result.did);

        if ("123456" == gateway_wizard_model.scan_result.password){
            $("#password").val(gateway_wizard_model.scan_result.password);
            toastr['remove']();toastr['error'](i18n('save gateway prompt2'));
        }
    }catch (err){
        console.log("something wrong!");
    }
}

function show_gateway_wizard_step3_save() {
    gateway_wizard_model.step = 3;
    var tpl = get_precompiled_template("gateway_wizard_step3_save", null);
    if (tpl == null) {
        $.get("./tpl/gateway_wizard_step3_save.tpl", function (ret) {
            var tpl = get_precompiled_template("gateway_wizard_step3_save", ret);
            var source = tpl(gateway_wizard_model);
            $("#main_content").html(source);

            setSearchResultValue();
        });
    } else {
        var source = tpl(gateway_wizard_model);
        $("#main_content").html(source);

        setSearchResultValue();
    }
}

function show_gateway_scanning_page() {
    if (typeof window.echo === "undefined"){
        show_gateway_wizard_step2();
    }else {
        // ask native to scan gateway
        show_gateway_wizard_step2();

        var scan_gw_rsp = window.native.searchGateway();
        console.log("scan_gw_rsp = " + scan_gw_rsp);
    }
}

function exit_search_gateway(){
    sm_force_to('gateway_list_page');
}

function show_gateway_edit_page() {
    show_gateway_wizard_step3_save();
}


// "{\"did\":\"%s\",\"checkSum\":\"%s\",\"password\":\"%s\"}"
function native_gateway_info(key, result) {
    console.log("native_gateway_info: " + key + " , " + result);
    if ("GatewayInfo" == key && 2 == gateway_wizard_model.step){
        gateway_wizard_model.scan_result = JSON.parse(result);
        show_gateway_wizard_step3_save();
    }
}

function save_gateway(){
    console.log("save_gateway");
    var name = document.getElementById("name").value;
    if (name.trim() == ""){
//        alert("Please input name");
        toastr['remove']();
        toastr['error'](i18n('warn input name'));
        return;
    }

    var did = document.getElementById("did").value;
    if (did.trim() == ""){
//        alert("Please input did");
        toastr['remove']();toastr['error'](i18n('warn input did'));
        return;
    }

    var passwd = document.getElementById("password").value;
    if (passwd.trim() == ""){
//        alert("Please input passwd");
        toastr['remove']();toastr['error'](i18n('warn input password'));
        return;
    }

    if (typeof window.echo === "undefined"){
        console.log("can not save in browser");
    }else {
        var add_gw_rsp = window.native.addGateway(name, did, passwd);
        console.log("add_gw_rsp = " + add_gw_rsp);
    }

    sm.fire("to_gateway_list_page");
}