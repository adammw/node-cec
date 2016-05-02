#ifndef CEC_WRAP_H
#define CEC_WRAP_H

#include <string>
#include <node.h>
#include "cec.h"
#include <nan.h>
#include <v8.h>

class CECWrap : public Nan::ObjectWrap {
 public:
  CECWrap(CEC::libcec_configuration *configuration);
  ~CECWrap();
  static void Init(v8::Handle<v8::Object> exports);

 private:
  CEC::ICECAdapter *cec_adapter;
  CEC::libcec_configuration *configuration;

  static v8::Persistent<v8::Function> constructor;

  static void OpenAsync(uv_work_t *req);
  static void OpenAsyncAfter(uv_work_t *req);

  // Methods
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Open(const v8::Arguments& args);
  static v8::Handle<v8::Value> DetectAdapters(const v8::Arguments& args);

  // Accessors
  static v8::Handle<v8::Value> GetClientVersion(v8::Local<v8::String>, const v8::AccessorInfo&);
  static v8::Handle<v8::Value> GetServerVersion(v8::Local<v8::String>, const v8::AccessorInfo&);
  static v8::Handle<v8::Value> GetFirmwareVersion(v8::Local<v8::String>, const v8::AccessorInfo&);
};

typedef struct OpenAsyncData {
  CEC::ICECAdapter *cec_adapter;
  std::string port;
  v8::Persistent<v8::Function> callback;
  bool success;
} OpenAsyncData;

#endif
