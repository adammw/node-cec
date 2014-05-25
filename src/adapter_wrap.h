#ifndef ADAPTER_WRAP_H
#define ADAPTER_WRAP_H

#include <node.h>
#include "cec.h"

class AdapterWrap : public node::ObjectWrap {
 public:
  static void Init();
  static v8::Handle<v8::Value> NewInstance(CEC::ICECAdapter *cec_adapter);

 private:
  CEC::ICECAdapter *cec_adapter;

  AdapterWrap(CEC::ICECAdapter *cec_adapter);
  ~AdapterWrap();

  static v8::Persistent<v8::Function> constructor;

  // Methods
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> PowerOnDevices(const v8::Arguments& args);
  static v8::Handle<v8::Value> StandbyDevices(const v8::Arguments& args);
};

#endif
