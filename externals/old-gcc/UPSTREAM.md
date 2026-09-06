# Upstream provenance

## Build recipes

- Project: <https://github.com/decompals/old-gcc>
- Tag: `0.17`
- Commit: `b74211c9d959e9724802f3177c8229cd67202c87`

The upstream repository snapshot is retained without its `.git` directory.
The matching pipeline uses only GCC 2.8.1.

## GCC source

- Source: <https://ftp.gnu.org/gnu/gcc/gcc-2.8.1.tar.gz>
- SHA-256:
  `3b30fbfdf93e628373d90d174243f3267b0eec9ebe792bb64fd15b8828c2ea4c`
- Local archive: `gcc-2.8.1.tar.gz`
- License: GNU GPL v2 (`GCC-COPYING`)

The `gcc-2.8.1-psx.Dockerfile` and `patches/` directory describe the PSX
target configuration and source modifications used by the upstream build.

## Tested compiler release

- Release:
  <https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.8.1-psx.tar.gz>
- Release archive SHA-256:
  `f6f6e883942d4d3289d048236c672e71ed410e546aaae8ff655952f1567e1be0`
- Platform: statically linked Linux i386
- Local directory: `prebuilt/linux-i386/2.8.1`

The files below are copied without modification from that release archive:

```text
1a48ec5ac87a146a074a9d66d7a67734a4d1ad56553d02d91e8b71c7fecc2d52  cc1
93e7b36546532deec58cbc221df4d2c2894d444b5685b31a439c46cc597a9572  cc1plus
8d7f4e90f995089a6c2d62214b607fc77acf43b61f60a871d130d713d9ac96bb  cpp
a2cf0a5f0a21fcd4836c6f7ac66f346ba5594c1e4d4ab9d88c48c73c77f876fd  g++
451e6521d860fc6ff49000567d94d983125263bb0ae2149f89d95eaed34b3a02  gcc
```
