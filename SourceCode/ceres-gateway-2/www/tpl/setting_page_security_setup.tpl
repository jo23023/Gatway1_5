<div class="header" style="position:fixed;top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="show_setting_page_nav();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Security"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="position:absolute;z-index: -10; top:64px; width: 100%;">
<div style="z-index: -10; top:64px; width: 100%;">
    <div class="pure-form pure-form-stacked" style="margin-left: 1%;">
        <label for="password">{{i18n "Security Code Setting"}}</label>
        <fieldset class="pure-group">
            <input type="text" id="old_security_code" style="width: 98%" class="pure-input-1-2"
                   placeholder="{{i18n 'Old Security Code'}}">
            <input type="text" id="new_security_code" style="width: 98%" class="pure-input-1-2"
                   placeholder="{{i18n 'New Security Code'}}">
            <input type="text" id="confirm_security_code" style="width: 98%" class="pure-input-1-2"
                   placeholder="{{i18n 'Confirm Security Code'}}">
        </fieldset>
    </div>
</div>

<div style="z-index: -10; top:64px; width: 100%;">
    <div class="pure-form pure-form-stacked" style="margin-left: 1%;">
        <label for="password">{{i18n "Modify Admin Password"}}</label>
        <fieldset class="pure-group">
            <input type="text" id="old_password" style="width: 98%" class="pure-input-1-2"
                   placeholder="{{i18n 'Old Password'}}">
            <input type="text" id="new_password" style="width: 98%" class="pure-input-1-2"
                   placeholder="{{i18n 'New Password'}}">
            <input type="text" id="confirm_password" style="width: 98%" class="pure-input-1-2"
                   placeholder="{{i18n 'Confirm Password'}}">
        </fieldset>
    </div>
</div>



<div style="z-index: -10; top:64px; width: 97%;margin: 1%;">


        <label for="password">{{i18n "PIN Lock"}}</label>
            <table class="pure-table pure-table-horizontal" style="width: 100%;background-color: white; ">
                <tbody>
                <tr>
                    <td>
                        <div class="is-left"">
                            {{i18n "PIN Lock Enable"}}
                        </div>
                    </td>
                    <td class="is-right">
                        <div class="slideThree">
                            <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">ON</div>
                            <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">OFF</div>
                            <input type="checkbox" value="None" class="pinlock_toggle" id="slideThree" name="check" {{checked pinlock_enable}} />
                            <label onclick="event.cancelBubble=true;change_pinlock_enable();" for="slideThree"></label>
                        </div>
                    </td>
                </tr>
                <tr onclick="Pinlock_Setup()">
                    <td>
                        {{i18n "PIN Lock Setup"}}</td>
                    <td class="is-center"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
                </tr>
                </tbody>
            </table>

</div>


<div class="pure-g" style="margin-top: 15px;margin-bottom: 95px;">
    <div class="pure-u-1-5 is-center">
    </div>
    <div class="pure-u-1-5 is-center">
        <img class="img-icon-3x" src="images/Cancel_N-N.png" onclick="show_setting_page_nav()"/>
    </div>
    <div class="pure-u-1-5 is-center">
    </div>
    <div class="pure-u-1-5 is-center">
        <img class="img-icon-3x" src="images/Save_N.png"
             onclick="save_security();"
                />
    </div>
    <div class="pure-u-1-5 is-center">
    </div>
</div>

</div>