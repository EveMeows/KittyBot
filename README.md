# KittyBot

Discord Bot written entirely in the C++ programming language!

So far, this project is only a proof of concept.

## Setup
To run this bot yourself, you will need to create a .env file in the project root.

It must look something like this;
```bash
# Do NOT use QUOTES ("") for the token.
KITTY_TOKEN=mycooltokenhere

# PSQL host ip
HOST=localhost

# PSQL service port
PORT=5432

# PSQL username and password
USER=myuser
PWD=mypassword

# PSQL database name (kittybot [REQUIRED])
DB=kittybot
```
And the project should look like this.
```bash
KittyBot
├── CMakeLists.txt
├── KittyBot
│   └── ...
├── .env
└── ...
```

## Dependencies

### TODO: Actually explain wtf is happening
cmake clang ninja curl-devel cpr-devel openssl-devel postgresql-server postgresql-devel libpqxx-devel <br>
OPTIONAL: dpp (will compile itself if not found)

## License
GPL 3.0
