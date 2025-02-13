using System.Reflection;

HostApplicationBuilder builder = Host.CreateApplicationBuilder(args);

// User Secrets.
builder.Configuration.AddUserSecrets(Assembly.GetExecutingAssembly());

// Services.
builder.Services.AddHostedService<KittyBot.KittyBot>();

// Build and Run.
IHost host = builder.Build();
host.Run();
