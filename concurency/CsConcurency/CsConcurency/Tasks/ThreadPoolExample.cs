using static System.Threading.Thread;

namespace CsConcurency.Tasks;

public class ThreadPoolExample : IExample
{
    public Task Run()
    {
        //ThreadPool is efficient for short, fire-and-forget background tasks
        //ThreadPool exposes a queue of threads that can be used to execute tasks asynchronously that are sleeping or waiting for I/O
        //ThreadPool is not suitable for long-running tasks, and it allows for more efficient resource management which are shared by application

        ThreadPool.QueueUserWorkItem(_ =>
        {
            Console.WriteLine($"Work in thread {Environment.CurrentManagedThreadId}");
            Sleep(500);
        });

        Console.WriteLine("Main continues...");
        Sleep(1000); // Ensure background task finishes

        return Task.CompletedTask;
    }
}