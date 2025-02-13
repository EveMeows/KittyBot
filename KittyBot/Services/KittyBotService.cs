using DSharpPlus;
using DSharpPlus.Entities;
using DSharpPlus.EventArgs;
using DSharpPlus.Exceptions;
using DSharpPlus.Interactivity;
using DSharpPlus.Interactivity.Extensions;
using DSharpPlus.SlashCommands;
using DSharpPlus.SlashCommands.EventArgs;
using System.Reflection;

namespace KittyBot.Services;

public class KittyBotService(DiscordClient client, IServiceProvider services, ILogger<KittyBotService> logger) : IHostedService
{
	private async Task SlashErrored(SlashCommandsExtension s, SlashCommandErrorEventArgs e)
	{
		logger.Log(LogLevel.Error, "Failed to Execute command: {}", e.Exception.Message);
		
		try
		{
			await e.Context.CreateResponseAsync(
				"An error occurred while executing the command!"
			);
		}
		catch (BadRequestException)
		{
			await e.Context.EditResponseAsync(new DiscordWebhookBuilder()
				.WithContent("An error occurred while executing the command!")
			);
		}
	}

	private async Task ClientReady(DiscordClient sender, ReadyEventArgs args)
	{
		logger.Log(LogLevel.Information, "Logged in as {}", sender.CurrentUser);
	}


	public async Task StartAsync(CancellationToken cancellationToken)
	{
		client.Ready += ClientReady;
		client.UseInteractivity(new InteractivityConfiguration { Timeout = TimeSpan.FromSeconds(30) });

		SlashCommandsExtension slash = client.UseSlashCommands(new SlashCommandsConfiguration { 
			Services = services
		});

		slash.RegisterCommands(Assembly.GetExecutingAssembly());
		slash.SlashCommandErrored += SlashErrored;

		await client.ConnectAsync();
	}

	public async Task StopAsync(CancellationToken cancellationToken)
	{
		logger.Log(LogLevel.Information, "Client disconnecting.");
		await client.DisconnectAsync();
	}
}
