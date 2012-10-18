var
  server = new (require('binaryjs').BinaryServer)({ port : 10001 }),
  kinect = require('../../index').createStream('depth'),
  ecstatic = require('ecstatic')(__dirname + '/static');

require('http').createServer(ecstatic).listen(10000);

server.on('connection', function(c) {
  //kinect.pipe(c.createStream());
  kinect.once('data', c.send.bind(c));
});

require('open')('http://localhost:10000');
