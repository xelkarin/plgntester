/* KJD 2004, public domain,
 * 2006 - portions based on NSIS so copyright NSIS license
 * NSIS plugin wrapper for testing plugins...
 */


//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#ifndef LVS_EX_LABELTIP
#define LVS_EX_LABELTIP 0x00004000 // listview thingy, defined in PSDK's later than MSVC 6
#endif
#include <windowsx.h>

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>


/* specifies the maximum size of a string pushed,
   popped, passed, etc.  set to match config.h
   or whatever for testing.  NSIS v2 default is 1024
*/
#define NSIS_MAX_STRLEN 1024


/* stack element, ie parameter to plugin, etc */
typedef struct stack_t
{
  struct stack_t *next;
  TCHAR text[NSIS_MAX_STRLEN];
} stack_t;


/* returns true if stack is empty, assumes initialized and valid stack */
bool empty(stack_t **stack)
{
  if ((stack == NULL) || (*stack == NULL))
    return true;
  else
    return false;
}


/* push an element onto the specified stack
   in a manner compatible with how NSIS pushes them
   copies str onto top of stack
   if bottom is true, then assumes this is the
   1st element pushed onto stack and inits underlying
   stack structure accordingly.
 */
void pushstring(stack_t **stack, LPCTSTR str, bool bottom=false)
{
  stack_t *element;

  /* verify a valid stack (at least nonNULL) was provided */
  if (stack == NULL)
  {
    _tprintf(_T("Error, passed NULL stack, unable to push [%s]\n"), str);
    exit(5);
  }

  /* allocate memory element, must match method plugins expect */
  element = (stack_t *)GlobalAlloc(GPTR /*fixed & zeroed*/, sizeof(stack_t));
  if (element == NULL)
  {
    _tprintf(_T("Error, unable to allocate memory to push [%s] onto stack\n"), str);
    exit(4);
  }

  /* copy string to stack */
  lstrcpyn(element->text, str, NSIS_MAX_STRLEN);

  /* update stack to actually 'push' this item onto top of it */
  if (bottom) element->next = NULL;
  else element->next = *stack;

  *stack = element;
}


/* pop an element off the specified stack
   in a manner compatible with how NSIS pops them
   copies top of stack to str
 */
void popstring(stack_t **stack, LPTSTR str)
{
  stack_t *element;;

  /* verify a valid stack (at least nonNULL) was provided */
  if (empty(stack))
  {
    printf("Warning, attempt to pop from an empty stack!\n");
    return;
  }
  element = *stack;

  /* copy text from top of stack to str */
  lstrcpyn(str, element->text, NSIS_MAX_STRLEN);

  /* mark new stack top */
  *stack = element->next;

  /* free memory in manner matching how NSIS plugins expect allocated */
  GlobalFree((HGLOBAL)element);
}


/* standard user variables,
   the 1st 0-9 are accessed as $# in NSIS (e.g. $0 )
   then 10-19 are R0-R9 accessed as $R# in NSIS (e.g. $R0 )
   then 20-24 are $CMDLINE, $INSTDIR, $OUTDIR, $EXEDIR, $LANGUAGE
 */
#define MAXUSERVARS 25


/* sets indicated user variable, i.e.
   copies str to user_vars[which]
   prints warning if which is invalid and returns
 */
void setuservar(LPTSTR user_vars, int which, LPCTSTR str)
{
  if ((which < 0) || (which >= MAXUSERVARS))
  {
    _tprintf(_T("Warning attempting to set invalid user_var[%i] with %s\n"), which, str);
    return;
  }

  /* array is stored in a flat char buffer */
  LPTSTR val = user_vars + (which * NSIS_MAX_STRLEN);

  /* actually copy the string to the variable */
  lstrcpyn(val, str, NSIS_MAX_STRLEN);
}


/* returns a pointer to the indicated user variable,
   warning, pointer returned should be treated as
   static buffer and copied to local buffer; contents
   may change via other calls to setuservar or by
   directly manipulating the string returned.
   returns NULL on error, such as invalid user var requested.
 */
const LPCTSTR getuservar(LPTSTR user_vars, int which)
{
  if ((which < 0) || (which >= MAXUSERVARS))
  {
    printf("Warning attempting to get invalid user_var[%i]\n", which);
    return NULL;
  }

  /* array is stored in a flat char buffer */
  LPCTSTR val = user_vars + (which * NSIS_MAX_STRLEN);

  return val;
}


/* initializes default values
   presently just clears to 0,
 */
void initpredefvars(LPTSTR user_vars)
{
  int i;
  /* simply set $0-$9 and $R0-$R10 to 0 */
  for (i = 0; i < 20; i++)
    setuservar(user_vars, i, _T("0"));

  /* set others to some reasonable value */
  setuservar(user_vars, 20, _T(""));     /* empty command line */
  setuservar(user_vars, 21, _T(".\\"));  /* $INSTDIR = current ??? */
  setuservar(user_vars, 22, _T(".\\"));  /* $OUTDIR */
  setuservar(user_vars, 23, _T(".\\"));  /* $EXEDIR */
  setuservar(user_vars, 24, _T("1033")); /* $LANGUAGE=english */
}




/* The exported API without name mangling */
extern "C" {
  typedef void (* pluginFunc)(HWND hwndParent, int string_size, LPTSTR variables, stack_t **stacktop);
}


/* returns pointer to function in plugin or exits with message on any error */
pluginFunc getPluginFunction(LPCTSTR plugin, const char *exportedName)
{
  HMODULE hMod = LoadLibrary(plugin);
  if (hMod == NULL)
  {
    _tprintf(_T("Failed to load %s\n"), plugin);
    exit(3);
  }
  pluginFunc pFn = (pluginFunc)GetProcAddress(hMod, exportedName);
  if (pFn == NULL)
  {
    _tprintf(_T("Failed to obtain address of function %S in module %s\n"), exportedName, plugin);
    exit(2);
  }
  return pFn;
}


/* displays the full contents of the stack and user variables */
void showstuff(LPTSTR variables, stack_t **stacktop)
{
  int i;
  stack_t *element;

  printf("User variables are:\n");
  for (i = 0; i < 10; i++)
    _tprintf(_T("$%i = [%s]\n"), i, getuservar(variables, i));
  for (i = 10; i < 20; i++)
    _tprintf(_T("$R%i = [%s]\n"), i-10, getuservar(variables, i));
  _tprintf(_T("$CMDLINE  = [%s]\n"), getuservar(variables, 20));
  _tprintf(_T("$INSTDIR  = [%s]\n"), getuservar(variables, 21));
  _tprintf(_T("$OUTDIR   = [%s]\n"), getuservar(variables, 22));
  _tprintf(_T("$EXEDIR   = [%s]\n"), getuservar(variables, 23));
  _tprintf(_T("$LANGUAGE = [%s]\n"), getuservar(variables, 24));
  printf("Stack is %s\n", empty(stacktop)?"empty":"");
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
    printf("No result returned on stack as stack is empty.\n");
    result[0] = '\0';
  }
  else
  {
    popstring(stack, result);
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


int _tmain(int argc, TCHAR *argv[])
{
  bool verbose=true;
  bool debug=false;

  pluginFunc pFn;
  int i, j, pn=0, pf=0;
  bool initstack=true;

  /* these are passed to plugin */
  stack_t *stack = NULL;
  TCHAR user_vars[NSIS_MAX_STRLEN*MAXUSERVARS];

  /* initialize to predefined stuff */
  initpredefvars(user_vars);


  /* check command line appears valid, else print basic help */
  if (argc < 3)
  {
    _tprintf(_T("NSIS plugin wrapper/tester\nUsage: %s [options] plugin function [args]\n"), argv[0]);
    printf("Where options may be ommited, /silent /debug\n");
    printf("Where plugin is the name (dll) of the plugin to load\n");
    printf("and function is the name of the exported function to invoke.\n");
    printf("User variables may be set using /VAR # str\n");
    printf("The remaining arguments (if any) are the strings pushed onto\n"
           "the stack and passed to the plugin (strings are pushed in\n"
           "reverse order passed on command line; ie use calling order).\n");
    return 0;
  }

  /* get plugin and function to invoke and possibly user variables */
  for (i = 1; (i < argc) && !pf; i++)
  {
    if (lstrcmp(_T("/VAR"), argv[i]) == 0)
    {
      setuservar(user_vars, _ttoi(argv[i+1]), argv[i+2]);
      i+=2;
    }
    else if (!pn)
    {
      if (lstrcmp(_T("/silent"), argv[i]) == 0) /* silent mode, no output */
        verbose = false; /* silent = true */
      else if (lstrcmp(_T("/debug"), argv[i]) == 0) /* current just display stack */
        debug = true;
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
      setuservar(user_vars, _ttoi(argv[j-1]), argv[j]);
      j-=2;
    }
    else
    {
      pushstring(&stack, argv[j], initstack);
      initstack = false;
    }
  }


  /* get the exported functions we want to test */
  char exportedName[2048];
  wcstombs(exportedName, argv[pf], 2048);
  pFn = getPluginFunction(argv[pn], exportedName);


  /* setup information passed around about plugin func to execute & its env */
  ExecData ed = { pFn, NULL, user_vars, &stack };
  execData = &ed;


  /*** NOTE: add any other initing your plugin may expect here ***/


  /* display to user stack contents prior to invoking and all variables */
  if (debug)
    showstuff(user_vars, &stack);

  /* perform the call to plugin and display the results */
  if (verbose)
    _tprintf(_T("now invoking %s.%s\n"), argv[pn], argv[pf]);

    plugin_thread(NULL);

  if (verbose)
    showresult(&stack);

  /* display to user stack contents after invoking and all variables */
  if (debug)
    showstuff(user_vars, &stack);

  return 0;
}
