#include <iostream>
#include <v8.h>

#include "priority_queue_v8_wrapper.h"
#include "priority_queue.h"

using namespace v8;
using namespace std;
using priority_queue::PriorityQueue;

/**
 * This namespace contains a wrapper around the standard PriorityQueue class, which is
 * framework-neutral (knows nothing about V8). This wrapper class maps the inputs/outputs
 * from V8 and invokes the underlying methods of the PriorityQueue class.
 */
namespace priority_queue_wrapper {

  /**
   * Small structure of information that's passed around. The priority_queue_wrapper
   * namespace does not contain classes, but instead contains a number of class-less
   * functions (this is necessary due to V8's approach of registering functions
   * instead of methods into it's FunctionTemplate handlers).
   *
   * This callback structure is created with the JavaScript object is created, and deleted when
   * the JavaScript object is destroyed.
   */
  struct callBackInfo {
    /* the V8 Isolate that the JavaScript object belongs to */
    Isolate *isolate;

    /* the underlying PriorityQueue object */
    PriorityQueue *queue;

    /* the V8 Function that we'll call when sorting the underlying PriorityQueue */
    Persistent<Function> *jsSortFunction;

    /* a back-reference to the JavaScript objectt that we're contained within */
    Persistent<Object> *persistentSelf;
  };

  /**
   * Helper for fetching the current object's underlying PriorityQueue,
   * given the JavaScript callback info.
   */
  static PriorityQueue *GetQueue(Local<Object> self) {
    return static_cast<callBackInfo *>(
      Local<External>::Cast(self->GetInternalField(0))->Value())->queue;
  }

  /**
   * Helper function for throwing an exception
   */
  static void ThrowTypeError(Isolate *isolate, const char *message) {
    isolate->ThrowException(
      Exception::TypeError(
        String::NewFromUtf8(
          isolate, message, NewStringType::kNormal).ToLocalChecked()));
  }

  /**
   * Default sorting function, if the user doesn't provide one. Simply
   * returns < 0, 0, or > 0 if a < b, a == b, or a > b.
   */
  static int DefaultSortAlgorithm(void *a, void *b, void *userData) {
    struct callBackInfo *cbInfo = static_cast<struct callBackInfo *>(userData);
    Local<Value> valA = static_cast<Persistent<Value> *>(a)->Get(cbInfo->isolate);
    Local<Value> valB = static_cast<Persistent<Value> *>(b)->Get(cbInfo->isolate);

    // TODO: make this work for all data types, using the < operator.
    double doubleA = valA->NumberValue(cbInfo->isolate->GetCurrentContext()).ToChecked();
    double doubleB = valB->NumberValue(cbInfo->isolate->GetCurrentContext()).ToChecked();

    return doubleA - doubleB;
  }

  /**
   * C++ wrapper function that invokes a JavaScript sort function. This function is
   * called by the underlying PriorityQueue object, and then invokes the user-provided
   * JavaScript function.
   */
  static int JsFunctionWrapper(void *a, void *b, void *userData) {
    struct callBackInfo *cbInfo = static_cast<struct callBackInfo *>(userData);
    Isolate *isolate = cbInfo->isolate;

    /* Two values we'll be comparing */
    Local<Value> args[2];
    args[0] = static_cast<Persistent<Value> *>(a)->Get(isolate);
    args[1] = static_cast<Persistent<Value> *>(b)->Get(isolate);

    /* call the function (it's not a method, so receiver is undefined) */
    Local<Value> result = cbInfo->jsSortFunction->Get(isolate)->CallAsFunction(
      isolate->GetCurrentContext(), Undefined(isolate), 2, args).ToLocalChecked();

    /* check that return value is a number */
    if (!result->IsNumber()) {
      ThrowTypeError(isolate, "Sort function must return a number.");
    }

    /* cast the result to an integer, which is what PriorityQueue expects */
    return static_cast<int>(Local<Number>::Cast(result)->Value());
  }

  /**
   * Finalizer, will be invoked whenever a PriorityQueue object is being garbage
   * collected. We know we're being garbage collected because this function is
   * the "weak" callback that'll be invoked when there are no non-weak references
   * left. The call back parameter is callBackInfo structure containing pointers
   * to all the sub-objects we allocated.
   */
  static void PriorityQueueDestructor(const WeakCallbackInfo<struct callBackInfo> &data) {
    struct callBackInfo *cbInfo = data.GetParameter();

    /* Delete underlying queue - this assumes that the finalizer is only called once */
    delete GetQueue(cbInfo->persistentSelf->Get(data.GetIsolate()));

    /* Remove the persistent handle to ourselves */
    cbInfo->persistentSelf->Reset();

    /* Remove the persistent sort function reference */
    if (cbInfo->jsSortFunction) {
      cbInfo->jsSortFunction->Reset();
    }

    /* Remove the callback information block */
    delete cbInfo;
  }

  /**
   * Constructor function for creating new PriorityQueue objects. The user may optionally
   * provide a function for sorting two values (the function returns -1, 0, 1 to indicate
   * ordering of two values). Within the new JavaScript object, we create a callBackInfo
   * structure that holds all the internal data structures.
   */
  static void PriorityQueueConstructor(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    Local<Object> localSelf = info.Holder();

    /* we'll use this info block to store all the internal data structures we're creating */
    struct callBackInfo *cbInfo = new callBackInfo();

    /* this is necessary later, when we need to access input/output values */
    cbInfo->isolate = isolate;

    /* either use the default sort function, or a wrapper around the user-provided sort function */
    int (*sortFunction)(void *, void *, void *) = nullptr;
    if (info.Length() == 0) {
      sortFunction = DefaultSortAlgorithm;
    } else {
      if (!info[0]->IsFunction()) {
        ThrowTypeError(isolate, "Sort function must be a function.");
      }
      cbInfo->jsSortFunction = new Persistent<Function>(isolate, Local<Function>::Cast(info[0]));
      sortFunction = JsFunctionWrapper;
    }

    /* create a new PriorityQueue object, passing in our callback info */
    cbInfo->queue = new PriorityQueue(sortFunction, static_cast<void *>(cbInfo));

    /*
     * Create a Persistent-Weak handle to ourself, with a callback that'll be invoked
     * when we're the only reference to the object. This must have kFinalizer callback
     * type so that we can still access the fields of the object in the finalizer.
     */
    cbInfo->persistentSelf = new Persistent<Object>(isolate, localSelf);
    cbInfo->persistentSelf->SetWeak(cbInfo, PriorityQueueDestructor, WeakCallbackType::kFinalizer);

    /* store the call back info in a hidden field within the JavaScript object */
    localSelf->SetInternalField(0, External::New(isolate, cbInfo));
  }

  /**
   * Get-accessor function for the "length" property
   */
  static void LengthGetter(Local<Name> property, const PropertyCallbackInfo<Value>& info) {
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
   * The "clear" method for the PriorityQueue object. This involves
   * popping all the values off the queue, and removing the persistent handle
   * that we created to refer to the values.
   */
  static void ClearFunction(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    PriorityQueue *queue = GetQueue(info.Holder());
    while (queue->Length() != 0) {
      Persistent<Value> *value = static_cast<Persistent<Value> *>(queue->pop());
      value->Reset();
      delete value;
    }
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