/**
 * Created by yulin on 14-8-4.
 */

function event_page_sm_init() {
    sm.onEnter("event_page", function (transition, eventName, next) {
        next();

        show_event_page_searching();
        get_eventlist();
    });
}


var event_page_model = {};

function show_event_page_searching() {
    event_page_model.search_result_valid = true;
    event_page_model.event_list_need_clear = true;

    var tpl = get_precompiled_template("event_page_searching", null);
    if (tpl == null) {
        $.get("./tpl/event_page_searching.tpl", function (ret) {
            var tpl = get_precompiled_template("event_page_searching", ret);
            var source = tpl(event_page_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(event_page_model);
        $("#main_content").html(source);
    }
}

function stop_searching_event() {
    event_page_model.search_result_valid = false;
    show_event_list();
}

function get_eventlist() {
    // 默认获取过去的 24 小时内的数据
    var today = new Date();
    var stop = parseInt(today.getTime());
    var start = stop - 24 * 3600 * 1000;

    get_eventlist_start_end(start, stop);
}

function get_eventlist_searchtime() {
    console.log("get_eventlist_searchtime");

    var start_date = $("#start_date").val();    // 2014-08-23
//    var start_time = $("#start_time").val();    // 7:30 AM
    console.log("start_date = " + start_date);

//    var start = start_date + " " + start_time;
    var start = start_date;
    start = new Date(Date.parse(start.replace(/-/g, "/")));
//    start = new Date(Date.parse(start.replace(/-/g, "/"))).Format("yyyy-MM-dd hh:mm");

    var stop_date = $("#stop_date").val();
//    var stop_time = $("#stop_time").val();
//    var stop = stop_date + " " + stop_time;
    var stop = stop_date;
    stop = new Date(Date.parse(stop.replace(/-/g, "/")));
//    stop = new Date(Date.parse(stop.replace(/-/g, "/"))).Format("yyyy-MM-dd hh:mm");

    console.log("start = " + start);  // 2014-08-14 16:25
    console.log("stop = " + stop);

    var start_long = parseInt(start.getTime());
    var stop_long = parseInt(stop.getTime());
    if (stop_long > start_long) {
        show_event_page_searching();
        get_eventlist_start_end(start_long, stop_long);
    } else {
        toastr['remove']();
        toastr['error'](i18n('time duration error'));
    }
}

function get_eventlist_start_end(start_long, stop_long) {
    //console.log("get_eventlist_start_end ");
    //console.log("start_long = " + start_long + ", stop_long = " + stop_long);

    get_eventlist_start_end_gateway(start_long, stop_long);

    get_eventlist_start_end_camera(start_long, stop_long);
}

function get_eventlist_start_end_camera(start_long, stop_long) {
    console.log("get_eventlist_start_end_camera");

    if (typeof window.echo == "undefined") {
        console.log("broser not support this operation");
    } else {
        // 如果直接传递 long 型数据，android 里接收到为 Null，所以要转成 String 类型
        var reply = window.native.getAllCamEventlist(String(start_long), String(stop_long));
        //console.log("reply = " + reply);
    }
}

function get_eventlist_start_end_gateway(start_long, stop_long) {
    console.log("get_eventlist_start_end_gateway");

    var actionid = new Date().getTime();
    var what = {};
    what.id = "all";
    what.start = parseInt(start_long / 1000);   // gateway里面的时间戳是 10 位数字
    what.stop = parseInt(stop_long / 1000);

    commit_command(actionid, "getEventListMix", what, null, function (data) {
        if (typeof window.echo == "undefined" &&
            (DATA_SOURCE_LAN == data_from || DATA_SOURCE_CHROME_DEBUG == data_from)) {
            data = JSON.parse(data);
        }

        console.log("===>gateway: get event list rsp");
        console.log("===>search_result_valid = " + event_page_model.search_result_valid);
        console.log("===>event_list_need_clear = " + event_page_model.event_list_need_clear);

        try {
            if (event_page_model.search_result_valid == true) {
                if (event_page_model.event_list_need_clear == true) {
                    event_page_model.event_list_need_clear = false;
                    event_page_model.event_list = [];
                }

                mergeGatewayEventList(data.result);
            }
        } catch (ex) {
            console.log("warn read data error");
            toastr['remove']();
            toastr['error'](i18n('warn read data error'));
            logout_gateway();
        }
        // else{
        //     console.log("===> user cancel searching!!!");
        // }
    });
}

function show_event_list() {
//    $("#part1").hide();

    var tpl = get_precompiled_template("event_page_list", null);
    if (tpl == null) {
        $.get("./tpl/event_page_list.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("event_page_list", ret);
            var source = tpl(event_page_model);
            $("#main_content").html(source);
        });
    } else {
        var source = tpl(event_page_model);
        $("#main_content").html(source);
    }
}

function show_event_page_search_time() {

    var tpl = get_precompiled_template("event_page_search_time", null);
    if (tpl == null) {
        $.get("./tpl/event_page_search_time.tpl", function (ret) {
            ret = ret + footer_html;
            var tpl = get_precompiled_template("event_page_search", ret);
            var source = tpl(event_page_model);
            $("#main_content").html(source);

//            new datepickr('start_date',{'dateFormat': 'Y-m-d'});
//            new datepickr('stop_date', {'dateFormat': 'Y-m-d'});
//
//            $('#start_time').ptTimeSelect();
//            $('#stop_time').ptTimeSelect();

//            $('#start_date').datetimepicker({
//                dateFormat: 'yy-mm-dd'
//            });
//            $('#stop_date').datetimepicker({
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
            $('#start_date').val('').scroller('destroy').scroller($.extend(opt['datetime'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
            $('#stop_date').val('').scroller('destroy').scroller($.extend(opt['datetime'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
        });
    } else {
        var source = tpl(event_page_model);
        $("#main_content").html(source);

//        new datepickr('start_date',{'dateFormat': 'Y-m-d'});
//        new datepickr('stop_date', {'dateFormat': 'Y-m-d'});
//
//        $('#start_time').ptTimeSelect();
//        $('#stop_time').ptTimeSelect();
//        $('#start_date').datetimepicker({
//            dateFormat: 'yy-mm-dd'
//        });
//        $('#stop_date').datetimepicker({
//            dateFormat: 'yy-mm-dd'
//        });

        var opt = {

        }

        opt.date = {preset: 'date'};
        opt.datetime = { preset: 'datetime', minDate: new Date(2000, 1, 1, 9, 22), maxDate: new Date(2037, 12, 31, 15, 44), stepMinute: 1  };
        opt.time = {preset: 'time'};
        opt.tree_list = {preset: 'list', labels: ['Region', 'Country', 'City']};
        opt.image_text = {preset: 'list', labels: ['Cars']};
        opt.select = {preset: 'select'};
        $('#start_date').val('').scroller('destroy').scroller($.extend(opt['datetime'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
        $('#stop_date').val('').scroller('destroy').scroller($.extend(opt['datetime'], { theme: 'default', mode: 'scroller', display: 'bottom', lang: i18n("date lang") }));
    }
}

// 接收摄像头的 EventList
/*
 result : [{"source":{"utctime":1407833147637,"id":"3","did":"CHXX-000338-TBZRJ","name":"cam338"},"t":1407833147},{"source":{"utctime":1407833068638,"id":"3","did":"CHXX-000338-TBZRJ","name":"cam338"},"t":1407833068}]
 */
function native_ipc_eventlist(key, result) {
//    console.log("native_ipc_eventlist key = " + key + ", result = " + result);
    var eventArray = JSON.parse(result);

    // console.log("===>gateway: get event list rsp");
    // console.log("===>search_result_valid = " + event_page_model.search_result_valid);
    // console.log("===>event_list_need_clear = " + event_page_model.event_list_need_clear);

    // 合并到 EventList， 然后显示
    if (event_page_model.search_result_valid == true) {
        if (event_page_model.event_list_need_clear == true) {
            event_page_model.event_list_need_clear = false;
            event_page_model.event_list = [];
        }

        mergeCameraEventlist(result);
    }
    // else{
    //     console.log("===> user cancel searching!!!");
    // }
}

function mergeCameraEventlist(result) {
    console.log("mergeCameraEventlist");
    var cam_events = JSON.parse(result);
    for (var i = 0; i < cam_events.length; ++i) {
        cam_events[i].source.fullmodelno = 'JSW-Camera-0001';
        event_page_model.event_list.push(cam_events[i]);
    }

    event_page_model.event_list.sort(function (a, b) {
        return a.t - b.t;
    });

    event_page_model.event_list.forEach(function (event, index) {
        event.source.icon = models_dict[event.source.fullmodelno].icon;
        console.log("====> " + event.source.icon);
    });

    show_event_list();
}

function mergeGatewayEventList(result) {
    console.log("mergeGatewayEventList result = " + JSON.stringify(result));
//    event_page_model.event_list = result;

    for (var i = 0; i < result.length; ++i) {
        result[i].source.icon = models_dict[result[i].source.fullmodelno].icon;
        event_page_model.event_list.push(result[i]);
    }

    // sort by time
    event_page_model.event_list.sort(function (a, b) {
        return a.t - b.t;
    });

    event_page_model.event_list.forEach(function (event, index) {
        event.source.icon = models_dict[event.source.fullmodelno].icon;

        console.log("====> " + event.source.icon);
    });

    if (sm.current == "event_page") {
        show_event_list();
    }
}


function event_item_clicked(t, id, did, utctime) {
    console.log("event_item_clicked. t = " + t + ", id = " + id + ", did = " + did + ", utctime = " + utctime);

    if (typeof(did) == "undefined" || did.length <= 0) {
        return;
    }

    if (typeof(utctime) == "undefined" || utctime.length <= 0) {
        return;
    }

    // open camera record view
    var what = {};
    what.id = id;
    what.did = did;
    what.utctime = utctime;
    what.t = t;

    window.echo("opencamerarecord", JSON.stringify(what), function (r) {
    })
}