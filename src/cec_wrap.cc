#include "cec_wrap.h"
#include "adapter_wrap.h"

using namespace v8;

Persistent<Function> CECWrap::constructor;

CECWrap::CECWrap(CEC::libcec_configuration *configuration) : configuration(configuration) {
  cec_adapter = static_cast<CEC::ICECAdapter*>(CECInitialise(configuration));
  cec_adapter->InitVideoStandalone();
}

CECWrap::~CECWrap() {
  CECDestroy(cec_adapter);
}

void CECWrap::Init(Handle<Object> exports) {
    
  // Prepare constructor template
  //Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    Local<FunctionTemplate> tpl  = FunctionTemplate::New();
  tpl->SetClassName(String::NewSymbol("CEC"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  //tpl->PrototypeTemplate()->Set(String::NewSymbol("open"), FunctionTemplate::New(Open)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("open"), FunctionTemplate::New()->GetFunction());
  //tpl->PrototypeTemplate()->Set(String::NewSymbol("detectAdapters"), FunctionTemplate::New(DetectAdapters)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("detectAdapters"), FunctionTemplate::New()->GetFunction());
  tpl->PrototypeTemplate()->SetAccessor(String::NewSymbol("clientVersion"), GetClientVersion);
  tpl->PrototypeTemplate()->SetAccessor(String::NewSymbol("serverVersion"), GetServerVersion);
  tpl->PrototypeTemplate()->SetAccessor(String::NewSymbol("firmwareVersion"), GetFirmwareVersion);

  // Export constructor
  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("CEC"), constructor);
}

Handle<Value> CECWrap::New(const Nan::FunctionCallbackInfo<v8::Value> &args) {
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

Handle<Value> CECWrap::Open(const Nan::FunctionCallbackInfo<v8::Value> &args) {
  HandleScope scope;

  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(args.This());
  if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
    ThrowException(Exception::TypeError(String::New("Invalid arguments")));
    return scope.Close(Undefined());
  }

  uv_work_t* req = new uv_work_t;
  OpenAsyncData* data = new OpenAsyncData;
  data->port = std::string(*String::Utf8Value(args[0]));
  data->cec_adapter = static_cast<CEC::ICECAdapter*>(CECInitialise(obj->configuration));
  data->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));
  req->data = data;

  uv_queue_work(
    uv_default_loop(),
    req,
    OpenAsync,
    (uv_after_work_cb)OpenAsyncAfter);

  return scope.Close(Undefined());
}

void CECWrap::OpenAsync(uv_work_t *req) {
  OpenAsyncData *data = (OpenAsyncData *)req->data;
  data->success = data->cec_adapter->Open(data->port.c_str());
}

void CECWrap::OpenAsyncAfter(uv_work_t *req) {
  HandleScope scope;
  OpenAsyncData *data = (OpenAsyncData *)req->data;

  // create adapter wrapper
  Handle<Value> adapter = AdapterWrap::NewInstance(data->cec_adapter);

  // callback arguments
  Handle<Value> argv[2];
  if (data->success) {
    argv[0] = Null();
    argv[1] = Local<Value>::New(adapter);
  } else {
    argv[0] = Exception::Error(String::New("libcec error"));
    argv[1] = Null();
  }

  // execute the callback function in a try/catch for safety
  TryCatch try_catch;
  data->callback->Call(Context::GetCurrent()->Global(), 2, argv);
  if (try_catch.HasCaught()) {
    node::FatalException(try_catch);
  }

  // clean up
  data->callback.Dispose();
  delete data;
  delete req;
}

Handle<Value> CECWrap::DetectAdapters(const Nan::FunctionCallbackInfo<v8::Value> &args) {
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
