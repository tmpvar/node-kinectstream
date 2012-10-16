var
  kinect = require('../'),
  cursor = require('ansi')(process.stdout, { enabled: true }),
  imageWidth = 320,
  imageHeight = 240,
  pixelWidth = imageWidth * 2,
  pixelHeight = imageHeight * 2,
  w, widthRatio, h, heightRatio, aspect,
  depthStream = kinect.createDepthStream();

depthStream.on('data', function(data) {

  w = process.stdout.columns;
  h = process.stdout.rows;
  aspect = w/h;
  heightRatio = Math.floor(pixelHeight/h);
  widthRatio = Math.floor(pixelWidth/w);


  cursor.goto(1, 1)


  for (var row = 0; row<=h; row++) {
    for (var col = 0; col <= w; col++) {
      cursor.goto(col, row);

      var normal = (row * pixelWidth*Math.floor(widthRatio/aspect))  + col*widthRatio;

      var val = (data[normal] << 8 |  data[normal++]) & 0x00FF;
      cursor.bg.rgb(val, 0, 0);
      process.stdout.write(' ');
    }
  }
});
