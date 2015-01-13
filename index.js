var EventEmitter = require('events').EventEmitter,
    types = require('./lib/types'),
    addon = require('./build/Release/addon');

var extend = function(target, source) {
  for (var k in source)
    target[k] = source[k];
};

var inherits = function(target, source) {
  extend(target.prototype, source.prototype);
};

inherits(addon.CECAdapter, EventEmitter);
extend(addon.CEC, types);

module.exports = addon.CEC;

