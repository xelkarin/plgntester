=====================================
plgntester - NSIS Plugin Tester v0.01
=====================================

This is a command line program that can be used to
invoke a NSIS plugin.  It supports passing any string
variable on the stack, setting any of the standard
variables (user defined variables are unsupported),
and can invoke any specified function in given plugin.

 Usage: ``plgnTester.exe [opts] plugin function {/VAR # str} [args]``

plugin is the name of the plugin (or dll) to load, passed
asis to LoadLibrary, so one can omit the .dll and normal
path search should be done, but if in the NSIS plugin
directory, you will need to explicitly provide its path.
An error message is returned if failed to load.

function is the name of the exported function to invoke.
It should be exactly as exported, which should match
the name used in a NSIS script.  An error message is
returned if function is not found or other error obtaining
its procedure address.

Any number (as permitted by the command line) of /VAR # str
sequences can be included (and in any location on cmd line).
/VAR marks the start of setting a variable.
# is 0-24, where 0-9 correspond to NSIS variables $0-$9,
and 10-19 correspond to NSIS variables $R0-$R9.
20-24 are $CMDLINE, $INSTDIR, $OUTDIR, $EXEDIR, $LANGUAGE

The remaining arguments (if any) are the strings pushed onto
the stack and passed to the plugin.  Strings are pushed on
the stack in opposite order from command line.  That is,
pass arguments on the command line in same order as if you
were invoking the plugin from a NSIS script (calling order,
which should match order plugin pops them).

This is primary meant as a debugging tool, so should be
modified if necessary to the plugin being tested, but
should work asis for many.  When using a debugger, one
should compile it with debugging information and the
plugin with debug information, then one can step through
them both and/or set break points.  Only the input stack
and variables and then output stack and variables are
shown.  Present version passes NULL for the HWND as no
window is created, though if request a future version will
add the neccessary window.  Future versions may also
add a /QUIET or /VERBOSE option allowing this to more
easily used be used as a wrapper.

| KJD
| 061122

*Modified by Robert Gill to support NSIS 3.0 and UNICODE*
