using KittyBot;

var builder = Host.CreateApplicationBuilder(args);
builder.Services.AddHostedService<KittyBot>();

var host = builder.Build();
host.Run();
