namespace CsConcurency.Tasks;

public class MutexExample : IExample
{
    private static readonly Mutex Mutex = new();
    public Task Run()
    {
        Console.WriteLine($"Thread {Environment.CurrentManagedThreadId} is waiting to enter the critical section.");
        try
        {
            var acquired = Mutex.WaitOne(1000);
            if (!acquired)
            {
                Console.WriteLine("Failed to acquire the mutex within the timeout.");
                return Task.CompletedTask;
            }

            Console.WriteLine($"Thread {Environment.CurrentManagedThreadId} has entered the critical section.");
            Thread.Sleep(2000);
            Console.WriteLine($"Thread {Environment.CurrentManagedThreadId} is leaving the critical section.");
        }
        finally
        {
            Mutex.ReleaseMutex();
            Console.WriteLine($"Thread {Environment.CurrentManagedThreadId} has released the mutex.");
        }

        return Task.CompletedTask;
    }
}
