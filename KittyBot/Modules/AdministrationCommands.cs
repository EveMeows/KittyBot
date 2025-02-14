using DSharpPlus;
using DSharpPlus.Entities;
using DSharpPlus.SlashCommands;
using KittyBot.Models;
using KittyBot.Services;
using Microsoft.EntityFrameworkCore;

namespace KittyBot.Modules;

[SlashCommandGroup("admin", "Administrate bot and database options.")]
public class AdministrationCommands(IDbContextFactory<PostgreService> factory, ILogger<AdministrationCommands> logger) : ApplicationCommandModule
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
			Users = new List<User>()
		};

		await context.Guilds.AddAsync(enroll);
		await context.SaveChangesAsync();

		await ctx.CreateResponseAsync(
			InteractionResponseType.ChannelMessageWithSource,
			new DSharpPlus.Entities.DiscordInteractionResponseBuilder()
				.WithContent("Thank you! You are now part of the family!")
		);
	}

	[SlashCommand("addexp", "Give XP to a certain member.")]
	public async Task AddXPCommand(
		InteractionContext ctx,
		[Option("xp", "The amount of XP to give")] long xp,
		[Option("user", "The user to give XP to.")] DiscordUser? user = null
	)
	{
		if (!await CheckAdmin(ctx)) return;

		await using PostgreService context = await factory.CreateDbContextAsync();

		Guild? guild = await context.Guilds.Include(g => g.Users).FirstOrDefaultAsync(g => g.ID == ctx.Guild.Id);
		if (guild is null)
		{
			await ctx.CreateResponseAsync(
				InteractionResponseType.ChannelMessageWithSource,
				new DiscordInteractionResponseBuilder()
					.WithContent("Your guild is not registered in the database!")
					.AsEphemeral()
			);

			return;
		}

		user ??= ctx.User;
		if (user.IsBot || xp < 0)
		{
			await ctx.CreateResponseAsync(
				InteractionResponseType.ChannelMessageWithSource,
				new DiscordInteractionResponseBuilder()
					.WithContent("You can't do that!")
					.AsEphemeral()
			);

			return;
		}

		User? contextUser = guild.Users.FirstOrDefault(u => u.ID == user.Id);
		if (contextUser is null)
		{
			await ctx.CreateResponseAsync(
				InteractionResponseType.ChannelMessageWithSource,
				new DiscordInteractionResponseBuilder()
					.WithContent($"{(user == ctx.User ? "You are" : "That user is")} not registered in the database!")
					.AsEphemeral()
			);

			return;
		}

		contextUser.XP = (uint)xp;

		// Handle XP.
		while (contextUser.XP >= contextUser.XPNext)
		{
			contextUser.XP -= contextUser.XPNext;

			contextUser.Level++;
			contextUser.XPNext = (uint)Math.Floor(Math.Pow(contextUser.Level / context.LevelIncrease, context.LevelGap));
		} 

		context.Guilds.Update(guild);
		await context.SaveChangesAsync();

		await ctx.CreateResponseAsync(
			InteractionResponseType.ChannelMessageWithSource,
			new DiscordInteractionResponseBuilder()
				.WithContent($"Succesfully gave {user.Username} {xp} XP")
				.AsEphemeral()
		);
	}
}
