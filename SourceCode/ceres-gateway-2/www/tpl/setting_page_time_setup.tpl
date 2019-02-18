<div class="header" style="position:fixed;top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button back" onclick="show_setting_page_nav();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Time Setup"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="position:absolute;z-index: -10; top:64px; width: 98%;">
    <div style="z-index: -10; top:64px; width: 98%; background-color: white; margin: 1%;">
        <table class="pure-table pure-table-horizontal" style="width: 100%; ">
            <tbody>
            <tr>
                <td>
                    {{i18n "Gateway Time"}}
                    <br/>
                    <input id="gateway-date" type="text" style="width: 200px;" readonly value="{{timesetup.date}}">

                </td>


            </tr>
            <tr onclick="use_current_time()">
                <td><img id="current_time" class="img-icon-1x" src="images/Confirm_N.png">{{i18n "Use current time"}}
                </td>

            </tr>

            <tr>
                <td>{{i18n "Time Zone"}}
                    <br/>
                    <select id="selected_timezone" style="width: 250px">
                        {{#each timezones}}
                        <option value="{{value}}" title="{{value}}" {{selected value}}>{{name}}</option>
                        {{/each}}
                    </select>

                </td>
            </tr>

            <tr onclick="NTPServer()">
                <td><img id="ntp" class="img-icon-1x" src="{{imgchecked NTPServer}}">{{i18n "NTP Server"}}</td>

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
                 onclick="save_time();"
                    />
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
    </div>

</div>