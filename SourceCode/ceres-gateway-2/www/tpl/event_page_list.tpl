<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center" onclick="show_event_page_search_time()">
            <p><img src="images/Search_N.png" class="img-button"/></p>
        </div>
        <div class="pure-u-1-8 is-center"></div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Event List"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>


<div style="z-index: -10; position: absolute; top:64px; width: 98%; margin: 1%; ">
    <table class="pure-table pure-table-horizontal" style="width: 100%;background-color: white; border: 0px;margin-bottom: 95px;">
        <tbody>
        {{#each event_list}}
        <tr onclick="event_item_clicked('{{t}}', '{{source.id}}', '{{source.did}}', '{{source.utctime}}');">
            <td>
                <img class="img-device-icon" src="images/model/{{source.icon}}_N.png">
            </td>
            <td>{{time_convert t}} {{source.name}} {{i18n source.value}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        {{/each}}
        </tbody>
    </table>
</div>
