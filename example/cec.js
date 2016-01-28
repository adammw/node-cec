// This example connects to the first available CEC adapter,
// sends a ping message, powers on the devices and then powers them off
var CEC = require('../');
var debug = require('debug')('example');
var cec = new CEC();

var adapters = cec.detectAdapters();
debug('adapters: %j', adapters);

console.log('connecting to adapter...');
var ret = cec.open(adapters[0].portName, function(err, adapter) {
  if (err) throw err;
  adapter.transmit({
    initiator: CEC.LogicalAddress.TUNER1,
    destination: CEC.LogicalAddress.TV
  });
  //adapter.on('logmessage', function(data) {
  //  debug(data.message);
  //});
  setTimeout(function() {
    console.log('getting powerstate...');
    var ret = adapter.getPowerState();
    console.log(ret);
  }, 1000);
  //setTimeout(function() {
  //  console.log('powering on...');
  //  adapter.powerOn();
  //}, 1000);
  //setTimeout(function() {
  //  console.log('powering off...');
  //  adapter.standby();
  //  process.exit(0);
  //}, 5000);
});
