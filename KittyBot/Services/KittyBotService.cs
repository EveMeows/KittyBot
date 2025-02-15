using DSharpPlus;
using DSharpPlus.Entities;
using DSharpPlus.EventArgs;
using DSharpPlus.Exceptions;
using DSharpPlus.Interactivity;
using DSharpPlus.Interactivity.Extensions;
using DSharpPlus.SlashCommands;
using DSharpPlus.SlashCommands.EventArgs;
using KittyBot.Models;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Internal;
using System.Reflection;

namespace KittyBot.Services;

public class KittyBotService(DiscordClient client, IServiceProvider services, ILogger<KittyBotService> logger, IDbContextFactory<PostgreService> factory) : IHostedService
{
	#region Events
	
	private async Task SlashErrored(SlashCommandsExtension s, SlashCommandErrorEventArgs e)
	{
		logger.LogError("Failed to Execute command: {}", e.Exception.Message);
		
		try
		{
			await e.Context.CreateResponseAsync(
				"An error occurred while executing the command!", true
			);
		}
		catch (BadRequestException)
		{
			await e.Context.EditResponseAsync(new DiscordWebhookBuilder()
				.WithContent("An error occurred while executing the command!")
			);
		}
	}


	private Task ClientReady(DiscordClient sender, ReadyEventArgs args)
	{
		logger.LogInformation("Logged in as {}", sender.CurrentUser);
		return Task.CompletedTask;
	}

	private async Task MessageCreated(DiscordClient sender, MessageCreateEventArgs args)
	{
		// Return if the author is a bot.
		if (args.Author.IsBot) return;

		await using PostgreService context = await factory.CreateDbContextAsync();

		Guild? guild = await context.Guilds.Include(g => g.Users).FirstOrDefaultAsync(g => g.ID == args.Guild.Id);
		if (guild is null) return;

		// Find the user
		// And create if doesn't exist.
		User? user = guild.Users.FirstOrDefault(u => u.ID == args.Author.Id);
		if (user is null)
		{
			user = new User
			{
				ID = args.Author.Id,
				Coins = 0, CoinMultiplier = 1.0f,
				XP = 1, XPNext = 20, XPStep = 1,
				Level = 1
			};

			guild.Users.Add(user);
			await context.Users.AddAsync(user);
		}
		else
		{
			user.XP += user.XPStep;
			int advanced = 0;
			uint coins = 0;
			while (user.XP >= user.XPNext)
			{
				coins += (uint)Math.Ceiling(context.BaseCoins * user.CoinMultiplier);
				context.HandleLevelUp(user);
				advanced++;
			}

			if (advanced > 0)
			{ 
				// Congratulate
				await sender.SendMessageAsync(
					args.Channel,
					$"Congratulations {args.Author.Mention}! You've reached level {user.Level}{(advanced > 1 ? $" and you've advanced {advanced} levels at once," : ",")} earning {coins} coins!"
				);
			}
		}

		context.Guilds.Update(guild);

		await context.SaveChangesAsync();
	}

	#endregion

	private async Task ReleaseDatabase()
	{
		await using PostgreService context = await factory.CreateDbContextAsync();

		bool deleted = context.Database.EnsureDeleted();
		logger.LogInformation("DataBase erasure status: {}", deleted ? "success" : "failure");

		bool recreated = context.Database.EnsureCreated();
		logger.LogInformation("DataBase creation status: {}", recreated ? "success" : "failure");

	}

	public async Task StartAsync(CancellationToken cancellationToken)
	{
		//! Testing Purposes ONLY
		//! Only ran during prototyping...
		//! We erase the db then create it again.
		await ReleaseDatabase();

		client.Ready += ClientReady;
		client.MessageCreated += MessageCreated;

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
		logger.LogInformation("Client disconnecting.");
		await client.DisconnectAsync();
	}
}
