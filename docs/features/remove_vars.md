## Remove variables

The GSC format stores the local variable names inside the end binary, this is completly useless and it gives information to the decompilers.

### Original

```gsc
function my_func( a, b )
{
    c = a + b;
    return c;
}
```

### Modified

```gsc
function my_func( var_1, var_2 )
{
    var_3 = var_1 + var_2;
    return var_3;
}
```
