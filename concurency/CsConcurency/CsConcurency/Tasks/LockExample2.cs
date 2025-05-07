namespace CsConcurency.Tasks;

internal class LockExample2 : IExample
{
    private readonly Queue<string> _orders = new();
    private readonly object _lock = new();
    private int _totalProcessed;
    private const int WorkerCount = 5;

    public Task Run()
    {
        Console.WriteLine($"Starting {WorkerCount} worker threads...");

        var workers = new Thread[WorkerCount];
        for (var i = 0; i < WorkerCount; i++)
        {
            workers[i] = new Thread(ProcessOrders)
            {
                Name = $"Worker-{i + 1}"
            };
            workers[i].Start();
        }
        var feeder = new Thread(AddOrders)
        {
            Name = "OrderFeeder"
        };
        feeder.Start();
        feeder.Join();
        Thread.Sleep(2000);
        foreach (var worker in workers)
        {
            worker.Interrupt();
        }
        Console.WriteLine($"\nAll orders processed. Total: {_totalProcessed}");

        return Task.CompletedTask;
    }

    private void AddOrders()
    {
        for (var i = 1; i <= 15; i++)
        {
            lock (_lock)
            {
                _orders.Enqueue($"Order-{i}");
                Monitor.Pulse(_lock);
            }
            Thread.Sleep(150);
        }
    }

    private void ProcessOrders()
    {
        try
        {
            while (true)
            {
                string? order;

                lock (_lock)
                {
                    while (_orders.Count == 0)
                    {
                        Monitor.Wait(_lock);
                    }

                    order = _orders.Dequeue();
                }
                Console.WriteLine($"{Thread.CurrentThread.Name} processing {order}");
                Thread.Sleep(new Random().Next(100, 400));
                Interlocked.Increment(ref _totalProcessed);
            }
        }
        catch (ThreadInterruptedException)
        {
            Console.WriteLine($"{Thread.CurrentThread.Name} shutting down...");
        }
    }
}
