#ifndef FRONTAL_FACE_DETECTOR_H
#define FRONTAL_FACE_DETECTOR_H

#include <nan.h>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_io.h>

class FrontalFaceDetector : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  dlib::frontal_face_detector detector;

 private:
  explicit FrontalFaceDetector();
  ~FrontalFaceDetector();

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Detect(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static Nan::Persistent<v8::Function> constructor;
};


#endif // FRONTAL_FACE_DETECTOR_H