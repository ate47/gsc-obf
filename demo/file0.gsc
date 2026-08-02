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
// Size: 0x1c
function private function_3()
{
    // Can't decompile export t7_obf_test::function_3 FIND errec Undefined (0xbeef > 0x3fff)
}

// Namespace t7_obf_test
// Params 0, eflags: 0x4
// Checksum 0x7f6567cb, Offset: 0x228
// Size: 0x9c
function private function_2()
{
    // Can't decompile export t7_obf_test::function_2 FIND errec Undefined (0xdead > 0x3fff)
}

// Namespace t7_obf_test
// Params 2, eflags: 0x4
// Checksum 0xd6d9c998, Offset: 0x2d8
// Size: 0x1e
function private function_1( var_1, var_2 )
{
    return var_1 + var_2;
}

