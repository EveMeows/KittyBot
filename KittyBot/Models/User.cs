namespace KittyBot.Models;

public class User : IDiscordEntity
{
	public ulong ID { get; set; }

	public uint Coins { get; set; }
	public float CoinMultiplier { get; set; }
	
	public uint XP { get; set; }
	public uint XPNext { get; set; }
	public uint XPStep { get; set; }

	public uint Level { get; set; }
}
