<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button"
                    onclick="show_control_page_list();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Control Setup"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%;">
    <div style="background: #ffffff">
        <!-- input name -->
        <input id="group_setup_name" type="text" value="{{arm_group.name}}"
               style="width: 80%; margin: 10px; background: #cfcac6; color: #000000;"><br/>

        <label style="margin: 10px; color: #129FEA;">{{ i18n "Select Device"}}</label>
    </div>

    <!-- Camera 列表 -->
    <div class="content pure-g">
        {{#each camera_list}}
        <div class="pure-u-1-2 camera border_right" style="border-bottom: 1px solid #cfcac6;"
             >

            <div style="margin-top: 5%;">
                <div class="pure-g">
                    <div class="pure-u-1-3 is-left" style="float: left; padding-left: 7%;">
                        <img class="img-device-icon" src="{{icon}}">
                    </div>
                    <div class="pure-u-5-12 is-right" style="float: left; padding-left: 10px; margin-top: -3%;">
                        <div>
                            <p class="jsw-truncate">{{name}}</p>
                        </div>
                        <div>
                            <p class="jsw-truncate">{{seat}}</p>
                        </div>
                    </div>
                    <div class="pure-u-3-12 is-right">
                        <img id="checkbox_{{id}}" class="img-icon-1x" src="images/Circle.png"
                                onclick="camera_select_changed('{{id}}')">
                    </div>
                </div>
            </div>
        </div>
        {{/each}}
    </div>

    <!-- Receiver 列表 -->
    <div class="content pure-g" style="border-bottom: 1px solid #cfcac6;">
        {{#each receiver_list}}
        <div class="pure-u-1-2 border_right">
            <div style="margin-top: 5%;">

                <div class="pure-g">
                    <div class="pure-u-1-3 is-right" style="float: left;padding-left:
                    10%">
                        <img class="img-device-icon" src="{{icon}}">
                    </div>
                    <div class="pure-u-5-12 is-center" style="float: left; padding-left: 10px; margin-top: -3%;">
                        <div>
                            <p class="jsw-truncate">{{name}}</p>
                        </div>
                        <div>
                            <p class="jsw-truncate">{{seat}}</p>
                        </div>
                    </div>
                    <div class="pure-u-3-12 is-right">
                        <img id="checkbox_{{id}}" class="img-icon-1x" src="images/Circle.png"
                             onclick="receiver_select_changed('{{id}}')">
                    </div>
                </div>
            </div>
        </div>
        {{/each}}
    </div>

    <!-- Sender 列表 -->
    <div class="content pure-g" style="border-bottom: 1px solid #cfcac6;">
        {{#each sender_list}}
        <div class="pure-u-1-2 border_right">
            <div style="margin-top: 5%;">

                <div class="pure-g">
                    <div class="pure-u-1-3 is-right" style="float: left;padding-left:
                    10%">
                        <img class="img-device-icon" src="{{icon}}">
                    </div>
                    <div class="pure-u-5-12 is-center" style="float: left; padding-left: 10px; margin-top: -3%;">
                        <div>
                            <p class="jsw-truncate">{{name}}</p>
                        </div>
                        <div>
                            <p class="jsw-truncate">{{seat}}</p>
                        </div>
                    </div>
                    <div class="pure-u-3-12 is-right">
                        <img id="checkbox_{{id}}" class="img-icon-1x" src="images/Circle.png"
                             onclick="sender_select_changed('{{id}}')">
                    </div>
                </div>
            </div>
        </div>
        {{/each}}
    </div>

    <div style="margin-bottom: 10px; margin-top: 10px;">
        <div class="pure-g">
            <div class="pure-u-1-5">
                <img onclick="show_control_page_list();" class="pure-img-responsive"
                     src="images/Cancel_N-N.png">
            </div>
            <div class="pure-u-1-5"></div>
            <div class="pure-u-1-5"></div>
            <div class="pure-u-1-5"></div>
            <div class="pure-u-1-5">
                <img onclick="save_arm_setup();" class="pure-img-responsive"
                     src="images/Save_N.png">
            </div>
        </div>
    </div>

    <br/>
    <br/>
    <br/>
</div>
