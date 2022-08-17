#ifndef COMMAND_H

  #define COMMAND_H

  #ifndef _INC_STDIO
    #include <stdio.h>
  #endif

  #ifndef _INC_STDLIB
    #include <stdlib.h>
  #endif

  #ifndef _INC_STRING
    #include <string.h>
  #endif

  #ifndef __ASSERT_H_
    #include <assert.h>
  #endif

  #ifndef __bool_true_false_are_defined
    #include <stdbool.h>
  #endif

  enum OptionType {
    BOOL_OPT,
    INT_OPT,
    STR_OPT,
  };

  // TODO: Ignore as_bool?
  // Isn't is_set enough?
  union OptionValue {
    bool as_bool;
    int as_int;
    char *as_str;
  };

  struct Option {
    char *name;
    enum OptionType type;
    union OptionValue value;
    bool is_set;
  };

  struct Option *NewOption(enum OptionType type, char *name) {
    struct Option *o = (struct Option*) malloc(sizeof(struct Option));
    if (o == NULL) {
      fprintf(stderr, "Memory allocation failed!\n");
      exit(EXIT_FAILURE);
    }
    o->name = name;
    o->type = type;
    o->is_set = false;
    return o;
  }

  struct Option *NewBoolOption(char *name) {
    return NewOption(BOOL_OPT, name);
  }

  struct Option *NewIntOption(char *name) {
    return NewOption(INT_OPT, name);
  }

  struct Option *NewStringOption(char *name) {
    return NewOption(STR_OPT, name);
  }

  struct Command {
    char *name;
    char *description;

    void (*run)(struct Command *command, void *data);

    struct Command **commands;
    size_t command_count;
    size_t command_capacity;

    struct Option **options;
    size_t option_count;
    size_t option_capacity;

    char **args;
    size_t arg_count;
    size_t arg_capacity;
  };

  struct Command *NewCommand(char *name, void (*run)(struct Command *command, void *data)) {
    struct Command *c = (struct Command*) malloc(sizeof(struct Command));

    if (c == NULL) {
      fprintf(stderr, "Memory allocation failed!\n");
      exit(EXIT_FAILURE);
    }

    c->name = name;
    c->description = NULL;
    c->run = run;
    c->commands = NULL;
    c->command_count= 0;
    c->command_capacity = 10;
    c->options = NULL;
    c->option_count = 0;
    c->option_capacity = 10;
    c->args = NULL;
    c->arg_count = 0;
    c->arg_capacity = 10;

    return c;
  }

  void AddSubCommand(struct Command *cmd, struct Command *sub) {
    assert(cmd != NULL);
    assert(sub != NULL);

    if (cmd->command_count == 0) {
      cmd->commands = (struct Command**) malloc(sizeof(struct Command*) * cmd->command_capacity);
      if (cmd->commands == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
      }
    }

    cmd->commands[cmd->command_count++] = sub;

    if (cmd->command_count == cmd->command_capacity) {
      cmd->command_capacity *= 2;
      cmd->commands = (struct Command**) realloc(cmd->commands, sizeof(struct Command*) * cmd->command_capacity);
      if (cmd->commands == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  void AddOption(struct Command *c, enum OptionType type, char *name) {
    assert(c != NULL);
    assert(type >= BOOL_OPT && type <= STR_OPT);
    assert(name != NULL);

    if (c->option_count == 0) {
      c->options = (struct Option**) malloc(sizeof(struct Option*) * c->option_capacity);
      if (c->options == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
      }
    }

    c->options[c->option_count++] = NewOption(type, name);

    if (c->option_count == c->option_capacity) {
      c->option_capacity *= 2;
      c->options = (struct Option**) realloc(c->options, sizeof(struct Option*) * c->option_capacity);
      if (c->options == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  void AddBoolOption(struct Command *c, char *name) {
    AddOption(c, BOOL_OPT, name);
  }

  void AddIntOption(struct Command *c, char *name) {
    AddOption(c, INT_OPT, name);
  }

  void AddStringOption(struct Command *c, char *name) {
    AddOption(c, STR_OPT, name);
  }

  struct Option *GetOption(struct Command *c, char *name) {
    assert(c != NULL);
    assert(name != NULL);

    for (size_t i = 0; i < c->option_count; i++) {
      struct Option *o = c->options[i];
      if (strcmp(o->name, name) == 0) {
        return o;
      }
    }
    return NULL;
  }

  void AddArg(struct Command *c, char *arg) {
    assert(c != NULL);
    assert(arg != NULL);

    if (c->arg_count == 0) {
      c->args = (char**) malloc(sizeof(char*) * c->arg_capacity);
      if (c->args == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
      }
    }

    c->args[c->arg_count++] = arg;

    if (c->arg_count == c->arg_capacity) {
      c->arg_capacity *= 2;
      c->args = (char**) realloc(c->args, sizeof(char*) * c->arg_capacity);
      if (c->args == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  void ParseAndRunCommand(int argc, char **argv, struct Command *cmd, void *data) {
    assert(argc > 0);
    assert(argv != NULL);
    assert(cmd != NULL);

    struct Command *tmp = cmd;
    struct Option *read_option = NULL;
    bool check_for_command = true;
    bool read_option_value = false;

    for (int i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (read_option_value) {
        read_option_value = false;
        if (read_option->type == INT_OPT) {
          read_option->value.as_int = atoi(arg);
        } else if (read_option->type == STR_OPT) {
          read_option->value.as_str = arg;
        }
        read_option->is_set = true;
        read_option = NULL;
        continue;
      }

      bool matched_option = false;
      bool matched_command = false;

      for (size_t j = 0; j < tmp->option_count; j++) {
        struct Option *o = tmp->options[j];
        if (strcmp(o->name, arg) == 0) {
          matched_option = true;
          read_option = o;
          if (o->type != BOOL_OPT) {
            read_option_value = true;
          } else {
            o->is_set = true;
          }
          break;
        }
      }

      if (matched_option) {
        continue;
      }

      if (check_for_command) for (size_t j = 0; j < tmp->command_count; j++) {
        if (strcmp(tmp->commands[j]->name, arg) == 0) {
          tmp = tmp->commands[j];
          matched_command = true;
          break;
        }
      }

      if (!matched_command) {
        check_for_command = false;
        AddArg(tmp, arg);
      }

    }

    if (tmp->run != NULL) {
      tmp->run(tmp, data);
    }
  }

#endif
