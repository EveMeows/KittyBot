using DSharpPlus;
using DSharpPlus.Entities;
using DSharpPlus.SlashCommands;

namespace KittyBot.Modules;

public class TestCommands : ApplicationCommandModule
{
	[SlashCommand("ping", "Pong!")]
	public async Task PingCommand(InteractionContext ctx)
	{
		await ctx.CreateResponseAsync(
			InteractionResponseType.ChannelMessageWithSource,
			new DiscordInteractionResponseBuilder()
				.WithContent("Pong!")
		);
	}
}
