#ifndef CEC_WRAP_H
#define CEC_WRAP_H

#include <node.h>
#include "cec.h"

class CECWrap : public node::ObjectWrap {
 public:
  CECWrap(CEC::libcec_configuration *configuration);
  ~CECWrap();
  static void Init(v8::Handle<v8::Object> exports);

 private:
  CEC::ICECAdapter *cec_adapter;
  CEC::libcec_configuration *configuration;

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Open(const v8::Arguments& args);
  static v8::Handle<v8::Value> DetectAdapters(const v8::Arguments& args);
  static v8::Handle<v8::Value> GetClientVersion(v8::Local<v8::String>, const v8::AccessorInfo&);
  static v8::Handle<v8::Value> GetServerVersion(v8::Local<v8::String>, const v8::AccessorInfo&);
  static v8::Handle<v8::Value> GetFirmwareVersion(v8::Local<v8::String>, const v8::AccessorInfo&);
  static v8::Persistent<v8::Function> constructor;
};

#endif
