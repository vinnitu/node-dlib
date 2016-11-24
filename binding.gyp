{
  "targets": [
    {
      "target_name": "dlib",
      "sources": [
        "dlib.cc",
        "frontalFaceDetector.cc"
      ],
      "include_dirs": ["<!(node -e \"require('nan')\")"],
      "cflags_cc!": [ "-fno-rtti" ],
      "cflags_cc": [ "-std=c++11", "-fexceptions" ],
      "libraries": [ "-lpthread", "-lgif", "-lpng", "-ljpeg", "-lopenblas", "-ldlib" ]
    }
  ]
}
