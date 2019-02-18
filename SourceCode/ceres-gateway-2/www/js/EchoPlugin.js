var exec = cordova.require('cordova/exec');

window.echo = function(action, str,  callback) {
    exec(callback, function(err) {
        callback('Nothing to echo.');
    }, "Echo", action, [str]);
};
