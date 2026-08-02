# ABStream

Creates stbsp files used by Titanfall 2 to select what textures need to be streamed into GPU memory.
It draws probes in the map and stores which textures are seen from that world position.
A full explanation of the system can be found in here [GDC Talk](https://gdcvault.com/play/1024105/Efficient-Texture-Streaming-in-Titanfall).

## Usage

```
abStream <path/to/map.bsp>
```

The tool expects the standard Titanfall 2 folder layout, with the `.bsp` passed as the argument and the game's `models/` directory reachable from it. Model paths are resolved relative to the `.bsp`'s parent folder (`<bspdir>/models/`), falling back to one level up (`<bspdir>/../models/`) if that doesn't exist — so this works:

```
mods/                          # mod folder
├── maps/
│   └── <mapName>.bsp          # pass this file as the argument
└── models/                    # .mdl files referenced by the bsp (static props)
    └── ...
```

