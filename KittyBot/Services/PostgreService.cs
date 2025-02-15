using KittyBot.Models;
using Microsoft.EntityFrameworkCore;

namespace KittyBot.Services;

public class PostgreService(IConfiguration config) : DbContext
{
	public float LevelIncrease = 0.15f;
	public float LevelGap = 2;
	public float BaseCoins = 100;

	public DbSet<Guild> Guilds { get; set; }
	public DbSet<User> Users { get; set; }

	public void HandleLevelUp(User user)
	{
		user.XP -= user.XPNext;
		user.Level++;

		// Conijm,,m
		user.Coins += (uint)Math.Ceiling(BaseCoins * user.CoinMultiplier);
		user.CoinMultiplier += 0.1f;

		user.XPNext = (uint)Math.Floor(Math.Pow(user.Level / LevelIncrease, LevelGap));
	}
	
	protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder)
	{
		string connection = config.GetValue<string>("ConnectionString") 
			?? throw new InvalidOperationException("Cannot find Connection String");

		optionsBuilder.UseNpgsql(connection);
	}
}
