#pragma once

#include <jni.h>
#include <string>
#include <memory>

class JString
{
public:
  const char* sz;
  JNIEnv* env = nullptr;
  jstring js = nullptr;
  std::string s;

  JString(JNIEnv* e, jstring j)
  {
    if (e == nullptr || j == nullptr)
      return;

    this->env = e;
    this->js = j;
    sz = env->GetStringUTFChars(j, nullptr);
    s.assign(sz);
  }

  virtual ~JString()
  {
    if (js == nullptr)
      return;
    env->ReleaseStringUTFChars(js, sz);
    s.clear();
  }
};

namespace
{
  struct FreeJString
  {
    void operator()(JString* p) { delete p; }
  };
}

#define JSTRING(_varname, _js) std::unique_ptr<JString, FreeJString> _varname(new JString(env, _js))
