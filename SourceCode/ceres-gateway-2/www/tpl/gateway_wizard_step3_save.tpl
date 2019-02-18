<div class="content" style="margin-top: 10%; margin-left: 10px; margin-right: 10px;">
    <div class="is-center">
        <p>{{i18n "save gateway prompt1"}}</p>
    </div>

    <div class="dialog" style="margin-bottom: 50px;">
        <div class="pure-form pure-form-stacked" style="margin: 20px;">
            <br>

            <div class="pure-control-group">
                <label for="name">{{ i18n "System Name:"}}</label>
                <input id="name" style="width: 100%;" type="text" placeholder="System Name">
            </div>

            <div class="pure-control-group">
                <label for="did">{{ i18n "System DID:"}}</label>
                <input id="did" style="width: 100%;" type="text" placeholder="System DID">
            </div>

            <div class="pure-control-group">
                <label for="password">{{i18n "Password:"}}</label>
                <input id="password" style="width: 100%;" type="password" placeholder="Password">
            </div>
        </div>

        <br>
        <div class="pure-g" style="margin-top: 15px;margin-bottom: 95px;">
            <div class="pure-u-1-5 is-center"></div>
            <div class="pure-u-1-5 is-center">
                <img class="img-icon-3x" src="images/rescan_button_n.png" onclick="show_gateway_wizard_step1();"/></div>
            <div class="pure-u-1-5 is-center"></div>
            <div class="pure-u-1-5 is-center">
                <img class="img-icon-3x" src="images/add_gateway/next_button_n.png" onclick="save_gateway();"/></div>
            <div class="pure-u-1-5 is-center"></div>
        </div>

        <br>
    </div>
</div>
<div class="is-center" style="position:fixed; bottom:0px; width: 100%;">
<img class="img-icon-3x" src="images/step3.png">
</div>