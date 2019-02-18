<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center">
            <p><img src="images/Back_N.png" class="img-button"
                    onclick="show_status_page()"/></p>
        </div>
        <div class="pure-u-1-8 is-center">
        </div>
        <div class="pure-u-1-2 is-center topic"><p>{{control_item.name}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div style="z-index: -10; position: absolute; top:64px; width: 100%; height:100%">

    <div class="pure-g" style="margin-top: 2em;text-align: center;">

        <div class="pure-md-1-4" style="margin-left: auto;margin-right: auto;">
            <img id="item_ctl_on" class="pure-img-responsive" style="width: 20%; margin-top: 10%;"
                 src="images/remote_controller/ON_N.png"
                    onclick="item_on('{{control_item.id}}')">
        </div>
        <div class="pure-md-1-4" style="margin-left: auto;margin-right: auto;">
            <img id="item_ctl_off" class="pure-img-responsive" style="width: 20%;margin-top: 10%;"
                 src="images/remote_controller/OFF_N.png"
                 onclick="item_off('{{control_item.id}}')">
        </div>
    </div>
</div>


