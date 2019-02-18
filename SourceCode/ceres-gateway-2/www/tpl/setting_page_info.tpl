<div class="header" style="position:fixed;top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="show_setting_page_nav();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "About"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="position:absolute;z-index: -10; top:64px; width: 98%; margin: 1%;margin-bottom: 95px;">
    <div style="background-color: white; ">
        <label style="color: #000000;">
            <p>{{i18n "F/W Information"}}</p>
            <p>{{i18n "Version:"}}{{gw_version}}</p>
            <p>{{i18n "DID:"}}{{gw_did}}</p>
        </label>
    </div>
    <div style="background-color: white;">
        <label style="color: #000000;">
            <p>{{i18n "App Information"}}</p>
            <p>{{i18n "Version:"}}{{app_version}}</p>
        </label>
    </div>
</div>

<!--
<div style="z-index: -10; top:64px; width: 98%; margin: 1%;">
    <div style="background-color: white;">
        <table class="pure-table pure-table-horizontal" style="width: 100%">
            <tbody>
            <tr>
                <td>{{i18n "F/W Information"}}</td>
                <td></td>
            </tr>

            <tr>
                <td>{{i18n "Version:"}}</td>
                <td>V1.0.0</td>
            </tr>

            <tr>
                <td>{{i18n "DID:"}}</td>
                <td>CHXX-0000001-ABCDE</td>
            </tr>
            </tbody>
        </table>
    </div>
    <br/>
    <div style="background-color: white;">
        <table class="pure-table pure-table-horizontal" style="width: 100%">
            <tbody>
            <tr>
                <td>{{i18n "App Information"}}</td>
                <td></td>
            </tr>

            <tr>
                <td>{{i18n "Version:"}}</td>
                <td>V1.0.0</td>
            </tr>
            <tr>
                <td></td>
                <td></td>
            </tr>
            </tbody>
        </table>
    </div>-->
</div>

