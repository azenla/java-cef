// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_app.h"
#include "jni_util.h"
#include "util.h"
#if defined(OS_MACOSX)
#include "util_mac.h"
#endif

ClientApp::ClientApp(const std::string& module_dir, const jobject app_handler)
    : module_dir_(module_dir), app_handler_(NULL) {
  JNIEnv *env = GetJNIEnv();
  if (env)
    app_handler_ = env->NewGlobalRef(app_handler);
  process_handler_ = new BrowserProcessHandler(app_handler_);
}

ClientApp::~ClientApp() {
  if (!app_handler_)
    return;
  BEGIN_ENV(env)
  env->DeleteGlobalRef(app_handler_);
  END_ENV(env)
}

void ClientApp::OnBeforeCommandLineProcessing(const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  // If the java code has registered an AppHandler, we'll forward
  // the commandline processing to it before we append the essential
  // switches "locale_pak" and "use-core-animation".
  if (app_handler_ != NULL && process_type.empty()) {
    BEGIN_ENV(env)
    jstring jprocess_type = NewJNIString(env, process_type);
    jobject jcommand_line = NewJNIObject(env, "org/cef/callback/CefCommandLine_N");
    if (jcommand_line != NULL) {
      SetCefForJNIObject(env, jcommand_line, command_line.get(), "CefCommandLine");
      JNI_CALL_VOID_METHOD(env,
                           app_handler_,
                           "onBeforeCommandLineProcessing",
                           "(Ljava/lang/String;Lorg/cef/callback/CefCommandLine;)V",
                           jprocess_type,
                           jcommand_line);
      SetCefForJNIObject<CefCommandLine>(env, jcommand_line, NULL, "CefCommandLine");
    }
    END_ENV(env)
  }

  if (process_type.empty()) {
#if defined(OS_MACOSX)
    // Specify a path for the locale.pak file because CEF will fail to locate
    // it based on the app bundle structure.
    const std::string& locale_path = util_mac::GetAbsPath(
        module_dir_ + "/../Frameworks/Chromium Embedded Framework.framework/"
                     "Resources/en.lproj/locale.pak");
    command_line->AppendSwitchWithValue("locale_pak", locale_path);
    // If windowed rendering is used, we need the browser window as CALayer
    // due Java7 is CALayer based instead of NSLayer based.
    command_line->AppendSwitch("use-core-animation");
#endif  // defined(OS_MACOSX)
  }
}

bool ClientApp::HandleTerminate() {
  BEGIN_ENV(env)
  jclass cls = env->FindClass("org/cef/CefApp");
  if (!cls) {
    return false;
  }

  jmethodID methodId =
      env->GetStaticMethodID(cls, "getInstance", "()Lorg/cef/CefApp;");
  if (!methodId) {
    return false;
  }

  jobject jcefApp = env->CallStaticObjectMethod(cls, methodId);
  if (!jcefApp) {
    return false;
  }

  JNI_CALL_VOID_METHOD(env, jcefApp, "handleBeforeTerminate", "()V");
  END_ENV(env)
  return true;
}

void ClientApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) {
  if (!app_handler_)
    return;

  BEGIN_ENV(env)
  jobject jregistrar = NewJNIObject(env, "org/cef/callback/CefSchemeRegistrar_N");
  if (jregistrar != NULL) {
    SetCefForJNIObject(env, jregistrar, registrar.get(), "CefSchemeRegistrar");
    JNI_CALL_VOID_METHOD(env,
                         app_handler_,
                         "onRegisterCustomSchemes",
                         "(Lorg/cef/callback/CefSchemeRegistrar;)V",
                         jregistrar);
    SetCefForJNIObject<CefSchemeRegistrar>(env, jregistrar, NULL, "CefSchemeRegistrar");
  }
  END_ENV(env)
}

CefRefPtr<CefBrowserProcessHandler> ClientApp::GetBrowserProcessHandler() {
  return process_handler_.get();
}
