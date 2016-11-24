#include "frontalFaceDetector.h"

#include <math.h>

class DetectAsyncWorker : public Nan::AsyncWorker {
 public:
  DetectAsyncWorker(FrontalFaceDetector * frontalFaceDetector, std::string filename, Nan::Callback *callback)
    : Nan::AsyncWorker(callback)
    , frontalFaceDetector(frontalFaceDetector)
    , filename(filename) {
    }

  ~DetectAsyncWorker() {}

  void Execute () {
    try {
      // dlib::array2d<unsigned char> img;
      dlib::array2d<dlib::rgb_pixel> img;
      dlib::load_image(img, filename);
      //dlib::pyramid_up(img);

      dets = frontalFaceDetector->detector(img);

      if (!frontalFaceDetector->shape.empty()) {
        for (unsigned int i = 0; i < dets.size(); i++) {
          dlib::full_object_detection shape = frontalFaceDetector->sp(img, dets[i]);

          std::map<std::string, dlib::point> points;

          dlib::vector<double, 2> l, r, n, m;
          double cnt = 0;

          // Find the center of the left eye by averaging the points around the eye.
          for (unsigned long j = 36; j <= 41; ++j) {
              l += shape.part(j);
              ++cnt;
          }
          l /= cnt;

          // Find the center of the right eye by averaging the points around the eye.
          cnt = 0;
          for (unsigned long j = 42; j <= 47; ++j) {
              r += shape.part(j);
              ++cnt;
          }
          r /= cnt;

          points["eye_left"] = l;
          points["eye_right"] = r;


          cnt = 0;
          // Line on top of nose
          for (unsigned long j = 28; j <= 30; ++j) {
              n += shape.part(j);
              ++cnt;
          }

          // Bottom part of the nose
          for (unsigned long j = 31; j <= 35; ++j) {
              n += shape.part(j);
              ++cnt;
          }
          n /= cnt;
          points["nose"] = n;


          cnt = 0;
          // Lips outer part
          for (unsigned long j = 49; j <= 59; ++j) {
              m += shape.part(j);
              ++cnt;
          }

          // Lips inside part
          for (unsigned long j = 61; j <= 67; ++j) {
              m += shape.part(j);
              ++cnt;
          }
          m /= cnt;
          points["mouth"] = m;


          features.push_back(points);
        }
      }
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      SetErrorMessage(e.what());
    }
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;

    v8::Local<v8::Array> arr = Nan::New<v8::Array>(dets.size());

    for (unsigned int i = 0; i < dets.size(); i++) {
      v8::Local<v8::Object> o = Nan::New<v8::Object>();

      o->Set(Nan::New("x").ToLocalChecked(), Nan::New<v8::Number>(dets[i].left()));
      o->Set(Nan::New("y").ToLocalChecked(), Nan::New<v8::Number>(dets[i].top()));
      o->Set(Nan::New("width").ToLocalChecked(), Nan::New<v8::Number>(dets[i].width()));
      o->Set(Nan::New("height").ToLocalChecked(), Nan::New<v8::Number>(dets[i].height()));

      if (!frontalFaceDetector->shape.empty()) {

        auto dx = features[i]["eye_right"].x()-features[i]["eye_left"].x();
        auto dy = features[i]["eye_right"].y()-features[i]["eye_left"].y();
        auto roll = atan2(dy, dx) * 180.0/M_PI;

        o->Set(Nan::New("roll").ToLocalChecked(), Nan::New<v8::Number>(roll));

        for (auto & feature : features[i]) {
          v8::Local<v8::Object> p = Nan::New<v8::Object>();
          p->Set(Nan::New("x").ToLocalChecked(), Nan::New<v8::Number>(feature.second.x()));
          p->Set(Nan::New("y").ToLocalChecked(), Nan::New<v8::Number>(feature.second.y()));

          o->Set(Nan::New(feature.first).ToLocalChecked(), p);
        }
      }

      arr->Set(i, o);
    }

    v8::Local<v8::Value> argv[2];
    argv[0] = Nan::Null();
    argv[1] = arr;

    Nan::TryCatch try_catch;
    callback->Call(2, argv);
    if (try_catch.HasCaught()) {
      Nan::FatalException(try_catch);
    }
  }

 private:
  FrontalFaceDetector * frontalFaceDetector;
  std::string filename;
  std::vector<dlib::rectangle> dets;
  std::vector<std::map<std::string, dlib::point>> features;
};

///////////////////////////

Nan::Persistent<v8::Function> FrontalFaceDetector::constructor;

FrontalFaceDetector::FrontalFaceDetector(const std::string & shape) : shape(shape) {
  detector = dlib::get_frontal_face_detector();
  if (!shape.empty()) {
    dlib::deserialize(shape) >> sp;
  }
}

FrontalFaceDetector::~FrontalFaceDetector() {
}

void FrontalFaceDetector::Init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("FrontalFaceDetector").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "detect", Detect);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("FrontalFaceDetector").ToLocalChecked(), tpl->GetFunction());
}

void FrontalFaceDetector::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new FrontalFaceDetector(...)`
    std::string shape = info[0]->IsUndefined() ? "" : std::string(*Nan::Utf8String(info[0]->ToString()));
    FrontalFaceDetector* frontalFaceDetector = new FrontalFaceDetector(shape);
    frontalFaceDetector->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `FrontalFaceDetector(...)`, turn into construct call.
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = { info[0] };
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

void FrontalFaceDetector::Detect(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  FrontalFaceDetector* frontalFaceDetector = ObjectWrap::Unwrap<FrontalFaceDetector>(info.Holder());

  std::string filename = std::string(*Nan::Utf8String(info[0]->ToString()));
  Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());

  Nan::AsyncQueueWorker(new DetectAsyncWorker(frontalFaceDetector, filename, callback));
}
