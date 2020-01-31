// hello.cc
#include <node.h>

namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;

/**
 * Constructor function for creating new PriorityQueue objects.
 */
void Method(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(
      isolate, "world", NewStringType::kNormal).ToLocalChecked());
}

/**
 * The main entry point for this Node native plugin. Node already creates
 * a v8::Object (`exports`) which we can populate with functions. However,
 * this implies we can't actually set `exports` itself to be a single function
 * (such as a constructor for the priority queue).
 * 
 * Instead, we put the constructor inside the object. Luckily, this allows
 * us to export multiple new data types from this module.
 */
void Initialize(Local<Object> exports) {
  NODE_SET_METHOD(exports, "PriorityQueue", Method);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

}  // namespace demo
