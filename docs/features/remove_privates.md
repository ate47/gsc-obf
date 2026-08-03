## Remove private function names

When using the `private` keyword, a function is known to only be used inside the current script, but the name is kept. 

### Original

```gsc

function my_func()
{
    println( "result: " + my_private_func( 4, 5 ) );
}

function private my_private_func( a, b )
{
    return a + b;
}
```

### Modified

```gsc

function my_func()
{
    println( "result: " + function_1( 4, 5 ) );
}

function private function_1( a, b )
{
    return a + b;
}
```
