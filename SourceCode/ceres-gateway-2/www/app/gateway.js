var gateway = (function () {

    var init = function() {
        console.log("gateway module init");
    };

    var destroy = function() {
        console.log("gateway module destroy");
    };

    var isCamera = function (item) {
        return models_dict[item.fullmodelno].type === 'Camera';
    };

    var isReceiver = function (item) {
        return models_dict[item.fullmodelno].mode === 'Receiver';
    };

    var isSender = function (item) {
        return models_dict[item.fullmodelno].mode === 'Sender' && models_dict[item.fullmodelno].type != "RemoteKey";
    };

    var isRemoteKey = function (item){
        return models_dict[item.fullmodelno].mode === 'Sender' && models_dict[item.fullmodelno].type == "RemoteKey";
    };

    var remove_item = function (item_id) {
        console.log("remove_item: " + item_id);
    };

    return {
        isCamera: isCamera,
        isReceiver: isReceiver,
        isSender: isSender,
        isRemoteKey: isRemoteKey,
        remove_item: remove_item
    };
})
(window);


