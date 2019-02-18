<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Verify Admin Password"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%; height=100%">
    <div class="pure-form pure-form-stacked" style="margin-left: 1%;">
        <label for="setting_password">{{ i18n "Password" }}</label>
        <input style="width: 98%;" id="setting_password" type="password" placeholder="Password">

        <button style="width: 98%;" onclick="event.cancelBubble=true;show_setting_page_nav();" href="#"
                class="pure-button pure-button-primary pure-u-1">{{ i18n
            "Confirm"}}
        </button>
    </div>
</div>
