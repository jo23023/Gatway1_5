<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button"
                    onclick="show_setting_schedule_setup()"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Add_N.png" class="img-button" onclick="schedule_add()"/></p>
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Schedule Setup"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Edit_N.png" class="img-button"
                    onclick="show_schedule_del()"/>
            </p>
        </div>
    </div>
</div>


<div style="z-index: -10; position: absolute; top:64px; width: 98%; background-color: white; margin: 1%;margin-bottom: 95px;">
    <table class="pure-table pure-table-horizontal" style="width: 100%; ">
        <tbody>
        {{#each schedule}}
        <tr onclick="schedule_edit({{@index}})">
            <td class="is-center">
                <div>{{#each week}}{{@key}} {{/each}}</div>
                <div>{{duration.On}} - {{duration.Off}}</div>
            </td>

            <td onclick="event.cancelBubble=true;"><div class="slideThree">
                    <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">ON</div>
                    <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">OFF</div>
                    <input type="checkbox" value="None" class="schedule_{{@index}}" id="slideThree_{{@index}}" name="check" {{checked enable}} />
                    <label onclick="event.cancelBubble=true;change_schedule_status({{@index}});" for="slideThree_{{@index}}"></label></div>
            </td>
            <td class="is-right"><img style="display: none" class="img-icon-1x del" onclick="event.cancelBubble=true;del_schedule({{@index}})" src="images/Cancel_R_N.png"></td>
        </tr>
        {{/each}}
        </tbody>
    </table>
</div>