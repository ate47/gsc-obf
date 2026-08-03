## Remove export checksums

The GSC format stores the checksum of each function, (aka export) this is not used by the game, but is used by a decompiler to know the function code size like [Scobalula Cerberus](https://github.com/Scobalula/Cerberus-Repo). By removing it, the decompiler might assume that the function is empty and return nothing.

### Original

```gsc
function my_func()
{
    println( "hello world" );
}
```

### Modified

***Only with checksum based decompiler.***

```gsc
function my_func()
{

}
```
