using System;
using System.Runtime.InteropServices;

namespace SlateEditor.Models;

public static class IOSurfaceInterop
{

    private const string iosurfacelib = "/System/Library/Frameworks/IOSurface.framework/IOSurface";
    
    [DllImport(iosurfacelib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr IOSurfaceLookup(uint ioSurfaceID);

}