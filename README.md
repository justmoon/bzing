# Bzing - Low-level Bitcoin database library

Bzing is a high performance blockchain data structure library in ANSI
C. It is meant to be used as a database backend in higher level
Bitcoin implementations.

# Installation

Before installing libbzing, we recommend you install one or more
supported persistent backends:

* [Kyoto Cabinet](http://fallabs.com/kyotocabinet/)
  ([Installation](http://fallabs.com/kyotocabinet/spex.html#installation);
  recommended)
* [Tokyo Cabinet](http://fallabs.com/tokyocabinet/)
  ([Installation](http://fallabs.com/tokyocabinet/spex-en.html#installation);
  supported)
* [BerkeleyDB](http://www.oracle.com/technetwork/products/berkeleydb/)
  ([Installation](http://docs.oracle.com/cd/E17076_02/html/installation/);
  supported)
* [LevelDB](http://code.google.com/p/leveldb/)
  (supported)
* [Localmemcache](http://localmemcache.rubyforge.org/)
  (unsupported)

If installed, they should be detected and linked
automatically. Alternatively you can run libbzing using one of the
built-in memory-only backends:

* [khash](https://github.com/attractivechaos/klib)
  (recommended)
* [alignhash](http://code.google.com/p/ulib/)
  (unsupported)
* [sparsehash](http://code.google.com/p/sparsehash/)
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
