#include "frontalFaceDetector.h"

class DetectAsyncWorker : public Nan::AsyncWorker {
 public:
  DetectAsyncWorker(FrontalFaceDetector * frontalFaceDetector, std::string filename, Nan::Callback *callback)
    : Nan::AsyncWorker(callback)
    , frontalFaceDetector(frontalFaceDetector)
    , filename(filename) {}

  ~DetectAsyncWorker() {}

  void Execute () {
    try
    {
      dlib::array2d<unsigned char> img;
      dlib::load_image(img, filename);

      //dlib::pyramid_up(img);
      std::vector<dlib::rectangle> dets = frontalFaceDetector->detector(img);

      res = dets;
    } catch (std::exception& e) {
      SetErrorMessage(e.what());
    }
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;

    v8::Local<v8::Value>argv[2];
    v8::Local<v8::Array> arr = Nan::New<v8::Array>(this->res.size());

    for (unsigned int i = 0; i < this->res.size(); i++) {
      v8::Local < v8::Object > x = Nan::New<v8::Object>();
      x->Set(Nan::New("x").ToLocalChecked(), Nan::New<v8::Number>(res[i].left()));
      x->Set(Nan::New("y").ToLocalChecked(), Nan::New<v8::Number>(res[i].top()));
      x->Set(Nan::New("width").ToLocalChecked(), Nan::New<v8::Number>(res[i].width()));
      x->Set(Nan::New("height").ToLocalChecked(), Nan::New<v8::Number>(res[i].height()));
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
  FrontalFaceDetector * frontalFaceDetector;
  std::string filename;
  std::vector<dlib::rectangle> res;
};

///////////////////////////

Nan::Persistent<v8::Function> FrontalFaceDetector::constructor;

FrontalFaceDetector::FrontalFaceDetector() {
  detector = dlib::get_frontal_face_detector();
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
    FrontalFaceDetector* frontalFaceDetector = new FrontalFaceDetector();
    frontalFaceDetector->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `FrontalFaceDetector(...)`, turn into construct call.
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance());
  }
}

void FrontalFaceDetector::Detect(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  FrontalFaceDetector* frontalFaceDetector = ObjectWrap::Unwrap<FrontalFaceDetector>(info.Holder());

  std::string filename = std::string(*Nan::Utf8String(info[0]->ToString()));
  Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());

  Nan::AsyncQueueWorker(new DetectAsyncWorker(frontalFaceDetector, filename, callback));
}
