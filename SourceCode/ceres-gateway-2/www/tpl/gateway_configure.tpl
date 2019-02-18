<div class="content" style="margin-top: 10%; margin-left: 10px; margin-right: 10px;">
    <div class="dialog" style="margin-bottom: 50px;">

        <div class="pure-form pure-form-aligned" style="margin: 20px;">
            <br>
            <div class="pure-control-group">
                <label for="name">{{ i18n "System Name:"}}</label>
                <input id="gw_name" style="width: 100%;" type="text" placeholder="System Name"
                       value="{{gateway_configure.name}}">
            </div>

            <div class="pure-control-group">
                <label for="did">{{i18n "System DID:"}}</label>
                <input id="gw_did" style="width: 100%;" type="text" placeholder="System DID"
                       value="{{gateway_configure.did}}">
            </div>

            <div class="pure-control-group">
                <label for="password">{{i18n "Password:"}}</label>
                <input id="gw_password" style="width: 100%;" type="password" placeholder="Password"
                       value="{{gateway_configure.password}}">
            </div>
        </div>

        <br>
        <div class="is-center">
            <p>
                <img class="img-icon-3x" src="images/next_button_n.png"
                     onclick="save_configure_gateway();"/>
            </p>
        </div>
        <br>
    </div>
</div>