<!-- 选择 搜索 到的 IPC -->
<div id="ipc_search_result_dialog" style="float:left; z-index: 999; width:95%; ; background-color: white; margin-top:
 20%; margin-left: 2.5%; border-radius: 5px;">
    <div>
        <p style="margin-left:10px; color: rgba(143, 211, 245, 255);">
            {{i18n "Select one camera" }}
        </p>
    </div>
    {{#each ipc_search_list}}
    <div style="border-bottom: 1px solid black; width: 100%;"></div>
    <div class="pure-g" style="width: 100%; margin-top: 10px; margin-bottom: 10px;"
         onclick="add_device_camera_choose('{{did}}')">
        <div class="pure-u-4-5"
             style="padding-left:1em; color: black; height: 4em;line-height: 4em;overflow: hidden;">{{did}}
        </div>
        <div class="pure-u-1-5">
            <div style="float: left; z-index:90;">
                <img class="img-icon-2x" src="images/Circle.png"
                     onclick="$('#{{did}}_confirm').show();">
            </div>
            <div style="position:absolute; z-index: 99; float: left; clear:left; display: none;"
                 id="{{did}}_confirm">
                <img class="img-icon-2x" src="images/confirm_symbol.png">
            </div>
        </div>
    </div>
    {{/each}}
    <div style="border-bottom: 1px solid white; width: 100%;"></div>
    <div class="pure-g">
        <div class="pure-u-1-3 is-center">
        </div>
        <div class="pure-u-1-3 is-center">
            <img class="img-icon-3x" src="images/Cancel_N_N_N.png" onclick="$('#dialog').hide()"/>
        </div>
        <div class="pure-u-1-3 is-center">
        </div>
    </div>
</div>