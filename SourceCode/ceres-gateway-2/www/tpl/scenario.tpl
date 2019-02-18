<div class="header" style="position:fixed;top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button"
                    onclick="show_scenario_page()"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Scenario Setup"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="position:absolute;z-index: -10; top:64px; width: 98%;">
    <div style="z-index: -10;top:64px; width: 98%; background-color: white; margin: 1%;">
        <table class="pure-table pure-table-horizontal" style="width: 100%; ">
            <tbody>
            <tr style="background-color:#000000;color: blue">
                <td>When...</td>
                <td></td>
                <td></td>
            </tr>
            <tr>
                <td>
                    <img class="img-device-icon" src="{{model_icon_url selected_scenario.fullmodelno}}">
                </td>
                <td class="is-left">
                    <div>{{selected_scenario.name}}</div>
                    <div>{{selected_scenario.seat}}</div>
                </td>
                <td>
                    {{#is selected_type 'sender'}}
                    <div class="slideThree">
                        <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">{{selected_scenario.command.0.value1}}</div>
                        <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">{{selected_scenario.command.0.value2}}</div>
                        <input type="checkbox" value="None" id="onoff_source"
                               name="check" {{checked selected_scenario.check}}/>
                        <label for="onoff_source" onclick="change_source_status()"></label>

                    </div>
                    {{/is}}
                </td>
            </tr>
            </tbody>
        </table>
    </div>

    <div style="z-index: -10;top:64px; width: 98%; background-color: white; margin: 1%;">
        <table class="pure-table pure-table-horizontal" style="width: 100%; ">
            <tbody>
            <tr style="background-color:#000000;color: blue">
                <td>Then...</td>
                <td></td>

                <td class="is-right">
                    <img class="img-button" src="images/Add_S.png"  onclick="select_device_page()"/>
                    &nbsp;&nbsp;&nbsp;&nbsp;
                    <img class="img-button" src="images/Edit_S.png" onclick="show_edit()"></td>

            </tr>
            {{#each links.schedule}}
            {{#if target}}
            <tr id="tr_{{target.id}}">
                <td>
                    <img class="img-device-icon" src="{{model_icon_url target.fullmodelno}}">
                </td>
                <td class="is-left">
                    <div>{{target.name}}</div>
                    <div>{{target.seat}}</div>
                </td>
                <td class="is-left">
                    {{#is target.action "OnOff"}}
                    <div class="slideThree noedit">
                        <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">{{target.actions1}}</div>
                        <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">{{target.actions2}}</div>
                        <input type="checkbox" value="None" class="scenario_{{id}}" id="onoff_{{target.id}}"
                               name="check" {{checked target.check}}/>
                        <label for="onoff_{{target.id}}" onclick="change_arm_status({{target.id}})"></label></div>
                    {{/is}}
                    {{#is target.action "Trigger"}}
                    <div class="slideThree noedit">
                        <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">{{target.actions1}}</div>
                        <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">{{target.actions2}}</div>
                        <input type="checkbox" value="None" class="scenario_{{id}}" id="onoff_{{target.id}}"
                               name="check" {{checked target.check}}/>
                        <label for="onoff_{{target.id}}" onclick="change_arm_status({{target.id}})"></label></div>
                    {{/is}}
                    {{#is target.action "OpenCamera"}}
                    <div class="noedit">
                        <div>
                        <img onclick="change_cam_status({{target.id}},'record')" class="img-icon-1x"
                             id="record_{{target.id}}" src="{{imgchecked target.record}}">Record</div>
                        <div>
                        <img id="view_{{target.id}}" onclick="change_cam_status({{target.id}},'view')"
                             class="img-icon-1x" src="{{imgchecked target.view}}">View</div>
                    </div>
                    {{/is}}
                    <img onclick="del_scenario_device({{target.id}})" src="images/Cancel_R_N.png"
                         class="img-button edit" style="display: none">
                </td>
            </tr>
            {{/if}}
            {{/each}}
            </tbody>
        </table>
    </div>

    <div class="pure-g" style="margin-top: 15px;margin-bottom: 95px;">
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Cancel_N-N.png" onclick="show_scenario_page()"/>
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Save_N.png"
                 onclick="save_scenario();"
                    />
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
    </div>
</div>