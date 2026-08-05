# ABStream

Creates stbsp files used by Titanfall 2 to select what textures need to be streamed into GPU memory.
It draws probes in the map and stores which textures are seen from that world position.
A full explanation of the system can be found in here [GDC Talk](https://gdcvault.com/play/1024105/Efficient-Texture-Streaming-in-Titanfall).

## Usage

```
abStream [OPTIONS] <path/to/map.bsp>
```

| Option | Description | Default |
|---|---|---|
| `-r, --resolution` | Resolution of the rendered cubemap | `1024` |
| `-c, --probeCount` | Max number of probes per cell | `8` |
| `-i, --iterations` | Number of iterations for reducing probe-options | `32` |
| `-s, --cellSize` | Cell size | `128` |
| `-b, --brushProbeGenerationGridSize` | How far apart probe-options are generated on brushes | `16` |
| `-H, --probeHeight` | Height of probes above geometry | `64` |

The tool expects the standard Titanfall 2 folder layout, with the `.bsp` passed as the argument and the game's `models/` directory reachable from it. Model paths are resolved relative to the `.bsp`'s parent folder (`<bspdir>/models/`), falling back to one level up (`<bspdir>/../models/`) if that doesn't exist

```
mods/                          # mod folder
├── maps/
│   └── <mapName>.bsp          # pass this file as the argument
└── models/                    # .mdl files referenced by the bsp (static props)
    └── ...
```

