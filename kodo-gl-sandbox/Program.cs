using System;

namespace kodo_gl_sandbox
{
    class Program
    {
        static void ErrorCallback(string message)
        {
            Console.WriteLine(message);
        }

        static void Main(string[] args)
        {
            using (var windowManager = new WindowManager(ErrorCallback))
            {
                var window = new Window("kodogl-sandbox", 1280, 720, WindowHints.Decorated | WindowHints.Resizable | WindowHints.Visible);
                window.SwapInterval(2);

                var contextArea = Rectangle.FromXYWH(10, 10, 512, (512.0f / (16.0f / 9.0f)));
                var context = new DrawingContext(window);
                context.Area = contextArea;

                var baseBrush = new ColorBrush(Color.LightSteelBlue);

                const int SpectrumSize = 512;
                const int SpectrumStart = 2;
                const int SpectrumEnd = 130;
                const int SpectrumLength = SpectrumEnd - SpectrumStart;

                float[] audioData = new float[SpectrumSize];

                float barWidth = 2;
                const float BarMinimum = 2;
                const float BarSpacer = 1;
                const float PeakDrop = 0.5f;

                var areaWidth = contextArea.Width;
                var areaHalfWidth = areaWidth / 2f;
                var areaHeight = contextArea.Height;
                var areaHalfHeight = areaHeight / 2f;

                var barCount = (int)Math.Ceiling((areaHalfWidth - BarSpacer / 2f) / (barWidth + BarSpacer));
                barCount = Math.Min(barCount, SpectrumLength);
                var dataPerBar = (int)Math.Floor((float)SpectrumLength / barCount);

                if (barCount == SpectrumLength)
                {
                    barWidth = (areaHalfWidth - (barCount * BarSpacer)) / barCount;
                }

                var peaks = new float[barCount];
                var quads = new Rectangle[barCount * 3 * 2];

                for (var i = 0; i < peaks.Length; i++)
                    peaks[i] = areaHalfHeight;

                var random = new Random();

                var frameBeginTime = 0.0;
                var frameEndTime = 0.0;

                while (!window.ShouldClose)
                {
                    windowManager.PollEvents();

                    frameBeginTime = windowManager.GetTime();

                    window.BeginFrame();

                    for (var i = 0; i < SpectrumEnd; i++)
                    {
                        audioData[i] = (float)(random.NextDouble());
                    }

                    var quadCounter = 0;
                    var lastBarHeight = BarMinimum;
                    // Calculate the initial horizontal coordinates.
                    var barX0 = areaHalfWidth - (BarSpacer / 2f);
                    var barX1 = areaHalfWidth + (BarSpacer / 2f);

                    for (var i = 0; i < barCount; i++)
                    {
                        var barHeight = 0f;

                        // Grab a "bucket" of audio data.
                        for (var j = 0; j < dataPerBar; j++)
                            barHeight += audioData[SpectrumStart + (i * dataPerBar) + j];

                        // Averaging the audio data "bucket".
                        barHeight /= dataPerBar;
                        // Actual pixel height calculation.
                        barHeight = BarMinimum + (barHeight * contextArea.Height);
                        // Averaging with the last bar.
                        barHeight = lastBarHeight = (barHeight + lastBarHeight) / 2;

                        // Calculate vertical coordinates for the bar.
                        var barY0 = areaHalfHeight - barHeight / 2f;
                        var barY1 = areaHalfHeight + barHeight / 2f;
                        var peakY = peaks[i] = Math.Min(peaks[i] + PeakDrop, barY0);

                        //
                        // Calculating the actual bar and peak rectangles, note the increment operator.
                        //
                        quads[quadCounter++] = new Rectangle(barX0 - barWidth, barY0, barX0, barY1);
                        quads[quadCounter++] = Rectangle.FromXYWH(barX0 - barWidth, peakY, barWidth, barWidth);
                        quads[quadCounter++] = Rectangle.FromXYWH(barX0 - barWidth, areaHeight - peakY, barWidth, barWidth);
                        // Mirrored
                        quads[quadCounter++] = new Rectangle(barX1, barY0, barX1 + barWidth, barY1);
                        quads[quadCounter++] = Rectangle.FromXYWH(barX1, peakY, barWidth, barWidth);
                        quads[quadCounter++] = Rectangle.FromXYWH(barX1, areaHeight - peakY, barWidth, barWidth);

                        // Advance the horizontal coordinates.
                        barX0 -= barWidth + BarSpacer;
                        barX1 += barWidth + BarSpacer;
                    }

                    context.DrawQuads(quads, baseBrush);

                    window.EndFrame();

                    frameEndTime = windowManager.GetTime();
                    var frameTime = frameEndTime - frameBeginTime;
                    var ms = Math.Round(frameTime * 1000);
                    //Console.Write( ms );
                    //Console.WriteLine( " ms" );
                }
            }
        }
    }
}
