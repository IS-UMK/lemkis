using System.Runtime.InteropServices;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace CsConcurency.Tasks;

public class ApiExample : IExample
{
    public async Task Run()
    {
        try
        {
            using var client = new HttpClient();
            const string url = "https://yesno.wtf/api";

            var response = await client.GetAsync(url); //Task<HttpResponseMessage>

            if (!response.IsSuccessStatusCode)
            {
                Console.WriteLine("Error: " + response.StatusCode);
                return;
            }

            var json = await response.Content.ReadAsStringAsync(); //Task<string>

            var yesNoResponse = JsonSerializer.Deserialize<YesNoResponse>(json); //Task<YesNoResponse>

            if (yesNoResponse == null)
            {
                Console.WriteLine("Error: Unable to deserialize response.");
                return;
            }

            Console.WriteLine($"Answer: {yesNoResponse.Answer}");
        }
        catch (Exception)
        {
            // Ignored
        }
    }

    [Serializable]
    private class YesNoResponse
    {
        [JsonPropertyName("answer")] public string Answer { get; set; } = string.Empty;
        [JsonPropertyName("forced")] public bool Forced { get; set; }
    }
}