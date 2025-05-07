namespace CsConcurency.Tasks;

public class ParallelAsyncExample : IExample
{
    public async Task Run()
    {
        try
        {
            var cts = new CancellationTokenSource();
            cts.CancelAfter(2000); // Cancel after 2 seconds

            var items = Enumerable.Range(1, 1000); // Simulate a collection of items

            try
            {
                await Parallel.ForEachAsync(items, cts.Token, async (item, token) =>
                {
                    Console.WriteLine($"Processing {item} on thread {Environment.CurrentManagedThreadId}");
                    await Task.Delay(1000, token); // Simulate async work
                    Console.WriteLine($"Done {item}");
                });
            }
            catch (OperationCanceledException)
            {
                Console.WriteLine("Operation was canceled.");
            }
        }
        catch (Exception)
        {
            // Ignored
        }
    }
}