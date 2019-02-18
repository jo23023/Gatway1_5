<div class="header" style="top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="show_add_device_step2();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Step 1"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%; ">
    <div class="pure-g" style="margin-top: 10px;  background-color: white;">
        <div class="pure-u-1-5 is-center" style="margin-top: 10px;">
            <img class="img-device-icon" src="images/Camera_N_N.png">
        </div>
    </div>

    <div class="pure-g" style="margin-top: 10px;  background-color: white;">
        <div class="pure-u-1 pure-form" style="">
            <div>
                {{ i18n "Camera Name" }}
            </div>
            <div>
                <input id="camera-name" type="text" placeholder="Camera Name" style="width: 100%;">
            </div>
        </div>
    </div>

    <div class="pure-g" style="background-color: white;">
        <div class="pure-u-1 pure-form" style="">
            <div>
                {{i18n "DID"}}
            </div>
            <div>
                <input id="camera-did" type="text" placeholder="" style="width: 100%;">
            </div>
        </div>
    </div>

    <div class="pure-g" style="background-color: white;">
        <div class="pure-u-1 pure-form" style="">
            <div>
                {{i18n "Password"}}
            </div>
            <div>
                <input id="camera-password" type="text" placeholder="" style="width: 100%;">
            </div>
        </div>
    </div>

    <div class="pure-g" style="background-color: white;">
        <div class="pure-u-1 pure-form" style="">
            <div>
                {{i18n "Location"}}
            </div>
            <div>
                <input id="camera-location" type="text" placeholder="Living Room" style="width: 100%;">
            </div>
        </div>
    </div>

    <div class="pure-g" style="margin-top: 15px;margin-bottom:90px;">
        <div class="pure-u-1-3 is-center">
            <img class="img-icon-3x" src="images/Search_N.png"
                 onclick="search_ipc_lan();"/>
        </div>
        <div class="pure-u-1-3 is-center">
            <img class="img-icon-3x" src="images/Cancel_N-N.png"
                 onclick="show_add_device_step2();"/>
        </div>
        <div class="pure-u-1-3 is-center">
            <img class="img-icon-3x" src="images/Save_N.png"
                    onclick="save_ipc();"/>
        </div>
    </div>
</div>

<div id="dialog" style="width:100%; height:auto; background-color: rgba(0,0,0,0.7);">

</div>