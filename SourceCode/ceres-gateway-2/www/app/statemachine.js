/**
 * Created by yulin on 14-7-31.
 */

function statemachine_init() {
    console.log("StateMachine is inited!");
    sm = new core.StateMachine({
        start: "starter",
        states: {
            starter: {
                enter: function (ev) {
                    console.log("entering state " + ev.to);
                }, leave: function (ev) {
                    console.log("leaving state from " + ev.from);
                }
            },
            gateway_list_page: {
                enter: function (ev) {
                    console.log("entering state " + ev.to);
                }, leave: function (ev) {
                    console.log("leaving state from " + ev.from);
                }
            },
            gateway_wizard_step1: {
                enter: function (ev) {
                    console.log("entering state " + ev.to);
                }, leave: function (ev) {
                    console.log("leaving state from " + ev.from);
                }
            },
            status_page: {
                enter: function (ev) {
                    console.log("entering state " + ev.to);
                }, leave: function (ev) {
                    console.log("leaving state from " + ev.from);
                }
            },
            remote_controller_page: {
            },
            remote_controller_page_receiver: {
            },
            add_device_starter: {
                enter: function () {
                    sm.fire("to_add_device_step1");
                }
            },
            add_device_step1: {
                enter: function (ev) {
                    console.log("entering state " + ev.to);
                }, leave: function (ev) {
                    console.log("leaving state from " + ev.from);
                }
            },
            add_device_step2_sensor: {
            },
            add_device_step2_camera: {
            },

            control_page_starter: {

            },

            control_page_list: {

            },

            event_page: {
            },
            scenario_page_starter: {
            },
            setting_page_starter: {
                enter: function () {
                    sm.fire("to_setting_page_verify_pwd");
                }
            },
            setting_page_verify_pwd: {
                enter: function () {
                    console.log("setting_page_verify_pwd");
                }
            }
        },
        transitions: {
            back_to_status_page: {
                from: ["status_page",
                    "add_device_step1", "setting_page_verify_pwd",
                    "control_page_list", "control_page_starter"],
                to: "status_page"
            },
            to_gateway_list_page: {
                from: ["starter", 
                        "status_page", 
                        "gateway_wizard_step1",
                        "gateway_list_page", 
                        "gateway_wizard_step1", 
                        "remote_controller_page", 
                        "remote_controller_page_receiver", 
                        "add_device_starter", 
                        "add_device_step1", 
                        "add_device_step2_sensor", 
                        "add_device_step2_camera", 
                        "control_page_starter", 
                        "control_page_list", 
                        "event_page",
                        "scenario_page_starter", 
                        "setting_page_starter", 
                        "setting_page_verify_pwd"],
                to: "gateway_list_page"
            },
            to_gateway_wizard_step1: {
                from: ["starter", "gateway_list_page"],
                to: "gateway_wizard_step1"
            },
            to_status_page: {
                from: ["starter", 
                        "gateway_list_page", 
                        "gateway_wizard_step1", 
                        "remote_controller_page", 
                        "remote_controller_page_receiver", 
                        "add_device_starter", 
                        "add_device_step1", 
                        "add_device_step2_sensor", 
                        "add_device_step2_camera", 
                        "control_page_starter", 
                        "control_page_list", 
                        "event_page",
                        "scenario_page_starter", 
                        "setting_page_starter", 
                        "setting_page_verify_pwd"],
                to: "status_page"
            },
            to_remote_controller_page: {
                from: ["status_page", "gateway_list_page"],
                to: "remote_controller_page"
            },
            to_remote_controller_page_receiver: {
                from: "status_page", to: "remote_controller_page_receiver"
            },
            to_add_device_starter: {
                from: "status_page", to: "add_device_starter"
            },
            to_add_device_step1: {
                from: ["add_device_starter", "add_device_step2_sensor", "add_device_step2_camera"],
                to: "add_device_step1"
            },
            to_add_device_step2_sensor: {
                from: "add_device_step1", to: "add_device_step2_sensor"
            },
            to_add_device_step2_camera: {
                from: "add_device_step1", to: "add_device_step2_camera"
            },

            to_control_page_starter: {
                from: ["starter", "status_page", ""],
                to: "control_page_starter"
            },

            to_control_page_list: {
                from: "control_page_starter", to: "control_page_list"
            },
            to_event_page: {
                from: "status_page", to: "event_page"
            },
            to_setting_page_starter: {
                from: "status_page", to: "setting_page_starter"
            },
            to_setting_page_verify_pwd: {
                from: "setting_page_starter", to: "setting_page_verify_pwd"
            }

        }
    });

    gateway_wizard_sm_init();
    gateway_list_page_sm_init();
    status_page_sm_init();
    add_device_page_sm_init();
    control_page_sm_init();
    setting_page_sm_init();

    remote_controller_page_sm_init();

    event_page_sm_init();
    scenario_page_sm_init();

}