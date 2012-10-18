var
  server = new (require('binaryjs').BinaryServer)({ port : 10001 }),
  kinect = require('../../index').createStream('video', '640x480', 3),
  ecstatic = require('ecstatic')(__dirname + '/static');

require('http').createServer(ecstatic).listen(10000);

server.on('connection', function(c) {
  kinect.pipe(c.createStream());
});

require('open')('http://localhost:10000');
