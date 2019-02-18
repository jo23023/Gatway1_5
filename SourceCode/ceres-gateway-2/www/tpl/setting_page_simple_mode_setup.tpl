<div class="header" style="top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button" onclick="show_setting_page_nav();"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{i18n "Simple Mode"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; top:64px; width: 98%; background-color: white; margin: 1%;">
    <div class="is-center">
        <p>{{i18n "simple mode prompt1"}}</p>
    </div>

    <br/>

    <div style="height: 50px; line-height: 50px">
        <div class="is-left" style="float: left; ">
            <label style="color:#000000; font-size: 25px; padding-left: 10px; ">{{i18n "Simple Mode"}}</label>
        </div>

        <div class="slideThree" style="float: right;">
            <div style="color: #00bf00;position: absolute;left: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;">ON</div>
            <div style="color: #000;position: absolute;right: 10px;z-index: 0;font: 12px/26px Arial, sans-serif;font-weight: bold;text-shadow: 1px 1px 0px rgba(255, 255, 255, 0.15);">OFF</div>
            <input type="checkbox" value="None" class="sm_toggle" id="slideThree" name="check" {{checked simple_mode}} />
            <label onclick="event.cancelBubble=true;change_simple_mode();" for="slideThree"></label>
        </div>
    </div>
</div>

