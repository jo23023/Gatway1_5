<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center" onclick="sm.fire('to_gateway_wizard_step1');">
            <p><img src="images/Add_N.png" class="img-button"/></p>
        </div>
        <div class="pure-u-1-8 is-center"></div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "System"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Edit_N.png" class="img-button"
                    onclick="gateway_list_page_enter_edit_mode()"/>
            </p>
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 98%;  margin: 1%;">
    <table class="pure-table pure-table-horizontal" style="width: 100%;background-color: white;">
        <tbody>
        {{#each gateway_list}}
        <tr onclick="connect_to_gateway('{{did}}','{{password}}')">
            <td>
                <div style="color: #000000; font-size: 20px;">
                    {{name}}
                </div>
                <div style="color: gray">
                    {{did}}
                </div>
            </td>
            <td class="is-right" style="" id="reset_{{did}}"><div style="visibility: hidden;height: 4em;line-height: 4em;overflow: hidden;color: green;">OK</div></td>
            <td class="is-right not-edit-button"><img class="img-icon-1x" src="images/Link_N.png"></td>
            <td class="is-right edit-button"
                style="display: none;">
                <img class="img-icon-1x" src="images/Setting_N.png" onclick="event.cancelBubble=true; configure_gateway('{{name}}','{{did}}','{{password}}')">
                <img class="img-icon-1x" src="images/Cancel_R_N.png" onclick="event.cancelBubble=true; remove_gateway('{{did}}','{{password}}');">
            </td>
        </tr>
        {{/each}}
        </tbody>
    </table>
    <br/><br/><br/>

    <div id="reset_btn" class="is-center" style="display: none">
        <img class="img-icon-2x" src="images/reset.png" onclick="reset_gateway()">
    </div>

    <br/><br/>
</div>

