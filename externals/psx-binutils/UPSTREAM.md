# Upstream provenance

## Tested binaries

- Distribution:
  <https://github.com/Lameguy64/PSn00bSDK/releases/tag/v0.24>
- Archive:
  `gcc-mipsel-none-elf-12.3.0-linux.zip`
- Archive SHA-256:
  `228f031a25cf2687d8845fd1421f625bafc211fa27da428e458e80d030a726f8`
- Tool version: GNU Binutils 2.40
- Platform: Linux x86-64
- Runtime dependencies: glibc and `libzstd.so.1`

Only the programs used by matching and inspection were retained from the
toolchain archive:

```text
8efe7b6109d13ee6e57abb208a48f6022428e5d7122c6b59135c563978e712e4  mipsel-none-elf-as
bc4c76abe09f82094fac2177047aaa20e094a4ab93690551d30ddb9b3b83922c  mipsel-none-elf-ld
d807c80fd6184f224b62a898e49241141eb1f85b66beeeb08c68b2c3c38cdaee  mipsel-none-elf-objcopy
bf0d4a1dea99b9488978a18e82e355759e567e3565e118545fdb33033d78cc42  mipsel-none-elf-objdump
```

## Corresponding source

- Source: <https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.xz>
- SHA-256:
  `0f8a4c272d7f17f369ded10a4aca28b8e304828e95526da482b0ccc4dfc9d8e1`
- Local archive: `binutils-2.40.tar.xz`
- Licenses: `COPYING` and `COPYING3`

The matching pipeline uses the retained release binaries, not a local rebuild
from the source archive.
