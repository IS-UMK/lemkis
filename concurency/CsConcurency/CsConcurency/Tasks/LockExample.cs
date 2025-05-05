namespace CsConcurency.Tasks;
internal class LockExample : IExample
{
    public static readonly Lock LockObject = new();
    private static readonly List<int> SharedList = [];

    public Task Run()
    {
        var task1 = Task.Run(() => AddNumbers(1, 5));
        var task2 = Task.Run(() => AddNumbers(1, 10));

        Task.WaitAll(task1, task2);

        Console.WriteLine("Final list contents:");

        //no need - good practice
        lock (LockObject)
        {
            foreach (var num in SharedList)
            {
                Console.WriteLine(num);
            }
        }

        return Task.CompletedTask;

        void AddNumbers(int start, int end)
        {
            for (int i = start; i <= end; i++)
            {
                lock (LockObject)
                {
                    SharedList.Add(i);
                    Console.WriteLine($"Thread {Thread.CurrentThread.ManagedThreadId} added {i}");
                    Thread.Sleep(100);
                }
            }
        }
    }
    
}

