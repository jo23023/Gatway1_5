/**
 * Created by yulin on 14-8-1.
 */

function setting_page_sm_init() {
    sm.onEnter("setting_page_starter", function (transition, eventName, next) {
        next();
        sm.fire("to_setting_page_verify_pwd");
    });
    sm.onLeave("setting_page_starter", function (transition, eventName, next) {
        next();
    });

    sm.onEnter("setting_page_verify_pwd", function (transition, eventName, next) {
        next();
        show_setting_page_verify_pwd();
    });

}

var setting_model = {};
setting_model.has_check_datetime = false;

function show_setting_page_verify_pwd() {
    var tpl = get_precompiled_template("setting_page_verify_pwd", null);

    if (tpl == null) {
        $.get("./tpl/setting_page_verify_pwd.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_verify_pwd", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
    }
}

function show_setting_page_nav() {
    var pw = $("#setting_password").val();
    if (typeof pw != 'undefined' && pw.trim() == "") {
//        alert(i18n("please input password"));
        toastr['remove']();
        toastr['error'](i18n('warn input password'));
        return;
    }

    if (devicejsontree == undefined || devicejsontree == null) {
        // Simple Mode error -TODO
        console.log("TODO");
    } else {
        if (typeof pw != 'undefined' && pw.trim() != devicejsontree.gateway.admin_password) {
            toastr['remove']();
            toastr['error'](i18n('warn wrong password'));
            return;
        }
    }

    // 自动获取gateway的日期和时间，如果是2000年，自动同步当前的时间到gateway
    console.log("setting_model.has_check_datetime = " + setting_model.has_check_datetime);
    if (true == setting_model.has_check_datetime) {
        loadSettingPageNav();
    }
    else {
        console.log("check if date and time...");
        setting_model.has_check_datetime = true;
        showWaitingUI();

        var gw_date = null;
        var set_current_time = false;

        var actionid = new Date().getTime();
        commit_command(actionid, "getTime", {}, {}, function (response) {
            try {
                console.log("response ", response);
                if (typeof(response) == "string") {
                    response = JSON.parse(response);
                }

                if (typeof(response.result) == "undefined") {
                    set_current_time = true;
                } else {
                    console.log("correct response!");
                    gw_date = response.result.date;
                }

                // 判断时间是否正确
                try {
                    console.log("gw_date = " + gw_date);
                    if (null == gw_date || typeof(gw_date) == "undefined") {
                        set_current_time = true;
                    } else {
                        var year = new Date(gw_date).getYear();
                        console.log("year = " + year);
                        if (100 == year || 2000 == year) {
                            set_current_time = true;
                        }
                    }
                } catch (err) {
                    set_current_time = true;
                }

                console.log("set_current_time = " + set_current_time);
                if (true == set_current_time) {
                    autoUpdateLocalDatetime();
                } else {
                    loadSettingPageNav();
                }
            } catch (ex) {
                console.log("warn read data error");
                toastr['remove']();
                toastr['error'](i18n('warn read data error'));
                logout_gateway();
            }
        });
    }
}

function autoUpdateLocalDatetime() {
    console.log("autoUpdateLocalDatetime");
    var d1 = new Date();
    var localTime = d1.getTime();
    var localOffset = d1.getTimezoneOffset() * 60000;
    var utcTime = localTime + localOffset;
    var utcDate = new Date(utcTime);
    var utcDateFormat = utcDate.Format("MMddhhmmyyyy");
    console.log("utcDateFormat = " + utcDateFormat);

    var actionid = new Date().getTime();
    var what = {};
    what.machineformat = utcDateFormat;
    commit_command(actionid, "setTime", what, {}, function () {
        console.log("check date time ok");
        loadSettingPageNav();
    });
}

function loadSettingPageNav() {
    var tpl = get_precompiled_template("setting_page_nav", null);

    if (tpl == null) {
        $.get("./tpl/setting_page_nav.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_nav", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
    }
}

function show_setting_time_setup() {

    setting_model.timesetup = {};
    var now = new Date().Format("yyyy-MM-dd hh:mm");
    setting_model.timesetup.date = now;
    setting_model.useCurrentTime = false;

    showWaitingUI();
    var actionid = new Date().getTime();
    commit_command(actionid, "getTime", {}, {}, function (response) {
        try {
            console.log("send getTime command ", response);
            if (typeof(response.result) == "undefined") {

            } else {
                console.log("correct response!");
                setting_model.timesetup = response.result;
            }

            if (typeof(setting_model.timesetup.date) == "undefined") {
                var now = new Date().Format("yyyy-MM-dd hh:mm");
                setting_model.timesetup.date = now;
            }

            if (typeof(setting_model.timesetup.timezone) == "undefined") {
                setting_model.timesetup.timezone = "GMT_000";
            }

            if (typeof(setting_model.NTPServer) == "undefined") {
                setting_model.NTPServer = "false";
            }

//        setting_model.timesetup.date = setting_model.timesetup.date.toString().split(" ")[0];
//        setting_model.timesetup.time = setting_model.timesetup.date.toString().split(" ")[1];

            setting_model.timezones = [
                {value: "UCT_-11", name: "(GMT-11:00) Midway Island, Samoa"},
                {value: "UCT_-10", name: "(GMT-10:00) Hawaii"},
                {value: "NAS_-09", name: "(GMT-09:00) Alaska"},
                {value: "PST_-08", name: "(GMT-08:00) Pacific Time"},
                {value: "MST_-07", name: "(GMT-07:00) Mountain Time"},
                {value: "MST_-07", name: "(GMT-07:00) Arizona"},
                {value: "CST_-06", name: "(GMT-06:00) Central Time"},
                {value: "UCT_-06", name: "(GMT-06:00) Middle America"},
                {value: "UCT_-05", name: "(GMT-05:00) Indiana East, Colombia"},
                {value: "EST_-05", name: "(GMT-05:00) Eastern Time"},
                {value: "AST_-04", name: "(GMT-04:00) Atlantic Time, Brazil West"},
                {value: "UCT_-04", name: "(GMT-04:00) Bolivia, Venezuela"},
                {value: "UCT_-03", name: "(GMT-03:00) Guyana"},
                {value: "EBS_-03", name: "(GMT-03:00) Brazil East, Greenland"},
                {value: "NOR_-02", name: "(GMT-02:00) Mid-Atlantic"},
                {value: "EUT_-01", name: "(GMT-01:00) Azores Islands"},
                {value: "UCT_000", name: "(GMT) Gambia, Liberia, Morocco"},
                {value: "GMT_000", name: "(GMT) England"},
                {value: "MET_001", name: "(GMT+01:00) Czech Republic, N"},
                {value: "MEZ_001", name: "(GMT+01:00) Germany"},
                {value: "UCT_001", name: "(GMT+01:00) Tunisia"},
                {value: "EET_002", name: "(GMT+02:00) Greece, Ukraine, Turkey"},
                {value: "SAS_002", name: "(GMT+02:00) South Africa"},
                {value: "IST_003", name: "(GMT+03:00) Iraq, Jordan, Kuwait"},
                {value: "MSK_003", name: "(GMT+03:00) Moscow Winter Time"},
                {value: "UCT_004", name: "(GMT+04:00) Armenia"},
                {value: "UCT_005", name: "(GMT+05:00) Pakistan, Russia"},
                {value: "UCT_006", name: "(GMT+06:00) Bangladesh, Russia"},
                {value: "UCT_007", name: "(GMT+07:00) Thailand, Russia"},
                {value: "CST_008", name: "(GMT+08:00) China Coast, Hong Kong"},
                {value: "CCT_008", name: "(GMT+08:00) Taipei"},
                {value: "SST_008", name: "(GMT+08:00) Singapore"},
                {value: "AWS_008", name: "(GMT+08:00) Australia (WA)"},
                {value: "JST_009", name: "(GMT+09:00) Japan, Korea"},
                {value: "KST_009", name: "(GMT+09:00) Korean"},
                {value: "UCT_010", name: "(GMT+10:00) Guam, Russia"},
                {value: "AES_010", name: "(GMT+10:00) Australia (QLD, TAS,NSW,ACT,VIC)"},
                {value: "UCT_011", name: "(GMT+11:00) Solomon Islands"},
                {value: "UCT_012", name: "(GMT+12:00) Fiji"},
                {value: "NZS_012", name: "(GMT+12:00) New Zealand"}
            ];

            var tpl = get_precompiled_template("setting_page_time_setup", null);

            if (tpl == null) {
                $.get("./tpl/setting_page_time_setup.tpl", function (ret) {
                    console.log(setting_model);
                    ret = ret + footer_html;
                    var tpl = get_precompiled_template("setting_page_time_setup", ret);
                    var source = tpl(setting_model);
                    $("#main_content").html(source);
//                new datepickr('gateway-date', {'dateFormat': 'Y-m-d'});
//                $('#gateway-time').ptTimeSelect();
//                $('#gateway-date').datetimepicker({
//                    dateFormat: 'yy-mm-dd'
//                });

                    var opt = {

                    }

                    opt.date = {preset: 'date'};
                    opt.datetime = { preset: 'datetime', minDate: new Date(2000, 1, 1, 9, 22), maxDate: new Date(2037, 12, 31, 15, 44), stepMinute: 1  };
                    opt.time = {preset: 'time'};
                    opt.tree_list = {preset: 'list', labels: ['Region', 'Country', 'City']};
                    opt.image_text = {preset: 'list', labels: ['Cars']};
                    opt.select = {preset: 'select'};
                    $('#gateway-date').val(setting_model.timesetup.date).scroller('destroy').scroller($.extend(opt['datetime'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
                });
            } else {
                var source = tpl(setting_model);
                $("#main_content").html(source);
//            new datepickr('gateway-date', {'dateFormat': 'Y-m-d'});
//            $('#gateway-time').ptTimeSelect();
//            $('#gateway-date').datetimepicker({
//                dateFormat: 'yy-mm-dd'
//            });
                var opt = {

                }

                opt.date = {preset: 'date'};
                opt.datetime = { preset: 'datetime', minDate: new Date(2000, 1, 1, 9, 22), maxDate: new Date(2037, 12, 31, 15, 44), stepMinute: 1  };
                opt.time = {preset: 'time'};
                opt.tree_list = {preset: 'list', labels: ['Region', 'Country', 'City']};
                opt.image_text = {preset: 'list', labels: ['Cars']};
                opt.select = {preset: 'select'};
                $('#gateway-date').val(setting_model.timesetup.date).scroller('destroy').scroller($.extend(opt['datetime'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
            }
        } catch (ex) {
            console.log("warn read data error");
            toastr['remove']();
            toastr['error'](i18n('warn read data error'));
            logout_gateway();
        }

    });

}

function use_current_time() {
    if (setting_model.useCurrentTime == false) {
        var now = new Date().Format("yyyy-MM-dd hh:mm");
        setting_model.timesetup.date = now;
//        var date_array = now.toString().split(" ");
        $("#gateway-date").val(now);
//        $("#gateway-time").val(date_array[1]);
        setting_model.useCurrentTime = true;
        $('#current_time').attr('src', 'images/Confirm_S.png');
    } else {
        setting_model.useCurrentTime = false;
        $('#current_time').attr('src', 'images/Confirm_N.png');
    }
}

function NTPServer() {
    if (setting_model.NTPServer == true) {
        setting_model.NTPServer = false;
        $('#ntp').attr('src', 'images/Confirm_N.png');
    } else {
        setting_model.NTPServer = true;
        $('#ntp').attr('src', 'images/Confirm_S.png');
    }
}

function save_time() {
//    what = setting_model.timesetup;

    what = {};
//    gateway_date = $("#gateway-date").val();
//    gateway_time = $("#gateway-time").val();
    what.date = $("#gateway-date").val();
//    console.log(new Date(what.date));return;
    what.date = new Date(Date.parse(what.date.replace(/-/g, "/"))).Format("yyyy-MM-dd hh:mm");
    what.machineformat = new Date(Date.parse(what.date.replace(/-/g, "/"))).Format("MMddhhmmyyyy");
    what.timezone = $('#selected_timezone option:selected').val();
    what.ntpserver = setting_model.NTPServer;
    var actionid = new Date().getTime();
    commit_command(actionid, "setTime", what, {}, function () {
        console.log("send setTime command");
        show_setting_page_nav();
    });
}

function show_setting_ip_setup() {

    setting_model.ipsetup = {};
    setting_model.ipsetup.dhcp_enable = true;

    showWaitingUI();
    var actionid = new Date().getTime();
    commit_command(actionid, "getIpValue", {}, {}, function (response) {
        try {
            setting_model.ipsetup = Object.extend(setting_model.ipsetup, response.result);
            console.log("ipsetup-> ", setting_model.ipsetup);
            var tpl = get_precompiled_template("setting_page_ip_setup", null);

            if (tpl == null) {
                $.get("./tpl/setting_page_ip_setup.tpl", function (ret) {
                    ret = ret + footer_html;
                    var tpl = get_precompiled_template("setting_page_ip_setup", ret);
                    var source = tpl(setting_model);
                    $("#main_content").html(source);
                });
            } else {
                var source = tpl(setting_model);
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

function change_dhcp() {
    if (setting_model.ipsetup.dhcp_enable == true) {
        setting_model.ipsetup.dhcp_enable = false;
        $('#dhcp').attr('src', 'images/Confirm_N.png');
    } else {
        setting_model.ipsetup.dhcp_enable = true;
        $('#dhcp').attr('src', 'images/Confirm_S.png');
    }
}

function save_ip() {
    var what = {};
    what.dhcp_enable = setting_model.ipsetup.dhcp_enable;
    what.wan_ip = $('#ip').val();
    what.wan_netmask = $('#netmask').val();
    what.wan_gateway = $('#gateway').val();
    what.wan_dns = $('#dns').val();
    var actionid = new Date().getTime();
    commit_command(actionid, "setIpValue", what, {}, function () {
        show_setting_page_nav();
    });
}

function show_setting_security_setup() {
    if (typeof window.echo === "undefined") {
        setting_model.pinlock_enable = false;
    } else {
        var yesno = window.native.getPINLockEnabled();
        console.log("show_setting_security_setup. yesno = " + yesno);
        if ("yes" == yesno) {
            setting_model.pinlock_enable = true;
        } else {
            setting_model.pinlock_enable = false;
        }
    }


    var tpl = get_precompiled_template("setting_page_security_setup", null);

    if (tpl == null) {
        $.get("./tpl/setting_page_security_setup.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_security_setup", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
    }
}

function save_security() {
    setting_model.admin_password = {};
    setting_model.admin_password.password = $('#new_password').val();
    setting_model.admin_password.confirm_password = $('#confirm_password').val();
    setting_model.admin_password.old_password = $('#old_password').val();
    setting_model.admin_password.err = "";

    setting_model.security_code = {};
    setting_model.security_code.code = $('#new_security_code').val();
    setting_model.security_code.confirm_code = $('#confirm_security_code').val();
    setting_model.security_code.old_code = $('#old_security_code').val();
    setting_model.security_code.err = "";

    setting_model.save_security = {};
    setting_model.save_security.success = "";

    setting_model.pinlock = {};
    setting_model.pinlock.enable = true;

    // get pinlock enabled or not
//    if (typeof window.echo != "undefined") {
//        var reply = window.native.getPINLockEnabled();
//        var obj = JSON.parse(reply);
//        setting_model.pinlock.enable = obj.enable;
//    }

    // 如果什么都没设置，直接退出
    if (setting_model.admin_password.password == "" &&
        setting_model.admin_password.confirm_password == "" &&
        setting_model.admin_password.old_password == "" &&
        setting_model.security_code.code == "" &&
        setting_model.security_code.confirm_code == "" &&
        setting_model.security_code.old_code == "") {
        show_setting_page_nav();
        return false;
    }


    if ((setting_model.admin_password.password != "" && setting_model.admin_password.confirm_password != "" && setting_model.admin_password.old_password != "") || (setting_model.security_code.code != "" && setting_model.security_code.confirm_code != "" && setting_model.security_code.old_code != "")) {
        var what = {};
        if (setting_model.admin_password.password != "" && setting_model.admin_password.confirm_password != "" && setting_model.admin_password.old_password != "") {
            if (setting_model.admin_password.confirm_password == setting_model.admin_password.password) {
                if (setting_model.admin_password.old_password == devicejsontree.gateway.admin_password) {
                    devicejsontree.gateway.admin_password = setting_model.admin_password.password;
                    what.admin_password = setting_model.admin_password.password;
                } else {
                    setting_model.admin_password.err = "Password error";
//                    setting_model.admin_password.show_err = true;
//                    alert("admin password error");
                    toastr['remove']();
                    toastr['error'](i18n('warn wrong addmin password'));
                    return false;
                }
            } else {
                setting_model.admin_password.err = "The password and confirm password does not match";
//                setting_model.admin_password.show_err = true;
//                alert("The password and confirm password does not match");
                toastr['remove']();
                toastr['error'](i18n('warn admin password not matched'));
                return false;
            }
        }
        if (setting_model.security_code.code != "" && setting_model.security_code.confirm_code != "" && setting_model.security_code.old_code != "") {
            if (setting_model.security_code.confirm_code == setting_model.security_code.code) {
                if (setting_model.security_code.old_code == devicejsontree.gateway.gateway_password) {
                    devicejsontree.gateway.gateway_password = setting_model.security_code.code;
                    what.gateway_password = setting_model.security_code.code;
                } else {
                    setting_model.security_code.err = "Security code error";
//                    setting_model.security_code.show_err = true;
//                    alert("security password error");
                    toastr['remove']();
                    toastr['error'](i18n('warn security password error'));
                    return false;
                }
            } else {
                setting_model.security_code.err = "The security code and confirm security code does not match";
//                setting_model.security_code.show_err = true;
//                alert("The security code and confirm security code does not match");
                toastr['remove']();
                toastr['error'](i18n('warn security password not matched'));
                return false;
            }
        }

        var actionid = new Date().getTime();
        commit_command(actionid, "modifyPassword", what, {}, function () {
            console.log("send modifyPassword command");

            show_setting_page_nav();
        });
    } else {
        toastr['remove']();
        toastr['error'](i18n('warn input password'));
        return false;
    }
}

function Pinlock_Setup() {
    if (typeof(window.echo) === "undefined") {
//                alert("Pinlock_Setup ");
    } else {
        window.echo("PinlockSetup", "", function (r) {
        });
    }
}

function change_pinlock_enable() {
    console.log("change_pinlock_enable");
    var pinlock_enable = $(".pinlock_toggle").is(':checked');
    console.log("pinlock_toggle = " + pinlock_enable);

    var yesno;
    var did;
    if (pinlock_enable == true) {
        yesno = 'no';

        setting_model.pinlock_enable = false;
    } else {
        yesno = 'yes';
        setting_model.pinlock_enable = true;
    }

    // send to native
    if (typeof window.echo != "undefined") {
        var reply = window.native.setPINLockEnabled(yesno);
        console.log("reply = " + reply);
    }
}

function show_Firmware_setup() {
    var tpl = get_precompiled_template("setting_page_firmware", null);
    if (tpl == null) {
        $.get("./tpl/setting_page_firmware.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_firmware", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
    }
}

function check_firmware_updates() {
    $("#updates_icon").attr('src', "images/Updates_S.png");
    $("#udpates_prompt").css('display', 'block');

    if (typeof window.echo === "undefined") {
        setTimeout(function () {
            $("#updates_icon").attr('src', "images/Updates_N.png");
            $("#udpates_prompt").css('display', 'none');
        }, 2000);
    } else {
        window.echo("checkGatewayUpgrade", {}, function (r) {
            console.log("checkGatewayUpgrade response:", r);
        });
    }
}


function native_gateway_firmware_updates(key, result) {
    console.log("native_gateway_firmware_updates. key = " + key + ", result = " + result);

    if ("Fail" == key) {
        setTimeout(function () {
            $("#updates_icon").attr('src', "images/Updates_N.png");
            $("#udpates_prompt").css('display', 'none');
        }, 1000);
    }
}

function show_info_setup() {
    if (typeof window.echo === "undefined") {
        setting_model.app_version = "V0.0.0";
        setting_model.gw_version = "V0.0.0";
        setting_model.gw_did = "null";
    } else {
        var apk_version = window.native.getApkVersion();
        console.log("apk_version = " + apk_version);

        var gw_version = window.native.getConnectGatewayVersion();
        console.log("gw_version = " + gw_version);

        var gw_did = window.native.getConnectGatewayDID();
        console.log("gw_did = " + gw_did);

        setting_model.app_version = apk_version;
        setting_model.gw_version = gw_version;
        setting_model.gw_did = gw_did;
    }

    var tpl = get_precompiled_template("setting_page_info", null);
    if (tpl == null) {
        $.get("./tpl/setting_page_info.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_info", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
    }
}

function show_simple_mode_setup() {

    var tpl = get_precompiled_template("setting_page_simple_mode_setup", null);
    if (tpl == null) {
        $.get("./tpl/setting_page_simple_mode_setup.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_simple_mode_setup", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
    }
}

function show_setting_schedule_setup() {
    if (typeof setting_model.schedule_list == 'undefined') {
        showWaitingUI();
        var actionid = new Date().getTime();
        commit_command(actionid, "getSchedule", {}, {}, function (response) {
            console.log("send getSchedule command");
            console.log(response);
            try {
                if (typeof response == 'string') {
                    response = JSON.parse(response);
                }
                setting_model.schedule_list = response.result;

                // not include siren
                setting_model.schedule_receiver_list = [];
                receiver_list.forEach(function (item, index) {
                    if (item.fullmodelno != "JSW-Siren-B001") {
                        setting_model.schedule_receiver_list.push(item);
                    }
                });

                var tpl = get_precompiled_template("setting_page_schedule_setup", null);

                if (tpl == null) {
                    $.get("./tpl/setting_page_schedule_setup.tpl", function (ret) {
                        ret = ret + footer_html;
                        var tpl = get_precompiled_template("setting_page_schedule_setup", ret);
                        var source = tpl(setting_model);
                        $("#main_content").html(source);
                    });
                } else {
                    var source = tpl(setting_model);
                    $("#main_content").html(source);
                }
            } catch (ex) {
                console.log("warn read data error");
                toastr['remove']();
                toastr['error'](i18n('warn read data error'));
                logout_gateway();
            }
        });
    } else {
        // not include siren
        setting_model.schedule_receiver_list = [];
        receiver_list.forEach(function (item, index) {
            if (item.fullmodelno != "JSW-Siren-B001") {
                setting_model.schedule_receiver_list.push(item);
            }
        });

        var tpl = get_precompiled_template("setting_page_schedule_setup", null);

        if (tpl == null) {
            $.get("./tpl/setting_page_schedule_setup.tpl", function (ret) {
                ret = ret + footer_html;
                var tpl = get_precompiled_template("setting_page_schedule_setup", ret);
                var source = tpl(setting_model);
                $("#main_content").html(source);
            });
        } else {
            var source = tpl(setting_model);
            $("#main_content").html(source);
        }
    }
}

function schedule(id) {
    if (typeof setting_model.schedule_list["id_" + id] == 'undefined') {
        setting_model.schedule_list["id_" + id] = [];
    }
    setting_model.schedule = setting_model.schedule_list["id_" + id];
//    setting_model.schedule.forEach(function(key,index){
//        hour = setting_model.schedule[index].duration.On.split(':')[0];
//        hour = Number(hour) - (new Date().getTimezoneOffset() / 60);
//        if (hour > 23) {
//            hour = hour - 24;
//        }
//        if (hour < 10 && hour >= 0) {
//            hour = '0' + hour;
//        }
//        setting_model.schedule[index].duration.On = hour + ':' + setting_model.schedule[index].duration.On.split(':')[1];
//
//        hour = setting_model.schedule[index].duration.Off.split(':')[0];
//        hour = Number(hour) - (new Date().getTimezoneOffset() / 60);
//        if (hour > 23) {
//            hour = hour - 24;
//        }
//        if (hour < 10 && hour >= 0) {
//            hour = '0' + hour;
//        }
//        setting_model.schedule[index].duration.Off = hour + ':' + setting_model.schedule[index].duration.Off.split(':')[1];
//    });
    setting_model.selected_schedule_id = id;
    setting_model.show_schedule_del = false;
    setting_model.schedule_receiver_list.forEach(function (item, index) {
        if (item.id == id) {
            setting_model.schedule_fullmodelno = item.fullmodelno;
        }
    });
    console.log("***", setting_model.schedule_list)
    var tpl = get_precompiled_template("setting_page_schedule", null);

    if (tpl == null) {
        $.get("./tpl/setting_page_schedule.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_schedule", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
    }
}

function change_simple_mode() {
    console.log("change_simple_mode");

    var sm_enable = $(".sm_toggle").is(':checked');
    console.log("sm_enable = " + sm_enable);

    var yesno;
    var did;
    if (sm_enable == true) {
        yesno = 'no';
        did = "empty";

        setting_model.simple_mode = false;
        setting_model.simple_mode_did = "empty";
    } else {
        yesno = 'yes';
        did = gateway_list_page_model.connected_did;

        setting_model.simple_mode = true;
        setting_model.simple_mode_did = gateway_list_page_model.connected_did;
    }

    // send to native

    if (typeof window.echo != "undefined") {
        var reply = window.native.setSimpleMode(yesno, did);
        console.log("reply = " + reply);
    }
}

function show_schedule_del() {
    if (setting_model.show_schedule_del == false) {
        $('.del').show();
        setting_model.show_schedule_del = true;
    } else {
        $('.del').hide();
        setting_model.show_schedule_del = false;
    }
}

function schedule_edit(index) {

    setting_model.selected_schedule = setting_model.schedule[index];
    setting_model.selected_schedule_index = index;
    var tpl = get_precompiled_template("setting_page_schedule_edit", null);

    if (tpl == null) {
        $.get("./tpl/setting_page_schedule_edit.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_schedule_edit", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
//            $('#start-time').ptTimeSelect();
//            $('#end-time').ptTimeSelect();
//            $('#start-time').timepicker({});
//            $('#end-time').timepicker({});

            var opt = {

            }

            opt.date = {preset: 'date'};
            opt.datetime = { preset: 'datetime', minDate: new Date(2000, 1, 1, 9, 22), maxDate: new Date(2037, 12, 31, 15, 44), stepMinute: 1  };
            opt.time = {preset: 'time'};
            opt.tree_list = {preset: 'list', labels: ['Region', 'Country', 'City']};
            opt.image_text = {preset: 'list', labels: ['Cars']};
            opt.select = {preset: 'select'};
            $('#start-time').val(setting_model.selected_schedule.duration.On).scroller('destroy').scroller($.extend(opt['time'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
            $('#end-time').val(setting_model.selected_schedule.duration.Off).scroller('destroy').scroller($.extend(opt['time'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));

        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
//        $('#start-time').timepicker({});
//        $('#end-time').timepicker({});

        var opt = {

        }

        opt.date = {preset: 'date'};
        opt.datetime = { preset: 'datetime', minDate: new Date(2000, 1, 1, 9, 22), maxDate: new Date(2037, 12, 31, 15, 44), stepMinute: 1  };
        opt.time = {preset: 'time'};
        opt.tree_list = {preset: 'list', labels: ['Region', 'Country', 'City']};
        opt.image_text = {preset: 'list', labels: ['Cars']};
        opt.select = {preset: 'select'};
        $('#start-time').val(setting_model.selected_schedule.duration.On).scroller('destroy').scroller($.extend(opt['time'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
        $('#end-time').val(setting_model.selected_schedule.duration.Off).scroller('destroy').scroller($.extend(opt['time'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));

    }
}

function schedule_add() {
    setting_model.selected_schedule = '';
    var tpl = get_precompiled_template("setting_page_schedule_add", null);

    if (tpl == null) {
        $.get("./tpl/setting_page_schedule_add.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("setting_page_schedule_add", ret);
            var source = tpl(setting_model);
            $("#main_content").html(source);
//            $('#start-time').timepicker({});
//            $('#end-time').timepicker({});

            var opt = {

            }

            opt.date = {preset: 'date'};
            opt.datetime = { preset: 'datetime', minDate: new Date(2000, 1, 1, 9, 22), maxDate: new Date(2037, 12, 31, 15, 44), stepMinute: 1  };
            opt.time = {preset: 'time'};
            opt.tree_list = {preset: 'list', labels: ['Region', 'Country', 'City']};
            opt.image_text = {preset: 'list', labels: ['Cars']};
            opt.select = {preset: 'select'};
            $('#start-time').val('00:00').scroller('destroy').scroller($.extend(opt['time'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
            $('#end-time').val('23:59').scroller('destroy').scroller($.extend(opt['time'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
        });
    } else {
        var source = tpl(setting_model);
        $("#main_content").html(source);
//        $('#start-time').timepicker({});
//        $('#end-time').timepicker({});

        var opt = {

        }

        opt.date = {preset: 'date'};
        opt.datetime = { preset: 'datetime', minDate: new Date(2000, 1, 1, 9, 22), maxDate: new Date(2037, 12, 31, 15, 44), stepMinute: 1  };
        opt.time = {preset: 'time'};
        opt.tree_list = {preset: 'list', labels: ['Region', 'Country', 'City']};
        opt.image_text = {preset: 'list', labels: ['Cars']};
        opt.select = {preset: 'select'};
        $('#start-time').val('00:00').scroller('destroy').scroller($.extend(opt['time'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
        $('#end-time').val('23:59').scroller('destroy').scroller($.extend(opt['time'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
    }
}

function change_week(id) {
    if (typeof setting_model.selected_schedule == 'undefined' || setting_model.selected_schedule == '') {
        setting_model.selected_schedule = {};
        setting_model.selected_schedule.week = {};
    }
    if (setting_model.selected_schedule.week[id] == true) {
        delete setting_model.selected_schedule.week[id];
        $('#' + id).attr('src', 'images/Confirm_N.png');
    } else {
        setting_model.selected_schedule.week[id] = true;
        $('#' + id).attr('src', 'images/Confirm_S.png');
    }
}

function compare_obj(obj1, obj2) {
    if (obj1 == obj2)
        return true;
    if (typeof(obj2) == "undefined" || obj2 == null || typeof(obj2) != "object")
        return false;
    var length = 0;
    var length1 = 0;
    for (var ele in obj1) {
        length++;
    }
    for (var ele in obj2) {
        length1++;
    }
    if (length != length1)
        return false;
    if (obj2.constructor == obj1.constructor) {
        for (var ele in obj1) {
            if (typeof(obj1[ele]) == "object") {
                if (!compare_obj(obj1[ele], obj2[ele]))
                    return false;
            }
            else if (typeof(obj1[ele]) == "function") {
                if (!compare_obj(obj1[ele].toString(), obj2[ele].toString()))
                    return false;
            }
            else if (obj1[ele] != obj2[ele])
                return false;
        }
        return true;
    }
    return false;
};

function edit_schedule() {
    start_time = $('#start-time').val();
    if (start_time.substr(-2, 2) == 'AM') {
        hour = start_time.split(':')[0];
        if (hour < '10') {
            start_time = '0' + Number(start_time.substr(0, start_time.length - 3));
        } else {
            start_time = start_time.substr(0, start_time.length - 3);
        }
    } else if (start_time.substr(-2, 2) == 'PM') {
        hour = start_time.split(':')[0];
        start_time = Number(Number(hour) + 12) + ':' + start_time.split(':')[1].substr(0, start_time.split(':')[1].length - 3);
    }

    end_time = $('#end-time').val();
    if (end_time.substr(-2, 2) == 'AM') {
        hour = end_time.split(':')[0];
        if (hour < '10') {
            end_time = '0' + Number(end_time.substr(0, end_time.length - 3));
        } else {
            end_time = end_time.substr(0, end_time.length - 3);
        }
    } else if (end_time.substr(-2, 2) == 'PM') {
        hour = end_time.split(':')[0];
        end_time = Number(Number(hour) + 12) + ':' + end_time.split(':')[1].substr(0, end_time.split(':')[1].length - 3);
    }

//    hour = start_time.split(':')[0];
//    hour = Number(hour) + (new Date().getTimezoneOffset() / 60);
//    if (hour < 0) {
//        hour = hour + 24;
//    } else if (hour < 10 && hour >= 0) {
//        hour = '0' + hour;
//    }
//    start_time_utc = hour + ':' + start_time.split(':')[1];
//
//    hour = end_time.split(':')[0];
//    hour = Number(hour) + (new Date().getTimezoneOffset() / 60);
//    if (hour < 0) {
//        hour = hour + 24;
//    } else if (hour < 10 && hour >= 0) {
//        hour = '0' + hour;
//    }
//    end_time_utc = hour + ':' + end_time.split(':')[1];


    //检测时间是否相同
    var tmp_schedule = {};
    var exist_schedule = {};
    tmp_schedule.week = setting_model.selected_schedule.week;
    tmp_schedule.duration = {};
    tmp_schedule.duration.On = start_time;
    tmp_schedule.duration.Off = end_time;
//    tmp_schedule = JSON.stringify(tmp_schedule);
    for (var i = 0; i < setting_model.schedule.length; i++) {
        exist_schedule.week = setting_model.schedule[i].week;
        exist_schedule.duration = setting_model.schedule[i].duration;
//        exist_schedule = JSON.stringify(exist_schedule);
        if (compare_obj(tmp_schedule, exist_schedule)) {
            toastr['remove']();
            toastr['error'](i18n('Duplicate time'));
            return;
        }
    }

    setting_model.selected_schedule.duration.On = start_time;
    setting_model.selected_schedule.duration.Off = end_time;
    setting_model.selected_schedule.offset = new Date().getTimezoneOffset();
    setting_model.schedule[setting_model.selected_schedule_index] = setting_model.selected_schedule;

    var what = {};
    what.schedule = setting_model.schedule;
    what.key = "id_" + setting_model.selected_schedule_id;
    var actionid = new Date().getTime();
    commit_command(actionid, "addSchedule", what, {}, function () {
        console.log("send addSchedule command");
//        setting_model.selected_schedule.duration.On = start_time;
//        setting_model.selected_schedule.duration.Off = end_time;
        schedule(setting_model.selected_schedule_id);
    });
}

function add_schedule() {
    start_time = $('#start-time').val();
    if (start_time.substr(-2, 2) == 'AM') {
        hour = start_time.split(':')[0];
        if (hour < '10') {
            start_time = '0' + Number(start_time.substr(0, start_time.length - 3));
        } else {
            start_time = start_time.substr(0, start_time.length - 3);
        }
    } else if (start_time.substr(-2, 2) == 'PM') {
        hour = start_time.split(':')[0];
        start_time = Number(Number(hour) + 12) + ':' + start_time.split(':')[1].substr(0, start_time.split(':')[1].length - 3);
    }

    end_time = $('#end-time').val();
    if (end_time.substr(-2, 2) == 'AM') {
        hour = end_time.split(':')[0];
        if (hour < '10') {
            end_time = '0' + Number(end_time.substr(0, end_time.length - 3));
        } else {
            end_time = end_time.substr(0, end_time.length - 3);
        }
    } else if (end_time.substr(-2, 2) == 'PM') {
        hour = end_time.split(':')[0];
        end_time = Number(Number(hour) + 12) + ':' + end_time.split(':')[1].substr(0, end_time.split(':')[1].length - 3);
    }
    if (typeof setting_model.selected_schedule.duration == 'undefined') {
        setting_model.selected_schedule.duration = {};
    }

//    hour = start_time.split(':')[0];
//    hour = Number(hour) + (new Date().getTimezoneOffset() / 60);
//    if (hour < 0) {
//        hour = hour + 24;
//    } else if (hour < 10 && hour >= 0) {
//        hour = '0' + hour;
//    }
//    start_time_utc = hour + ':' + start_time.split(':')[1];
//
//    hour = end_time.split(':')[0];
//    hour = Number(hour) + (new Date().getTimezoneOffset() / 60);
//    if (hour < 0) {
//        hour = hour + 24;
//    } else if (hour < 10 && hour >= 0) {
//        hour = '0' + hour;
//    }
//    end_time_utc = hour + ':' + end_time.split(':')[1];


    //检测时间是否相同，数量是否超过6条
    setting_model.schedule_count = 6;
    var schedule_count = 0;
    var tmp_schedule = {};
    var exist_schedule = {};
    tmp_schedule.week = setting_model.selected_schedule.week;
    tmp_schedule.duration = {};
    tmp_schedule.duration.On = start_time;
    tmp_schedule.duration.Off = end_time;
//    tmp_schedule = JSON.stringify(tmp_schedule);
    for (var i = 0; i < setting_model.schedule.length; i++) {
        schedule_count++;
        exist_schedule.week = setting_model.schedule[i].week;
        exist_schedule.duration = setting_model.schedule[i].duration;
//        exist_schedule = JSON.stringify(exist_schedule);
        if (compare_obj(tmp_schedule, exist_schedule)) {
            toastr['remove']();
            toastr['error'](i18n('Duplicate time'));
            return;
        }
    }
    if (schedule_count >= setting_model.schedule_count) {
        toastr['remove']();
        toastr['error'](i18n('The schedule limit 6 is reached'));
        return;
    }

    if (typeof setting_model.selected_schedule == 'undefined' || setting_model.selected_schedule == '') {
        toastr['remove']();
        toastr['error'](i18n('Please select a day'));
        return;
    }
    setting_model.selected_schedule.action = models_dict[setting_model.schedule_fullmodelno].command[0].type;
    setting_model.selected_schedule.value0 = models_dict[setting_model.schedule_fullmodelno].command[0].value[0];
    setting_model.selected_schedule.value1 = models_dict[setting_model.schedule_fullmodelno].command[0].value[1];
    setting_model.selected_schedule.enable = 'yes';
    setting_model.selected_schedule.duration = {};
    setting_model.selected_schedule.duration.On = start_time;
    setting_model.selected_schedule.duration.Off = end_time;
    setting_model.selected_schedule.offset = new Date().getTimezoneOffset();

    if (typeof setting_model.schedule == 'undefined') {
        setting_model.schedule = [];
    }
    setting_model.schedule.push(setting_model.selected_schedule);

    var what = {};
    what.schedule = setting_model.schedule;
    what.key = "id_" + setting_model.selected_schedule_id;
    var actionid = new Date().getTime();
    commit_command(actionid, "addSchedule", what, {}, function () {
        console.log("send addSchedule command");
//        setting_model.selected_schedule.duration.On = start_time;
//        setting_model.selected_schedule.duration.Off = end_time;
        schedule(setting_model.selected_schedule_id);
    });
}

function del_schedule(index) {
    var what = {};
    what.key = "id_" + setting_model.selected_schedule_id;
    what.index = index;

    var actionid = new Date().getTime();
    commit_command(actionid, "removeSchedule", what, {}, function () {
        console.log("send removeSchedule command");
        setting_model.schedule.splice(index, 1);
        schedule(setting_model.selected_schedule_id);
    });
}

function change_schedule_status(index) {
    if (setting_model.schedule[index].enable == 'yes') {
        setting_model.schedule[index].enable = 'no';
    } else {
        setting_model.schedule[index].enable = 'yes';
    }

    var what = {};
    what.schedule = setting_model.schedule;
    what.key = "id_" + setting_model.selected_schedule_id;
    var actionid = new Date().getTime();
    commit_command(actionid, "addSchedule", what, {}, function () {
        console.log("send addSchedule command");
//        schedule(setting_model.selected_schedule_id);
    });
}

function i18n(key) {
    if (typeof i18n_data[key] == "undefined") {
        return key;
    } else {
        return i18n_data[key];
    }
}

function show_setting_notification_setup() {
    console.log("show_setting_notification_setup");
    setting_model.message = {};

    showWaitingUI();
    var actionid = new Date().getTime();
    commit_command(actionid, "getMessageValue", {}, {}, function (response) {
        try {
            if (typeof response === 'string') {
                response = JSON.parse(response);
            }

            console.log("response->result = ", response.result);

            if (response.result != undefined && response.result != null) {
                console.log("get message into");
                setting_model.message = response.result;
            } else {
                console.log("message result is null");
                setting_model.message = {};
            }

            var tpl = get_precompiled_template("setting_page_notification_setup", null);
            if (tpl == null) {
                $.get("./tpl/setting_page_notification_setup.tpl", function (ret) {
                    ret = ret + footer_html;
                    var tpl = get_precompiled_template("setting_page_notification_setup", ret);
                    var source = tpl(setting_model.message);
                    $("#main_content").html(source);

                    updateEmailNotificationIcon();
                });
            } else {
                var source = tpl(setting_model.message);
                $("#main_content").html(source);
                updateEmailNotificationIcon();
            }
        } catch (ex) {
            console.log("warn read data error");
            toastr['remove']();
            toastr['error'](i18n('warn read data error'));
            logout_gateway();
        }
    });
}

function updateEmailNotificationIcon() {
    console.log("updateEmailNotificationIcon");

    // receiver
    if (setting_model.message.receiver != undefined) {
        $('#receiver').val(setting_model.message.receiver);
    }

    // email enable or not
    if (setting_model.message.email_enable != undefined) {
        if (setting_model.message.email_enable == true) {
            $('#email').attr('src', 'images/Confirm_S.png');
        } else {
            $('#email').attr('src', 'images/Confirm_N.png');
        }
    }

    // notification enable or not
    if (setting_model.message.notification_enable != undefined) {
        if (setting_model.message.notification_enable == true) {
            $('#notification').attr('src', 'images/Confirm_S.png');
        } else {
            $('#notification').attr('src', 'images/Confirm_N.png');
        }
    }
}

function change_email_state() {
    if (setting_model.message.email_enable == false || setting_model.message.email_enable == undefined) {
        setting_model.message.email_enable = true;
        $('#email').attr('src', 'images/Confirm_S.png');
    } else {
        setting_model.message.email_enable = false;
        $('#email').attr('src', 'images/Confirm_N.png');
    }
}

function change_notification_state() {
    if (setting_model.message.notification_enable == false || setting_model.message.notification_enable == undefined) {
        setting_model.message.notification_enable = true;
        $('#notification').attr('src', 'images/Confirm_S.png');
    } else {
        setting_model.message.notification_enable = false;
        $('#notification').attr('src', 'images/Confirm_N.png');
    }
}

function save_message() {
    // get your@mail.com

    var what = {};
    what.email_enable = setting_model.message.email_enable;
    what.notification_enable = setting_model.message.notification_enable;

    if (true == what.email_enable) {
        what.receiver = $('#receiver').val();

        if (typeof what.receiver != 'undefined' && what.receiver.trim() == "") {
            toastr['remove']();
            toastr['error'](i18n('warn input email'));
            return;
        }
    }

    var actionid = new Date().getTime();
    commit_command(actionid, "setMessageValue", what, {}, function () {
        show_setting_page_nav();
    });
}

function showWaitingUI() {
    $("#main_content").html('<div><div class="spinner circles"><div></div><div></div><div></div><div></div><div></div><div></div><div></div><div></div></div></div>');
}