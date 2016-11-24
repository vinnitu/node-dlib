#include <nan.h>
#include "frontalFaceDetector.h"

void InitAll(v8::Local<v8::Object> exports) {
  FrontalFaceDetector::Init(exports);
}

NODE_MODULE(dlib, InitAll)
