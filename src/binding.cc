

#define BUILDING_NODE_EXTENSION
#include <node.h>


#include "kinect.h"

using namespace node;
using namespace v8;



freenect_resolution getResolution(int res) {

  switch (res) {
    case 2:
      return FREENECT_RESOLUTION_HIGH;
    break;

    case 1:
      return FREENECT_RESOLUTION_MEDIUM;
    break;

    case 0:
      return FREENECT_RESOLUTION_LOW;
    break;
  }

  return FREENECT_RESOLUTION_LOW;
}

void freenect_tick(uv_idle_t* handle, int status) {
  freenect_process_events(f_ctx);
}


Handle<Value> Init(const Arguments& args) {
  HandleScope scope;

  if (freenect_init(&f_ctx, NULL) < 0) {
    ThrowException(Exception::TypeError(String::New("freenect_init() failed")));
    return scope.Close(Undefined());
  }

  return scope.Close(Undefined());
}

Handle<Value> GetDeviceCount(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Number::New(freenect_num_devices (f_ctx)));
}

void init(Handle<Object> target) {
  target->Set(String::NewSymbol("init"),
      FunctionTemplate::New(Init)->GetFunction());

  target->Set(String::NewSymbol("getDeviceCount"),
      FunctionTemplate::New(GetDeviceCount)->GetFunction());
}

NODE_MODULE(kinect, init)
