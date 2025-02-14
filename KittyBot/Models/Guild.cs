namespace KittyBot.Models;

public class Guild : IDiscordEntity
{
	public ulong ID { get; set; }
	public required List<User> Users { get; set; }
}
