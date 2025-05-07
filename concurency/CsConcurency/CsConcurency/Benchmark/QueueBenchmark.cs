using System.Collections.Concurrent;
using System.Diagnostics;
using CsConcurency.Queue;

namespace CsConcurency.Benchmark;

public class QueueBenchmark : IExample
{
    private static int ItemLimit => 1_000_000;
    private static int IdOffSet => 10_000;

    public Task Run()
    {
        int[] producers = [1, 2, 4];
        int[] consumers = [1, 2, 4];

        IQueue<int>[] queues =
        [
            new ConcurrentQueueWrapper<int>(), // ConcurrentQueue
            new MutexQueue<int>(), // State of the art MutexQueue
            new StConcurrentQueue<int>() // State of the art ConcurrentQueue
        ];

        foreach (var producerIndex in producers)
        {
            foreach (var consumer in consumers)
            {
                foreach (var q in queues)
                {
                    Console.WriteLine("--------------------------------------------------");
                    RunSequentialQueueTest(q, producerIndex, consumer);
                    RunThreadPoolQueueTest(q, producerIndex, consumer);
                    Console.WriteLine("--------------------------------------------------\n");
                }
            }
        }

        return Task.CompletedTask;
    }

    public static void RunSequentialQueueTest(IQueue<int> queue, int producers, int consumers)
    {
        int pushedCount = 0, poppedCount = 0;
        var producerCountdown = new CountdownEvent(producers);

        var threads = new List<Thread>();
        var itemsPerProducer = ItemLimit / producers;

        var stopWatch = Stopwatch.StartNew();

        // Producer tasks
        for (var i = 0; i < producers; i++)
        {
            var localI = i; //hold value
            var thread = new Thread(() =>
            {
                for (var j = 0; j < itemsPerProducer; j++)
                {
                    queue.Enqueue(j + localI * IdOffSet);
                    Interlocked.Increment(ref pushedCount);
                }
                producerCountdown.Signal();
            });
            thread.Start();
            threads.Add(thread);
        }

        // Consumer tasks
        for (var i = 0; i < consumers; i++)
        {
            var thread = new Thread(() =>
            {
                while (true)
                {
                    if (producerCountdown.IsSet && queue.IsEmpty) break;

                    if (queue.TryDequeue(out _))
                    {
                        Interlocked.Increment(ref poppedCount);
                    }
                    else
                    {
                        Thread.Sleep(1);
                    }
                }
            });

            thread.Start();
            threads.Add(thread);
        }

        // Wait for all threads
        foreach (var t in threads)
            t.Join();

        stopWatch.Stop();

        PrintPerformance($"Sequential Queue - Producers: {producers}, Consumers: {consumers}, Pushed: {pushedCount}, Popped: {poppedCount}", stopWatch);

        return;
        
        void PrintPerformance(string data, Stopwatch stopwatch)
        {
            Console.WriteLine($"{queue.GetType().Name} - {data} - Elapsed time: {stopwatch.ElapsedMilliseconds} ms");
        }
    }

    public static void RunThreadPoolQueueTest(IQueue<int> queue, int producers, int consumers)
    {
        int pushedCount = 0, poppedCount = 0;
        var producerCountdown = new CountdownEvent(producers);
        var completionEvent = new ManualResetEventSlim(false);
        var itemsPerProducer = ItemLimit / producers;
        var stopWatch = Stopwatch.StartNew();

        // Producer tasks
        for (var i = 0; i < producers; i++)
        {
            var localI = i; // Capture value for closure
            ThreadPool.QueueUserWorkItem(_ =>
            {
                for (var j = 0; j < itemsPerProducer; j++)
                {
                    queue.Enqueue(j + localI * IdOffSet);
                    Interlocked.Increment(ref pushedCount);
                }
                producerCountdown.Signal();
            });
        }

        // Consumer tasks
        for (var i = 0; i < consumers; i++)
        {
            ThreadPool.QueueUserWorkItem(_ =>
            {
                while (true)
                {
                    if (producerCountdown.IsSet && queue.IsEmpty)
                    {
                        if (Interlocked.Decrement(ref consumers) == 0)
                        {
                            completionEvent.Set();
                        }
                        break;
                    }

                    if (queue.TryDequeue(out var _))
                    {
                        Interlocked.Increment(ref poppedCount);
                    }
                    else
                    {
                        Thread.Sleep(1);
                    }
                }
            });
        }

        // Wait for all work to complete
        completionEvent.Wait();
        stopWatch.Stop();

        PrintPerformance($"ThreadPool Queue - Producers: {producers}, Consumers: {consumers}, Pushed: {pushedCount}, Popped: {poppedCount}", stopWatch);

        return;

        void PrintPerformance(string data, Stopwatch stopwatch)
        {
            Console.WriteLine($"{queue.GetType().Name} - {data} - Elapsed time: {stopwatch.ElapsedMilliseconds} ms");
        }
    }

}