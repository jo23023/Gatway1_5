<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center" onclick="sm.fire('back_to_status_page');">
            <p><img src="images/Back_N.png" class="img-button"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Add Device"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%;">

    <div class="pure-g" style="margin-top: 3em;">
        <div class="pure-u-2-5"></div>
        <div class="pure-u-1-5">
            <img class="pure-img-responsive" src="images/add_device/add_n.png">
        </div>
        <div class="pure-u-2-5"></div>
    </div>
    <br><br><br>

    <div style="margin-bottom: 3em;">
        <div class="pure-g">
            <div class="pure-u-1-5"></div>
            <div class="pure-u-1-5">
                <img onclick="show_add_device_step2_sensor();" class="pure-img-responsive"
                     src="images/add_device/Scensor_N.png">
            </div>
            <div class="pure-u-1-5"></div>
            <div class="pure-u-1-5">
                <img onclick="show_add_device_step2_camera();" class="pure-img-responsive"
                     src="images/add_device/Camera_N.png">
            </div>
            <div class="pure-u-1-5" style="height: 3em;"></div>
        </div>
    </div>

</div>
