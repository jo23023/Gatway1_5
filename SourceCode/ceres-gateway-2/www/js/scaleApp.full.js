/*!
scaleapp - v0.4.5 - 2014-07-30
This program is distributed under the terms of the MIT license.
Copyright (c) 2011-2014 Markus Kohlhase <mail@markus-kohlhase.de>
*/
(function() {
  var CACHE_PREFIX, Core, DEFAULT_PATH, DEFAULT_PORT, DEFAULT_PROTOCOL, Mediator, api, argRgx, checkType, clearData, create_connection_obj, doForAll, fnRgx, getArgumentNames, get_bosh_url, hasConnectionData, key2cache, plugin, restoreData, runParallel, runSeries, runWaterfall, saveData, statusCodeToString, util, _base,
    __slice = [].slice,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
    __indexOf = [].indexOf || function(item) { for (var i = 0, l = this.length; i < l; i++) { if (i in this && this[i] === item) return i; } return -1; },
    __bind = function(fn, me){ return function(){ return fn.apply(me, arguments); }; };

  fnRgx = /function[^(]*\(([^)]*)\)/;

  argRgx = /([^\s,]+)/g;

  getArgumentNames = function(fn) {
    var _ref;
    return ((fn != null ? (_ref = fn.toString().match(fnRgx)) != null ? _ref[1] : void 0 : void 0) || '').match(argRgx) || [];
  };

  runParallel = function(tasks, cb, force) {
    var count, errors, hasErr, i, results, t, _i, _len, _results;
    if (tasks == null) {
      tasks = [];
    }
    if (cb == null) {
      cb = (function() {});
    }
    count = tasks.length;
    results = [];
    if (count === 0) {
      return cb(null, results);
    }
    errors = [];
    hasErr = false;
    _results = [];
    for (i = _i = 0, _len = tasks.length; _i < _len; i = ++_i) {
      t = tasks[i];
      _results.push((function(t, i) {
        var e, next;
        next = function() {
          var err, res;
          err = arguments[0], res = 2 <= arguments.length ? __slice.call(arguments, 1) : [];
          if (err) {
            errors[i] = err;
            hasErr = true;
            if (!force) {
              return cb(errors, results);
            }
          } else {
            results[i] = res.length < 2 ? res[0] : res;
          }
          if (--count <= 0) {
            if (hasErr) {
              return cb(errors, results);
            } else {
              return cb(null, results);
            }
          }
        };
        try {
          return t(next);
        } catch (_error) {
          e = _error;
          return next(e);
        }
      })(t, i));
    }
    return _results;
  };

  runSeries = function(tasks, cb, force) {
    var count, errors, hasErr, i, next, results;
    if (tasks == null) {
      tasks = [];
    }
    if (cb == null) {
      cb = (function() {});
    }
    i = -1;
    count = tasks.length;
    results = [];
    if (count === 0) {
      return cb(null, results);
    }
    errors = [];
    hasErr = false;
    next = function() {
      var e, err, res;
      err = arguments[0], res = 2 <= arguments.length ? __slice.call(arguments, 1) : [];
      if (err) {
        errors[i] = err;
        hasErr = true;
        if (!force) {
          return cb(errors, results);
        }
      } else {
        if (i > -1) {
          results[i] = res.length < 2 ? res[0] : res;
        }
      }
      if (++i >= count) {
        if (hasErr) {
          return cb(errors, results);
        } else {
          return cb(null, results);
        }
      } else {
        try {
          return tasks[i](next);
        } catch (_error) {
          e = _error;
          return next(e);
        }
      }
    };
    return next();
  };

  runWaterfall = function(tasks, cb) {
    var i, next;
    i = -1;
    if (tasks.length === 0) {
      return cb();
    }
    next = function() {
      var err, res;
      err = arguments[0], res = 2 <= arguments.length ? __slice.call(arguments, 1) : [];
      if (err != null) {
        return cb(err);
      }
      if (++i >= tasks.length) {
        return cb.apply(null, [null].concat(__slice.call(res)));
      } else {
        return tasks[i].apply(tasks, __slice.call(res).concat([next]));
      }
    };
    return next();
  };

  doForAll = function(args, fn, cb, force) {
    var a, tasks;
    if (args == null) {
      args = [];
    }
    tasks = (function() {
      var _i, _len, _results;
      _results = [];
      for (_i = 0, _len = args.length; _i < _len; _i++) {
        a = args[_i];
        _results.push((function(a) {
          return function(next) {
            return fn(a, next);
          };
        })(a));
      }
      return _results;
    })();
    return util.runParallel(tasks, cb, force);
  };

  util = {
    doForAll: doForAll,
    runParallel: runParallel,
    runSeries: runSeries,
    runWaterfall: runWaterfall,
    getArgumentNames: getArgumentNames,
    hasArgument: function(fn, idx) {
      if (idx == null) {
        idx = 1;
      }
      return util.getArgumentNames(fn).length >= idx;
    }
  };

  Mediator = (function() {
    function Mediator(obj, cascadeChannels) {
      this.cascadeChannels = cascadeChannels != null ? cascadeChannels : false;
      this.channels = {};
      if (obj instanceof Object) {
        this.installTo(obj);
      } else if (obj === true) {
        this.cascadeChannels = true;
      }
    }

    Mediator.prototype.on = function(channel, fn, context) {
      var id, k, subscription, that, v, _base, _i, _len, _results, _results1;
      if (context == null) {
        context = this;
      }
      if ((_base = this.channels)[channel] == null) {
        _base[channel] = [];
      }
      that = this;
      if (channel instanceof Array) {
        _results = [];
        for (_i = 0, _len = channel.length; _i < _len; _i++) {
          id = channel[_i];
          _results.push(this.on(id, fn, context));
        }
        return _results;
      } else if (typeof channel === "object") {
        _results1 = [];
        for (k in channel) {
          v = channel[k];
          _results1.push(this.on(k, v, fn));
        }
        return _results1;
      } else {
        if (typeof fn !== "function") {
          return false;
        }
        if (typeof channel !== "string") {
          return false;
        }
        subscription = {
          context: context,
          callback: fn
        };
        return {
          attach: function() {
            that.channels[channel].push(subscription);
            return this;
          },
          detach: function() {
            Mediator._rm(that, channel, subscription.callback);
            return this;
          }
        }.attach();
      }
    };

    Mediator.prototype.off = function(ch, cb) {
      var id;
      switch (typeof ch) {
        case "string":
          if (typeof cb === "function") {
            Mediator._rm(this, ch, cb);
          }
          if (typeof cb === "undefined") {
            Mediator._rm(this, ch);
          }
          break;
        case "function":
          for (id in this.channels) {
            Mediator._rm(this, id, ch);
          }
          break;
        case "undefined":
          for (id in this.channels) {
            Mediator._rm(this, id);
          }
          break;
        case "object":
          for (id in this.channels) {
            Mediator._rm(this, id, null, ch);
          }
      }
      return this;
    };

    Mediator.prototype.emit = function(channel, data, cb) {
      var chnls, sub, subscribers, tasks;
      if (cb == null) {
        cb = function() {};
      }
      if (typeof data === "function") {
        cb = data;
        data = void 0;
      }
      if (typeof channel !== "string") {
        return false;
      }
      subscribers = this.channels[channel] || [];
      tasks = (function() {
        var _i, _len, _results;
        _results = [];
        for (_i = 0, _len = subscribers.length; _i < _len; _i++) {
          sub = subscribers[_i];
          _results.push((function(sub) {
            return function(next) {
              var e;
              try {
                if (util.hasArgument(sub.callback, 3)) {
                  return sub.callback.apply(sub.context, [data, channel, next]);
                } else {
                  return next(null, sub.callback.apply(sub.context, [data, channel]));
                }
              } catch (_error) {
                e = _error;
                return next(e);
              }
            };
          })(sub));
        }
        return _results;
      })();
      util.runSeries(tasks, (function(errors, results) {
        var e, x;
        if (errors) {
          e = new Error(((function() {
            var _i, _len, _results;
            _results = [];
            for (_i = 0, _len = errors.length; _i < _len; _i++) {
              x = errors[_i];
              if (x != null) {
                _results.push(x.message);
              }
            }
            return _results;
          })()).join('; '));
        }
        return cb(e);
      }), true);
      if (this.cascadeChannels && (chnls = channel.split('/')).length > 1) {
        this.emit(chnls.slice(0, -1).join('/'), data, cb);
      }
      return this;
    };

    Mediator.prototype.installTo = function(obj, force) {
      var k, v;
      if (typeof obj === "object") {
        for (k in this) {
          v = this[k];
          if (force) {
            obj[k] = v;
          } else {
            if (obj[k] == null) {
              obj[k] = v;
            }
          }
        }
      }
      return this;
    };

    Mediator._rm = function(o, ch, cb, ctxt) {
      var s;
      if (o.channels[ch] == null) {
        return;
      }
      return o.channels[ch] = (function() {
        var _i, _len, _ref, _results;
        _ref = o.channels[ch];
        _results = [];
        for (_i = 0, _len = _ref.length; _i < _len; _i++) {
          s = _ref[_i];
          if ((cb != null ? s.callback !== cb : ctxt != null ? s.context !== ctxt : s.context !== o)) {
            _results.push(s);
          }
        }
        return _results;
      })();
    };

    return Mediator;

  })();

  checkType = function(type, val, name) {
    if (typeof val !== type) {
      return "" + name + " has to be a " + type;
    }
  };

  Core = (function() {
    function Core(Sandbox) {
      this.Sandbox = Sandbox;
      this._modules = {};
      this._plugins = [];
      this._instances = {};
      this._sandboxes = {};
      this._running = {};
      this._mediator = new Mediator(this);
      this.Mediator = Mediator;
      if (this.Sandbox == null) {
        this.Sandbox = function(core, instanceId, options, moduleId) {
          this.instanceId = instanceId;
          this.options = options != null ? options : {};
          this.moduleId = moduleId;
          core._mediator.installTo(this);
          return this;
        };
      }
    }

    Core.prototype.log = {
      error: function() {},
      log: function() {},
      info: function() {},
      warn: function() {},
      enable: function() {}
    };

    Core.prototype.register = function(id, creator, options) {
      var err;
      if (options == null) {
        options = {};
      }
      err = checkType("string", id, "module ID") || checkType("function", creator, "creator") || checkType("object", options, "option parameter");
      if (err) {
        this.log.error("could not register module '" + id + "': " + err);
        return this;
      }
      if (id in this._modules) {
        this.log.warn("module " + id + " was already registered");
        return this;
      }
      this._modules[id] = {
        creator: creator,
        options: options,
        id: id
      };
      return this;
    };

    Core.prototype.start = function(moduleId, opt, cb) {
      var e, id, initInst;
      if (opt == null) {
        opt = {};
      }
      if (cb == null) {
        cb = function() {};
      }
      if (arguments.length === 0) {
        return this._startAll();
      }
      if (moduleId instanceof Array) {
        return this._startAll(moduleId, opt);
      }
      if (typeof moduleId === "function") {
        return this._startAll(null, moduleId);
      }
      if (typeof opt === "function") {
        cb = opt;
        opt = {};
      }
      e = checkType("string", moduleId, "module ID") || checkType("object", opt, "second parameter") || (!this._modules[moduleId] ? "module doesn't exist" : void 0);
      if (e) {
        return this._startFail(e, cb);
      }
      id = opt.instanceId || moduleId;
      if (this._running[id] === true) {
        return this._startFail(new Error("module was already started"), cb);
      }
      initInst = (function(_this) {
        return function(err, instance, opt) {
          if (err) {
            return _this._startFail(err, cb);
          }
          try {
            if (util.hasArgument(instance.init, 2)) {
              return instance.init(opt, function(err) {
                if (!err) {
                  _this._running[id] = true;
                }
                return cb(err);
              });
            } else {
              instance.init(opt);
              _this._running[id] = true;
              return cb();
            }
          } catch (_error) {
            e = _error;
            return _this._startFail(e, cb);
          }
        };
      })(this);
      return this.boot((function(_this) {
        return function(err) {
          if (err) {
            return _this._startFail(err, cb);
          }
          return _this._createInstance(moduleId, opt, initInst);
        };
      })(this));
    };

    Core.prototype._startFail = function(e, cb) {
      this.log.error(e);
      cb(new Error("could not start module: " + e.message));
      return this;
    };

    Core.prototype._createInstance = function(moduleId, o, cb) {
      var Sandbox, iOpts, id, key, module, obj, opt, sb, val, _i, _len, _ref;
      id = o.instanceId || moduleId;
      opt = o.options;
      module = this._modules[moduleId];
      if (this._instances[id]) {
        return cb(this._instances[id]);
      }
      iOpts = {};
      _ref = [module.options, opt];
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        obj = _ref[_i];
        if (obj) {
          for (key in obj) {
            val = obj[key];
            if (iOpts[key] == null) {
              iOpts[key] = val;
            }
          }
        }
      }
      Sandbox = typeof o.sandbox === 'function' ? o.sandbox : this.Sandbox;
      sb = new Sandbox(this, id, iOpts, moduleId);
      return this._runSandboxPlugins('init', sb, (function(_this) {
        return function(err) {
          var instance;
          instance = new module.creator(sb);
          if (typeof instance.init !== "function") {
            return cb(new Error("module has no 'init' method"));
          }
          _this._instances[id] = instance;
          _this._sandboxes[id] = sb;
          return cb(null, instance, iOpts);
        };
      })(this));
    };

    Core.prototype._runSandboxPlugins = function(ev, sb, cb) {
      var p, tasks;
      tasks = (function() {
        var _i, _len, _ref, _ref1, _results;
        _ref = this._plugins;
        _results = [];
        for (_i = 0, _len = _ref.length; _i < _len; _i++) {
          p = _ref[_i];
          if (typeof ((_ref1 = p.plugin) != null ? _ref1[ev] : void 0) === "function") {
            _results.push((function(p) {
              var fn;
              fn = p.plugin[ev];
              return function(next) {
                if (util.hasArgument(fn, 3)) {
                  return fn(sb, p.options, next);
                } else {
                  fn(sb, p.options);
                  return next();
                }
              };
            })(p));
          }
        }
        return _results;
      }).call(this);
      return util.runSeries(tasks, cb, true);
    };

    Core.prototype._startAll = function(mods, cb) {
      var done, m, startAction;
      if (mods == null) {
        mods = (function() {
          var _results;
          _results = [];
          for (m in this._modules) {
            _results.push(m);
          }
          return _results;
        }).call(this);
      }
      startAction = (function(_this) {
        return function(m, next) {
          return _this.start(m, _this._modules[m].options, next);
        };
      })(this);
      done = function(err) {
        var e, i, k, mdls, modErrors, x, _i, _len;
        if ((err != null ? err.length : void 0) > 0) {
          modErrors = {};
          for (i = _i = 0, _len = err.length; _i < _len; i = ++_i) {
            x = err[i];
            if (x != null) {
              modErrors[mods[i]] = x;
            }
          }
          mdls = (function() {
            var _results;
            _results = [];
            for (k in modErrors) {
              _results.push("'" + k + "'");
            }
            return _results;
          })();
          e = new Error("errors occoured in the following modules: " + mdls);
          e.moduleErrors = modErrors;
        }
        return typeof cb === "function" ? cb(e) : void 0;
      };
      util.doForAll(mods, startAction, done, true);
      return this;
    };

    Core.prototype.stop = function(id, cb) {
      var instance, x;
      if (cb == null) {
        cb = function() {};
      }
      if (arguments.length === 0 || typeof id === "function") {
        util.doForAll((function() {
          var _results;
          _results = [];
          for (x in this._instances) {
            _results.push(x);
          }
          return _results;
        }).call(this), ((function(_this) {
          return function() {
            return _this.stop.apply(_this, arguments);
          };
        })(this)), id, true);
      } else if (instance = this._instances[id]) {
        delete this._instances[id];
        this._mediator.off(instance);
        this._runSandboxPlugins('destroy', this._sandboxes[id], (function(_this) {
          return function(err) {
            if (util.hasArgument(instance.destroy)) {
              return instance.destroy(function(err2) {
                delete _this._running[id];
                return cb(err || err2);
              });
            } else {
              if (typeof instance.destroy === "function") {
                instance.destroy();
              }
              delete _this._running[id];
              return cb(err);
            }
          };
        })(this));
      }
      return this;
    };

    Core.prototype.use = function(plugin, opt) {
      var p, _i, _len;
      if (plugin instanceof Array) {
        for (_i = 0, _len = plugin.length; _i < _len; _i++) {
          p = plugin[_i];
          switch (typeof p) {
            case "function":
              this.use(p);
              break;
            case "object":
              this.use(p.plugin, p.options);
          }
        }
      } else {
        if (typeof plugin !== "function") {
          return this;
        }
        this._plugins.push({
          creator: plugin,
          options: opt
        });
      }
      return this;
    };

    Core.prototype.boot = function(cb) {
      var core, p, tasks;
      core = this;
      tasks = (function() {
        var _i, _len, _ref, _results;
        _ref = this._plugins;
        _results = [];
        for (_i = 0, _len = _ref.length; _i < _len; _i++) {
          p = _ref[_i];
          if (p.booted !== true) {
            _results.push((function(p) {
              if (util.hasArgument(p.creator, 3)) {
                return function(next) {
                  var plugin;
                  return plugin = p.creator(core, p.options, function(err) {
                    if (!err) {
                      p.booted = true;
                      p.plugin = plugin;
                    }
                    return next();
                  });
                };
              } else {
                return function(next) {
                  p.plugin = p.creator(core, p.options);
                  p.booted = true;
                  return next();
                };
              }
            })(p));
          }
        }
        return _results;
      }).call(this);
      util.runSeries(tasks, cb, true);
      return this;
    };

    return Core;

  })();

  api = {
    VERSION: "0.4.5",
    util: util,
    Mediator: Mediator,
    Core: Core,
    plugins: {},
    modules: {}
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return api;
    });
  } else if (typeof window !== "undefined" && window !== null) {
    if (window.scaleApp == null) {
      window.scaleApp = api;
    }
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = api;
  }

  plugin = function(core) {
    var cleanHTML, html;
    cleanHTML = function(str) {
      return str.replace(/\n/g, "").replace(/[\t ]+\</g, "<").replace(/\>[\t ]+\</g, "><").replace(/\>[\t ]+$/g, ">");
    };
    core.html = html = {
      clean: cleanHTML
    };
    return {
      init: function(sb) {
        sb.getContainer = function() {
          switch (typeof sb.options.container) {
            case "string":
              return document.getElementById(sb.options.container);
            case "object":
              return sb.options.container;
            default:
              return document.getElementById(sb.instanceId);
          }
        };
        return {
          html: html
        };
      }
    };
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.dom = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

  plugin = function(core, options) {
    var addLocal, baseLanguage, channelName, get, getBrowserLanguage, getLanguage, getText, global, hasParent, lang, mediator, onChange, setGlobal, setLanguage, unsubscribe, _;
    if (options == null) {
      options = {};
    }
    hasParent = function() {
      var _ref;
      return (_ref = core._parentCore) != null ? _ref.i18n : void 0;
    };
    baseLanguage = "en";
    getBrowserLanguage = function() {
      return ((typeof navigator !== "undefined" && navigator !== null ? navigator.language : void 0) || (typeof navigator !== "undefined" && navigator !== null ? navigator.browserLanguage : void 0) || baseLanguage).split("-")[0];
    };
    channelName = "i18n";
    getText = function(key, x, l, global) {
      var _ref, _ref1;
      return ((_ref = x[l]) != null ? _ref[key] : void 0) || ((_ref1 = global[l]) != null ? _ref1[key] : void 0);
    };
    get = function(key, x, lang, global) {
      if (x == null) {
        x = {};
      }
      if (lang == null) {
        lang = "";
      }
      return getText(key, x, lang, global) || getText(key, x, lang.substring(0, 2), global) || getText(key, x, baseLanguage, global) || key;
    };
    addLocal = function(dict, i18n) {
      var k, lang, txt, v, _results;
      if (typeof dict !== "object") {
        return false;
      }
      _results = [];
      for (lang in dict) {
        txt = dict[lang];
        if (i18n[lang] == null) {
          i18n[lang] = {};
        }
        _results.push((function() {
          var _base, _results1;
          _results1 = [];
          for (k in txt) {
            v = txt[k];
            _results1.push((_base = i18n[lang])[k] != null ? _base[k] : _base[k] = v);
          }
          return _results1;
        })());
      }
      return _results;
    };
    mediator = new core.Mediator;
    lang = getBrowserLanguage();
    global = options.global || {};
    core.getBrowserLanguage = getBrowserLanguage;
    core.baseLanguage = baseLanguage;
    getLanguage = function() {
      return lang;
    };
    unsubscribe = function() {
      return mediator.off.apply(mediator, [channelName].concat(__slice.call(arguments)));
    };
    onChange = function() {
      return mediator.on.apply(mediator, [channelName].concat(__slice.call(arguments)));
    };
    setLanguage = function(code) {
      if (typeof code === "string") {
        lang = code;
        return mediator.emit(channelName, lang);
      }
    };
    setGlobal = function(obj) {
      var p;
      if (typeof obj === "object") {
        if ((p = hasParent()) != null) {
          p.setGlobal(obj);
        } else {
          global = obj;
        }
        return true;
      } else {
        return false;
      }
    };
    _ = function(text, o) {
      var p;
      if ((p = hasParent()) != null) {
        return p._(text, o);
      } else {
        return get(text, o, lang, global);
      }
    };
    core.i18n = {
      setLanguage: setLanguage,
      getLanguage: getLanguage,
      setGlobal: setGlobal,
      onChange: onChange,
      _: _,
      unsubscribe: unsubscribe,
        getBrowserLanguage: getBrowserLanguage
    };
    core.Sandbox.prototype.i18n = {
      onChange: onChange,
      unsubscribe: unsubscribe,
      getLanguage: getLanguage
    };
    return {
      id: "i18n",
      init: function(sb) {
        sb.i18n.addLocal = function(dict) {
          var _base;
          if ((_base = sb.options).i18n == null) {
            _base.i18n = {};
          }
          return addLocal(dict, sb.options.i18n);
        };
        return sb._ = (function(_this) {
          return function(text) {
            return _(text, sb.options.localDict || sb.options.i18n);
          };
        })(this);
      }
    };
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.i18n = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

  plugin = function(core) {
    var ls;
    ls = function(o) {
      var id, m, _results;
      _results = [];
      for (id in o) {
        m = o[id];
        _results.push(id);
      }
      return _results;
    };
    core.lsInstances = function() {
      return ls(core._instances);
    };
    core.lsModules = function() {
      return ls(core._modules);
    };
    return core.lsPlugins = function() {
      var p, _i, _len, _ref, _ref1, _results;
      _ref = core._plugins;
      _results = [];
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        p = _ref[_i];
        if (((_ref1 = p.plugin) != null ? _ref1.id : void 0) != null) {
          _results.push(p.plugin.id);
        }
      }
      return _results;
    };
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.ls = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

  plugin = function(core) {
    var s;
    core.state = s = new core.Mediator(true);
    return {
      init: function(sb) {
        return s.emit("init/" + sb.moduleId + "/" + sb.instanceId, {
          instanceId: sb.instanceId,
          moduleId: sb.moduleId
        });
      },
      destroy: function(sb) {
        return s.emit("destroy/" + sb.moduleId + "/" + sb.instanceId, {
          instanceId: sb.instanceId,
          moduleId: sb.moduleId
        });
      }
    };
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.modulestate = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

  plugin = function(core) {
    var Controller, Model, View;
    Model = (function(_super) {
      __extends(Model, _super);

      function Model(obj) {
        var k, v;
        Model.__super__.constructor.call(this);
        for (k in obj) {
          v = obj[k];
          if (this[k] == null) {
            this[k] = v;
          }
        }
      }

      Model.prototype.set = function(key, val, silent) {
        var k, v;
        if (silent == null) {
          silent = false;
        }
        switch (typeof key) {
          case "object":
            for (k in key) {
              v = key[k];
              this.set(k, v, true);
            }
            if (!silent) {
              this.emit(Model.CHANGED, (function() {
                var _results;
                _results = [];
                for (k in key) {
                  v = key[k];
                  _results.push(k);
                }
                return _results;
              })());
            }
            break;
          case "string":
            if (!(key === "set" || key === "get") && this[key] !== val) {
              this[key] = val;
              if (!silent) {
                this.emit(Model.CHANGED, [key]);
              }
            }
            break;
          default:
            if (typeof console !== "undefined" && console !== null) {
              if (typeof console.error === "function") {
                console.error("key is not a string");
              }
            }
        }
        return this;
      };

      Model.prototype.change = function(cb, context) {
        if (typeof cb === "function") {
          return this.on(Model.CHANGED, cb, context);
        } else if (arguments.length === 0) {
          return this.emit(Model.CHANGED);
        }
      };

      Model.prototype.notify = function() {
        return this.change();
      };

      Model.prototype.get = function(key) {
        return this[key];
      };

      Model.prototype.toJSON = function() {
        var json, k, v;
        json = {};
        for (k in this) {
          if (!__hasProp.call(this, k)) continue;
          v = this[k];
          json[k] = v;
        }
        return json;
      };

      Model.CHANGED = "changed";

      return Model;

    })(core.Mediator);
    View = (function() {
      function View(model) {
        if (model) {
          this.setModel(model);
        }
      }

      View.prototype.setModel = function(model) {
        this.model = model;
        return this.model.change((function() {
          return this.render();
        }), this);
      };

      View.prototype.render = function() {};

      return View;

    })();
    Controller = (function() {
      function Controller(model, view) {
        this.model = model;
        this.view = view;
      }

      return Controller;

    })();
    core.Model = Model;
    core.View = View;
    return core.Controller = Controller;
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.mvc = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

  plugin = function(core) {
    var addPermission, controlledActions, grantAction, hasPermission, permissions, removePermission, tweakSandboxMethod;
    permissions = {};
    controlledActions = ["on", "emit", "off"];
    addPermission = function(id, action, channels) {
      var a, act, c, k, p, v, _i, _len, _ref, _ref1, _ref2;
      if (typeof id === "object") {
        return _ref = !false, __indexOf.call((function() {
          var _results;
          _results = [];
          for (k in id) {
            v = id[k];
            _results.push(addPermission(k, v));
          }
          return _results;
        })(), _ref) >= 0;
      } else if (typeof action === "object") {
        return _ref1 = !false, __indexOf.call((function() {
          var _results;
          _results = [];
          for (k in action) {
            v = action[k];
            _results.push(addPermission(id, k, v));
          }
          return _results;
        })(), _ref1) >= 0;
      } else if (channels != null) {
        p = permissions[id] != null ? permissions[id] : permissions[id] = {};
        if (typeof channels === "string") {
          channels = channels === '*' ? ["__all__"] : [channels];
        }
        if (typeof action === "string") {
          if (action === '*') {
            return _ref2 = !false, __indexOf.call((function() {
              var _i, _len, _results;
              _results = [];
              for (_i = 0, _len = controlledActions.length; _i < _len; _i++) {
                act = controlledActions[_i];
                _results.push(addPermission(id, act, channels));
              }
              return _results;
            })(), _ref2) >= 0;
          } else {
            a = p[action] != null ? p[action] : p[action] = {};
            for (_i = 0, _len = channels.length; _i < _len; _i++) {
              c = channels[_i];
              a[c] = true;
            }
            return true;
          }
        } else {
          return false;
        }
      } else {
        return false;
      }
    };
    removePermission = function(id, action, channel) {
      var p, _ref;
      p = permissions[id];
      if (channel == null) {
        delete p[action];
        return true;
      } else if (!(p != null ? (_ref = p[action]) != null ? _ref[channel] : void 0 : void 0)) {
        return false;
      } else {
        delete p[action][channel];
        return true;
      }
    };
    hasPermission = function(id, action, channel) {
      var p, _ref;
      p = ((_ref = permissions[id]) != null ? _ref[action] : void 0) || {};
      if ((channel != null) && (p[channel] || p["__all__"])) {
        return true;
      } else {
        console.warn("'" + id + "' has no permissions for '" + action + "' with '" + channel + "'");
        return false;
      }
    };
    grantAction = function(sb, action, method, args) {
      var c, channel, p;
      if ((args != null ? args.length : void 0) > 0) {
        channel = args[0];
      }
      p = channel instanceof Array ? ((function() {
        var _i, _len, _results;
        _results = [];
        for (_i = 0, _len = channel.length; _i < _len; _i++) {
          c = channel[_i];
          if (!hasPermission(sb.instanceId, action, c)) {
            _results.push(c);
          }
        }
        return _results;
      })()).length === 0 : hasPermission(sb.instanceId, action, channel);
      if (p === true) {
        return method.apply(sb, args);
      } else {
        return false;
      }
    };
    tweakSandboxMethod = function(sb, methodName) {
      var originalMethod;
      originalMethod = sb[methodName];
      if (typeof originalMethod === "function") {
        return sb[methodName] = function() {
          return grantAction(sb, methodName, originalMethod, arguments);
        };
      }
    };
    core.permission = {
      add: addPermission,
      remove: removePermission
    };
    return {
      init: function(sb) {
        var a, _i, _len, _results;
        _results = [];
        for (_i = 0, _len = controlledActions.length; _i < _len; _i++) {
          a = controlledActions[_i];
          _results.push(tweakSandboxMethod(sb, a));
        }
        return _results;
      }
    };
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.permission = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

  plugin = function(core) {
    var StateMachine, enterChannel, leaveChannel;
    leaveChannel = function(state) {
      return "" + state + "/leave";
    };
    enterChannel = function(state) {
      return "" + state + "/enter";
    };
    StateMachine = (function(_super) {
      __extends(StateMachine, _super);

      function StateMachine(opts) {
        var id, t, _ref;
        if (opts == null) {
          opts = {};
        }
        this.fire = __bind(this.fire, this);
        StateMachine.__super__.constructor.call(this);
        this.states = [];
        this.transitions = {};
        if (opts.states != null) {
          this.addState(opts.states);
        }
        if (opts.start != null) {
          this.addState(opts.start);
          this.start = opts.start;
          this.current = opts.start;
          this.emit(enterChannel(this.start));
        }
        if (opts.transitions != null) {
          _ref = opts.transitions;
          for (id in _ref) {
            t = _ref[id];
            this.addTransition(id, t);
          }
        }
      }

      StateMachine.prototype.start = null;

      StateMachine.prototype.current = null;

      StateMachine.prototype.exit = null;

      StateMachine.prototype.addState = function(id, opt) {
        var k, s, success, v;
        if (opt == null) {
          opt = {};
        }
        if (id instanceof Array) {
          return !(__indexOf.call((function() {
            var _i, _len, _results;
            _results = [];
            for (_i = 0, _len = id.length; _i < _len; _i++) {
              s = id[_i];
              _results.push(this.addState(s));
            }
            return _results;
          }).call(this), false) >= 0);
        } else if (typeof id === "object") {
          return !(__indexOf.call((function() {
            var _results;
            _results = [];
            for (k in id) {
              v = id[k];
              _results.push(this.addState(k, v));
            }
            return _results;
          }).call(this), false) >= 0);
        } else {
          if (typeof id !== "string") {
            return false;
          }
          if (__indexOf.call(this.states, id) >= 0) {
            return false;
          }
          this.states.push(id);
          success = [];
          if (opt.enter != null) {
            success.push(this.on(enterChannel(id), opt.enter));
          }
          if (opt.leave != null) {
            success.push(this.on(leaveChannel(id), opt.leave));
          }
          return !(__indexOf.call(success, false) >= 0);
        }
      };

      StateMachine.prototype.addTransition = function(id, edge) {
        var err, i, _ref;
        if (!((typeof id === "string") && (typeof edge.to === "string") && (this.transitions[id] == null) && (_ref = edge.to, __indexOf.call(this.states, _ref) >= 0))) {
          return false;
        }
        if (edge.from instanceof Array) {
          err = __indexOf.call((function() {
            var _i, _len, _ref1, _results;
            _ref1 = edge.from;
            _results = [];
            for (_i = 0, _len = _ref1.length; _i < _len; _i++) {
              i = _ref1[_i];
              _results.push(__indexOf.call(this.states, i) >= 0);
            }
            return _results;
          }).call(this), false) >= 0;
          if (err !== false) {
            return false;
          }
        } else if (typeof edge.from !== "string") {
          return false;
        }
        this.transitions[id] = {
          from: edge.from,
          to: edge.to
        };
        return true;
      };

      StateMachine.prototype.onEnter = function(state, cb) {
        var _ref;
        if (_ref = !state, __indexOf.call(this.states, _ref) >= 0) {
          return false;
        }
        return this.on(enterChannel(state), cb);
      };

      StateMachine.prototype.onLeave = function(state, cb) {
        var _ref;
        if (_ref = !state, __indexOf.call(this.states, _ref) >= 0) {
          return false;
        }
        return this.on(leaveChannel(state), cb);
      };

      StateMachine.prototype.fire = function(id, callback) {
        var t;
        if (callback == null) {
          callback = function() {};
        }
        t = this.transitions[id];
        if (!((t != null) && this.can(id))) {
          return false;
        }
        this.emit(leaveChannel(this.current), t, (function(_this) {
          return function(err) {
            if (err != null) {
              return callback(err);
            } else {
              return _this.emit(enterChannel(t.to), t, function(err) {
                if (err == null) {
                  _this.current = t.to;
                }
                return callback(err);
              });
            }
          };
        })(this));
        return true;
      };

      StateMachine.prototype.can = function(id) {
        var t, _ref;
        t = this.transitions[id];
        return (t != null ? t.from : void 0) === this.current || (_ref = this.current, __indexOf.call(t.from, _ref) >= 0) || t.from === "*";
      };

      return StateMachine;

    })(core.Mediator);
    return core.StateMachine = StateMachine;
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    if ((_base = window.scaleApp.plugins).state == null) {
      _base.state = plugin;
    }
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }


  /*
  Copyright (c) 2012 - 2014 Markus Kohlhase <mail@markus-kohlhase.de>
   */

  DEFAULT_PATH = "http-bind/";

  DEFAULT_PROTOCOL = "http";

  DEFAULT_PORT = 5280;

  CACHE_PREFIX = "scaleApp.xmpp.cache.";

  get_bosh_url = function(opt) {
    var domain;
    domain = document.domain;
    if (typeof opt === "object") {
      if (opt.port) {
        opt.port = opt.port * 1;
        if (isNaN(opt.port)) {
          console.warn("the defined port " + opt.port + " is not a number.");
          opt.port = null;
        }
      }
      if (opt.host && opt.port && opt.path) {
        return "" + opt.protocol + "://" + opt.host + ":" + opt.port + "/" + opt.path;
      }
      if (opt.host && opt.port && !opt.path) {
        return "" + opt.protocol + "://" + opt.host + ":" + opt.port + "/" + DEFAULT_PATH;
      }
      if (opt.host && !opt.port && opt.path) {
        return "" + opt.protocol + "://" + opt.host + "/" + opt.path;
      }
      if (opt.host && !opt.port && !opt.path) {
        return "" + opt.protocol + "://" + opt.host + "/" + DEFAULT_PATH;
      }
      if (!opt.host && opt.port && opt.path) {
        return "" + opt.protocol + "://" + domain + ":" + opt.port + "/" + opt.path;
      }
      if (!opt.host && opt.port && !opt.path) {
        return "" + opt.protocol + "://" + domain + ":" + opt.port + "/" + DEFAULT_PATH;
      }
      if (!opt.host && !opt.port && opt.path) {
        return "" + opt.protocol + "://" + domain + "/" + opt.path;
      }
      if (!opt.host && !opt.port && !opt.path) {
        return "" + opt.protocol + "://" + domain + "/" + DEFAULT_PATH;
      }
    }
    return "http://" + domain + "/" + DEFAULT_PATH;
  };

  create_connection_obj = function(opt) {
    return new Strophe.Connection(get_bosh_url({
      path: opt.path,
      host: opt.host,
      port: opt.port,
      protocol: opt.protocol
    }));
  };

  key2cache = function(k) {
    return "" + CACHE_PREFIX + k;
  };

  saveData = function(conn, opt) {
    var k, _i, _j, _len, _len1, _ref, _ref1, _results;
    if (typeof sessionStorage === "undefined" || sessionStorage === null) {
      return;
    }
    _ref = ["jid", "sid", "rid"];
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      k = _ref[_i];
      if (conn[k] != null) {
        sessionStorage[key2cache(k)] = conn[k];
      }
    }
    _ref1 = ["host", "port", "path", "protocol"];
    _results = [];
    for (_j = 0, _len1 = _ref1.length; _j < _len1; _j++) {
      k = _ref1[_j];
      if (opt[k] != null) {
        _results.push(sessionStorage[key2cache(k)] = opt[k]);
      }
    }
    return _results;
  };

  clearData = function() {
    var k, _i, _len, _ref, _results;
    if (typeof sessionStorage === "undefined" || sessionStorage === null) {
      return;
    }
    _ref = ["jid", "sid", "rid", "host", "port", "path", "protocol"];
    _results = [];
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      k = _ref[_i];
      _results.push(sessionStorage.removeItem(key2cache(k)));
    }
    return _results;
  };

  restoreData = function() {
    var j, k, o, _i, _len, _ref, _results;
    if (typeof sessionStorage === "undefined" || sessionStorage === null) {
      return;
    }
    o = {};
    _ref = ["jid", "sid", "rid", "host", "port", "path", "protocol"];
    _results = [];
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      k = _ref[_i];
      j = key2cache(k);
      if (sessionStorage[j]) {
        _results.push(o[k] = sessionStorage[j]);
      } else {
        _results.push(void 0);
      }
    }
    return _results;
  };

  hasConnectionData = function(opt) {
    var k, _i, _len, _ref;
    _ref = ["jid", "sid", "rid"];
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      k = _ref[_i];
      if (!opt[k] || opt[k] === 'null' || opt[k] === 'undefined') {
        return false;
      }
    }
    return true;
  };

  statusCodeToString = function(s) {
    switch (s) {
      case 0:
        return "Error";
      case 1:
        return "Connecting";
      case 2:
        return "Connection failed";
      case 3:
        return "Authenticating";
      case 4:
        return "Authentication failed";
      case 5:
        return "Connected";
      case 6:
        return "Disconnected";
      case 7:
        return "Disconnecting";
      case 8:
        return "Reconnected";
    }
  };

  plugin = function(core) {
    var attach_connection, connection, connection_options, disconnect, login, mediator, onConnected, on_connection_change, resetPlugin, updatePlugin;
    if (typeof window === "undefined" || window === null) {
      throw new Error("This plugin only can be used in the browser");
    }
    if (window.Strophe == null) {
      console.warn("This plugin requires strophe.js");
    }
    mediator = new core.Mediator;
    connection = null;
    resetPlugin = function() {
      core.xmpp.connection = null;
      return core.xmpp.jid = "";
    };
    updatePlugin = function(conn) {
      core.xmpp.connection = conn;
      return core.xmpp.jid = conn.jid;
    };
    onConnected = function() {
      var e, fn, onunload, _i, _j, _len, _len1, _ref, _ref1;
      fn = function(ev) {
        if (ev.keyCode === 27) {
          return typeof ev.preventDefault === "function" ? ev.preventDefault() : void 0;
        }
      };
      onunload = function() {
        if (connection) {
          return saveData(connection, connection_options);
        } else {
          return clearData();
        }
      };
      if (document.addEventListener != null) {
        _ref = ["keydown", "keypress", "keyup"];
        for (_i = 0, _len = _ref.length; _i < _len; _i++) {
          e = _ref[_i];
          document.addEventListener(e, fn, false);
        }
        window.addEventListener("unload", onunload, false);
      } else if (document.attachEvent != null) {
        _ref1 = ["onkeydown", "onkeypress", "onkeyup"];
        for (_j = 0, _len1 = _ref1.length; _j < _len1; _j++) {
          e = _ref1[_j];
          document.attachEvent(e, fn);
        }
        document.attachEvent("onunload", onunload);
      }
      return connection.send($pres());
    };
    on_connection_change = function(status) {
      var s;
      console.info("xmpp status changed: " + statusCodeToString(status));
      s = Strophe.Status;
      switch (status) {
        case s.ERROR:
          resetPlugin();
          return mediator.emit("error", "an error occoured");
        case s.CONNECTING:
          resetPlugin();
          return mediator.emit("connecting");
        case s.CONNFAIL:
          resetPlugin();
          return mediator.emit("error", "could not connect to xmpp server");
        case s.AUTHENTICATING:
          resetPlugin();
          return mediator.emit("authenticating");
        case s.AUTHFAIL:
          resetPlugin();
          return mediator.emit("authfail");
        case s.CONNECTED:
          updatePlugin(connection);
          onConnected();
          return mediator.emit("connected");
        case s.DISCONNECTED:
          clearData();
          resetPlugin();
          return mediator.emit("disconnected");
        case s.DISCONNECTING:
          resetPlugin();
          return mediator.emit("disconnecting");
        case s.ATTACHED:
          updatePlugin(connection);
          onConnected();
          return mediator.emit("attached");
      }
    };
    attach_connection = function(opt) {
      connection = create_connection_obj(opt);
      return connection.attach(opt.jid, opt.sid, opt.rid, on_connection_change, 60);
    };
    connection_options = restoreData();
    if (connection_options && hasConnectionData(connection_options)) {
      attach_connection(connection_options);
    } else {
      mediator.emit("disconnected");
      connection_options = {
        host: document.domain,
        port: DEFAULT_PORT,
        path: DEFAULT_PATH,
        protocol: DEFAULT_PROTOCOL,
        jid: null,
        sid: null,
        rid: null
      };
    }
    login = function(jid, pw, opt) {
      if (opt != null) {
        if (opt.path) {
          connection_options.path = opt.path;
        }
        if (opt.port) {
          connection_options.port = opt.port;
        }
        if (opt.host) {
          connection_options.host = opt.host;
        }
        if (opt.protocol) {
          connection_options.protocol = opt.protocol;
        }
      }
      connection = create_connection_obj(connection_options);
      return connection.connect(jid, pw, on_connection_change);
    };
    disconnect = function() {
      var e;
      console.debug("try to disconnect");
      if ((connection != null ? connection.connected : void 0) === true) {
        try {
          connection.send($pres({
            type: "unavailable"
          }));
          connection.pause();
          connection.disconnect();
        } catch (_error) {
          e = _error;
        }
      }
      return clearData();
    };
    core.xmpp = {
      jid: "",
      connection: null,
      login: login,
      logout: disconnect,
      on: function() {
        return mediator.on.apply(mediator, arguments);
      },
      off: function() {
        return mediator.off.apply(mediator, arguments);
      },
      _mediator: mediator
    };
    return null;
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.xmpp = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

  plugin = function(core, options) {
    var install, methods;
    if (options == null) {
      options = {};
    }
    methods = ["register", "start", "stop", "on", "off", "emit"];
    install = function(sb, subCore) {
      var fn, _fn, _i, _len;
      sb.sub = {};
      _fn = (function(_this) {
        return function(fn) {
          return sb.sub[fn] = function() {
            subCore[fn].apply(subCore, arguments);
            return sb;
          };
        };
      })(this);
      for (_i = 0, _len = methods.length; _i < _len; _i++) {
        fn = methods[_i];
        _fn(fn);
      }
      if (subCore.permission != null) {
        return sb.sub.permission = {
          add: subCore.permission.add,
          remove: subCore.permission.remove
        };
      }
    };
    return {
      init: function(sb, opt, done) {
        var SubSandbox, p, plugins, subCore, _i, _j, _len, _len1, _ref, _ref1, _ref2;
        sb._subCore = subCore = new core.constructor;
        subCore._parentCore = core;
        if (options.useGlobalMediator) {
          core._mediator.installTo(subCore._mediator, true);
        } else if (options.mediator != null) {
          if ((_ref = options.mediator) != null) {
            if (typeof _ref.installTo === "function") {
              _ref.installTo(subCore._mediator, true);
            }
          }
        }
        subCore.Sandbox = SubSandbox = (function(_super) {
          __extends(SubSandbox, _super);

          function SubSandbox() {
            return SubSandbox.__super__.constructor.apply(this, arguments);
          }

          return SubSandbox;

        })(core.Sandbox);
        plugins = [];
        if (options.inherit) {
          _ref1 = core._plugins;
          for (_i = 0, _len = _ref1.length; _i < _len; _i++) {
            p = _ref1[_i];
            plugins.push({
              plugin: p.creator,
              options: p.options
            });
          }
        }
        if (options.use instanceof Array) {
          _ref2 = options.use;
          for (_j = 0, _len1 = _ref2.length; _j < _len1; _j++) {
            p = _ref2[_j];
            plugins.push(p);
          }
        } else if (typeof options.use === "function") {
          plugins.push(options.use);
        }
        return subCore.use(plugins).boot(function(err) {
          if (err) {
            return done(err);
          }
          install(sb, subCore);
          return done();
        });
      },
      destroy: function(sb) {
        return sb._subCore.stop();
      }
    };
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.submodule = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

  plugin = function(core) {
    var mix;
    mix = function(giv, rec, override) {
      var k, v, _results, _results1;
      if (override === true) {
        _results = [];
        for (k in giv) {
          v = giv[k];
          _results.push(rec[k] = v);
        }
        return _results;
      } else {
        _results1 = [];
        for (k in giv) {
          v = giv[k];
          if (!rec.hasOwnProperty(k)) {
            _results1.push(rec[k] = v);
          }
        }
        return _results1;
      }
    };
    core.uniqueId = function(length) {
      var id;
      if (length == null) {
        length = 8;
      }
      id = "";
      while (id.length < length) {
        id += Math.random().toString(36).substr(2);
      }
      return id.substr(0, length);
    };
    core.clone = function(data) {
      var copy, k, v;
      if (data instanceof Array) {
        copy = (function() {
          var _i, _len, _results;
          _results = [];
          for (_i = 0, _len = data.length; _i < _len; _i++) {
            v = data[_i];
            _results.push(v);
          }
          return _results;
        })();
      } else {
        copy = {};
        for (k in data) {
          v = data[k];
          copy[k] = v;
        }
      }
      return copy;
    };
    core.countObjectKeys = function(o) {
      var k, v;
      if (typeof o === "object") {
        return ((function() {
          var _results;
          _results = [];
          for (k in o) {
            v = o[k];
            _results.push(k);
          }
          return _results;
        })()).length;
      }
    };
    return core.mixin = function(receivingClass, givingClass, override) {
      if (override == null) {
        override = false;
      }
      switch ("" + (typeof givingClass) + "-" + (typeof receivingClass)) {
        case "function-function":
          return mix(givingClass.prototype, receivingClass.prototype, override);
        case "function-object":
          return mix(givingClass.prototype, receivingClass, override);
        case "object-object":
          return mix(givingClass, receivingClass, override);
        case "object-function":
          return mix(givingClass, receivingClass.prototype, override);
      }
    };
  };

  if ((typeof define !== "undefined" && define !== null ? define.amd : void 0) != null) {
    define(function() {
      return plugin;
    });
  } else if ((typeof window !== "undefined" && window !== null ? window.scaleApp : void 0) != null) {
    window.scaleApp.plugins.util = plugin;
  } else if ((typeof module !== "undefined" && module !== null ? module.exports : void 0) != null) {
    module.exports = plugin;
  }

}).call(this);
