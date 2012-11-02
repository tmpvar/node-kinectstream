#ifndef _KINECT_H
#define _KINECT_H

#include <node.h>
#include <node_buffer.h>
#include <v8.h>

using namespace v8;
using namespace node;

extern "C" {
  #include <libfreenect/libfreenect.h>
}

freenect_context *f_ctx;

void callJSCallbackWithBuffer(Persistent<Function> cb, Buffer *buffer);



class Kinect : public ObjectWrap {
 public:
  static void Init(Handle<Object> target);
  int CreateDevice(int deviceIndex);
  void callback(freenect_device *dev, void *incoming_buffer, uint32_t timestamp);

 private:
  Kinect();
  ~Kinect();

  static Handle<Value> New(const Arguments& args);
  Handle<Value> CreateDevice(const Arguments& args);
  Handle<Value> GetDepthStream(const Arguments& args);
  Handle<Value> GetVideoStream(const Arguments& args);
  Handle<Value> SetLED(const Arguments& args);
  Handle<Value> SetTilt(const Arguments& args);
  Handle<Value> GetTilt(const Arguments& args);

  uv_idle_t idler;
  freenect_device *device;
  uint8_t *rgb_buffer, *depth_buffer;
  freenect_resolution video_resolution;

  Persistent<Function> getVideoStreamCallback;
  Persistent<Function> getDepthStreamCallback;
};

#endif