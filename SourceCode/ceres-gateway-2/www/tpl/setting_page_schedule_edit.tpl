<div class="header" style="position:fixed;top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button back"
                    onclick="schedule({{selected_schedule_id}})"/></p>
        </div>
        <div class="pure-u-1-8 is-center">

        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Schedule Setup"}}</p></div>
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
                <td class="is-center">
                    <div style="float: left"><img class="img-button" src="{{imgchecked selected_schedule.week.sun}}"
                                                  id="sun" onclick="change_week('sun')"></div>
                    <div style="float: left">&nbsp;{{i18n "Sunday"}}</div>
                </td>
                <td class="is-center">
                    <div style="float: left"><img class="img-button" src="{{imgchecked selected_schedule.week.thr}}"
                                                  id="thr" onclick="change_week('thr')"></div>
                    <div style="float: left">&nbsp;{{i18n "Thursday"}}</div>
                </td>
            </tr>
            <tr>
                <td class="is-center">
                    <div style="float: left"><img class="img-button" src="{{imgchecked selected_schedule.week.mon}}"
                                                  id="mon" onclick="change_week('mon')"></div>
                    <div style="float: left">&nbsp;{{i18n "Monday"}}</div>
                </td>
                <td class="is-center">
                    <div style="float: left"><img class="img-button" src="{{imgchecked selected_schedule.week.fri}}"
                                                  id="fri" onclick="change_week('fri')"></div>
                    <div style="float: left">&nbsp;{{i18n "Friday"}}</div>
                </td>
            </tr>
            <tr>
                <td class="is-center">
                    <div style="float: left"><img class="img-button" src="{{imgchecked selected_schedule.week.tue}}"
                                                  id="tue" onclick="change_week('tue')"></div>
                    <div style="float: left">&nbsp;{{i18n "Tuesday"}}</div>
                </td>
                <td class="is-center">
                    <div style="float: left"><img class="img-button" src="{{imgchecked selected_schedule.week.sat}}"
                                                  id="sat" onclick="change_week('sat')"></div>
                    <div style="float: left">&nbsp;{{i18n "Saturday"}}</div>
                </td>
            </tr>
            <tr>
                <td class="is-center">
                    <div style="float: left"><img class="img-button" src="{{imgchecked selected_schedule.week.wed}}"
                                                  id="wed" onclick="change_week('wed')"></div>
                    <div style="float: left">&nbsp;{{i18n "Wednesday"}}</div>
                </td>
                <td class="is-center">

                </td>
            </tr>

            </tbody>
        </table>
    </div>

    <div style="z-index: -10; top:64px; width: 98%; background-color: white; margin: 1%;">
        <table class="pure-table pure-table-horizontal" style="width: 100%; ">
            <tbody>

            <tr>
                <td>
                    {{i18n "start time"}} : <br/><input style="width: 100%" type="text" readonly id="start-time"
                                                   value="{{selected_schedule.duration.On}}">
                </td>
            </tr>
            <tr>
                <td>
                    {{i18n "end time"}} : <br/><input style="width: 100%" type="text" readonly id="end-time" value="{{selected_schedule.duration.Off}}">
                </td>
            </tr>


            </tbody>
        </table>
    </div>

    <div class="pure-g" style="margin-top: 15px;margin-bottom: 95px;">
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center cancel">
            <img class="img-icon-3x" src="images/Cancel_N-N.png" onclick="schedule({{selected_schedule_id}})"/>
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
        <div class="pure-u-1-5 is-center">
            <img class="img-icon-3x" src="images/Save_N.png"
                 onclick="edit_schedule();"
                    />
        </div>
        <div class="pure-u-1-5 is-center">
        </div>
    </div>
</div>