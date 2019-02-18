<div class="header" style="top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="show_status_page();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Step 2"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%;">

    <div class="pure-form pure-form-stacked" style="width:100%;">
        <label for="sensor-name">{{i18n "Sensor Name"}}</label>
        <input id="sensor-name" type="text" style="width: 100%;"
                value="{{confiure_item.name}}">

        <label for="sensor-seat">{{i18n "Location"}}</label>
        <input id="sensor-seat" type="text" style="width: 100%;"
                value="{{confiure_item.seat}}">
    </div>

    <div class="pure-g" style="margin-top: 15px;">
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Cancel_N-N.png"
                 onclick="show_status_page();"/>
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Save_N.png"
                 onclick="update_sensor();show_status_page();"
                    />
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
    </div>
</div>
<div id="dialog" style="width:100%; height:auto; background-color: rgba(0,0,0,0.7);">

</div>