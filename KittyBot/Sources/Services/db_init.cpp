#include "Services/db_init.h"
#include "Services/shared_services.h"
#include <iostream>

bool Kitty::Services::DB::Init::drop_base_tables(std::shared_ptr<Kitty::Services::SharedServices> shared)
{
  if (!shared->client)
  {
    std::cerr << "ERROR: No connection provided." << std::endl;
    return false;
  }

  try
  {
    pqxx::work erase(*shared->client);
    erase.exec("DROP TABLE IF EXISTS guild, member, guildmember, note");

    erase.commit();
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not drop default tables: " << e.what() << std::endl;
    return false;
  }

  return true;
}

bool Kitty::Services::DB::Init::create_base_tables(std::shared_ptr<Kitty::Services::SharedServices> shared)
{
  if (!shared->client)
  {
    std::cerr << "ERROR: No connection provided." << std::endl;
    return false;
  }

  try
  {
    pqxx::work trans(*shared->client);

    trans.exec(R"(
        CREATE TABLE IF NOT EXISTS note (
          id BIGINT PRIMARY KEY,
          name TEXT,
          content TEXT

          guildid BIGINT NOT NULL,
          FOREIGN KEY (guild_id) REFERENCES guild(id) ON DELETE CASCADE
        )
      )"
    );

    trans.exec(R"(
        CREATE TABLE IF NOT EXISTS guild (
          id BIGINT PRIMARY KEY,

          noteprefix TEXT DEFAULT '$',
          noteminlevel INT DEFAULT 1
        );
      )"
    );

    trans.exec(R"(
        CREATE TABLE IF NOT EXISTS member (
          id BIGINT PRIMARY KEY
        );
      )"
    );

    trans.exec(R"(
        CREATE TABLE IF NOT EXISTS guildmember (
          guildid BIGINT NOT NULL, memberid BIGINT NOT NULL,

          coins INT DEFAULT 0,

          xp INT DEFAULT 0,
          xpstep INT DEFAULT 1,
          xpnext INT DEFAULT 30,

          level INT DEFAULT 1,

          PRIMARY KEY (guildid, memberid),
          FOREIGN KEY (guildid) REFERENCES guild(id) ON DELETE CASCADE,
          FOREIGN KEY (memberid) REFERENCES member(id) ON DELETE CASCADE
        );
      )"
    );

    trans.commit();
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not create default tables: " << e.what() << std::endl;
    return false;
  }

  return true;
}
