using System;


namespace SlateEditor.Models;

public class IOSurfaceImageView
{
    // CIContext _context = CIContext.FromOptions(null);
    // IntPtr _surface;
    //
    // public void SetIOSurface(IntPtr surface)
    // {
    //     _surface = surface;
    //     NeedsDisplay = true;
    // }
    //
    // public override void DrawRect(CoreGraphics.CGRect dirtyRect)
    // {
    //     base.DrawRect(dirtyRect);
    //
    //     if (_surface != IntPtr.Zero)
    //     {
    //         var ioSurface = new IOSurface.IOSurface(_surface);
    //         var ciImage = new CIImage(ioSurface);
    //         _context.DrawImage(ciImage, dirtyRect, ciImage.Extent);
    //     }
    // }
}