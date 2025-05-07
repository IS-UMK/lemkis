namespace CsConcurency.Tasks;

public class SemaphoreExample : IExample
{
    public async Task Run()
    {
        try
        {
            var semaphore = new SemaphoreSlim(3); // Max 3 concurrent accesses

            var tasks = Enumerable.Range(1, 10).Select(QueryDatabaseAsync);
            await Task.WhenAll(tasks);

            return;

            async Task QueryDatabaseAsync(int id)
            {
                await semaphore.WaitAsync();
                try
                {
                    Console.WriteLine($"Thread {id} is querying the database...");
                    await Task.Delay(1000); // Simulate query time
                }
                finally
                {
                    var now = DateTime.Now;
                    Console.WriteLine($"Thread {id} done. Time - {now}");
                    semaphore.Release();
                }
            }
        }
        catch (Exception)
        {
            // Ignored
        }
    }
}