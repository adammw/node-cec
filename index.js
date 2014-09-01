var EventEmitter = require('events').EventEmitter,
    addon = require('./build/Release/addon');

var inherits = function(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
};

inherits(addon.CECAdapter, EventEmitter);

module.exports = addon.CEC;
