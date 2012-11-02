#include "kinect.h"

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

void Kinect::callback(freenect_device *dev, void *incoming_buffer, uint32_t timestamp) {
  Buffer *outgoing_buffer = Buffer::New(this->bufferLength);
  memcpy(Buffer::Data(outgoing_buffer), incoming_buffer, this->bufferLength);
  callJSCallbackWithBuffer(getVideoStreamCallback, outgoing_buffer);
}

static void Kinect::Init(Handle<Object> target) {

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Kinect"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set(String::NewSymbol("getVideoStream"),
      FunctionTemplate::New(GetVideoStream)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("getDepthStream"),
      FunctionTemplate::New(GetDepthStream)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("setLED"),
      FunctionTemplate::New(SetLED)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("setTilt"),
      FunctionTemplate::New(SetTilt)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("getTilt"),
      FunctionTemplate::New(GetTilt)->GetFunction());
}

Handle<Value> Kinect::New(const Arguments& args) {
  HandleScope scope;

  Kinect* obj = new Kinect();

  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("provide a kinect device index")));
    return scope.Close(Undefined());
  }

  if (!obj->CreateDevice(args[0]->Int32Value())) {
    return ThrowException(Exception::TypeError(String::New("could not create kinect device")));
  }

  obj->Wrap(args.This());

  return args.This();
}

int Kinect::CreateDevice(int deviceIndex) {
  int nr_devices = freenect_num_devices (f_ctx);

  if (nr_devices < 1) {
    return 0;
  }

  if (freenect_open_device(f_ctx, &this->device, deviceIndex) < 0) {
    return 0;
  }

  return 1;
}

Handle<Value> Kinect::GetDepthStream(const Arguments& args) {
  HandleScope scope;

  freenect_set_depth_callback(this->device, this->callback);
  this->bufferLength = freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED).bytes;
  buffer = (uint8_t*)malloc(this->bufferLength);
  freenect_set_depth_mode(this->device, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED));
  freenect_set_depth_buffer(this->device, depth_buffer);
  freenect_start_depth(this->device);
  freenect_stop_video(this->device);

  getDepthStreamCallback = Persistent<Function>::New(Handle<Function>::Cast(args[0]));

  uv_idle_init(uv_default_loop(), &idler);
  uv_idle_start(&idler, freenect_tick);

  return scope.Close(Undefined());
}

Handle<Value> Kinect::GetVideoStream(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("video stream expects resolution")));
    return scope.Close(Undefined());
  }

  video_resolution = getResolution(args[0]->IntegerValue());
  this.bufferLength = freenect_find_video_mode(video_resolution, FREENECT_VIDEO_RGB).bytes;
  this->buffer = (uint8_t*)malloc(this.bufferLength);

  freenect_set_video_callback(this->device, this->callback);
  freenect_set_video_mode(this->device, freenect_find_video_mode(video_resolution, FREENECT_VIDEO_RGB));

  freenect_set_video_buffer(this->device, rgb_buffer);
  freenect_start_video(this->device);
  freenect_stop_depth(this->device);

  getVideoStreamCallback = Persistent<Function>::New(Handle<Function>::Cast(args[1]));

  uv_idle_init(uv_default_loop(), &idler);
  uv_idle_start(&idler, freenect_tick);

  return scope.Close(Number::New(1));
}

Handle<Value> Kinect::SetLED(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("color code should be a number")));
    return scope.Close(Boolean::New(false));
  }

  freenect_led_options code = (freenect_led_options) args[0]->IntegerValue();

  if (freenect_set_led(this->device, code) < 0) {
    return scope.Close(Boolean::New(false));
  }

  return scope.Close(Boolean::New(true));
}

Handle<Value> Kinect::SetTilt(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("tilt degrees should be a number")));
    return scope.Close(Boolean::New(false));
  }

  if (freenect_set_tilt_degs(this->device, args[0]->IntegerValue()) < 0) {
    return scope.Close(Boolean::New(false));
  }

  return scope.Close(Boolean::New(true));
}

Handle<Value> Kinect::GetTilt(const Arguments& args) {
  HandleScope scope;

  if (freenect_update_tilt_state(this->device) < 0){
    return scope.Close(Undefined());
  }

  freenect_raw_tilt_state* state = freenect_get_tilt_state(this->device);

  return scope.Close(Integer::New(freenect_get_tilt_degs(state)));
}