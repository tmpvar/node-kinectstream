var
  binding = require('bindings')('kinect'),
  Stream = require('stream');

binding.init();

module.exports = {
  createDepthStream : function() {
    var s = new Stream();
    s.readable = true;

    binding.getDepthStream(function(image) {
      s.emit('data', image);
    });

    return s;
  },

  createVideoStream : function() {
    var s = new Stream()
    s.readable = true;

    binding.getVideoStream(function(image) {
      s.emit('data', image);
    });

    return s;
  }
};