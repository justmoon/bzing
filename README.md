# Bzing - Low-level Bitcoin database library

Bzing is a high performance blockchain data structure library in ANSI
C. It is meant to be used as a database backend in higher level
Bitcoin implementations.

# Installation

Before installing libbzing, we recommend you install one or more
supported persistent backends:

* [http://fallabs.com/kyotocabinet/](Kyoto Cabinet)
  ([http://fallabs.com/kyotocabinet/spex.html#installation](Installation);
  recommended)
* [http://fallabs.com/tokyocabinet/](Tokyo Cabinet)
  ([http://fallabs.com/tokyocabinet/spex-en.html#installation](Installation);
  supported)
* [http://www.oracle.com/technetwork/products/berkeleydb/](BerkeleyDB)
  ([http://docs.oracle.com/cd/E17076_02/html/installation/](Installation);
  supported)
* [http://code.google.com/p/leveldb/](LevelDB)
  (supported)
* [http://localmemcache.rubyforge.org/](Localmemcache)
  (unsupported)

If installed, they should be detected and linked
automatically. Alternatively you can run libbzing using one of the
built-in memory-only backends:

* [https://github.com/attractivechaos/klib](khash)
  (recommended)
* [http://code.google.com/p/ulib/](alignhash)
  (unsupported)
* [http://code.google.com/p/sparsehash/](sparsehash)
  (`--with-sparse`, unsupported)

```sh
./configure
make
sudo make install
```

# Test

``` sh
make test
```

# License

This library is free and open-source software released under the MIT
license.
