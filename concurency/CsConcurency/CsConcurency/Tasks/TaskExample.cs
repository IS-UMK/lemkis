namespace CsConcurency.Tasks;

public class TaskExample : IExample
{
    public async Task Run()
    {
        try
        {
            await Main();

            return;

            static async Task Main()
            {
                // offloads work to a thread pool thread.
                var task = Task.Run(LongCalculation);
                Console.WriteLine("Doing something else...");

                // await - lets you asynchronously wait without blocking the main thread.
                var result = await task;
                Console.WriteLine($"Result: {result}");
            }

            static int LongCalculation()
            {
                Task.Delay(1000).Wait(); // Simulate work
                return 42;
            }
        }
        catch (Exception)
        {
            // Ignored
        }
    }
}