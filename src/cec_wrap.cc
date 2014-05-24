#include <node.h>
#include "cec_wrap.h"

using namespace v8;

Persistent<Function> CECWrap::constructor;

CECWrap::CECWrap(CEC::libcec_configuration *configuration) : configuration(configuration) {
  cec_adapter = static_cast<CEC::ICECAdapter*>(CECInitialise(configuration));
}

CECWrap::~CECWrap() {
  CECDestroy(cec_adapter);
}

void CECWrap::Init(Handle<Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("CECWrap"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("open"), FunctionTemplate::New(Open)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("detectAdapters"), FunctionTemplate::New(DetectAdapters)->GetFunction());
  tpl->PrototypeTemplate()->SetAccessor(String::NewSymbol("clientVersion"), GetClientVersion);
  tpl->PrototypeTemplate()->SetAccessor(String::NewSymbol("serverVersion"), GetServerVersion);
  tpl->PrototypeTemplate()->SetAccessor(String::NewSymbol("firmwareVersion"), GetFirmwareVersion);

  // Export constructor
  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("CEC"), constructor);
}

Handle<Value> CECWrap::New(const Arguments& args) {
  HandleScope scope;

  if (args.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)`

    // Create configuration
    CEC::libcec_configuration *configuration = new CEC::libcec_configuration;
    configuration->deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);

    // Create wrapper
    CECWrap* obj = new CECWrap(configuration);
    obj->Wrap(args.This());
    return args.This();
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}

Handle<Value> CECWrap::Open(const Arguments& args) {
  HandleScope scope;

  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(args.This());
  if (args.Length() != 1 || !args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Invalid arguments")));
    return scope.Close(Undefined());
  }
  return scope.Close(Boolean::New(obj->cec_adapter->Open(*String::Utf8Value(args[0]))));
}

Handle<Value> CECWrap::DetectAdapters(const Arguments& args) {
  HandleScope scope;

  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(args.This());
  int alloc_devices = 10;
  CEC::cec_adapter_descriptor* deviceList = new CEC::cec_adapter_descriptor[alloc_devices];
  int8_t device_count = obj->cec_adapter->DetectAdapters(deviceList, sizeof(CEC::cec_adapter_descriptor) * alloc_devices);
  if (device_count == -1) {
    delete[] deviceList;
    ThrowException(Exception::Error(String::New("DetectAdapters CEC Error")));
    return scope.Close(Undefined());
  }
  if (device_count > alloc_devices) {
    delete[] deviceList;
    alloc_devices = device_count;
    deviceList = new CEC::cec_adapter_descriptor[alloc_devices];
    int8_t device_count = obj->cec_adapter->DetectAdapters(deviceList, sizeof(CEC::cec_adapter_descriptor) * alloc_devices);
    if (device_count == -1) {
      delete[] deviceList;
      ThrowException(Exception::Error(String::New("DetectAdapters CEC Error")));
      return scope.Close(Undefined());
    }
  }

  Local<Array> ret = Array::New(device_count);
  for(int8_t i = 0; i < device_count; i++) {
    Local<Object> device = Object::New();
    device->Set(String::NewSymbol("portPath"), String::New(deviceList[i].strComPath));
    device->Set(String::NewSymbol("portName"), String::New(deviceList[i].strComName));
    device->Set(String::NewSymbol("vendorId"), Number::New(deviceList[i].iVendorId));
    device->Set(String::NewSymbol("productId"), Number::New(deviceList[i].iProductId));
    device->Set(String::NewSymbol("firmwareVersion"), Number::New(deviceList[i].iFirmwareVersion));
    device->Set(String::NewSymbol("physicalAddress"), Number::New(deviceList[i].iPhysicalAddress));
    if (deviceList[i].iFirmwareBuildDate != CEC_FW_BUILD_UNKNOWN) {
      device->Set(String::NewSymbol("firmwareBuildDate"), Date::New(deviceList[i].iFirmwareBuildDate));
    }
    device->Set(String::NewSymbol("adapterType"), String::New(obj->cec_adapter->ToString(deviceList[i].adapterType)));
    ret->Set(i, device);
  }

  delete[] deviceList;
  return scope.Close(ret);
}

Handle<Value> CECWrap::GetClientVersion(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;

  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(info.This());
  return scope.Close(Number::New(obj->configuration->clientVersion));
}

Handle<Value> CECWrap::GetServerVersion(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;

  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(info.This());
  return scope.Close(Number::New(obj->configuration->serverVersion));
}

Handle<Value> CECWrap::GetFirmwareVersion(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;

  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(info.This());
  return scope.Close(Number::New(obj->configuration->iFirmwareVersion));
}
