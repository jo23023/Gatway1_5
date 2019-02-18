<div class="header" style="top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center"></div>
        <div class="pure-u-1-8 is-center"></div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Firmware Updates"}}</p></div>
        <div class="pure-u-1-8 is-center"></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div class="is-center">
    <p id="udpates_prompt" style="display: none">{{i18n "Checking Updates..."}}</p>
</div>

<div class="pure-g" style="margin-top: 15px;">
    <div class="pure-u-1-5 is-center">
    </div>
    <div class="pure-u-1-5 is-center">
        <img class="img-icon-3x" src="images/Cancel_N_S.png" onclick="show_setting_page_nav()"/>
    </div>
    <div class="pure-u-1-5 is-center">
    </div>
    <div class="pure-u-1-5 is-center">
        <img id="updates_icon" class="img-icon-3x" src="images/Updates_N.png"
             onclick="check_firmware_updates();"
                />
    </div>
    <div class="pure-u-1-5 is-center">
    </div>
</div>

</div>