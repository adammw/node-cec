#ifndef TYPES_H
#define TYPES_H

#include <v8.h>
#include "cec.h"

class cec_command_wrap {
 public:
  static cec_command_wrap* Parse(v8::Handle<v8::Value> value);

  operator CEC::cec_command() const {
    return command;
  }
 private:
  CEC::cec_command command;
};

class cec_logical_address_wrap {
 public:
  static cec_logical_address_wrap* Parse(v8::Handle<v8::Value> value);
 
  operator CEC::cec_logical_address() const {
    return address;
  }
 private:
  CEC::cec_logical_address address;
};

class cec_opcode_wrap {
 public:
  static cec_opcode_wrap* Parse(v8::Handle<v8::Value> value);

  operator CEC::cec_opcode() const {
    return opcode;
  }
 private:
  CEC::cec_opcode opcode;
};

class cec_datapacket_wrap {
 public:
  static cec_datapacket_wrap* Parse(v8::Handle<v8::Value> value);

  operator CEC::cec_datapacket() const {
    return datapacket;
  }
 private:
  CEC::cec_datapacket datapacket;
};

#endif
