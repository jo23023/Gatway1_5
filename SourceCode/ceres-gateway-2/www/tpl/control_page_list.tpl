<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center" onclick="sm.fire('back_to_status_page');">
            <p><img src="images/Back_N.png" class="img-button"/></p>
        </div>
        <div class="pure-u-1-8 is-center" onclick="show_all_control_page_setup(-1);">
            <p><img src="images/Add_N.png" class="img-button"/></p>
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Control Setup"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
            <p>
                <img src="images/Edit_N.png" class="img-button"
                     onclick="control_page_enter_edit_mode()"/>
            </p>
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%;">

    <div class="content pure-g">
        <!-- ARM -->
        <div class="pure-u-1-3 is-center control-square-parent" style="padding: 2px">
            <div class="control-square" style="border: 1px solid white; float: left; z-index: 90;">
                <img id="icon_97" class="pure-img-responsive"
                     style="padding-top: 15%; padding-left: 20%; padding-right: 20%;"
                     onclick="send_arm_control_command('97')" src="images/Disarm_N.png">

                <p class="jsw-truncate" style="margin-top: 1px;">
                    {{arm_group.name}}
                </p>
            </div>

            <div class="control-square edit-button"
                 style="position: absolute; float: left; z-index: 99; padding-top: 3%; display: none">
                <div class="pure-g">
                    <div class="pure-u-1-5"></div>

                    <img class="img-icon-20p" src="images/Setting_G_N.png"

                         onclick="event.cancelBubble=true;show_arm_control_page_setup('97')">
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5"></div>
                </div>
            </div>
        </div>

        <!-- Camera -->
        <div class="pure-u-1-3 is-center control-square-parent" style="padding: 2px">
            <div class="control-square" style="border: 1px solid white; float: left; z-index: 90;">
                <img id="icon_98" class="pure-img-responsive" style="padding-top: 15%; padding-left: 20%; padding-right: 20%;"
                     onclick="send_group_control_command('98')" src="images/CameraRecording_N.png">

                <p class="jsw-truncate" style="margin-top: 1px;">
                    {{camera_group.name}}
                </p>
            </div>

            <div class="control-square edit-button"
                 style="position: absolute; float: left; z-index: 99; padding-top: 3%; display: none">
                <div class="pure-g">
                    <div class="pure-u-1-5"></div>

                    <img class="img-icon-20p" src="images/Setting_G_N.png"

                         onclick="event.cancelBubble=true;show_all_control_page_setup('98')">
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5"></div>
                </div>
            </div>
        </div>

        <!-- Panic -->
        <div class="pure-u-1-3 is-center control-square-parent" style="padding: 2px">
            <div class="control-square" style="border: 1px solid white; float: left; z-index: 90;">
                <img id="icon_99" class="pure-img-responsive" style="padding-top: 15%; padding-left: 20%; padding-right: 20%;"
                     onclick="send_group_control_command('99')" src="images/Panic_N.png">

                <p class="jsw-truncate" style="margin-top: 1px;">{{panic_group.name}}</p>
            </div>

            <div class="control-square edit-button"
                 style="position: absolute; float: left; z-index: 99; padding-top: 3%; display: none">
                <div class="pure-g">
                    <div class="pure-u-1-5"></div>

                    <img class="img-icon-20p" src="images/Setting_G_N.png"

                         onclick="event.cancelBubble=true;show_all_control_page_setup('99')">
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5"></div>
                    <div class="pure-u-1-5"></div>
                </div>
            </div>
        </div>
    </div>

    <!-- General Control -->
    <div class="content pure-g" style="margin-bottom:90px">
        {{#each general_group}}
        <div class="pure-u-1-3 is-center control-square-parent" style="padding: 2px;" id="control_{{index}}">
            <div class="control-square"
                 style="border: 1px solid white; width: 50px; height: 50px; float: left; z-index: 90;">
                <img id="icon_{{control.id}}" class="pure-img-responsive" style="padding-top: 15%; padding-left: 20%; padding-right: 20%;"
                     onclick="send_group_control_command('{{control.id}}')" src="images/{{index}}_N.png">

                <p class="jsw-truncate" style="margin-top: 1px;">{{name}}</p>
            </div>
            <div class="control-square edit-button"
                 style="position:absolute; float: left; z-index:99; padding-top: 3%; display: none;">
                <div class="pure-g">
                    <div class="pure-u-1-5"></div>

                    <img class="img-icon-20p" src="images/Setting_G_N.png"
                         onclick="event.cancelBubble=true;show_all_control_page_setup('{{control.id}}')">

                    <div class="pure-u-1-5"></div>
                    <img class="img-icon-20p" src="images/Cancel_R_N.png"

                            onclick="event.cancelBubble=true;delete_control_group('{{control.id}}');">

                    <div class="pure-u-1-5"></div>
                </div>
            </div>
        </div>
        {{/each}}
    </div>
</div>

