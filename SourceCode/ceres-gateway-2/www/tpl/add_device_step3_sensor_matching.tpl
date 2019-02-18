<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-3 is-center"></div>
        <div class="pure-u-1-3 is-center topic"><p id="add_sensor_title">{{ i18n "step 1 of 3"}}</p></div>
        <div class="pure-u-1-3 is-center">
        </div>
    </div>
</div>
<div class="content is-center" style="z-index: -10; position: absolute; top:64px; width: 100%; ">
    <div id="matching_part">
        <h2 style="margin: 10%;">
            <span class="i18n">{{ i18n "Matching" }}</span>
        </h2>

        <div>
            <div class="spinner circles">
                <div></div>
                <div></div>
                <div></div>
                <div></div>
                <div></div>
                <div></div>
                <div></div>
                <div></div>
            </div>
        </div>
    </div>

    <h2 style="margin: 10%;">
        <span id="timeout_text">{{ i18n "match prompt" }}</span>
    </h2>

    <div class="pure-g" style="margin-bottom: 90px;">
        <div class="pure-u-1-5"></div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Cancel_N_S.png"
                 onclick="show_add_device_step2();"/>
        </div>
        <div class="pure-u-1-5"></div>

        <div class="pure-u-1-5 is-center">
            <img id="add_device_pair" class="img-icon-3x" src="images/Pair_N.png"
                 onclick="start_pair('{{fullmodelno}}')"/>
        </div>

        <div class="pure-u-1-5"></div>
    </div>
</div>