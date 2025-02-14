using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KittyBot.Models;

public interface IDiscordEntity
{
	public ulong ID { get; set; }
}
