node-dlib
=========

Node wrapper for dlib with limited functionality.

Build on Debian/Ubuntu:

```bash
apt install -y libdlib-dev libgif-dev libpng-dev libjpeg-dev
npm i node-dlib
```

Basic usage:

```js
var dlib = require('node-dlib'),
    faced = new dlib.FrontalFaceDetector();

faced.detect('frame.jpeg', (err, faces) => {
  if (err) throw err;
  faces.forEach(f => {
    console.log('Face at', f.x, f.y);
  });
});
```
