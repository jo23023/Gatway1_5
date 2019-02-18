<div class="header" style="position:fixed; top:0px; width: 100%; height: 64px;">
    <div class="pure-g top-menu">
        <div class="pure-u-1-8 is-center" onclick="stop_searching_event()">
            <p><img src="images/Back_N.png" class="img-button"/></p>
        </div>
        <div class="pure-u-1-8 is-center"></div>
        <div class="pure-u-1-2 is-center topic"><p>{{ i18n "Event List"}}</p></div>
        <div class="pure-u-1-8 is-center"><p></p></div>
        <div class="pure-u-1-8 is-center">
        </div>
    </div>
</div>

<div class="content is-center" style="z-index: -10; position: absolute; top:64px; width: 100%;"">

    <div id="part1">
        <h2 style="margin: 10%;">
            <span class="i18n">{{ i18n "Searching" }}</span>
        </h2>

        <div>
            <div class="spinner circles">
                <div></div>
                <div></div>
                <div></div>
                <div></div>
                <div></div>
                <div></div>
                <div></div>
                <div></div>
            </div>
        </div>
    </div>
</div>