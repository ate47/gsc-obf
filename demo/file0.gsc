#using scripts/codescripts/struct;
#using scripts/shared/array_shared;
#using scripts/shared/callbacks_shared;
#using scripts/shared/scoreevents_shared;
#using scripts/shared/system_shared;
#using scripts/shared/table_shared;
#using scripts/shared/util_shared;

#namespace t7_obf_test;

// Namespace t7_obf_test
// Params 0, eflags: 0x2
// Checksum 0xacb39330, Offset: 0x198
// Size: 0x34
function autoexec auto()
{
    system::register( "t7_obf_test", &function_3, undefined, undefined );
}

// Namespace t7_obf_test
// Params 0, eflags: 0x4
// Checksum 0xacb39330, Offset: 0x1d8
// Size: 0x24
function private function_3()
{
    callback::on_connect( &function_2 );
}

// Namespace t7_obf_test
// Params 0, eflags: 0x4
// Checksum 0xdbb4a3a6, Offset: 0x208
// Size: 0x7c
function private function_2()
{
    wait 5;
    var_1 = 0;
    
    for ( ;; )
    {
        self iprintlnbold( "Hello " + var_1 );
        var_1 = function_1( var_1, 1 );
        wait 1;
    }
    
    /#
        function_dead( "test dev block" );
    #/
}

// Namespace t7_obf_test
// Params 2, eflags: 0x4
// Checksum 0xc2d895d7, Offset: 0x298
// Size: 0x1e
function private function_1( var_1, var_2 )
{
    return var_1 + var_2;
}

