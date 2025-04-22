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

# PSQL database name
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

## Setup
KittyBot OS specific setup!
Though some setup is agnostic, as there's partial support for VCPKG.

> [!IMPORTANT]
> You can install vcpkg [here](https://learn.microsoft.com/ro-ro/vcpkg/get_started/get-started?pivots=shell-powershell)!

Once you download vcpkg, open your favourite terminal.

```bash
git clone https://github.com/EveMeows/KittyBot
cd KittyBot
```


Though, the bot still relies on some packages. We will need to install PostgreSQL, CMake, Ninja.

### Linux
On linux, we use G++ or Clang++ for compilation. Below you'll see instructions for installation.
```bash
# Fedora
sudo dnf in postgresql-server postgresql-contrib clang gcc-g++ ninja cmake

# Ubuntu/Debian
sudo apt install postgresql-server clang g++ ninja-build cmake

# Arch
sudo pacman -S postgresql clang g++ ninja cmake
```

Each distro will have it's own way to setup PostgreSQL, for example, here's [fedora's](https://docs.fedoraproject.org/en-US/quick-docs/postgresql/).

Now, we are free to compile and run.
```bash
# Create and setup the .env
nano .env

mkdir build; cd build

cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release

ninja
```

Once everything compiles, and if no errors arise, you can now run KittyBot by typing
```bash
./KittyBot
```

### Docker
TODO

### Windows
TODO


## License
GPL 3.0
