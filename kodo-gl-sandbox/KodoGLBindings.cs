using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security;

namespace kodo_gl_sandbox
{
    /// <summary>
    /// A set of four values (<see cref="float"/>) that describes a rectangle.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    struct Rectangle
    {
        public readonly float Left;
        public readonly float Top;
        public readonly float Right;
        public readonly float Bottom;

        /// <summary>
        /// Gets the width of the <see cref="Rectangle"/>.
        /// </summary>
        public float Width => Right - Left;
        /// <summary>
        /// Gets the height of the <see cref="Rectangle"/>.
        /// </summary>
        public float Height => Bottom - Top;

        /// <summary>
        /// Creates a new <see cref="Rectangle"/> from left, top, right and bottom coordinates.
        /// </summary>
        /// <param name="l">Left.</param>
        /// <param name="t">Top.</param>
        /// <param name="r">Right.</param>
        /// <param name="b">Bottom.</param>
        public Rectangle(float l, float t, float r, float b)
        {
            Debug.Assert(l < r && t < b, "Rectangle coordinates must be defined such that l < r && t < b");

            Left = l;
            Top = t;
            Right = r;
            Bottom = b;
        }

        /// <summary>
        /// Creates a new <see cref="Rectangle"/> from left, top, right and bottom coordinates.
        /// </summary>
        /// <param name="l">Left.</param>
        /// <param name="t">Top.</param>
        /// <param name="r">Right.</param>
        /// <param name="b">Bottom.</param>
        public static Rectangle FromLTRB(float l, float t, float r, float b)
            => new Rectangle(l, t, r, b);

        /// <summary>
        /// Creates a new <see cref="Rectangle"/> at x, y location with w, h dimensions.
        /// </summary>
        /// <param name="x">X.</param>
        /// <param name="y">Y.</param>
        /// <param name="w">Width.</param>
        /// <param name="h">Height.</param>
        public static Rectangle FromXYWH(float x, float y, float w, float h)
            => new Rectangle(x, y, x + w, y + h);

        /// <summary>
        /// Indicates whether or not the <see cref="Rectangle"/> contains the specified coordinates.
        /// </summary>
        /// <param name="x">X.</param>
        /// <param name="y">Y.</param>
        public bool Contains(float x, float y)
            => (Left <= x) && (Top <= y) && (Right >= x) && (Bottom >= y);
    }

    /// <summary>
    /// A set of four values (<see cref="float"/>, 0..1) that describes a color. 
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    struct Color
    {
        public readonly float Red;
        public readonly float Green;
        public readonly float Blue;
        public readonly float Alpha;

        /// <summary>
        /// Creates a new <see cref="Color"/> from r, g, b, a values.
        /// </summary>
        /// <param name="r">Red.</param>
        /// <param name="g">Green.</param>
        /// <param name="b">Blue.</param>
        /// <param name="a">Alpha.</param>
        public Color(float r, float g, float b, float a)
        {
            Debug.Assert(r <= 1.0 && g <= 1.0 && b <= 1.0 && a <= 1.0, "Color values must be defined such that r <= 1.0 && g <= 1.0 && b <= 1.0 && a <= 1.0");

            Red = r;
            Green = g;
            Blue = b;
            Alpha = a;
        }

        /// <summary>
        /// Creates a new <see cref="Color"/> from a packed (<see cref="uint"/>, 0..255, RGBA) value.
        /// </summary>
        /// <param name="argb">A packed integer containing all four color components.</param>
        public static Color FromRGBA(uint rgba)
            => new Color(((rgba >> 24) & 255) / 255.0f, ((rgba >> 16) & 255) / 255.0f, ((rgba >> 8) & 255) / 255.0f, ((rgba & 255)) / 255.0f);

        /// <summary>
        /// The light steel blue.
        /// </summary>
        public static readonly Color LightSteelBlue = FromRGBA(0xB0C4DEFF);
    }

    class WindowManager : IDisposable
    {
        public WindowManager(KodoGLBindings.KodoGLErrorCallback errorCallback)
        {
            KodoGLBindings.KodoGLSystemCreate(errorCallback);
        }

        public double GetTime()
        {
            return KodoGLBindings.KodoGLGetTime();
        }

        public void SetTime(double time)
        {
            KodoGLBindings.KodoGLSetTime(time);
        }

        public void WaitEvents()
        {
            KodoGLBindings.KodoGLWaitEvents();
        }

        public void PollEvents()
        {
            KodoGLBindings.KodoGLPollEvents();
        }

        public void Dispose()
        {
            KodoGLBindings.KodoGLTerminate();
        }
    }

    abstract class Control
    {
        public abstract void OnDraw();
    }

    abstract class Brush
    {
        protected IntPtr handle;

        public static explicit operator IntPtr(Brush brush)
            => brush.handle;
    }

    class ColorBrush : Brush
    {
        public ColorBrush(Color color)
        {
            handle = KodoGLBindings.KodoGLBrushCreateColor(color);
        }

        public ColorBrush(Color color0, Color color1)
        {
            handle = KodoGLBindings.KodoGLBrushCreateGradient(color0, color1);
        }
    }

    public class Texture
    {
        public Texture(string filename, bool opacityOnly)
        {
            KodoGLBindings.KodoGLTextureCreate(filename, opacityOnly);
        }
    }

    [Flags]
    public enum WindowHints : int
    {
        Visible = 1,
        Decorated = 2,
        Resizable = 4
    }

    class Window
    {
        readonly IntPtr handle;

        public static explicit operator IntPtr(Window w)
            => w.handle;

        /// <summary>
        /// Indicates whether the <see cref="Window"/> should close or not.
        /// </summary>
        public bool ShouldClose
            => KodoGLBindings.KodoGLWindowShouldClose(handle);

        /// <summary>
        /// Creates a new <see cref="Window"/>.
        /// </summary>
        /// <param name="title">Title.</param>
        /// <param name="width">Width.</param>
        /// <param name="height">Height.</param>
        /// <param name="hints">Hints.</param>
        public Window(string title, int width, int height, WindowHints hints = WindowHints.Decorated | WindowHints.Resizable)
        {
            handle = KodoGLBindings.KodoGLWindowCreate(width, height, hints, title);

            KodoGLBindings.KodoGLWindowSetMouseContainedCallback(handle, WindowMouseContainedCallback);
            KodoGLBindings.KodoGLWindowSetMousePositionCallback(handle, WindowMousePositionCallback);
            KodoGLBindings.KodoGLWindowSetRefreshCallback(handle, WindowRefreshCallback);
            KodoGLBindings.KodoGLWindowSetSizeCallback(handle, WindowSizeCallback);
        }

        /// <summary>
        /// Sets the <see cref="Window"/> swap interval.
        /// </summary>
        /// <param name="interval"></param>
        public void SwapInterval(int interval)
        {
            KodoGLBindings.KodoGLWindowSwapInterval(handle, interval);
        }

        /// <summary>
        /// Begins a frame.
        /// </summary>
        public void BeginFrame()
        {
            KodoGLBindings.KodoGLWindowFrameBegin(handle);
        }

        /// <summary>
        /// Ends a frame.
        /// </summary>
        public void EndFrame()
        {
            KodoGLBindings.KodoGLWindowFrameEnd(handle);
        }

        void WindowMouseContainedCallback(bool contained)
        {
        }

        void WindowMousePositionCallback(float x, float y)
        {
        }

        void WindowSizeCallback(int width, int height)
        {

        }

        void WindowRefreshCallback()
        {
        }
    }

    class DrawingContext
    {
        Rectangle area;
        readonly IntPtr handle;

        public Rectangle Area
        {
            get { return area; }
            set
            {
                area = value;
                KodoGLBindings.KodoGLDrawingContextSetArea(handle, area);
            }
        }

        public DrawingContext(Window window)
        {
            handle = KodoGLBindings.KodoGLDrawingContextCreate((IntPtr)window);
            KodoGLBindings.KodoGLDrawingContextGetArea(handle, out area);
        }

        public void DrawQuad(Rectangle quad, Brush brush)
            => KodoGLBindings.KodoGLDrawingContextDrawQuad(handle, quad, (IntPtr)brush);

        public void DrawQuads(Rectangle[] quads, Brush brush)
            => KodoGLBindings.KodoGLDrawingContextDrawQuads(handle, Marshal.UnsafeAddrOfPinnedArrayElement(quads, 0), quads.Length, (IntPtr)brush);
    }

    [SuppressUnmanagedCodeSecurity]
    static class KodoGLBindings
    {
        const CallingConvention KodoGLConvention = CallingConvention.Cdecl;
        const string KodoGL = "kodo-gl.dll";

        [UnmanagedFunctionPointer(KodoGLConvention)]
        public delegate void KodoGLErrorCallback(string message);

        [UnmanagedFunctionPointer(KodoGLConvention)]
        public delegate void KodoGLWindowSizeCallback(int width, int height);

        [UnmanagedFunctionPointer(KodoGLConvention)]
        public delegate void KodoGLWindowRefreshCallback();

        [UnmanagedFunctionPointer(KodoGLConvention)]
        public delegate void KodoGLWindowMousePositionCallback(float x, float y);

        [UnmanagedFunctionPointer(KodoGLConvention)]
        public delegate void KodoGLWindowMouseContainedCallback(bool contained);

        public static bool KodoGLIsValid(IntPtr ptr)
        {
            return ptr != IntPtr.Zero;
        }

        //
        // WindowManager
        //

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLSystemCreate(KodoGLErrorCallback callback);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLTerminate();

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLPollEvents();

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWaitEvents();

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern double KodoGLGetTime();

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLSetTime(double time);

        //
        // Window
        //

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowFrameBegin(IntPtr window);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowFrameEnd(IntPtr window);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowSwapInterval(IntPtr window, int interval);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern IntPtr KodoGLWindowCreate(int width, int height, WindowHints flags, string title);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowSetSize(IntPtr window, int width, int height);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern bool KodoGLWindowShouldClose(IntPtr window);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowSetVisible(IntPtr window, bool visible);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowSetMouseContainedCallback(IntPtr window, KodoGLWindowMouseContainedCallback callback);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowSetMousePositionCallback(IntPtr window, KodoGLWindowMousePositionCallback callback);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowSetSizeCallback(IntPtr window, KodoGLWindowSizeCallback callback);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLWindowSetRefreshCallback(IntPtr window, KodoGLWindowRefreshCallback callback);

        //
        // DrawingContext
        //

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern IntPtr KodoGLDrawingContextCreate(IntPtr window);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLDrawingContextGetArea(IntPtr context, out Rectangle area);
        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLDrawingContextSetArea(IntPtr context, Rectangle area);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLDrawingContextDraw(IntPtr context, int r, IntPtr brush);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLDrawingContextDrawQuad(IntPtr context, Rectangle quad, IntPtr brush);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern void KodoGLDrawingContextDrawQuads(IntPtr context, IntPtr quads, int quadsLength, IntPtr brush);

        //
        // Texture
        //

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern IntPtr KodoGLTextureCreate(string filename, bool opacityOnly);

        //
        // Brush
        //

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern IntPtr KodoGLBrushCreateColor(Color color);

        [DllImport(KodoGL, CallingConvention = KodoGLConvention)]
        public static extern IntPtr KodoGLBrushCreateGradient(Color color0, Color color1);
    }
}
