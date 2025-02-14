using DSharpPlus;
using DSharpPlus.Entities;
using DSharpPlus.EventArgs;
using DSharpPlus.Exceptions;
using DSharpPlus.Interactivity;
using DSharpPlus.Interactivity.Extensions;
using DSharpPlus.SlashCommands;
using DSharpPlus.SlashCommands.EventArgs;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Internal;
using System.Reflection;

namespace KittyBot.Services;

public class KittyBotService(DiscordClient client, IServiceProvider services, ILogger<KittyBotService> logger, IDbContextFactory<PostgreService> factory) : IHostedService
{
	#region Events
	
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

	#endregion

	private async Task ReleaseDatabase()
	{
		await using PostgreService context = await factory.CreateDbContextAsync();

		// bool deleted = context.Database.EnsureDeleted();
		// logger.Log(LogLevel.Information, "DataBase erasure status: {}", deleted ? "success" : "failure");

		bool recreated = context.Database.EnsureCreated();
		logger.Log(LogLevel.Information, "DataBase creation status: {}", recreated ? "success" : "failure");

	}

	public async Task StartAsync(CancellationToken cancellationToken)
	{
		//! Testing Purposes ONLY
		//! Only ran during prototyping...
		//! We erase the db then create it again.
		await ReleaseDatabase();

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
