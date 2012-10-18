var
  kinect = require('../'),
  cursor = require('ansi')(process.stdout, { enabled: true }),
  imageWidth = 320,
  imageHeight = 240,
  pixelWidth = imageWidth * 2,
  pixelHeight = imageHeight * 2,
  w, widthRatio, h, heightRatio, aspect;

kinect.createStream('depth').on('data', function(data) {

  w = process.stdout.columns;
  h = process.stdout.rows;
  aspect = w/h;
  heightRatio = Math.floor(pixelHeight/h);
  widthRatio = Math.floor(pixelWidth/w);

  cursor.goto(1, 1)

  for (var row = 0; row<=h; row++) {
    for (var col = 0; col <= w; col++) {
      cursor.goto(col+1, row+1);

      var normal = (row * pixelWidth)  + col * widthRatio;

      var val = (data[normal] << 8 | data[normal++]) & 0x00FF;
      cursor.bg.rgb(val, val, val);
      process.stdout.write(' ');
    }
  }
});
