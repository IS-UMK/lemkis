namespace CsConcurency.Tasks;

public class UiExample : IExample
{
    public Task Run()
    {
        var label = new TextLabel();

        ButtonClick(this, EventArgs.Empty);

        return Task.CompletedTask;

        void ButtonClick(object sender, EventArgs e)
        {
            Task.Run(() =>
            {
                Thread.Sleep(1000); // Simulate work
                Dispatcher.Invoke(() =>
                {
                    label.Text = "Updated from background thread!";
                });

                //without Dispatcher.Invoke() it will throw an exception
            });
        }
    }


    /// <summary>
    /// Simulates a UI thread dispatcher.
    /// </summary>
    private static class Dispatcher
    {
        public static void Invoke(Action action)
        {
            // Simulate UI thread invocation
            action();
        }
    }

    private class TextLabel
    {
        public string Text { get; set; } = string.Empty;
    }
}