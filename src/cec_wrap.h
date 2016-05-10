#ifndef CEC_WRAP_H
#define CEC_WRAP_H

#include <string>
#include <node.h>
#include "cec.h"
#include <nan.h>

class CECWrap : public Nan::ObjectWrap {
 public:
  CECWrap(CEC::libcec_configuration *configuration);
  ~CECWrap();
  static NAN_MODULE_INIT(Init);

 private:
  CEC::ICECAdapter *cec_adapter;
  CEC::libcec_configuration *configuration;

  static Nan::Persistent<v8::Function> constructor;

  static void OpenAsync(uv_work_t *req);
  static void OpenAsyncAfter(uv_work_t *req);

  // Methods
  static NAN_METHOD(New);
  static NAN_METHOD(Open);
  static NAN_METHOD(DetectAdapters);

  // Accessors
  static NAN_GETTER(GetClientVersion);
  static NAN_GETTER(GetServerVersion);
  static NAN_GETTER(GetFirmwareVersion);
};

typedef struct OpenAsyncData {
  CEC::ICECAdapter *cec_adapter;
  std::string port;
  v8::Persistent<v8::Function> callback;
  bool success;
} OpenAsyncData;

#endif
