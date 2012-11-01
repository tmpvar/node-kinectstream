

#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <node_buffer.h>
#include <v8.h>

extern "C" {
  #include <libfreenect/libfreenect.h>
}

using namespace node;
using namespace v8;

Persistent<Function> getVideoStreamCallback;
Persistent<Function> getDepthStreamCallback;
freenect_context *f_ctx;
freenect_device *f_dev;
uint8_t *rgb_buffer, *depth_buffer;
freenect_resolution video_resolution;

uv_idle_t idler;

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

  int nr_devices = freenect_num_devices (f_ctx);

  if (nr_devices < 1) {
    return scope.Close(Number::New(0));
  }

  // TODO: make the last argument (device index) configurable
  if (freenect_open_device(f_ctx, &f_dev, 0) < 0) {
    ThrowException(Exception::TypeError(String::New("freenect_init() failed")));
  }

  return scope.Close(Undefined());
}


void callJSCallbackWithBuffer(Persistent<Function> cb, Buffer *buffer) {
  const unsigned argc = 1;

  if (cb->IsFunction()) {
    Local<v8::Object> globalObj = v8::Context::GetCurrent()->Global();
    Local<Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::String::New("Buffer")));
    Handle<Value> constructorArgs[3] = { buffer->handle_, v8::Integer::New(Buffer::Length(buffer)), v8::Integer::New(0) };
    Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

    Local<Value> argv[argc] = { actualBuffer };

    cb->Call(Context::GetCurrent()->Global(), argc, argv);
  }
}


void depth_callback(freenect_device *dev, void *depth, uint32_t timestamp) {

  HandleScope scope;
  unsigned int length = 320*240*3;
  Buffer *buf = Buffer::New(length);

  memcpy(Buffer::Data(buf), depth_buffer, length);

  callJSCallbackWithBuffer(getDepthStreamCallback, buf);

  scope.Close(Undefined());
}

void video_callback(freenect_device *dev, void *rgb, uint32_t timestamp) {

  HandleScope scope;

  unsigned int length = freenect_find_video_mode(video_resolution, FREENECT_VIDEO_RGB).bytes;

  Buffer *buf = Buffer::New(length);

  memcpy(Buffer::Data(buf), rgb_buffer, length);

  callJSCallbackWithBuffer(getVideoStreamCallback, buf);

  scope.Close(Undefined());
}

Handle<Value> GetDepthStream(const Arguments& args) {
  HandleScope scope;

  freenect_set_depth_callback(f_dev, depth_callback);
  depth_buffer = (uint8_t*)malloc(freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED).bytes);
  freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED));
  freenect_set_depth_buffer(f_dev, depth_buffer);
  freenect_start_depth(f_dev);
  freenect_stop_video(f_dev);

  getDepthStreamCallback = Persistent<Function>::New(Handle<Function>::Cast(args[0]));

  uv_idle_init(uv_default_loop(), &idler);
  uv_idle_start(&idler, freenect_tick);

  return scope.Close(Undefined());
}

Handle<Value> GetVideoStream(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("video stream expects resolution")));
    return scope.Close(Undefined());
  }

  video_resolution = getResolution(args[0]->IntegerValue());
  rgb_buffer = (uint8_t*)malloc(freenect_find_video_mode(video_resolution, FREENECT_VIDEO_RGB).bytes);

  freenect_set_video_callback(f_dev, video_callback);
  freenect_set_video_mode(f_dev, freenect_find_video_mode(video_resolution, FREENECT_VIDEO_RGB));

  freenect_set_video_buffer(f_dev, rgb_buffer);
  freenect_start_video(f_dev);
  freenect_stop_depth(f_dev);

  getVideoStreamCallback = Persistent<Function>::New(Handle<Function>::Cast(args[1]));

  uv_idle_init(uv_default_loop(), &idler);
  uv_idle_start(&idler, freenect_tick);

  return scope.Close(Number::New(1));
}

Handle<Value> SetLED(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("color code should be a number")));
    return scope.Close(Boolean::New(false));
  }

  freenect_led_options code = (freenect_led_options) args[0]->IntegerValue();

  if (freenect_set_led(f_dev, code) < 0) {
    return scope.Close(Boolean::New(false));
  }

  return scope.Close(Boolean::New(true));
}

Handle<Value> SetTilt(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("tilt degrees should be a number")));
    return scope.Close(Boolean::New(false));
  }

  if (freenect_set_tilt_degs(f_dev, args[0]->IntegerValue()) < 0) {
    return scope.Close(Boolean::New(false));
  }

  return scope.Close(Boolean::New(true));
}

Handle<Value> GetTilt(const Arguments& args) {
  HandleScope scope;

  if (freenect_update_tilt_state(f_dev) < 0){
    return scope.Close(Undefined());
  }

  freenect_raw_tilt_state* state = freenect_get_tilt_state(f_dev);

  return scope.Close(Integer::New(freenect_get_tilt_degs(state)));
}

void init(Handle<Object> target) {
  target->Set(String::NewSymbol("init"),
      FunctionTemplate::New(Init)->GetFunction());

  target->Set(String::NewSymbol("getVideoStream"),
      FunctionTemplate::New(GetVideoStream)->GetFunction());

  target->Set(String::NewSymbol("getDepthStream"),
      FunctionTemplate::New(GetDepthStream)->GetFunction());

  target->Set(String::NewSymbol("setLED"),
      FunctionTemplate::New(SetLED)->GetFunction());

  target->Set(String::NewSymbol("setTilt"),
      FunctionTemplate::New(SetTilt)->GetFunction());

  target->Set(String::NewSymbol("getTilt"),
      FunctionTemplate::New(GetTilt)->GetFunction());
}

NODE_MODULE(kinect, init)
