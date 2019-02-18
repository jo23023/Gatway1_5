<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "System Setting"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 98%; background-color: white; margin: 1%;margin-bottom: 95px;">
    <table class="pure-table pure-table-horizontal" style="width: 100%; ">
        <tbody>
<!--         <tr onclick="show_setting_time_setup()">
            <td>{{i18n "Time Setup"}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr> -->
        <tr onclick="show_setting_ip_setup()">
            <td>{{i18n "IP Setup"}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        <tr onclick="show_setting_security_setup()">
            <td>{{i18n "Security Setup"}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        <tr onclick="show_setting_notification_setup()">
            <td>{{i18n "Notification Setup"}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        <tr onclick="show_setting_schedule_setup()">
            <td>{{i18n "Schedule Setup"}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        <tr onclick="show_Firmware_setup()">
            <td>{{i18n "Firmware Updates"}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        <tr onclick="show_simple_mode_setup()">
            <td>{{i18n "Simple Mode"}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        <tr onclick="show_info_setup()">
            <td>{{i18n "Info"}}</td>
            <td class="is-right"><img class="img-icon-1x" src="images/add_device/Next_symbol_N.png"></td>
        </tr>
        </tbody>
    </table>
</div>
