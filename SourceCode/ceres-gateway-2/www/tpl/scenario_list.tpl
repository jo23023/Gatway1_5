<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-3 is-center">
        </div>
        <div class="pure-u-1-3 is-center topic"><p>{{i18n "Scenario Setup"}}</p></div>
        <div class="pure-u-1-3 is-center">
        </div>
    </div>
</div>


<div style="z-index: -10; position: absolute; top:64px; width: 98%; background-color: white; margin: 1%;margin-bottom: 95px;">
    <table class="pure-table pure-table-horizontal" style="width: 100%; ">
        <tbody>
{{#each sender_list}}
<tr onclick="scenario({{id}},'sender')">
<td>
<img class="img-device-icon" src="{{model_icon_url fullmodelno}}">
            </td>
            <td class="is-left"><div>{{name}}</div><div>{{seat}}</div></td>
    <td onclick="event.cancelBubble=true;"><div class="slideThree">
            <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">ON</div>
            <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">OFF</div>
        <input type="checkbox" value="None" class="scenario_{{id}}" id="slideThree_{{id}}" name="check" {{checked enable}} />
        <label onclick="event.cancelBubble=true;change_scenario_status({{id}});" for="slideThree_{{id}}"></label></div>
    </td>

</tr>
{{/each}}

{{#each camera_list}}
<tr onclick="scenario({{id}},'camera')">
    <td>
        <img class="img-device-icon" src="{{model_icon_url fullmodelno}}">
    </td>
    <td class="is-left"><div>{{name}}</div><div>{{seat}}</div></td>
    <td onclick="event.cancelBubble=true;"><div class="slideThree">
            <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">ON</div>
            <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">OFF</div>
            <input type="checkbox" value="None" class="scenario_{{id}}" id="slideThree_{{id}}" name="check" {{checked enable}} />
            <label onclick="event.cancelBubble=true;change_scenario_status({{id}});" for="slideThree_{{id}}"></label></div>
    </td>

</tr>
{{/each}}
</tbody>
</table>
</div>