namespace CsConcurency.Tasks;

internal class MutexExample2 : IExample
{
    private static readonly Mutex FileMutex = new(false, "Global\\CsConcurency");
    private const string LogFilePath = "application.log";

    public Task Run()
    {
        for (var i = 0; i < 3; i++)
        {
            LogMessage($"Log entry {i} from Process {Environment.ProcessId}");
            Thread.Sleep(500);
        }

        return Task.CompletedTask;

        void LogMessage(string message)
        {
            try
            {
                if (FileMutex.WaitOne(2000))
                {
                    try
                    {
                        File.AppendAllText(LogFilePath, $"{DateTime.Now}: {message}\n");
                        Console.WriteLine($"Logged: {message}");
                    }
                    finally
                    {
                        FileMutex.ReleaseMutex();
                    }
                }
                else
                {
                    Console.WriteLine("Timeout: Could not access the log file.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }
        }
    }
}
