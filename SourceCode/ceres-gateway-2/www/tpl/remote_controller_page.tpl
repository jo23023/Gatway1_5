<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            {{#is simple_mode false}}
            <p><img src="images/Back_N.png" class="img-button" onclick="sm_force_to('status_page')"/></p>
            {{/is}}
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n title}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%;">

    <div class="pure-g" style="margin-top: 1em;text-align: center;">

        <div class="pure-md-1-4" style="margin-left: auto;margin-right: auto;">
            <img id="remote_lock_icon" class="pure-img-responsive" style="width: 20%; margin-top: 10%;"
                 onclick="remotekey_lock();" src="images/Arm_N.png">
        </div>
        <div class="pure-md-1-4" style="margin-left: auto;margin-right: auto;">
            <img id="remote_unlock_icon" class="pure-img-responsive" style="width: 20%;margin-top: 10%;"
                 onclick="remotekey_unlock();" src="images/Disarm_N.png">
        </div>
        <div class="pure-md-1-4" style="margin-left: auto;margin-right: auto;">
            <img id="remote_record_icon" class="pure-img-responsive" style="width: 20%;margin-top: 10%;"
                 onclick="remotekey_ipc_recording();" src="images/CameraRecording_N.png">
        </div>
        <div class="pure-md-1-4" style="margin-left: auto;margin-right: auto;">
            <img id="remote_siren_icon" class="pure-img-responsive" style="width: 20%;margin-top: 10%;"
                 onclick="remotekey_siren();"
                 src="images/Panic_N.png">
        </div>

    </div>
</div>


