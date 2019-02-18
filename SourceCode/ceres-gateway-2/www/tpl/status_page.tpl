<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="logout_gateway();"/></p>
        </div>
        <div class="pure-u-1-8 is-center" onclick="sm.fire('to_add_device_starter');">
            <p><img src="images/Add_N.png" class="img-button"/></p>
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Device Status"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Edit_N.png" class="img-button"
                    onclick="status_page_enter_edit_mode()"/>
            </p>
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%;">
    <!-- Camera 列表 -->
    <div class="content pure-g" style="border-bottom: 1px solid #cfcac6;">
        {{#each camera_list}}
        <div class="pure-u-1-2 camera border_right"
             onclick="open_camera({{id}});" style="border-bottom: 1px solid #cfcac6;">

            <div style="margin-top: 3%;">
                <div class="pure-g" style="margin-bottom: -20px;">
                    <div class="pure-u-1-5">
                    </div>
                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state"
                             id="camera_{{id}}_state"
                            src="images/Power_{{camera_state id}}.png"
                        >
                    </div>
                    <div class="pure-u-3-5">
                    </div>
                </div>

                <div class="pure-g">
                    <div class="pure-u-1-3 is-right" style="float: left; padding-left: 10%;">
                        <img class="img-device-icon" src="{{icon}}">
                    </div>
                    <div class="pure-u-2-3 is-left" style="float: left; padding-left: 20px; margin-top: -3%;">
                        <div>
                            <p class="jsw-truncate">{{name}}</p>
                        </div>
                        <div>
                            {{seat}}
                        </div>
                    </div>
                </div>

                <div class="edit-button" style="margin-top: -13px; display: none;">
                    <div style="float: right; margin-right: 10px; margin-left: 20px;">
                        <img class="img-device-state" src="images/Cancel_R_N.png"
                             onclick="event.cancelBubble=true;db_remove_item('{{id}}')">
                    </div>
                    <div style="float: right; ">
                        <img class="img-device-state" src="images/Setting_N.png"
                             onclick="event.cancelBubble=true;configure_item({{id}});">
                    </div>
                </div>
            </div>
        </div>
        {{/each}}
    </div>

    <!-- Receiver 列表 -->
    <div class="content pure-g" style="border-bottom: 1px solid #cfcac6;">
        {{#each receiver_list}}
        <div class="pure-u-1-2 border_right"
             onclick="show_remote_controller_page_receiver('{{id}}');" style="border-bottom: 1px solid #cfcac6;">
            <div style="margin-top: 3%;">
                <div class="pure-g" style="margin-bottom: -20px;">
                    <div class="pure-u-1-5">
                    </div>
                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state"
                             id="sensor_{{id}}_state"
                             src="images/Power_{{sensor_state id}}.png">
                    </div>
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state blink" src="images/Battery-low.png"
                             id="alarm_{{id}}_battery" style="visibility: {{show_battery id}};">
                    </div>
                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state blink" src="images/Status_N.png"
                             id="alarm_{{id}}_tamper" style="visibility: {{show_tamper id}};">
                    </div>
                </div>

                <div class="pure-g">
                    <div class="pure-u-1-3 is-right" style="float: left;padding-left:
                    10%">
                        <img class="img-device-icon" src="{{icon}}">
                    </div>
                    <div class="pure-u-2-3 is-left" style="float: left; padding-left: 20px; margin-top: -3%;">
                        <div>
                            <p class="jsw-truncate">{{name}}</p>
                        </div>
                        <div>
                            {{seat}}
                        </div>
                    </div>
                </div>

                <div class="edit-button" style="margin-top: -13px; display: none;">
                    <div style="float: right; margin-right: 10px; margin-left: 20px;">
                        <img class="img-device-state" src="images/Cancel_R_N.png"
                             onclick="event.cancelBubble=true;db_remove_item('{{id}}')">
                    </div>
                    <div style="float: right; ">
                        <img class="img-device-state" src="images/Setting_N.png"
                             onclick="event.cancelBubble=true;configure_item({{id}});">
                    </div>
                </div>
            </div>


        </div>
        {{/each}}
    </div>

    <!-- Sender 列表 -->
    <div class="content pure-g" style="border-bottom: 1px solid #cfcac6;">
        {{#each sender_list}}
        <div class="pure-u-1-2 border_right"
             onclick="clear_alarm_status('{{id}}')" id="item_{{id}}_block" style="border-bottom: 1px solid #cfcac6;">

            <div style="margin-top: 3%;">
                <div class="pure-g" style="margin-bottom: -20px;">
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state" src="images/Siren_Alert.png"
                             id="alarm_{{id}}_state" style="visibility: {{show_alarm id}};">
                    </div>

                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state blink" src="images/Battery-low.png"
                             id="alarm_{{id}}_battery" style="visibility: {{show_battery id}};">
                    </div>
                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state blink" src="images/Status_N.png"
                             id="alarm_{{id}}_tamper" style="visibility: {{show_tamper id}};">
                    </div>
                </div>

                <div class="pure-g">
                    <div class="pure-u-1-3 is-right" style="float: left; padding-left: 10%">
                        <img class="img-device-icon" src="{{icon}}">
                    </div>
                    <div class="pure-u-2-3 is-left" style="float: left; padding-left: 20px; margin-top: -3%;">
                        <div>
                            <p class="jsw-truncate">{{name}}</p>
                        </div>
                        <div>
                            {{seat}}
                        </div>
                    </div>
                </div>

                <div class="edit-button" style="margin-top: -13px; display: none;">
                    <div style="float: right; margin-right: 10px; margin-left: 20px;">
                        <img class="img-device-state" src="images/Cancel_R_N.png"
                             onclick="event.cancelBubble=true;db_remove_item('{{id}}');">
                    </div>
                    <div style="float: right; ">
                        <img class="img-device-state" src="images/Setting_N.png"
                             onclick="event.cancelBubble=true;configure_item('{{id}}');">
                    </div>
                </div>
            </div>
        </div>
        {{/each}}
    </div>

    <!-- Remote Controller 列表 -->
    <div class="content pure-g" style="border-bottom: 1px solid #cfcac6;margin-bottom: 74px;">
        {{#each remote_list}}
        <div class="pure-u-1-2 border_right"
             onclick="goto_remotekey_sm('{{id}}');" id="item_{{id}}_block" style="border-bottom: 1px solid #cfcac6;">

            <div style="margin-top: 3%;">
                <div class="pure-g" style="margin-bottom: -20px;">
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5"></div>

                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state" src="images/Siren_Alert.png"
                             id="alarm_{{id}}_state" style="visibility: hidden;">
                    </div>

                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state blink" src="images/Battery-low.png"
                             id="alarm_{{id}}_battery" style="visibility: {{show_battery id}};">
                    </div>
                    <div class="pure-u-1-5 is-center">
                        <img class="img-device-state blink" src="images/Status_N.png"
                             id="alarm_{{id}}_tamper" style="visibility: {{show_tamper id}};">
                    </div>
                </div>

                <div class="pure-g">
                    <div class="pure-u-1-3 is-right" style="float: left; padding-left: 10%">
                        <img class="img-device-icon" src="{{icon}}">
                    </div>
                    <div class="pure-u-2-3 is-left" style="float: left; padding-left: 20px; margin-top: -3%;">
                        <div>
                            <p class="jsw-truncate">{{name}}</p>
                        </div>
                        <div>
                            {{seat}}
                        </div>
                    </div>
                </div>

                <div class="edit-button" style="margin-top: -13px; display: none;">
                    <div style="float: right; margin-right: 10px; margin-left: 20px;">
                        <img class="img-device-state" src="images/Cancel_R_N.png"
                             onclick="event.cancelBubble=true;db_remove_item('{{id}}');">
                    </div>
                    <div style="float: right; ">
                        <img class="img-device-state" src="images/Setting_N.png"
                             onclick="event.cancelBubble=true;configure_item('{{id}}');">
                    </div>
                </div>
            </div>
        </div>
        {{/each}}
    </div>

</div>
