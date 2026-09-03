#!/bin/bash

cd /home/jonkadelic/win7/Shogo
env MESA_DEBUG=1 __GLX_VENDOR_LIBRARY_NAME=mesa __EGL_VENDOR_LIBRARY_FILENAMES=/usr/share/glvnd/egl_vendor.d/50_mesa.json MESA_LOADER_DRIVER_OVERRIDE=zink GALLIUM_DRIVER=zink LIBGL_KOPPER_DRI2=1 wine ../w64devkit/bin/gdb.exe -x gdbinit --args Client.exe -workingdir C:\\jonkwin\\Shogo -rez shogo.rez -rez shogop.rez -rez shogop2.rez -rez shogop3.rez -rez shogop4.rez -rez sound.rez
