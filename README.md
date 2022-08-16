# ccli

`ccli` is a library that provides useful functions for developing command-line interfacing applications in the C programming language.

## Usage example

```c
#include "ccli.h"

void cmd_app(struct Command *c, void *data)
{
  struct Option *opt_version = GetOption(c, "-v");
  if (opt_version->is_set)
  {
    puts("1.23.5");
  }
  else
  {
    puts("No options or arguments provided!");
  }
}

void cmd_greet(struct Command *c, void *data)
{
  if (c->arg_count == 0)
  {
    printf("Please provide some people to be greeted!\n");
    return;
  }
  struct Option *opt_lang = GetOption(c, "--language");
  for (size_t i = 0; i < c->arg_count; i++)
  {
    if (opt_lang->is_set && strcmp(opt_lang->value.as_str, "spanish") == 0)
    {
      printf("Hola, %s!\n", c->args[i]);
    }
    else
    {
      printf("Hello, %s!\n", c->args[i]);
    }
  }
}

int main(int argc, char **argv)
{
  // The base command should not have a name but be NULL
  // ./my-app -v ...
  struct Command *base = NewCommand(NULL, cmd_app);
  AddBoolOption(base, "-v"); // Boolean options don't expect arguments

  // ./my-app greet Alice Bob
  // => Hello, Alice!
  // => Hello, Bob!
  struct Command *greet = NewCommand("greet", cmd_greet);
  AddStringOption(greet, "--language"); // String and integer options expect arguments
  AddSubCommand(base, greet);

  ParseAndRunCommand(argc, argv, base, NULL);

  return 0;
}
```
