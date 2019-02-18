<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-3 is-left">
            <p>
                <img src="images/Back_N.png" class="img-button back"
                     onclick="show_event_list()"/>
            </p>
        </div>
        <div class="pure-u-1-3 is-center topic"><p>{{ i18n "Event List"}}</p></div>
        <div class="pure-u-1-3">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 98%; margin: 1%;">
    <form class="pure-form pure-g">
        <div class="pure-u-1">
            <label>{{i18n "Start Time"}}</label>
        </div>
        <div class="pure-u-1">
            <input class="pure-input-1" id="start_date" type="text" readonly placeholder="{{i18n 'Date'}}" style="width: 100%">
        </div>


        <div class="pure-u-1">
            <label>{{i18n "Stop Time"}}</label>
        </div>
        <div class="pure-u-1">
            <input class="pure-input-1" id="stop_date" type="text" readonly placeholder="{{i18n 'Date'}}" style="width: 100%">
        </div>


        <br/>
        <br/>
        <br/>
        <div class="pure-u-1 is-center">
            <img src="images/Search_N.png" class="img-icon-3x"
                 onclick="get_eventlist_searchtime()"/>
        </div>
    </form>

    <br/>
    <br/>
</div>
