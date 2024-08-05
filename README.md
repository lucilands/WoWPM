# WoWPM - A CLI plugin manager for WoW on Linux

## Installation
Installing wpm is as easy as running this one command
```bash
sudo make install
```

## Usage
First, select an instance, this only needs to be done right after installation, or when the path to your installation changes
```bash
wpm select /full/path/to/wow/instance
```

Installing an addon is then as easy as this:
```bash
wpm install nameOfAddon
```

If you don't know the name of the addon, just use this command to see all addons in your local database
```bash
wpm list
```

If you don't see a plugin there, it either hasn't been added yet, or it is in there, you just need to refresh your database
```bash
wpm refresh
```
