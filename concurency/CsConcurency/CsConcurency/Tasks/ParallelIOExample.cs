using System.Diagnostics;

namespace CsConcurency.Tasks;

public class ParallelIoExample : IExample
{
    private const int N = 10000;

    public Task Run()
    {
        var outputDir = Path.Combine(Path.GetTempPath(), "FileBenchmarkTest");

        if (Directory.Exists(outputDir))
            Directory.Delete(outputDir, true);
        Directory.CreateDirectory(outputDir);

        var files = Enumerable.Range(0, N).ToArray();

        Console.WriteLine("Starting sequential file creation...");
        var sw1 = Stopwatch.StartNew();
        foreach (var i in files)
        {
            File.WriteAllText(Path.Combine(outputDir, $"file_{i}.txt"), $"This is file {i}");
        }
        sw1.Stop();
        Console.WriteLine($"Sequential time: {sw1.ElapsedMilliseconds} ms");

        // Clean up
        Directory.Delete(outputDir, true);
        Directory.CreateDirectory(outputDir);

        Console.WriteLine("Starting parallel file creation...");
        var sw2 = Stopwatch.StartNew();
        Parallel.ForEach(files, i =>
        {
            File.WriteAllText(Path.Combine(outputDir, $"file_{i}.txt"), $"This is file {i}");
        });
        sw2.Stop();
        Console.WriteLine($"Parallel time: {sw2.ElapsedMilliseconds} ms");

        // Clean up again
        Directory.Delete(outputDir, true);

        return Task.CompletedTask;
    }
}