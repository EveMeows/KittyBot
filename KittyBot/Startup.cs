using DSharpPlus;
using KittyBot.Services;
using System.Reflection;

HostApplicationBuilder builder = Host.CreateApplicationBuilder(args);

// User Secrets.
builder.Configuration.AddUserSecrets(Assembly.GetExecutingAssembly());

// Services.
builder.Services.AddSingleton(x => 
	new DiscordClient(new DiscordConfiguration { 
		Token = x.GetRequiredService<IConfiguration>()["Token"], 
		Intents = DiscordIntents.AllUnprivileged
	})
);

builder.Services.AddHostedService<KittyBotService>();

// Build and Run.
IHost host = builder.Build();
host.Run();
