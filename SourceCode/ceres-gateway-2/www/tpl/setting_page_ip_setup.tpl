<div class="header" style="position:fixed;top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="show_setting_page_nav();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "IP Setup"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>
<div style="position:absolute;margin-bottom: 95px;top:64px;z-index:-10;margin: 1%;width: 98%;margin-top: 10px;">
    <div class="pure-g" style="background-color: white;padding: .5em 1em;margin: 1%;width: 98%;"
         onclick="change_dhcp()">
        <div class="pure-u-4-5" style="margin-top: 10px;">
            {{i18n "DHCP"}}
        </div>

        <div class="pure-u-1-5" style="margin-top: 10px;">
            <img id="dhcp" class="img-icon-1x" src="{{imgchecked ipsetup.dhcp_enable}}">
        </div>
    </div>
    <div style="z-index: -10; top:64px; width: 98%; background-color: white; margin: 1%;">

        <table class="pure-table pure-table-horizontal pure-form pure-form-stacked" style="width: 100%; ">
            <tbody>
            <tr>
                <td>
                    {{i18n "IP Address"}}
                    <br/>
                    <input style="width: 100%" id="ip" type="text" placeholder="192.168.1.100" ng-model="ipsetup.wan_ip">
                </td>

            </tr>
            <tr>
                <td>
                    {{i18n "Subnet Mask"}}
                    <br/>
                    <input style="width: 100%" id="netmask" type="text" placeholder="255.255.255.0" ng-model="ipsetup.wan_netmask">
                </td>

            </tr>
            <tr>
                <td>
                    {{i18n "Gatway"}}
                    <br/>
                    <input style="width: 100%" id="gateway" type="text" placeholder="192.168.1.1" ng-model="ipsetup.wan_gateway">
                </td>

            </tr>
            <tr>
                <td>
                    {{i18n "DNS Server"}}
                    <br/>
                    <input style="width: 100%" id="dns" type="text" placeholder="221.225.228.1" ng-model="ipsetup.wan_dns">
                </td>

            </tr>

            </tbody>
        </table>
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
                 onclick="save_ip();"
                    />
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
    </div>
</div>
