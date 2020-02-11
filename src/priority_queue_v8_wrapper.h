#ifndef _PRIORITY_QUEUE_V8_WRAPPER_H_
#define _PRIORITY_QUEUE_V8_WRAPPER_H_

#include <v8.h>

using v8::Local;
using v8::Object;

namespace priority_queue_wrapper {
  /**
   * This function provides a V8 wrapper around a standard PriorityQueue class. That is, it hooks
   * the standard PriorityQueue functionality into a V8 engine by adding functions and properties
   * as necessary to allow JavaScript code to create priority queues.
   */
  void Initialize(Local<Object> exports);
}

#endif /* _PRIORITY_QUEUE_V8_WRAPPER_H_ */