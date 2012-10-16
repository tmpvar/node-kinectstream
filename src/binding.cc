

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

uv_idle_t idler;

void freenect_tick(uv_idle_t* handle, int status) {
  freenect_process_events(f_ctx);
}

Handle<Value> Init(const Arguments& args) {
  HandleScope scope;

  rgb_buffer = (uint8_t*)malloc(freenect_find_video_mode(FREENECT_RESOLUTION_HIGH, FREENECT_VIDEO_RGB).bytes);

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


  uv_idle_init(uv_default_loop(), &idler);
  uv_idle_start(&idler, freenect_tick);

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
  unsigned int length = 640*480;
  Buffer *buf = Buffer::New(length);

  memcpy(Buffer::Data(buf), depth_buffer, length);

  callJSCallbackWithBuffer(getDepthStreamCallback, buf);

  scope.Close(Undefined());
}

void video_callback(freenect_device *dev, void *rgb, uint32_t timestamp) {

  HandleScope scope;

  unsigned int length = 1280*1024*3;
  Buffer *buf = Buffer::New(length);

  memcpy(Buffer::Data(buf), rgb_buffer, length);

  callJSCallbackWithBuffer(getVideoStreamCallback, buf);

  scope.Close(Undefined());
}

Handle<Value> GetDepthStream(const Arguments& args) {
  freenect_set_depth_callback(f_dev, depth_callback);
  freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_HIGH, FREENECT_DEPTH_11BIT));
  freenect_set_depth_buffer(f_dev, depth_buffer);
  freenect_start_depth(f_dev);
}

Handle<Value> GetVideoStream(const Arguments& args) {
  HandleScope scope;

  freenect_set_video_callback(f_dev, video_callback);
  freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_IR_8BIT));
  freenect_set_video_buffer(f_dev, rgb_buffer);
  freenect_start_video(f_dev);

  getVideoStreamCallback = Persistent<Function>::New(Handle<Function>::Cast(args[0]));

  return scope.Close(Number::New(1));
}

void init(Handle<Object> target) {
  target->Set(String::NewSymbol("init"),
      FunctionTemplate::New(Init)->GetFunction());

  target->Set(String::NewSymbol("getVideoStream"),
      FunctionTemplate::New(GetVideoStream)->GetFunction());

  target->Set(String::NewSymbol("getDepthStream"),
      FunctionTemplate::New(GetVideoStream)->GetFunction());
}

NODE_MODULE(kinect, init)
