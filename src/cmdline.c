/*
  File autogenerated by gengetopt version 2.22.6
  generated with the following command:
  /usr/bin/gengetopt --output-dir=src/ <command line options>

  The developers of gengetopt consider the fixed text that goes in all
  gengetopt output files to be in the public domain:
  we make no copyright claims on it.
*/

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIX_UNUSED
#define FIX_UNUSED(X) (void) (X) /* avoid warnings for unused params */
#endif

#include <getopt.h>

#include "cmdline.h"

const char *gengetopt_args_info_purpose = "Roguelike based on the Warhammer Roleplay version 2 rules from Black\nIndustries.";

const char *gengetopt_args_info_usage = "Usage: heresyrl [OPTIONS]...";

const char *gengetopt_args_info_versiontext = "";

const char *gengetopt_args_info_description = "";

const char *gengetopt_args_info_help[] = {
  "  -h, --help              Print help and exit",
  "  -V, --version           Print version and exit",
  "\nBasic Options:",
  "      --playback          play a savegame from start until current turn\n                            (default=off)",
  "      --pb_delay=INT      delay when playing a savegame in miliseconds, default\n                            is 1 second  (default=`100')",
  "      --pb_stop=INT       when playing a savegame, stop at after turn N\n                            (default=`0')",
  "      --log_file=STRING   log file name  (default=`/tmp/heresyrl.log')",
  "      --save_file=STRING  save file name  (default=`/tmp/heresyrl.save')",
  "      --name=STRING       name of character  (default=`')",
  "      --race=ENUM         race of character  (possible values=\"dwarf\",\n                            \"elf\", \"halfling\", \"human\")",
  "\nTesting Options:",
  "  -d, --debug             show debug output  (default=off)",
  "  -m, --map               show the complete map  (default=off)",
  "      --test_auto         same as playback, but quite when done and show\n                            nothing  (default=off)",
  "  -l, --no_load           do not load a previous made character  (default=off)",
  "  -s, --no_save           do not save a made character  (default=off)",
  "      --print_map_only    only print the map and close  (default=off)",
    0
};

typedef enum {ARG_NO
  , ARG_FLAG
  , ARG_STRING
  , ARG_INT
  , ARG_ENUM
} cmdline_parser_arg_type;

static
void clear_given (struct gengetopt_args_info *args_info);
static
void clear_args (struct gengetopt_args_info *args_info);

static int
cmdline_parser_internal (int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error);


const char *cmdline_parser_race_values[] = {"dwarf", "elf", "halfling", "human", 0}; /*< Possible values for race. */

static char *
gengetopt_strdup (const char *s);

static
void clear_given (struct gengetopt_args_info *args_info)
{
  args_info->help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->playback_given = 0 ;
  args_info->pb_delay_given = 0 ;
  args_info->pb_stop_given = 0 ;
  args_info->log_file_given = 0 ;
  args_info->save_file_given = 0 ;
  args_info->name_given = 0 ;
  args_info->race_given = 0 ;
  args_info->debug_given = 0 ;
  args_info->map_given = 0 ;
  args_info->test_auto_given = 0 ;
  args_info->no_load_given = 0 ;
  args_info->no_save_given = 0 ;
  args_info->print_map_only_given = 0 ;
}

static
void clear_args (struct gengetopt_args_info *args_info)
{
  FIX_UNUSED (args_info);
  args_info->playback_flag = 0;
  args_info->pb_delay_arg = 100;
  args_info->pb_delay_orig = NULL;
  args_info->pb_stop_arg = 0;
  args_info->pb_stop_orig = NULL;
  args_info->log_file_arg = gengetopt_strdup ("/tmp/heresyrl.log");
  args_info->log_file_orig = NULL;
  args_info->save_file_arg = gengetopt_strdup ("/tmp/heresyrl.save");
  args_info->save_file_orig = NULL;
  args_info->name_arg = gengetopt_strdup ("");
  args_info->name_orig = NULL;
  args_info->race_arg = race__NULL;
  args_info->race_orig = NULL;
  args_info->debug_flag = 0;
  args_info->map_flag = 0;
  args_info->test_auto_flag = 0;
  args_info->no_load_flag = 0;
  args_info->no_save_flag = 0;
  args_info->print_map_only_flag = 0;
  
}

static
void init_args_info(struct gengetopt_args_info *args_info)
{


  args_info->help_help = gengetopt_args_info_help[0] ;
  args_info->version_help = gengetopt_args_info_help[1] ;
  args_info->playback_help = gengetopt_args_info_help[3] ;
  args_info->pb_delay_help = gengetopt_args_info_help[4] ;
  args_info->pb_stop_help = gengetopt_args_info_help[5] ;
  args_info->log_file_help = gengetopt_args_info_help[6] ;
  args_info->save_file_help = gengetopt_args_info_help[7] ;
  args_info->name_help = gengetopt_args_info_help[8] ;
  args_info->race_help = gengetopt_args_info_help[9] ;
  args_info->debug_help = gengetopt_args_info_help[11] ;
  args_info->map_help = gengetopt_args_info_help[12] ;
  args_info->test_auto_help = gengetopt_args_info_help[13] ;
  args_info->no_load_help = gengetopt_args_info_help[14] ;
  args_info->no_save_help = gengetopt_args_info_help[15] ;
  args_info->print_map_only_help = gengetopt_args_info_help[16] ;
  
}

void
cmdline_parser_print_version (void)
{
  printf ("%s %s\n",
     (strlen(CMDLINE_PARSER_PACKAGE_NAME) ? CMDLINE_PARSER_PACKAGE_NAME : CMDLINE_PARSER_PACKAGE),
     CMDLINE_PARSER_VERSION);

  if (strlen(gengetopt_args_info_versiontext) > 0)
    printf("\n%s\n", gengetopt_args_info_versiontext);
}

static void print_help_common(void) {
  cmdline_parser_print_version ();

  if (strlen(gengetopt_args_info_purpose) > 0)
    printf("\n%s\n", gengetopt_args_info_purpose);

  if (strlen(gengetopt_args_info_usage) > 0)
    printf("\n%s\n", gengetopt_args_info_usage);

  printf("\n");

  if (strlen(gengetopt_args_info_description) > 0)
    printf("%s\n\n", gengetopt_args_info_description);
}

void
cmdline_parser_print_help (void)
{
  int i = 0;
  print_help_common();
  while (gengetopt_args_info_help[i])
    printf("%s\n", gengetopt_args_info_help[i++]);
}

void
cmdline_parser_init (struct gengetopt_args_info *args_info)
{
  clear_given (args_info);
  clear_args (args_info);
  init_args_info (args_info);
}

void
cmdline_parser_params_init(struct cmdline_parser_params *params)
{
  if (params)
    { 
      params->override = 0;
      params->initialize = 1;
      params->check_required = 1;
      params->check_ambiguity = 0;
      params->print_errors = 1;
    }
}

struct cmdline_parser_params *
cmdline_parser_params_create(void)
{
  struct cmdline_parser_params *params = 
    (struct cmdline_parser_params *)malloc(sizeof(struct cmdline_parser_params));
  cmdline_parser_params_init(params);  
  return params;
}

static void
free_string_field (char **s)
{
  if (*s)
    {
      free (*s);
      *s = 0;
    }
}


static void
cmdline_parser_release (struct gengetopt_args_info *args_info)
{

  free_string_field (&(args_info->pb_delay_orig));
  free_string_field (&(args_info->pb_stop_orig));
  free_string_field (&(args_info->log_file_arg));
  free_string_field (&(args_info->log_file_orig));
  free_string_field (&(args_info->save_file_arg));
  free_string_field (&(args_info->save_file_orig));
  free_string_field (&(args_info->name_arg));
  free_string_field (&(args_info->name_orig));
  free_string_field (&(args_info->race_orig));
  
  

  clear_given (args_info);
}

/**
 * @param val the value to check
 * @param values the possible values
 * @return the index of the matched value:
 * -1 if no value matched,
 * -2 if more than one value has matched
 */
static int
check_possible_values(const char *val, const char *values[])
{
  int i, found, last;
  size_t len;

  if (!val)   /* otherwise strlen() crashes below */
    return -1; /* -1 means no argument for the option */

  found = last = 0;

  for (i = 0, len = strlen(val); values[i]; ++i)
    {
      if (strncmp(val, values[i], len) == 0)
        {
          ++found;
          last = i;
          if (strlen(values[i]) == len)
            return i; /* exact macth no need to check more */
        }
    }

  if (found == 1) /* one match: OK */
    return last;

  return (found ? -2 : -1); /* return many values or none matched */
}


static void
write_into_file(FILE *outfile, const char *opt, const char *arg, const char *values[])
{
  int found = -1;
  if (arg) {
    if (values) {
      found = check_possible_values(arg, values);      
    }
    if (found >= 0)
      fprintf(outfile, "%s=\"%s\" # %s\n", opt, arg, values[found]);
    else
      fprintf(outfile, "%s=\"%s\"\n", opt, arg);
  } else {
    fprintf(outfile, "%s\n", opt);
  }
}


int
cmdline_parser_dump(FILE *outfile, struct gengetopt_args_info *args_info)
{
  int i = 0;

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot dump options to stream\n", CMDLINE_PARSER_PACKAGE);
      return EXIT_FAILURE;
    }

  if (args_info->help_given)
    write_into_file(outfile, "help", 0, 0 );
  if (args_info->version_given)
    write_into_file(outfile, "version", 0, 0 );
  if (args_info->playback_given)
    write_into_file(outfile, "playback", 0, 0 );
  if (args_info->pb_delay_given)
    write_into_file(outfile, "pb_delay", args_info->pb_delay_orig, 0);
  if (args_info->pb_stop_given)
    write_into_file(outfile, "pb_stop", args_info->pb_stop_orig, 0);
  if (args_info->log_file_given)
    write_into_file(outfile, "log_file", args_info->log_file_orig, 0);
  if (args_info->save_file_given)
    write_into_file(outfile, "save_file", args_info->save_file_orig, 0);
  if (args_info->name_given)
    write_into_file(outfile, "name", args_info->name_orig, 0);
  if (args_info->race_given)
    write_into_file(outfile, "race", args_info->race_orig, cmdline_parser_race_values);
  if (args_info->debug_given)
    write_into_file(outfile, "debug", 0, 0 );
  if (args_info->map_given)
    write_into_file(outfile, "map", 0, 0 );
  if (args_info->test_auto_given)
    write_into_file(outfile, "test_auto", 0, 0 );
  if (args_info->no_load_given)
    write_into_file(outfile, "no_load", 0, 0 );
  if (args_info->no_save_given)
    write_into_file(outfile, "no_save", 0, 0 );
  if (args_info->print_map_only_given)
    write_into_file(outfile, "print_map_only", 0, 0 );
  

  i = EXIT_SUCCESS;
  return i;
}

int
cmdline_parser_file_save(const char *filename, struct gengetopt_args_info *args_info)
{
  FILE *outfile;
  int i = 0;

  outfile = fopen(filename, "w");

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot open file for writing: %s\n", CMDLINE_PARSER_PACKAGE, filename);
      return EXIT_FAILURE;
    }

  i = cmdline_parser_dump(outfile, args_info);
  fclose (outfile);

  return i;
}

void
cmdline_parser_free (struct gengetopt_args_info *args_info)
{
  cmdline_parser_release (args_info);
}

/** @brief replacement of strdup, which is not standard */
char *
gengetopt_strdup (const char *s)
{
  char *result = 0;
  if (!s)
    return result;

  result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}

int
cmdline_parser (int argc, char **argv, struct gengetopt_args_info *args_info)
{
  return cmdline_parser2 (argc, argv, args_info, 0, 1, 1);
}

int
cmdline_parser_ext (int argc, char **argv, struct gengetopt_args_info *args_info,
                   struct cmdline_parser_params *params)
{
  int result;
  result = cmdline_parser_internal (argc, argv, args_info, params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser2 (int argc, char **argv, struct gengetopt_args_info *args_info, int override, int initialize, int check_required)
{
  int result;
  struct cmdline_parser_params params;
  
  params.override = override;
  params.initialize = initialize;
  params.check_required = check_required;
  params.check_ambiguity = 0;
  params.print_errors = 1;

  result = cmdline_parser_internal (argc, argv, args_info, &params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser_required (struct gengetopt_args_info *args_info, const char *prog_name)
{
  FIX_UNUSED (args_info);
  FIX_UNUSED (prog_name);
  return EXIT_SUCCESS;
}


static char *package_name = 0;

/**
 * @brief updates an option
 * @param field the generic pointer to the field to update
 * @param orig_field the pointer to the orig field
 * @param field_given the pointer to the number of occurrence of this option
 * @param prev_given the pointer to the number of occurrence already seen
 * @param value the argument for this option (if null no arg was specified)
 * @param possible_values the possible values for this option (if specified)
 * @param default_value the default value (in case the option only accepts fixed values)
 * @param arg_type the type of this option
 * @param check_ambiguity @see cmdline_parser_params.check_ambiguity
 * @param override @see cmdline_parser_params.override
 * @param no_free whether to free a possible previous value
 * @param multiple_option whether this is a multiple option
 * @param long_opt the corresponding long option
 * @param short_opt the corresponding short option (or '-' if none)
 * @param additional_error possible further error specification
 */
static
int update_arg(void *field, char **orig_field,
               unsigned int *field_given, unsigned int *prev_given, 
               char *value, const char *possible_values[],
               const char *default_value,
               cmdline_parser_arg_type arg_type,
               int check_ambiguity, int override,
               int no_free, int multiple_option,
               const char *long_opt, char short_opt,
               const char *additional_error)
{
  char *stop_char = 0;
  const char *val = value;
  int found;
  char **string_field;
  FIX_UNUSED (field);

  stop_char = 0;
  found = 0;

  if (!multiple_option && prev_given && (*prev_given || (check_ambiguity && *field_given)))
    {
      if (short_opt != '-')
        fprintf (stderr, "%s: `--%s' (`-%c') option given more than once%s\n", 
               package_name, long_opt, short_opt,
               (additional_error ? additional_error : ""));
      else
        fprintf (stderr, "%s: `--%s' option given more than once%s\n", 
               package_name, long_opt,
               (additional_error ? additional_error : ""));
      return 1; /* failure */
    }

  if (possible_values && (found = check_possible_values((value ? value : default_value), possible_values)) < 0)
    {
      if (short_opt != '-')
        fprintf (stderr, "%s: %s argument, \"%s\", for option `--%s' (`-%c')%s\n", 
          package_name, (found == -2) ? "ambiguous" : "invalid", value, long_opt, short_opt,
          (additional_error ? additional_error : ""));
      else
        fprintf (stderr, "%s: %s argument, \"%s\", for option `--%s'%s\n", 
          package_name, (found == -2) ? "ambiguous" : "invalid", value, long_opt,
          (additional_error ? additional_error : ""));
      return 1; /* failure */
    }
    
  if (field_given && *field_given && ! override)
    return 0;
  if (prev_given)
    (*prev_given)++;
  if (field_given)
    (*field_given)++;
  if (possible_values)
    val = possible_values[found];

  switch(arg_type) {
  case ARG_FLAG:
    *((int *)field) = !*((int *)field);
    break;
  case ARG_INT:
    if (val) *((int *)field) = strtol (val, &stop_char, 0);
    break;
  case ARG_ENUM:
    if (val) *((int *)field) = found;
    break;
  case ARG_STRING:
    if (val) {
      string_field = (char **)field;
      if (!no_free && *string_field)
        free (*string_field); /* free previous string */
      *string_field = gengetopt_strdup (val);
    }
    break;
  default:
    break;
  };

  /* check numeric conversion */
  switch(arg_type) {
  case ARG_INT:
    if (val && !(stop_char && *stop_char == '\0')) {
      fprintf(stderr, "%s: invalid numeric value: %s\n", package_name, val);
      return 1; /* failure */
    }
    break;
  default:
    ;
  };

  /* store the original value */
  switch(arg_type) {
  case ARG_NO:
  case ARG_FLAG:
    break;
  default:
    if (value && orig_field) {
      if (no_free) {
        *orig_field = value;
      } else {
        if (*orig_field)
          free (*orig_field); /* free previous string */
        *orig_field = gengetopt_strdup (value);
      }
    }
  };

  return 0; /* OK */
}


int
cmdline_parser_internal (
  int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error)
{
  int c;	/* Character of the parsed option.  */

  int error_occurred = 0;
  struct gengetopt_args_info local_args_info;
  
  int override;
  int initialize;
  int check_required;
  int check_ambiguity;
  
  package_name = argv[0];
  
  override = params->override;
  initialize = params->initialize;
  check_required = params->check_required;
  check_ambiguity = params->check_ambiguity;

  if (initialize)
    cmdline_parser_init (args_info);

  cmdline_parser_init (&local_args_info);

  optarg = 0;
  optind = 0;
  opterr = params->print_errors;
  optopt = '?';

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] = {
        { "help",	0, NULL, 'h' },
        { "version",	0, NULL, 'V' },
        { "playback",	0, NULL, 0 },
        { "pb_delay",	1, NULL, 0 },
        { "pb_stop",	1, NULL, 0 },
        { "log_file",	1, NULL, 0 },
        { "save_file",	1, NULL, 0 },
        { "name",	1, NULL, 0 },
        { "race",	1, NULL, 0 },
        { "debug",	0, NULL, 'd' },
        { "map",	0, NULL, 'm' },
        { "test_auto",	0, NULL, 0 },
        { "no_load",	0, NULL, 'l' },
        { "no_save",	0, NULL, 's' },
        { "print_map_only",	0, NULL, 0 },
        { 0,  0, 0, 0 }
      };

      c = getopt_long (argc, argv, "hVdmls", long_options, &option_index);

      if (c == -1) break;	/* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':	/* Print help and exit.  */
          cmdline_parser_print_help ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'V':	/* Print version and exit.  */
          cmdline_parser_print_version ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'd':	/* show debug output.  */
        
        
          if (update_arg((void *)&(args_info->debug_flag), 0, &(args_info->debug_given),
              &(local_args_info.debug_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "debug", 'd',
              additional_error))
            goto failure;
        
          break;
        case 'm':	/* show the complete map.  */
        
        
          if (update_arg((void *)&(args_info->map_flag), 0, &(args_info->map_given),
              &(local_args_info.map_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "map", 'm',
              additional_error))
            goto failure;
        
          break;
        case 'l':	/* do not load a previous made character.  */
        
        
          if (update_arg((void *)&(args_info->no_load_flag), 0, &(args_info->no_load_given),
              &(local_args_info.no_load_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "no_load", 'l',
              additional_error))
            goto failure;
        
          break;
        case 's':	/* do not save a made character.  */
        
        
          if (update_arg((void *)&(args_info->no_save_flag), 0, &(args_info->no_save_given),
              &(local_args_info.no_save_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "no_save", 's',
              additional_error))
            goto failure;
        
          break;

        case 0:	/* Long option with no short option */
          /* play a savegame from start until current turn.  */
          if (strcmp (long_options[option_index].name, "playback") == 0)
          {
          
          
            if (update_arg((void *)&(args_info->playback_flag), 0, &(args_info->playback_given),
                &(local_args_info.playback_given), optarg, 0, 0, ARG_FLAG,
                check_ambiguity, override, 1, 0, "playback", '-',
                additional_error))
              goto failure;
          
          }
          /* delay when playing a savegame in miliseconds, default is 1 second.  */
          else if (strcmp (long_options[option_index].name, "pb_delay") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->pb_delay_arg), 
                 &(args_info->pb_delay_orig), &(args_info->pb_delay_given),
                &(local_args_info.pb_delay_given), optarg, 0, "100", ARG_INT,
                check_ambiguity, override, 0, 0,
                "pb_delay", '-',
                additional_error))
              goto failure;
          
          }
          /* when playing a savegame, stop at after turn N.  */
          else if (strcmp (long_options[option_index].name, "pb_stop") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->pb_stop_arg), 
                 &(args_info->pb_stop_orig), &(args_info->pb_stop_given),
                &(local_args_info.pb_stop_given), optarg, 0, "0", ARG_INT,
                check_ambiguity, override, 0, 0,
                "pb_stop", '-',
                additional_error))
              goto failure;
          
          }
          /* log file name.  */
          else if (strcmp (long_options[option_index].name, "log_file") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->log_file_arg), 
                 &(args_info->log_file_orig), &(args_info->log_file_given),
                &(local_args_info.log_file_given), optarg, 0, "/tmp/heresyrl.log", ARG_STRING,
                check_ambiguity, override, 0, 0,
                "log_file", '-',
                additional_error))
              goto failure;
          
          }
          /* save file name.  */
          else if (strcmp (long_options[option_index].name, "save_file") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->save_file_arg), 
                 &(args_info->save_file_orig), &(args_info->save_file_given),
                &(local_args_info.save_file_given), optarg, 0, "/tmp/heresyrl.save", ARG_STRING,
                check_ambiguity, override, 0, 0,
                "save_file", '-',
                additional_error))
              goto failure;
          
          }
          /* name of character.  */
          else if (strcmp (long_options[option_index].name, "name") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->name_arg), 
                 &(args_info->name_orig), &(args_info->name_given),
                &(local_args_info.name_given), optarg, 0, "", ARG_STRING,
                check_ambiguity, override, 0, 0,
                "name", '-',
                additional_error))
              goto failure;
          
          }
          /* race of character.  */
          else if (strcmp (long_options[option_index].name, "race") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->race_arg), 
                 &(args_info->race_orig), &(args_info->race_given),
                &(local_args_info.race_given), optarg, cmdline_parser_race_values, 0, ARG_ENUM,
                check_ambiguity, override, 0, 0,
                "race", '-',
                additional_error))
              goto failure;
          
          }
          /* same as playback, but quite when done and show nothing.  */
          else if (strcmp (long_options[option_index].name, "test_auto") == 0)
          {
          
          
            if (update_arg((void *)&(args_info->test_auto_flag), 0, &(args_info->test_auto_given),
                &(local_args_info.test_auto_given), optarg, 0, 0, ARG_FLAG,
                check_ambiguity, override, 1, 0, "test_auto", '-',
                additional_error))
              goto failure;
          
          }
          /* only print the map and close.  */
          else if (strcmp (long_options[option_index].name, "print_map_only") == 0)
          {
          
          
            if (update_arg((void *)&(args_info->print_map_only_flag), 0, &(args_info->print_map_only_given),
                &(local_args_info.print_map_only_given), optarg, 0, 0, ARG_FLAG,
                check_ambiguity, override, 1, 0, "print_map_only", '-',
                additional_error))
              goto failure;
          
          }
          
          break;
        case '?':	/* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          goto failure;

        default:	/* bug: option not considered.  */
          fprintf (stderr, "%s: option unknown: %c%s\n", CMDLINE_PARSER_PACKAGE, c, (additional_error ? additional_error : ""));
          abort ();
        } /* switch */
    } /* while */




  cmdline_parser_release (&local_args_info);

  if ( error_occurred )
    return (EXIT_FAILURE);

  return 0;

failure:
  
  cmdline_parser_release (&local_args_info);
  return (EXIT_FAILURE);
}
