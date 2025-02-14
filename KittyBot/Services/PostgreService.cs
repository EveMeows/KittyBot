using KittyBot.Models;
using Microsoft.EntityFrameworkCore;

namespace KittyBot.Services;

public class PostgreService(IConfiguration config) : DbContext
{
	public float LevelIncrease = 0.15f;
	public float LevelGap = 2;

	public DbSet<Guild> Guilds { get; set; }
	public DbSet<User> Users { get; set; }
	
	protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder)
	{
		string connection = config.GetValue<string>("ConnectionString") 
			?? throw new InvalidOperationException("Cannot find Connection String");

		optionsBuilder.UseNpgsql(connection);
	}
}
