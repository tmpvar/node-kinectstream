var
  kinect = require('../'),
  cursor = require('ansi')(process.stdout, { enabled: true }),
  imageWidth = 1280,
  imageHeight = 1024,
  pixelWidth = imageWidth * 3,
  pixelHeight = imageHeight * 3,
  w, widthRatio, h, heightRatio, aspect;

kinect.getVideoStream(function(data) {

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
      normal-= normal%3;
      var r = data[normal], g = data[normal+1], b = data[normal+2];

      cursor.bg.rgb(r, g, b);
      process.stdout.write(' ');
    }
  }
});
