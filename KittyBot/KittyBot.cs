namespace KittyBot;

public class KittyBot(IConfiguration config) : BackgroundService
{
	protected override async Task ExecuteAsync(CancellationToken stoppingToken)
	{
		while (!stoppingToken.IsCancellationRequested)
		{
			Console.WriteLine($"Token: {config["Token"]}");
			await Task.Delay(1000, stoppingToken);
		}
	}
}
