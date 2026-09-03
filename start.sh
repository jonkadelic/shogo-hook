#!/bin/bash

cd /home/jonkadelic/win7/Shogo/
env __GLX_VENDOR_LIBRARY_NAME=mesa __EGL_VENDOR_LIBRARY_FILENAMES=/usr/share/glvnd/egl_vendor.d/50_mesa.json wine ./start.bat
