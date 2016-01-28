#include "adapter_wrap.h"
#include "types.h"

using namespace v8;

Persistent<Function> AdapterWrap::constructor;

AdapterWrap::AdapterWrap(CEC::ICECAdapter *cec_adapter) : cec_adapter(cec_adapter) {

}

AdapterWrap::~AdapterWrap() {
  CECDestroy(cec_adapter);
  if (async_handle) {
    UvAsyncData* asyncData = static_cast<UvAsyncData*>(async_handle->data);
    uv_mutex_destroy(&asyncData->mutex);
    delete asyncData;
    uv_close((uv_handle_t*) async_handle, NULL);
  }
}

int AdapterWrap::OnCecLogMessage(void *data, const CEC::cec_log_message message) {
  uv_async_t* async = static_cast<uv_async_t*>(data);
  UvAsyncData* asyncData = static_cast<UvAsyncData*>(async->data);
  uv_mutex_lock(&asyncData->mutex);
  asyncData->logQueue.push(message);
  uv_mutex_unlock(&asyncData->mutex);
  uv_async_send(async);
  return 1;
}

void AdapterWrap::OnUvAsync(uv_async_t *req, int status) {
  UvAsyncData* asyncData = static_cast<UvAsyncData*>(req->data);
  uv_mutex_lock(&asyncData->mutex);
  while(!asyncData->logQueue.empty()) {
    CEC::cec_log_message message = asyncData->logQueue.front();

    const unsigned argc = 2;
    Local<Object> data = Object::New();
    data->Set(String::NewSymbol("message"), String::New(message.message));
    data->Set(String::NewSymbol("level"), Number::New(message.level));
    data->Set(String::NewSymbol("time"), Number::New(message.time));
    Local<Value> argv[argc] = { Local<Value>::New(String::New("logmessage")), data };
    EmitEvent(asyncData->argThis, argc, argv);
    asyncData->logQueue.pop();
  }
  uv_mutex_unlock(&asyncData->mutex);
}

void AdapterWrap::Init(Handle<Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("CECAdapter"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("powerOn"), FunctionTemplate::New(PowerOnDevices)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("standby"), FunctionTemplate::New(StandbyDevices)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("transmit"), FunctionTemplate::New(Transmit)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("getPowerState"), FunctionTemplate::New(GetDevicePowerStatus)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("setHDMIPort"), FunctionTemplate::New(SetHDMIPort)->GetFunction());

  // Save constructor for use in NewInstance
  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("CECAdapter"), constructor);
}

Handle<Value> AdapterWrap::NewInstance(CEC::ICECAdapter *cec_adapter) {
  HandleScope scope;
  Local<Object> instance = constructor->NewInstance();
  AdapterWrap* obj = new AdapterWrap(cec_adapter);
  obj->Wrap(instance);

  const unsigned argc = 2;
  Local<Value> argv[argc] = { Local<Value>::New(String::New("newListener")), FunctionTemplate::New(OnNewListener)->GetFunction() };
  Handle<Function>::Cast(instance->Get(String::New("addListener")))->Call(instance, argc, argv);
  return scope.Close(instance);
}

Handle<Value> AdapterWrap::New(const Arguments& args) {
  return args.This();
}

Handle<Value> AdapterWrap::OnNewListener(const Arguments& args) {
  HandleScope scope;

  AdapterWrap* obj = ObjectWrap::Unwrap<AdapterWrap>(args.This());
  if (args[0]->Equals(String::New("logmessage")) && obj->cec_callbacks.CBCecLogMessage == NULL) {
    UvAsyncData* asyncData = new UvAsyncData;
    asyncData->argThis = Persistent<Object>::New(args.This());
    uv_mutex_init(&asyncData->mutex);
    obj->async_handle = new uv_async_t;
    obj->async_handle->data = (void *) asyncData;
    uv_async_init(uv_default_loop(), obj->async_handle, OnUvAsync);
    obj->cec_callbacks.CBCecLogMessage = &OnCecLogMessage;
    obj->cec_adapter->EnableCallbacks((void *) obj->async_handle, &(obj->cec_callbacks));
  }

  return scope.Close(Undefined());
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

Handle<Value> AdapterWrap::Transmit(const Arguments& args) {
  HandleScope scope;

  AdapterWrap* obj = ObjectWrap::Unwrap<AdapterWrap>(args.This());

  CEC::cec_command cmd;
  if (args.Length() != 1) {
    ThrowException(Exception::TypeError(String::New("Invalid arguments")));
    return scope.Close(Undefined());
  }

  cec_command_wrap* command_wrap = cec_command_wrap::Parse(args[0]);
  if (command_wrap == NULL) {
    return scope.Close(Undefined());
  }
  bool ret = obj->cec_adapter->Transmit(*command_wrap);
  delete command_wrap;
  return scope.Close(Boolean::New(ret));
}

bool AdapterWrap::EmitEvent(Handle<Object> argThis, const unsigned int argc, Handle<Value> argv[]) {
  HandleScope scope;
  Handle<Value> emitVal = argThis->Get(String::New("emit"));
  if (!emitVal->IsFunction()) {
    return false;
  }

  Handle<Function> emitFn = Handle<Function>::Cast(emitVal);
  emitFn->Call(argThis, argc, argv);

  return true;
}

int AdapterWrap::ListenerCount(Handle<Object> argThis, const char *eventName) {
  HandleScope scope;
  Handle<Value> listenersVal = argThis->Get(String::New("listeners"));
  if (!listenersVal->IsFunction()) {
    return 0;
  }

  Handle<Function> listenersFn = Handle<Function>::Cast(listenersVal);

  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New(eventName)) };
  Local<Value> listeners = listenersFn->Call(argThis, argc, argv);
  Local<Array> listenersArr = Local<Array>::Cast(listeners);

  return listenersArr->Length();
}

Handle<Value> AdapterWrap::GetDevicePowerStatus(const Arguments& args) {
    HandleScope scope;
    
    AdapterWrap* obj = ObjectWrap::Unwrap<AdapterWrap>(args.This());
    CEC::cec_power_status ret;
    if (args.Length() && args[0]->IsNumber()) {
        ret = obj->cec_adapter->GetDevicePowerStatus((CEC::cec_logical_address) args[0]->IntegerValue());
    } else {
        ret = obj->cec_adapter->GetDevicePowerStatus(CEC::CECDEVICE_TV);
    }
    
    bool turnedOn = false;
    if(ret == CEC::CEC_POWER_STATUS_ON) turnedOn = true;
    
    return scope.Close(Boolean::New(turnedOn));
}

Handle<Value> AdapterWrap::SetHDMIPort(const Arguments& args) {
    HandleScope scope;
    
    AdapterWrap* obj = ObjectWrap::Unwrap<AdapterWrap>(args.This());
    bool ret = false;
    if (args.Length() > 1 && args[0]->IsNumber() && args[1]->IsNumber()) {
        ret = obj->cec_adapter->SetHDMIPort((CEC::cec_logical_address) args[0]->IntegerValue(),(uint8_t) args[1]->IntegerValue());
    }
    else if(args.Length() > 0 && args[0]->IsNumber()) {
        ret = obj->cec_adapter->SetHDMIPort(CEC::CECDEVICE_TV,(uint8_t) args[0]->IntegerValue());
    }
    return scope.Close(Boolean::New(ret));
}

