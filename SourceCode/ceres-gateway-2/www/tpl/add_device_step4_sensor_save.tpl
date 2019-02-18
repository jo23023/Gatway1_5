<div class="header" style="top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-3 is-center"></div>
        <div class="pure-u-1-3 is-center topic"><p>{{ i18n "step 3 of 3"}}</p></div>
        <div class="pure-u-1-3 is-center"></div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%;">

    <div class="pure-form pure-form-stacked" style="width:100%;">
        <label for="sensor-name">{{i18n "Device Name"}}</label>
        <input id="sensor-name" type="text" style="width: 100%;">

        <label for="sensor-seat">{{i18n "Location"}}</label>
        <input id="sensor-seat" type="text" style="width: 100%;">
    </div>

    <div class="pure-g" style="margin-top: 15px;margin-bottom:300px;">
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Cancel_N-N.png" onclick="show_add_device_step2()"/>
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Save_N.png"
                 onclick="save_item();"
                    />
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
    </div>
</div>
<div id="dialog" style="width:100%; height:auto; background-color: rgba(0,0,0,0.7);">

</div>