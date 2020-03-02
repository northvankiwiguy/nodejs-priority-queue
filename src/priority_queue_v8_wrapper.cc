#include <iostream>
#include <v8.h>

#include "priority_queue_v8_wrapper.h"
#include "priority_queue.h"

using namespace v8;
using namespace std;
using priority_queue::PriorityQueue;

namespace priority_queue_wrapper {

  /**
   * Small structure of information that's passed to us whenever our sort function
   * is called. This is necessary so we can correctly invoke the underlying JavaScript
   * function.
   */
  struct callBackInfo {
    Isolate *isolate;
    Persistent<Function> *sortFunction;
  };

  /**
   * Helper for fetching the current object's PriorityQueue, given the
   * JavaScript callback info.
   */
  PriorityQueue *GetQueue(Local<Object> self) {
    return static_cast<PriorityQueue *>(
      Local<External>::Cast(self->GetInternalField(0))->Value());
  }

  /**
   * Helper function for throwing an exception
   */
  void ThrowTypeError(Isolate *isolate, const char *message) {
    isolate->ThrowException(
      Exception::TypeError(
        String::NewFromUtf8(
          isolate, message, NewStringType::kNormal).ToLocalChecked()));
  }

  /**
   * Default sorting function, if the user doesn't provide one. Simply
   * returns > 0, 0, or <0 if a < b, a == b, or a > b.
   */
  static int DefaultSortAlgorithm(void *a, void *b, void *optData) {
    struct callBackInfo *cbInfo = static_cast<struct callBackInfo *>(optData);
    Local<Value> valA = static_cast<Persistent<Value> *>(a)->Get(cbInfo->isolate);
    Local<Value> valB = static_cast<Persistent<Value> *>(b)->Get(cbInfo->isolate);

    // TODO: make this work for all data types, using the < operator.
    double doubleA = valA->NumberValue(cbInfo->isolate->GetCurrentContext()).ToChecked();
    double doubleB = valB->NumberValue(cbInfo->isolate->GetCurrentContext()).ToChecked();

    return doubleB - doubleA;
  }

  /**
   *  C++ wrapper function that invokes a JavaScript sort function.
   */
  static int JsFunctionWrapper(void *a, void *b, void *optData) {
    /* details of the JavaScript function we need to call */
    struct callBackInfo *cbInfo = static_cast<struct callBackInfo *>(optData);
    Isolate *isolate = cbInfo->isolate;

    /* Two values we'll be comparing */
    Local<Value> args[2];
    args[0] = static_cast<Persistent<Value> *>(a)->Get(isolate);
    args[1] = static_cast<Persistent<Value> *>(b)->Get(isolate);

    /* call the function (it's not a method, so receiver is undefined) */
    Local<Value> result = cbInfo->sortFunction->Get(isolate)->CallAsFunction(
      isolate->GetCurrentContext(), Undefined(isolate), 2, args).ToLocalChecked();

    /* check that return value is a number */
    if (!result->IsNumber()) {
      ThrowTypeError(isolate, "Sort function must return a number.");
    }

    /* cast the result to an integer */
    return static_cast<int>(Local<Number>::Cast(result)->Value());
  }

  /**
   * Finalizer, will be invoked whenever a PriorityQueue object is being garbage
   * collected. The call back parameter will be a weak persistent reference to
   * the PriorityQueue object. When called, we'll be the last reference
   */
  static void PriorityQueueDestructor(const WeakCallbackInfo<Persistent<Object>> &data) {
    Persistent<Object> *persistentSelf = data.GetParameter();

    /* Delete underlying queue - this assumes that the finalizer is only called once */
    delete GetQueue(persistentSelf->Get(data.GetIsolate()));

    /* reset the persistent handle - so it no longer refers to self */
    persistentSelf->Reset();

    // TODO: remove the persistent Function reference
  }

  /**
   * Constructor function for creating new PriorityQueue objects.
   */
  static void PriorityQueueConstructor(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    Local<Object> self = info.Holder();

    int (*sortFunction)(void *, void *, void *) = nullptr;
    struct callBackInfo *cbInfo = new callBackInfo();
    cbInfo->isolate = isolate;

    /* either use the default sort function, or a wrapper around the user-provided sort function */
    if (info.Length() == 0) {
      sortFunction = DefaultSortAlgorithm;
    } else {
      if (!info[0]->IsFunction()) {
        ThrowTypeError(isolate, "Sort function must be a function.");
      }
      cbInfo->sortFunction = new Persistent<Function>(isolate, Local<Function>::Cast(info[0]));
      sortFunction = JsFunctionWrapper;
    }

    /* create a new PriorityQueue, and store it in a local/hidden field */
    PriorityQueue *queue = new PriorityQueue(sortFunction, static_cast<void *>(cbInfo));
    self->SetInternalField(0, External::New(isolate, queue));

    /*
     * Create a Persistent-Weak handle to ourself, with a callback that'll be invoked
     * when we're the only reference to the object. This must have kFinalizer callback
     * type so that we can still access the fields of the object in the finalizer.
     */
    Persistent<Object> *persistentSelf = new Persistent<Object>(isolate, self);
    persistentSelf->SetWeak(persistentSelf, PriorityQueueDestructor, WeakCallbackType::kFinalizer);
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
    PriorityQueue *queue = GetQueue(info.Holder());
    info.GetReturnValue().Set(Boolean::New(isolate, queue->Length() == 0));
  }

  /**
   * The "push" method for the PriorityQueue object.
   */
  static void PushFunction(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    PriorityQueue *queue = GetQueue(info.Holder());
    for (int i = 0; i < info.Length(); i++){
      Persistent<Value> *value = new Persistent<Value>(isolate, info[i]);
      queue->push(value);
    }
  }

  /**
   * The "pop" method for the PriorityQueue object.
   */
  static void PopFunction(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    PriorityQueue *queue = GetQueue(info.Holder());
    Persistent<Value> *value = static_cast<Persistent<Value> *>(queue->pop());
    if (value != nullptr) {
      info.GetReturnValue().Set(value->Get(isolate));
      value->Reset(); /* disconnect the value from the persistent reference */
      delete value; /* destroy the persistent reference */
    }
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
    Local<FunctionTemplate> fn_templ = FunctionTemplate::New(isolate, PriorityQueueConstructor);

    /*
     * The "Prototype Template" defines the properties of the function's prototype. These will
     * appear in the prototype chain of any instances that are created.
     */
    Local<ObjectTemplate> proto_templ = fn_templ->PrototypeTemplate();
    AddPropertyToTemplate(isolate, proto_templ, "isEmpty", FunctionTemplate::New(isolate, IsEmptyFunction));
    AddPropertyToTemplate(isolate, proto_templ, "push", FunctionTemplate::New(isolate, PushFunction));
    AddPropertyToTemplate(isolate, proto_templ, "pop", FunctionTemplate::New(isolate, PopFunction));
    AddPropertyToTemplate(isolate, proto_templ, "clear", FunctionTemplate::New(isolate, ClearFunction));

    /*
     * The "Instance Template" contains the properties that will be added directly to the
     * object instances themselves. This is where we also reserve space for internal (hidden)
     * fields that will be stored inside each object.
     */
    Local<ObjectTemplate> instance_templ = fn_templ->InstanceTemplate();
    AddAccessorToTemplate(isolate, instance_templ, "length", LengthGetter);
    instance_templ->SetInternalFieldCount(1); /* hidden - not accessible to JavaScript code */

    /* create an instance of the Function, within the current context */
    Local<Function> fn = fn_templ->GetFunction(context).ToLocalChecked();

    /* insert this Function into the "exports" */
    Local<String> fn_name = String::NewFromUtf8(isolate, "PriorityQueue", NewStringType::kInternalized).ToLocalChecked();
    fn->SetName(fn_name);
    exports->Set(context, fn_name, fn).Check();
  }
}