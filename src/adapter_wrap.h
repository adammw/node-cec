#ifndef ADAPTER_WRAP_H
#define ADAPTER_WRAP_H

#include <queue>
#include <node.h>
#include "cec.h"

class AdapterWrap : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(CEC::ICECAdapter *cec_adapter);

 private:
  CEC::ICECAdapter *cec_adapter;
  CEC::ICECCallbacks cec_callbacks;
  uv_async_t* async_handle;

  AdapterWrap(CEC::ICECAdapter *cec_adapter);
  ~AdapterWrap();

  static v8::Persistent<v8::Function> constructor;

  // Methods
  static void OnUvAsync(uv_async_t *req, int status);
  static int OnCecLogMessage(void *data, const CEC::cec_log_message message);
  static int ListenerCount(v8::Handle<v8::Object> argThis, const char *eventName);
  static bool EmitEvent(v8::Handle<v8::Object> argThis, const unsigned int argc, v8::Handle<v8::Value> argv[]);

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> PowerOnDevices(const v8::Arguments& args);
  static v8::Handle<v8::Value> StandbyDevices(const v8::Arguments& args);
  static v8::Handle<v8::Value> Transmit(const v8::Arguments& args);
  static v8::Handle<v8::Value> OnNewListener(const v8::Arguments& args);
};

typedef struct UvAsyncData {
  uv_mutex_t mutex;
  v8::Persistent<v8::Object> argThis;
  std::queue<CEC::cec_log_message> logQueue;
} UvAsyncData;

#endif
