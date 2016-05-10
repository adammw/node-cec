#include "cec_wrap.h"
#include "adapter_wrap.h"

using namespace v8;

Nan::Persistent<Function> CECWrap::constructor;

CECWrap::CECWrap(CEC::libcec_configuration *configuration) : configuration(configuration) {
  cec_adapter = static_cast<CEC::ICECAdapter*>(CECInitialise(configuration));
  cec_adapter->InitVideoStandalone();
}

CECWrap::~CECWrap() {
  CECDestroy(cec_adapter);
}

NAN_MODULE_INIT(CECWrap::Init) {
    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>();
    tpl->SetClassName(Nan::New("CEC").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    Nan::SetPrototypeMethod(tpl, "open", Open);
    Nan::SetPrototypeMethod(tpl, "detectAdapters", DetectAdapters);

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("clientVersion").ToLocalChecked(), GetClientVersion);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("serverVersion").ToLocalChecked(), GetServerVersion);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("firmwareVersion").ToLocalChecked(), GetFirmwareVersion);
    // Export constructor
    //constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    //exports->Set(String::NewSymbol("CEC"), constructor);
//    +    Nan::Set(target, Nan::New("Tag").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
//    +    Nan::SetMethod(target, "tag", AsyncTag);
//    +    Nan::SetMethod(target, "tagSync", SyncTag);
}
/*
v8::Handle<v8::Value> CECWrap::New(const Nan::FunctionCallbackInfo<v8::Value> &args) {
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
*/
NAN_METHOD(CECWrap::New) {
    
    HandleScope scope;
    
    if (info.IsConstructCall()) {
        // Invoked as constructor: `new MyObject(...)`
        
        // Create configuration
        CEC::libcec_configuration *configuration = new CEC::libcec_configuration;
        configuration->deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
        
        // Create wrapper
        CECWrap* obj = new CECWrap(configuration);
        obj->Wrap(info.This());
        
        //info.GetReturnValue(); return info.This();
        
        info.GetReturnValue().Set(info.This());
        
        return;
    } else {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { info[0] };
        
        //  scope.Close(constructor->NewInstance(argc, argv));
        Local<Object> inst = Nan::NewInstance(Nan::New(constructor)).ToLocalChecked();
        
        info.GetReturnValue().Set(inst);
        
        return;
    }
}

NAN_METHOD(CECWrap::Open) {
    
    CECWrap* obj = Nan::ObjectWrap::Unwrap<CECWrap>(info.This());
    if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsFunction()) {
        Nan::ThrowError(Exception::TypeError(String::New("Invalid arguments")));
        return;
    }
    
    uv_work_t* req = new uv_work_t;
    OpenAsyncData* data = new OpenAsyncData;
    data->port = std::string(*String::Utf8Value(info[0]));
    data->cec_adapter = static_cast<CEC::ICECAdapter*>(CECInitialise(obj->configuration));
    data->callback = Persistent<Function>::New(Local<Function>::Cast(info[1]));
    req->data = data;

    uv_queue_work(
        uv_default_loop(),
        req,
        OpenAsync,
        (uv_after_work_cb)OpenAsyncAfter);
    
    info.GetReturnValue().SetUndefined();
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
/*
v8::Handle<v8::Value> CECWrap::DetectAdapters(const Nan::FunctionCallbackInfo<v8::Value> &args) {
  HandleScope scope;

  CECWrap* obj = Nan::ObjectWrap::Unwrap<CECWrap>(args.This());
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
*/
NAN_METHOD(CECWrap::DetectAdapters) {
    HandleScope scope;
    
    CECWrap* obj = Nan::ObjectWrap::Unwrap<CECWrap>(info.This());
    int alloc_devices = 10;
    CEC::cec_adapter_descriptor* deviceList = new CEC::cec_adapter_descriptor[alloc_devices];
    int8_t device_count = obj->cec_adapter->DetectAdapters(deviceList, sizeof(CEC::cec_adapter_descriptor) * alloc_devices);
    if (device_count == -1) {
        delete[] deviceList;
        ThrowException(Exception::Error(String::New("DetectAdapters CEC Error")));
        info.GetReturnValue().SetUndefined();
        return;
    }
    if (device_count > alloc_devices) {
        delete[] deviceList;
        alloc_devices = device_count;
        deviceList = new CEC::cec_adapter_descriptor[alloc_devices];
        int8_t device_count = obj->cec_adapter->DetectAdapters(deviceList, sizeof(CEC::cec_adapter_descriptor) * alloc_devices);
        if (device_count == -1) {
            delete[] deviceList;
            ThrowException(Exception::Error(String::New("DetectAdapters CEC Error")));
            info.GetReturnValue().SetUndefined();
            return;
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
    scope.Close(ret);
    return;
}

NAN_GETTER(CECWrap::GetClientVersion) {
  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(info.This());
  info.GetReturnValue().Set(obj->configuration->clientVersion);
}

NAN_GETTER(CECWrap::GetServerVersion) {
  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(info.This());
  info.GetReturnValue().Set(obj->configuration->serverVersion);
}

NAN_GETTER(CECWrap::GetFirmwareVersion) {
  CECWrap* obj = ObjectWrap::Unwrap<CECWrap>(info.This());
  info.GetReturnValue().Set(obj->configuration->iFirmwareVersion);
}
