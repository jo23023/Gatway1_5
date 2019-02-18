<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="show_add_device_step2();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Select Sensor"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 98%; background-color: white; margin: 1%;">
    <table class="pure-table pure-table-horizontal" style="width: 100%; ">
        <tbody>
        {{#each models_list}}
        <tr onclick="match_sensor('{{fullmodelno}}')">
            <td>
                <img class="img-device-icon" src="{{iconN}}">
            </td>
            <td>{{name}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        {{/each}}
        </tbody>
    </table>
</div>