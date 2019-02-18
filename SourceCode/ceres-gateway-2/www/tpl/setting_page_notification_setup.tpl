<div class="header" style="position:fixed;top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="show_setting_page_nav();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Notification Setup"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>
<div style="position:absolute;margin-bottom: 95px;top:64px;z-index:-10;margin: 1%;width: 98%;margin-top: 10px;">
    <div class="pure-g" style="background-color: white;padding: .5em 1em;margin: 1%;width: 98%;"
         onclick="change_email_state()">
        <div class="pure-u-2-5" style="margin-top: 10px;height: 4em;line-height: 4em;overflow: hidden;">
            {{i18n "Use Email"}}
        </div>

        <div class="pure-u-3-5" style="margin-top: 10px;">
            <div class="slideThree is-left">
                <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">ON</div>
                <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">OFF</div>
                <input type="checkbox" value="None" class="scenario_{{id}}" id="email" name="check" {{checked email_enable}} />
                <label onclick="event.cancelBubble=true;" for="email"></label></div>
        </div>
    </div>
    <div style="z-index: -10; top:64px; width: 98%; background-color: white; margin: 1%;">

        <table class="pure-table pure-table-horizontal pure-form pure-form-stacked" style="width: 100%; ">
            <tbody>
            <tr>
                <td>
                    {{i18n "Receiver"}}
                    <br/>
                    <input id="receiver" type="text" placeholder="your@mail.com" style="width: 100%; ">
                </td>

            </tbody>
        </table>
    </div>

    <div class="pure-g" style="background-color: white;padding: .5em 1em;margin: 1%;width: 98%;"
         onclick="change_notification_state()">
        <div class="pure-u-2-5" style="margin-top: 10px;height: 4em;line-height: 4em;overflow: hidden;">
            {{i18n "Use Notification"}}
        </div>

        <div class="pure-u-3-5" style="margin-top: 10px;">
            <div class="slideThree is-left">
                <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">ON</div>
                <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">OFF</div>
                <input type="checkbox" value="None" class="scenario_{{id}}" id="notification" name="check" {{checked notification_enable}} />
                <label onclick="event.cancelBubble=true;" for="notification"></label></div>
        </div>
    </div>

    <div class="pure-g" style="margin-top: 15px;margin-bottom: 74px;">
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Cancel_N-N.png" onclick="show_setting_page_nav()"/>
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Save_N.png"
                 onclick="save_message();"
                    />
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
    </div>
</div>
