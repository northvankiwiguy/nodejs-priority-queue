#include <node.h>

#include "priority_queue_v8_wrapper.h"

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
  priority_queue_wrapper::Initialize(exports);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)



