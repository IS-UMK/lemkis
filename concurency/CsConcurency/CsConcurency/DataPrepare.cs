using System.Reflection;

namespace CsConcurency;

public class ExampleManager
{
    private Dictionary<string, IExample> Examples { get; }
    public ExampleManager()
    {
        var assembly = Assembly.GetExecutingAssembly();

        var iExamples = assembly.GetTypes()
            .Where(t => t is { IsClass: true, IsAbstract: false } && typeof(IExample).IsAssignableFrom(t))
            .Select(t => (IExample)Activator.CreateInstance(t)!);

        Examples = iExamples.ToDictionary(e => e.GetType().Name, e => e);
    }

    public void PrintMenu()
    {
        Console.WriteLine("Available examples:");
        Console.WriteLine("===================================");
        foreach (var example in Examples)
        {
            Console.WriteLine($"- {example.Key}");
        }
        Console.WriteLine("===================================");
        Console.WriteLine("Enter the name of the example to run:");
    }

    public async Task RunExample(string exampleName)
    {
        if (Examples.TryGetValue(exampleName, out var example))
        {
            await example.Run();
        }
        else
        {
            Console.WriteLine($"Example '{exampleName}' not found.");
        }
    }
}