var
  binding = require('bindings')('kinect'),
  Stream = require('stream'),
  s;


module.exports.createStream = function(type) {
  // TODO: allow multiple devices
  if (s) { return s; }

  s = new Stream();
  s.readable = true;

  binding.init();

  if (type === 'depth') {
    binding.getDepthStream(function(image) {
      !s.paused && s.emit('data', image);
    });
  } else {
    binding.getVideoStream(function(image) {
      !s.paused && s.emit('data', image);
    });
  }
  return s;
};