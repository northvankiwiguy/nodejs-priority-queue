#include <iostream>
#include <v8.h>

#include "priority_queue_v8_wrapper.h"
#include "priority_queue.h"

using namespace v8;
using namespace std;
using priority_queue::PriorityQueue;

namespace priority_queue_wrapper {

  /**
   * Helper for fetching the current object's PriorityQueue, given the
   * JavaScript callback info.
   */
  PriorityQueue *GetQueue(Local<Object> self) {
    return static_cast<PriorityQueue *>(
      Local<External>::Cast(self->GetInternalField(0))->Value());
  }

  /**
   * Constructor function for creating new PriorityQueue objects.
   */
  static void PriorityQueueFunction(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    Local<Object> self = info.Holder();

    /* create a new PriorityQueue, and store it in a local/hidden field */
    PriorityQueue *queue = new PriorityQueue();
    self->SetInternalField(0, External::New(isolate, queue));
  }

  /**
   * Get-accessor function for the "length" property
   */
  void LengthGetter(Local<Name> property, const PropertyCallbackInfo<Value>& info) {
    info.GetReturnValue().Set(GetQueue(info.Holder())->Length());
  }

  /**
   * The "isEmpty" method for the PriorityQueue object.
   */
  static void IsEmptyFunction(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    info.GetReturnValue().Set(Boolean::New(isolate, true));
  }

  /**
   * The "push" method for the PriorityQueue object.
   */
  static void PushFunction(const FunctionCallbackInfo<Value>& info) {
    //Isolate* isolate = args.GetIsolate();
    printf("Push Called");
  }

  /**
   * The "pop" method for the PriorityQueue object.
   */
  static void PopFunction(const FunctionCallbackInfo<Value>& info) {
    //Isolate* isolate = args.GetIsolate();
    printf("Pop Called");
  }

  /**
   * The "clear" method for the PriorityQueue object.
   */
  static void ClearFunction(const FunctionCallbackInfo<Value>& info) {
    //Isolate* isolate = args.GetIsolate();
    printf("Clear Called");
  }

  /**
   * Helper function for adding a property to an Object Template.
   */
  static void AddPropertyToTemplate(
    Isolate *isolate,
    Local<ObjectTemplate> templ,
    const char *name,
    Local<Data> value,
    int attributes = v8::ReadOnly | v8::DontDelete)
  {
    templ->Set(
      Local<Name>::Cast(String::NewFromUtf8(isolate, name, NewStringType::kInternalized).ToLocalChecked()),
      value,
      v8::PropertyAttribute(attributes));
  }

  /**
   * Helper function for adding a pair of accessor callbacks to an Object Template.
   */
  static void AddAccessorToTemplate(
    Isolate *isolate,
    Local<ObjectTemplate> templ,
    const char *name,
    AccessorNameGetterCallback getter,
    AccessorNameSetterCallback setter = nullptr,
    int attributes = v8::ReadOnly | v8::DontDelete)
  {
    templ->SetAccessor(
      Local<Name>::Cast(String::NewFromUtf8(isolate, name, NewStringType::kInternalized).ToLocalChecked()),
      getter,
      setter,
      Local<Value>(),
      DEFAULT,
      v8::PropertyAttribute(attributes));
  }

  /**
   * Main entry point for initializing the PriorityQueue support in V8. This function
   * adds all the necessary functions/properties to enable priority queues to be
   * usable from JavaScript.
   */
  void Initialize(Local<Object> exports) {
    /* an isolate is the main container for storing V8's objects (that is, a "VM") */
    Isolate *isolate = Isolate::GetCurrent();

    /* This handle_scope helps us track and clean-up all the Local<...> objects */
    HandleScope handle_scope(isolate);

    /* 
      * A context is a sub-container of an Isolate, in which different programs can exist
      * and run simultaneously.
      */
    Local<Context> context = isolate->GetCurrentContext();

    /* 
      * Create a template for the PriorityQueue function that'll be used to construct new objects.
      * This template can be used to create the actual Function, but only once per context.
      */ 
    Local<FunctionTemplate> fn_templ = FunctionTemplate::New(isolate, PriorityQueueFunction);

    /*
      * The Prototype Template defines the properties of the function's prototype. These will
      * appear in the prototype chain of any instances that are created.
      */
    Local<ObjectTemplate> proto_templ = fn_templ->PrototypeTemplate();
    AddPropertyToTemplate(isolate, proto_templ, "isEmpty", FunctionTemplate::New(isolate, IsEmptyFunction));
    AddPropertyToTemplate(isolate, proto_templ, "push", FunctionTemplate::New(isolate, PushFunction));
    AddPropertyToTemplate(isolate, proto_templ, "pop", FunctionTemplate::New(isolate, PopFunction));
    AddPropertyToTemplate(isolate, proto_templ, "push", FunctionTemplate::New(isolate, ClearFunction));

    /*
      * The instance template contains the prototypes that will be added directly to the
      * object instances themselves. This is where we also reserve space for internal (hidden)
      * fields that will be stored inside each object.
      */
    Local<ObjectTemplate> instance_templ = fn_templ->InstanceTemplate();
    instance_templ->SetInternalFieldCount(1);
    // TODO: make this an accessor.
    AddAccessorToTemplate(isolate, instance_templ, "length", LengthGetter);

    // TODO: Use ReadOnlyPrototype() to ensure that properties are read-only.

    Local<Function> fn = fn_templ->GetFunction(context).ToLocalChecked();
    Local<String> fn_name = String::NewFromUtf8(isolate, "PriorityQueue", NewStringType::kInternalized).ToLocalChecked();
    fn->SetName(fn_name);

    exports->Set(context, fn_name, fn).Check();
  }
}