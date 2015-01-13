#include "types.h"

using namespace v8;

cec_command_wrap* cec_command_wrap::Parse(Handle<Value> value) {
  if (!value->IsObject()) {
    ThrowException(Exception::TypeError(String::New("Invalid argument for cec_command value, not an object")));
    return NULL;
  }

  Local<Object> obj = value->ToObject();
  
  cec_command_wrap* command_wrap = new cec_command_wrap;
  cec_logical_address_wrap* initiator_wrap = NULL;
  cec_logical_address_wrap* destination_wrap = NULL;
  cec_opcode_wrap* opcode_wrap = NULL;

  initiator_wrap = cec_logical_address_wrap::Parse(obj->Get(String::New("initiator")));
  if (initiator_wrap == NULL) { 
    goto fail;
  } 
  command_wrap->command.initiator = *initiator_wrap;
  delete initiator_wrap;

  destination_wrap = cec_logical_address_wrap::Parse(obj->Get(String::New("destination")));
  if (destination_wrap == NULL) {
    goto fail;
  }
  
  command_wrap->command.destination = *destination_wrap;
  delete destination_wrap;

  command_wrap->command.ack = obj->Get(String::New("ack"))->BooleanValue();
  command_wrap->command.eom = obj->Get(String::New("eom"))->BooleanValue();

  if (obj->Has(String::New("opcode"))) {
    opcode_wrap = cec_opcode_wrap::Parse(obj->Get(String::New("opcode")));
    if (opcode_wrap == NULL) {
      goto fail;
    }
    command_wrap->command.opcode = *opcode_wrap;
    delete opcode_wrap;

    command_wrap->command.opcode_set = 1;
  }

  if (obj->Has(String::New("transmit_timeout"))) {
    command_wrap->command.transmit_timeout = obj->Get(String::New("transmit_timeout"))->IntegerValue();
  }

  return command_wrap;

fail:
  delete command_wrap;
  return NULL;
};

cec_logical_address_wrap* cec_logical_address_wrap::Parse(Handle<Value> value) {
  if (!value->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Invalid argument for cec_logical_address value, not a number")));
    return NULL;
  }
  int intval = value->IntegerValue();
  if ( intval < 0 || intval > 15 ) {
    ThrowException(Exception::TypeError(String::New("Invalid argument for cec_logical_address value, out of range")));
    return NULL;
  }
  cec_logical_address_wrap* wrapper = new cec_logical_address_wrap;
  wrapper->address = (CEC::cec_logical_address) intval;
  return wrapper;
};

cec_opcode_wrap* cec_opcode_wrap::Parse(Handle<Value> value) {
  if (!value->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Invalid argument for cec_opcode value, not a number")));
    return NULL;
  }
  int intval = value->IntegerValue();
  cec_opcode_wrap* wrapper = new cec_opcode_wrap;
  wrapper->opcode = (CEC::cec_opcode) intval;
  return wrapper;
};

