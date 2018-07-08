# Vita Moonlight

This is a vita port of Moonlight Embedded.
Moonlight is an open source implementation of NVIDIA GameStream.

## Documentation

More information can find [moonlight-docs][1], [moonlight-embedded][2], and our [wiki][3].
If you need more help, join a [discord][4].

[1]: https://github.com/moonlight-stream/moonlight-docs/wiki
[2]: https://github.com/irtimmer/moonlight-embedded/wiki
[3]: https://github.com/xyzz/vita-moonlight/wiki
[4]: https://discord.gg/atkmxxT

# Build deps

You can install build dependencies with [vdpm](https://github.com/vitasdk/vdpm).

# Build Moonlight

```
# if you do git pull, make sure submodules are updated first
git submodule update --init
mkdir build && cd build
cmake ..
make
```

# Assets

- Icon - [moonlight-stream][moonlight] project logo
- Livearea background - [Moonlight Reflection][reflection] Public domain

[moonlight]: https://github.com/moonlight-stream
[reflection]: http://www.publicdomainpictures.net/view-image.php?image=130014&picture=moonlight-reflection
