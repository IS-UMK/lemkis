using System.Diagnostics.Metrics;

namespace CsConcurency.Tasks;

public class AtomicExample : IExample
{
    // Lock - forces thread to wait for the lock to be released by another thread, which causes context switching and increased latency.
    // Interlocked (atomic operations) - allows multiple threads to update a variable without locking, which is faster and more efficient.

    private static int _counter;
    public Task Run()
    {
        const int threadCount = 100;

        List<Thread> threads = [];

        // Create threads to simulate concurrent updates to the counter
        for (var i = 0; i < threadCount; i++)
        {
            threads.Add(new Thread(IncrementCounter));
        }

        // Start all threads
        foreach (var thread in threads)
        {
            thread.Start();
        }

        // Wait for all threads to finish
        foreach (var thread in threads)
        {
            thread.Join();
        }

        // Output the final value of the counter
        Console.WriteLine($"Final Counter Value: {_counter}");

        return Task.CompletedTask;

        // This method will be executed by each thread
        static void IncrementCounter()
        {
            for (var i = 0; i < 1000; i++)  // Each thread will increment the counter 1000 times
            {
                // Use Interlocked to safely increment the counter
                Interlocked.Increment(ref _counter);
            }
        }
    }
}