using static System.Threading.Thread;

namespace CsConcurency.Tasks;

public class ParallelExample : IExample
{
    public Task Run()
    {
        Parallel.For(0, 5, i =>
        {
            Console.WriteLine($"Task {i} running on thread {Environment.CurrentManagedThreadId}");
            Sleep(500); // Simulate work
        });

        Console.WriteLine("Parallel loop complete.");

        return Task.CompletedTask;
    }
}