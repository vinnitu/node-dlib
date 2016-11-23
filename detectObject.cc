#include "detectObject.h"

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_io.h>

using v8::Function;
using v8::Local;
using v8::Number;
using v8::Value;
using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::HandleScope;
using Nan::New;
using Nan::Null;
using Nan::To;

class DetectObjectWorker : public AsyncWorker {
 public:
  DetectObjectWorker(Callback *callback, std::string filename)
    : AsyncWorker(callback), filename(filename) {}
  ~DetectObjectWorker() {}

  void Execute () {
    try
    {
      dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

      dlib::array2d<unsigned char> img;
      dlib::load_image(img, filename);

      dlib::pyramid_up(img);
      std::vector<dlib::rectangle> dets = detector(img);

      res = dets;
    } catch (std::exception& e) {
      SetErrorMessage(e.what());
    }
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;

    Local<Value>argv[2];
    v8::Local<v8::Array> arr = Nan::New<v8::Array>(this->res.size());

    for (unsigned int i = 0; i < this->res.size(); i++) {
      v8::Local < v8::Object > x = Nan::New<v8::Object>();
      x->Set(Nan::New("x").ToLocalChecked(), Nan::New<Number>(res[i].left()));
      x->Set(Nan::New("y").ToLocalChecked(), Nan::New<Number>(res[i].top()));
      x->Set(Nan::New("width").ToLocalChecked(), Nan::New<Number>(res[i].width()));
      x->Set(Nan::New("height").ToLocalChecked(), Nan::New<Number>(res[i].height()));
      arr->Set(i, x);
    }

    argv[0] = Nan::Null();
    argv[1] = arr;

    Nan::TryCatch try_catch;
    callback->Call(2, argv);
    if (try_catch.HasCaught()) {
      Nan::FatalException(try_catch);
    }
  }

 private:
  std::string filename;
  std::vector<dlib::rectangle> res;
};

NAN_METHOD(DetectObject) {
  std::string filename = std::string(*Nan::Utf8String(info[0]->ToString()));
  Callback *callback = new Callback(info[1].As<Function>());

  AsyncQueueWorker(new DetectObjectWorker(callback, filename));
}
