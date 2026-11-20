# Big Money: PS3 Edition
Welcome to Big Money! This is the fan-made project that aims to recreate Big Money: Deluxe for PlayStation 3 system using PSL1GHT.

# How to compile?
You need a machine running Linux. WSL is supported.

1. Get [PS3DEV](https://github.com/ps3dev/ps3dev) and it's dependencies.
2. Copy `/usr/local/ps3dev/portlibs/ppu/include/freetype2/freetype` folder to `/usr/local/ps3dev/portlibs/ppu/include` cuz idk why they don't fix it.
3. Compile the game using `make pkg` command.
4. Install PKG in your PS3 system.

# What else I need?
1. You need to actually own the copy of a game. Get it on [Steam](https://store.steampowered.com/app/3360/Big_Money_Deluxe/), like c'mon, it ain't that expensive.
2. After PKG installation, move the `images`, `music`, `sounds` and `properties` folder from your copy to `/dev_hdd0/game/BIGMONEY0/USRDIR/` folder.

# Additional Notes
* The game's code is partially AI-Assisted.
* This project uses [stb_image](https://github.com/nothings/stb).
* This project uses RSXUtil. The license for this utility is the same as the homebrew's license.
* This project is not official and not either endorsed or approved by PopCap Games Inc.
* This project does not contain any trade secrets of PopCap Games Inc.