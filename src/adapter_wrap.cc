#include "adapter_wrap.h"

using namespace v8;

Persistent<Function> AdapterWrap::constructor;

AdapterWrap::AdapterWrap(CEC::ICECAdapter *cec_adapter) : cec_adapter(cec_adapter) {

}

AdapterWrap::~AdapterWrap() {
  CECDestroy(cec_adapter);
}

void AdapterWrap::Init() {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("CECAdapter"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("powerOn"), FunctionTemplate::New(PowerOnDevices)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("standby"), FunctionTemplate::New(StandbyDevices)->GetFunction());

  // Save constructor for use in NewInstance
  constructor = Persistent<Function>::New(tpl->GetFunction());
}

Handle<Value> AdapterWrap::NewInstance(CEC::ICECAdapter *cec_adapter) {
  HandleScope scope;
  Local<Object> instance = constructor->NewInstance();
  AdapterWrap* obj = new AdapterWrap(cec_adapter);
  obj->Wrap(instance);
  return scope.Close(instance);
}

Handle<Value> AdapterWrap::New(const Arguments& args) {
  return args.This();
}

Handle<Value> AdapterWrap::PowerOnDevices(const Arguments& args) {
  HandleScope scope;

  AdapterWrap* obj = ObjectWrap::Unwrap<AdapterWrap>(args.This());
  bool ret;
  if (args.Length() && args[0]->IsNumber()) {
    ret = obj->cec_adapter->PowerOnDevices((CEC::cec_logical_address) args[0]->IntegerValue());
  } else {
    ret = obj->cec_adapter->PowerOnDevices();
  }

  return scope.Close(Boolean::New(ret));
}

Handle<Value> AdapterWrap::StandbyDevices(const Arguments& args) {
  HandleScope scope;

  AdapterWrap* obj = ObjectWrap::Unwrap<AdapterWrap>(args.This());
  bool ret;
  if (args.Length() && args[0]->IsNumber()) {
    ret = obj->cec_adapter->StandbyDevices((CEC::cec_logical_address) args[0]->IntegerValue());
  } else {
    ret = obj->cec_adapter->StandbyDevices();
  }

  return scope.Close(Boolean::New(ret));
}

