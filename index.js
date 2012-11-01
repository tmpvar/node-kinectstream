var
  binding = require('bindings')('kinect'),
  Stream = require('stream'),
  s;

binding.init();

module.exports = {

  createStream: function(type, resolution, fps) {
    // TODO: allow multiple devices
    if (s) { return s; }

    fps = fps || 30;

    s = new Stream();
    s.readable = true;

    s.lastFrame = 0;

    var emit = function(image) {
      if (1000/fps + s.lastFrame < Date.now()) {
        !s.paused && s.emit('data', image);
        s.lastFrame = Date.now();
      }
    };

    if (type === 'depth') {
      binding.getDepthStream(emit);
    } else {

      var res = [null, '640x480', '1280x1024'];
      var selected = res.indexOf(resolution);
      selected = selected > -1 ? selected : 0;

      binding.getVideoStream(selected, emit);
    }
    return s;
  },

  led: function(color) {
    colors = ["off", "green", "red", "yellow", "blink yellow", "blink green", "blink red yellow"];
    var idx = colors.indexOf(color);
    if (idx === -1) {
      return false;
    }
    binding.setLED(idx);
  }

};