using DSharpPlus;
using DSharpPlus.SlashCommands;
using KittyBot.Models;
using KittyBot.Services;
using Microsoft.EntityFrameworkCore;

namespace KittyBot.Modules;

[SlashCommandGroup("admin", "Administrate bot and database options.")]
public class AdministrationCommands(IDbContextFactory<PostgreService> factory) : ApplicationCommandModule
{
	#region Helpers

	private async Task<bool> CheckAdmin(InteractionContext ctx)
	{
		if (!ctx.Member.Permissions.HasPermission(Permissions.Administrator)) 
		{
			await ctx.CreateResponseAsync(
				InteractionResponseType.ChannelMessageWithSource,
				new DSharpPlus.Entities.DiscordInteractionResponseBuilder()
					.WithContent("You do not have permission to do that!\nAsk your server administrator to do this...")
					.AsEphemeral()
			);
			return false;
		}

		return true;
	}

	#endregion

	[SlashCommand("enroll", "Enroll your server into the database.")]
	public async Task EnrollCommand(InteractionContext ctx)
	{
		if (!await CheckAdmin(ctx)) return;

		await using PostgreService context = await factory.CreateDbContextAsync();

		Guild? guild = await context.Guilds.FirstOrDefaultAsync(g => g.ID == ctx.Guild.Id);
		if (guild is not null)
		{ 
			await ctx.CreateResponseAsync(
				InteractionResponseType.ChannelMessageWithSource,
				new DSharpPlus.Entities.DiscordInteractionResponseBuilder()
					.WithContent("Your guild is already enrolled into the database!")
					.AsEphemeral()
			);

			return;
		}

		Guild enroll = new Guild
		{
			ID = ctx.Guild.Id,
			Users = []
		};

		await context.Guilds.AddAsync(enroll);
		await context.SaveChangesAsync();

		await ctx.CreateResponseAsync(
			InteractionResponseType.ChannelMessageWithSource,
			new DSharpPlus.Entities.DiscordInteractionResponseBuilder()
				.WithContent("Thank you! You are now part of the family!")
		);
	}
}
