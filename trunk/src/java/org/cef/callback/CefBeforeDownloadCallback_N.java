// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

package org.cef.callback;

class CefBeforeDownloadCallback_N implements CefBeforeDownloadCallback {
  // Used internally to store a pointer to the CEF object.
  private long N_CefHandle = 0;

  @Override
  public void setNativeRef(String identifer, long nativeRef) {
    N_CefHandle = nativeRef;
  }

  @Override
  public long getNativeRef(String identifer) {
    return N_CefHandle;
  }

  CefBeforeDownloadCallback_N() {
  }

  @Override
  public void Continue(String downloadPath, boolean showDialog) {
    try {
      N_Continue(downloadPath, showDialog);
    } catch (UnsatisfiedLinkError ule) {
      ule.printStackTrace();
    }
  }

  private final native void N_Continue(String downloadPath, boolean showDialog);
}
