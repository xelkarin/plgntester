/* KJD 2004, public domain,
 * 2006 - portions based on NSIS so copyright NSIS license
 * NSIS plugin wrapper for testing plugins...
 */


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "pluginapi.h"

#define PROCNAME_BUFSZ 2048
CHAR lpProcName[PROCNAME_BUFSZ];

/* specifies the maximum size of a string pushed,
   popped, passed, etc.  set to match config.h
   or whatever for testing.  NSIS v2 default is 1024
*/
#define NSIS_MAX_STRLEN 1024


/* returns true if stack is empty, assumes initialized and valid stack */
bool empty(stack_t **stack)
{
  if ((stack == NULL) || (*stack == NULL))
    return true;
  else
    return false;
}


/* standard user variables,
   the 1st 0-9 are accessed as $# in NSIS (e.g. $0 )
   then 10-19 are R0-R9 accessed as $R# in NSIS (e.g. $R0 )
   then 20-24 are $CMDLINE, $INSTDIR, $OUTDIR, $EXEDIR, $LANGUAGE
 */
#define MAXUSERVARS __INST_LAST


/* initializes default values
   presently just clears to 0,
 */
void initpredefvars(LPTSTR user_vars)
{
  int i;
  /* simply set $0-$9 and $R0-$R10 to 0 */
  for (i = 0; i < 20; i++)
    setuservariable(i, _T("0"));

  /* set others to some reasonable value */
  setuservariable(20, _T(""));     /* empty command line */
  setuservariable(21, _T(".\\"));  /* $INSTDIR = current ??? */
  setuservariable(22, _T(".\\"));  /* $OUTDIR */
  setuservariable(23, _T(".\\"));  /* $EXEDIR */
  setuservariable(24, _T("1033")); /* $LANGUAGE=english */
}




/* The exported API */
typedef void (* pluginFunc)(HWND hwndParent, int string_size, LPTSTR variables, stack_t **stacktop);


/* returns pointer to function in plugin or exits with message on any error */
pluginFunc getPluginFunction(LPCTSTR plugin, LPCTSTR exportedName)
{
#ifdef UNICODE
  wcstombs(lpProcName, exportedName, PROCNAME_BUFSZ);
#else
  strncpy_s(lpProcName, PROCNAME_BUFSZ, exportedName, _TRUNCATE);
#endif

  HMODULE hMod = LoadLibrary(plugin);
  if (hMod == NULL)
  {
    _tprintf(_T("Failed to load %s\n"), plugin);
    exit(3);
  }
  pluginFunc pFn = (pluginFunc)GetProcAddress(hMod, lpProcName);
  if (pFn == NULL)
  {
    _tprintf(_T("Failed to obtain address of function %s in module %s\n"), exportedName, plugin);
    exit(2);
  }
  return pFn;
}


/* displays the full contents of the stack and user variables */
void showstuff(LPTSTR variables, stack_t **stacktop)
{
  int i;
  stack_t *element;

  _tprintf(_T("User variables are:\n"));
  for (i = 0; i < 10; i++)
    _tprintf(_T("$%i = [%s]\n"), i, getuservariable(i));
  for (i = 10; i < 20; i++)
    _tprintf(_T("$R%i = [%s]\n"), i-10, getuservariable(i));
  _tprintf(_T("$CMDLINE  = [%s]\n"), getuservariable(20));
  _tprintf(_T("$INSTDIR  = [%s]\n"), getuservariable(21));
  _tprintf(_T("$OUTDIR   = [%s]\n"), getuservariable(22));
  _tprintf(_T("$EXEDIR   = [%s]\n"), getuservariable(23));
  _tprintf(_T("$LANGUAGE = [%s]\n"), getuservariable(24));
  _tprintf(_T("Stack is %s\n"), empty(stacktop)?_T("empty"):_T(""));
  if (!empty(stacktop))  /* peek at all elements */
  {
    element = *stacktop;
    do {
      _tprintf(_T("[%s]\n"), element->text);
      element = element->next;
    } while (element != NULL);
  }
}


/* displays a single result pushed on to stack after returning from plugin
   sets global variable result to top of stack, overwritten on all calls.
 */
TCHAR result[NSIS_MAX_STRLEN];
void showresult(stack_t **stack)
{
  if (empty(stack))  /* we expect a success or error message pushed on stack */
  {
    _tprintf(_T("No result returned on stack as stack is empty.\n"));
    result[0] = '\0';
  }
  else
  {
    popstring(result);
    _tprintf(_T("Found [%s] on stack and now stack is %sempty.\n"), result, empty(stack)?_T(""):_T("NOT "));
  }
}



typedef struct ExecData {
  pluginFunc pFn;
  HWND hwndBG;
  LPTSTR user_vars;
  stack_t **stack;
} ExecData;
ExecData *execData;


static DWORD WINAPI plugin_thread(LPVOID param)
{
  execData->pFn(execData->hwndBG, NSIS_MAX_STRLEN, execData->user_vars, execData->stack);
  return 0;
}


int _tmain(int argc, _TCHAR *argv[])
{
  bool verbose=true;
  pluginFunc pFn;
  int i, j, pn=0, pf=0;

  /* these are passed to plugin */
  stack_t *stack = (stack_t *) GlobalAlloc(GPTR, sizeof(stack_t));
  TCHAR user_vars[sizeof(TCHAR)*NSIS_MAX_STRLEN*MAXUSERVARS];

  /* initialize global plugin variables */
  g_stringsize = NSIS_MAX_STRLEN;
  g_stacktop = &stack;
  g_variables = user_vars;

  /* initialize to predefined stuff */
  initpredefvars(g_variables);


  /* check command line appears valid, else print basic help */
  if (argc < 3)
  {
    _tprintf(_T("NSIS plugin wrapper/tester\nUsage: %s [options] plugin function [args]\n"), argv[0]);
    _tprintf(_T("Where options may be ommited, /silent\n"));
    _tprintf(_T("Where plugin is the name (dll) of the plugin to load\n"));
    _tprintf(_T("and function is the name of the exported function to invoke.\n"));
    _tprintf(_T("User variables may be set using /VAR # str\n"));
    _tprintf(_T("The remaining arguments (if any) are the strings pushed onto\n")
             _T("the stack and passed to the plugin (strings are pushed in\n")
             _T("reverse order passed on command line; ie use calling order).\n"));
    return 0;
  }

  /* get plugin and function to invoke and possibly user variables */
  for (i = 1; (i < argc) && !pf; i++)
  {
    if (lstrcmp(_T("/VAR"), argv[i]) == 0)
    {
      setuservariable(_ttoi(argv[i+1]), argv[i+2]);
      i+=2;
    }
    else if (!pn)
    {
      if (lstrcmp(_T("/silent"), argv[i]) == 0) /* silent mode, no output */
        verbose = false; /* silent = true */
      else  /* plugin name */
        pn = i;
    }
    else /* if (!pf) then function */
    {
      pf = i;
    }
  }
  /* now push items onto stack in reverse order */
  for (j=argc-1; j >= i; j--)
  {
    if (lstrcmp(_T("/VAR"), argv[j-2]) == 0)
    {
      setuservariable(_ttoi(argv[j-1]), argv[j]);
      j-=2;
    }
    else
    {
      pushstring(argv[j]);
    }
  }


  /* get the exported functions we want to test */
  pFn = getPluginFunction(argv[pn], argv[pf]);


  /* setup information passed around about plugin func to execute & its env */
  ExecData ed = { pFn, NULL, g_variables, g_stacktop };
  execData = &ed;


  /*** NOTE: add any other initing your plugin may expect here ***/


  /* perform the call to plugin and display the results */
  if (verbose)
  {
    showstuff(g_variables, g_stacktop);
    _tprintf(_T("now invoking %s.%s\n"), argv[pn], argv[pf]);
  }

    plugin_thread(NULL);

  if (verbose)
  {
    showresult(g_stacktop);
    showstuff(g_variables, g_stacktop);
  }

  return 0;
}
