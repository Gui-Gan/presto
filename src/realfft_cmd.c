/*****
  command line parser -- generated by clig
  (http://wsd.iitb.fhg.de/~kir/clighome/)

  The Command line parser `clig':
  (C) 1995---2001 Harald Kirsch (kirschh@lionbioscience.com)
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "realfft_cmd.h"

char *Program;

/*@-null*/

static Cmdline cmd = {
  /***** -fwd: Force an forward FFT (sign=-1) to be performed */
   /* forwardP = */ 0,
  /***** -inv: Force an inverse FFT (sign=+1) to be performed */
   /* inverseP = */ 0,
  /***** -del: Delete the original file(s) when performing the FFT */
   /* deleteP = */ 0,
  /***** -disk: Force the use of the out-of-core memory FFT */
   /* diskfftP = */ 0,
  /***** -mem: Force the use of the in-core memory FFT */
   /* memfftP = */ 0,
  /***** -tmpdir: Scratch directory for temp file(s) in out-of-core FFT */
   /* tmpdirP = */ 0,
   /* tmpdir = */ (char *) 0,
   /* tmpdirC = */ 0,
  /***** -outdir: Directory where result file(s) will reside */
   /* outdirP = */ 0,
   /* outdir = */ (char *) 0,
   /* outdirC = */ 0,
  /***** uninterpreted rest of command line */
   /* argc = */ 0,
   /* argv = */ (char **) 0,
  /***** the original command line concatenated */
   /* full_cmd_line = */ NULL
};

/*@=null*/

/***** let LCLint run more smoothly */
/*@-predboolothers*/
/*@-boolops*/


/******************************************************************/
/*****
 This is a bit tricky. We want to make a difference between overflow
 and underflow and we want to allow v==Inf or v==-Inf but not
 v>FLT_MAX. 

 We don't use fabs to avoid linkage with -lm.
*****/
static void checkFloatConversion(double v, char *option, char *arg)
{
   char *err = NULL;

   if ((errno == ERANGE && v != 0.0)    /* even double overflowed */
       ||(v < HUGE_VAL && v > -HUGE_VAL && (v < 0.0 ? -v : v) > (double) FLT_MAX)) {
      err = "large";
   } else if ((errno == ERANGE && v == 0.0)
              || (v != 0.0 && (v < 0.0 ? -v : v) < (double) FLT_MIN)) {
      err = "small";
   }
   if (err) {
      fprintf(stderr,
              "%s: parameter `%s' of option `%s' to %s to represent\n",
              Program, arg, option, err);
      exit(EXIT_FAILURE);
   }
}

int getIntOpt(int argc, char **argv, int i, int *value, int force)
{
   char *end;
   long v;

   if (++i >= argc)
      goto nothingFound;

   errno = 0;
   v = strtol(argv[i], &end, 0);

  /***** check for conversion error */
   if (end == argv[i])
      goto nothingFound;

  /***** check for surplus non-whitespace */
   while (isspace((int) *end))
      end += 1;
   if (*end)
      goto nothingFound;

  /***** check if it fits into an int */
   if (errno == ERANGE || v > (long) INT_MAX || v < (long) INT_MIN) {
      fprintf(stderr,
              "%s: parameter `%s' of option `%s' to large to represent\n",
              Program, argv[i], argv[i - 1]);
      exit(EXIT_FAILURE);
   }
   *value = (int) v;

   return i;

 nothingFound:
   if (!force)
      return i - 1;

   fprintf(stderr,
           "%s: missing or malformed integer value after option `%s'\n",
           Program, argv[i - 1]);
   exit(EXIT_FAILURE);
}

/**********************************************************************/

int getIntOpts(int argc, char **argv, int i, int **values, int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
   int alloced, used;
   char *end;
   long v;
   if (i + cmin >= argc) {
      fprintf(stderr,
              "%s: option `%s' wants at least %d parameters\n",
              Program, argv[i], cmin);
      exit(EXIT_FAILURE);
   }

  /***** 
    alloc a bit more than cmin values. It does not hurt to have room
    for a bit more values than cmax.
  *****/
   alloced = cmin + 4;
   *values = (int *) calloc((size_t) alloced, sizeof(int));
   if (!*values) {
    outMem:
      fprintf(stderr,
              "%s: out of memory while parsing option `%s'\n", Program, argv[i]);
      exit(EXIT_FAILURE);
   }

   for (used = 0; (cmax == -1 || used < cmax) && used + i + 1 < argc; used++) {
      if (used == alloced) {
         alloced += 8;
         *values = (int *) realloc(*values, alloced * sizeof(int));
         if (!*values)
            goto outMem;
      }

      errno = 0;
      v = strtol(argv[used + i + 1], &end, 0);

    /***** check for conversion error */
      if (end == argv[used + i + 1])
         break;

    /***** check for surplus non-whitespace */
      while (isspace((int) *end))
         end += 1;
      if (*end)
         break;

    /***** check for overflow */
      if (errno == ERANGE || v > (long) INT_MAX || v < (long) INT_MIN) {
         fprintf(stderr,
                 "%s: parameter `%s' of option `%s' to large to represent\n",
                 Program, argv[i + used + 1], argv[i]);
         exit(EXIT_FAILURE);
      }

      (*values)[used] = (int) v;

   }

   if (used < cmin) {
      fprintf(stderr,
              "%s: parameter `%s' of `%s' should be an "
              "integer value\n", Program, argv[i + used + 1], argv[i]);
      exit(EXIT_FAILURE);
   }

   return i + used;
}

/**********************************************************************/

int getLongOpt(int argc, char **argv, int i, long *value, int force)
{
   char *end;

   if (++i >= argc)
      goto nothingFound;

   errno = 0;
   *value = strtol(argv[i], &end, 0);

  /***** check for conversion error */
   if (end == argv[i])
      goto nothingFound;

  /***** check for surplus non-whitespace */
   while (isspace((int) *end))
      end += 1;
   if (*end)
      goto nothingFound;

  /***** check for overflow */
   if (errno == ERANGE) {
      fprintf(stderr,
              "%s: parameter `%s' of option `%s' to large to represent\n",
              Program, argv[i], argv[i - 1]);
      exit(EXIT_FAILURE);
   }
   return i;

 nothingFound:
  /***** !force means: this parameter may be missing.*/
   if (!force)
      return i - 1;

   fprintf(stderr,
           "%s: missing or malformed value after option `%s'\n",
           Program, argv[i - 1]);
   exit(EXIT_FAILURE);
}

/**********************************************************************/

int getLongOpts(int argc, char **argv, int i, long **values, int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
   int alloced, used;
   char *end;

   if (i + cmin >= argc) {
      fprintf(stderr,
              "%s: option `%s' wants at least %d parameters\n",
              Program, argv[i], cmin);
      exit(EXIT_FAILURE);
   }

  /***** 
    alloc a bit more than cmin values. It does not hurt to have room
    for a bit more values than cmax.
  *****/
   alloced = cmin + 4;
   *values = calloc((size_t) alloced, sizeof(long));
   if (!*values) {
    outMem:
      fprintf(stderr,
              "%s: out of memory while parsing option `%s'\n", Program, argv[i]);
      exit(EXIT_FAILURE);
   }

   for (used = 0; (cmax == -1 || used < cmax) && used + i + 1 < argc; used++) {
      if (used == alloced) {
         alloced += 8;
         *values = realloc(*values, alloced * sizeof(long));
         if (!*values)
            goto outMem;
      }

      errno = 0;
      (*values)[used] = strtol(argv[used + i + 1], &end, 0);

    /***** check for conversion error */
      if (end == argv[used + i + 1])
         break;

    /***** check for surplus non-whitespace */
      while (isspace((int) *end))
         end += 1;
      if (*end)
         break;

    /***** check for overflow */
      if (errno == ERANGE) {
         fprintf(stderr,
                 "%s: parameter `%s' of option `%s' to large to represent\n",
                 Program, argv[i + used + 1], argv[i]);
         exit(EXIT_FAILURE);
      }

   }

   if (used < cmin) {
      fprintf(stderr,
              "%s: parameter `%s' of `%s' should be an "
              "integer value\n", Program, argv[i + used + 1], argv[i]);
      exit(EXIT_FAILURE);
   }

   return i + used;
}

/**********************************************************************/

int getFloatOpt(int argc, char **argv, int i, float *value, int force)
{
   char *end;
   double v;

   if (++i >= argc)
      goto nothingFound;

   errno = 0;
   v = strtod(argv[i], &end);

  /***** check for conversion error */
   if (end == argv[i])
      goto nothingFound;

  /***** check for surplus non-whitespace */
   while (isspace((int) *end))
      end += 1;
   if (*end)
      goto nothingFound;

  /***** check for overflow */
   checkFloatConversion(v, argv[i - 1], argv[i]);

   *value = (float) v;

   return i;

 nothingFound:
   if (!force)
      return i - 1;

   fprintf(stderr,
           "%s: missing or malformed float value after option `%s'\n",
           Program, argv[i - 1]);
   exit(EXIT_FAILURE);

}

/**********************************************************************/

int getFloatOpts(int argc, char **argv, int i, float **values, int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
   int alloced, used;
   char *end;
   double v;

   if (i + cmin >= argc) {
      fprintf(stderr,
              "%s: option `%s' wants at least %d parameters\n",
              Program, argv[i], cmin);
      exit(EXIT_FAILURE);
   }

  /***** 
    alloc a bit more than cmin values.
  *****/
   alloced = cmin + 4;
   *values = (float *) calloc((size_t) alloced, sizeof(float));
   if (!*values) {
    outMem:
      fprintf(stderr,
              "%s: out of memory while parsing option `%s'\n", Program, argv[i]);
      exit(EXIT_FAILURE);
   }

   for (used = 0; (cmax == -1 || used < cmax) && used + i + 1 < argc; used++) {
      if (used == alloced) {
         alloced += 8;
         *values = (float *) realloc(*values, alloced * sizeof(float));
         if (!*values)
            goto outMem;
      }

      errno = 0;
      v = strtod(argv[used + i + 1], &end);

    /***** check for conversion error */
      if (end == argv[used + i + 1])
         break;

    /***** check for surplus non-whitespace */
      while (isspace((int) *end))
         end += 1;
      if (*end)
         break;

    /***** check for overflow */
      checkFloatConversion(v, argv[i], argv[i + used + 1]);

      (*values)[used] = (float) v;
   }

   if (used < cmin) {
      fprintf(stderr,
              "%s: parameter `%s' of `%s' should be a "
              "floating-point value\n", Program, argv[i + used + 1], argv[i]);
      exit(EXIT_FAILURE);
   }

   return i + used;
}

/**********************************************************************/

int getDoubleOpt(int argc, char **argv, int i, double *value, int force)
{
   char *end;

   if (++i >= argc)
      goto nothingFound;

   errno = 0;
   *value = strtod(argv[i], &end);

  /***** check for conversion error */
   if (end == argv[i])
      goto nothingFound;

  /***** check for surplus non-whitespace */
   while (isspace((int) *end))
      end += 1;
   if (*end)
      goto nothingFound;

  /***** check for overflow */
   if (errno == ERANGE) {
      fprintf(stderr,
              "%s: parameter `%s' of option `%s' to %s to represent\n",
              Program, argv[i], argv[i - 1], (*value == 0.0 ? "small" : "large"));
      exit(EXIT_FAILURE);
   }

   return i;

 nothingFound:
   if (!force)
      return i - 1;

   fprintf(stderr,
           "%s: missing or malformed value after option `%s'\n",
           Program, argv[i - 1]);
   exit(EXIT_FAILURE);

}

/**********************************************************************/

int getDoubleOpts(int argc, char **argv, int i, double **values, int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
   int alloced, used;
   char *end;

   if (i + cmin >= argc) {
      fprintf(stderr,
              "%s: option `%s' wants at least %d parameters\n",
              Program, argv[i], cmin);
      exit(EXIT_FAILURE);
   }

  /***** 
    alloc a bit more than cmin values.
  *****/
   alloced = cmin + 4;
   *values = (double *) calloc((size_t) alloced, sizeof(double));
   if (!*values) {
    outMem:
      fprintf(stderr,
              "%s: out of memory while parsing option `%s'\n", Program, argv[i]);
      exit(EXIT_FAILURE);
   }

   for (used = 0; (cmax == -1 || used < cmax) && used + i + 1 < argc; used++) {
      if (used == alloced) {
         alloced += 8;
         *values = (double *) realloc(*values, alloced * sizeof(double));
         if (!*values)
            goto outMem;
      }

      errno = 0;
      (*values)[used] = strtod(argv[used + i + 1], &end);

    /***** check for conversion error */
      if (end == argv[used + i + 1])
         break;

    /***** check for surplus non-whitespace */
      while (isspace((int) *end))
         end += 1;
      if (*end)
         break;

    /***** check for overflow */
      if (errno == ERANGE) {
         fprintf(stderr,
                 "%s: parameter `%s' of option `%s' to %s to represent\n",
                 Program, argv[i + used + 1], argv[i],
                 ((*values)[used] == 0.0 ? "small" : "large"));
         exit(EXIT_FAILURE);
      }

   }

   if (used < cmin) {
      fprintf(stderr,
              "%s: parameter `%s' of `%s' should be a "
              "double value\n", Program, argv[i + used + 1], argv[i]);
      exit(EXIT_FAILURE);
   }

   return i + used;
}

/**********************************************************************/

/**
  force will be set if we need at least one argument for the option.
*****/
int getStringOpt(int argc, char **argv, int i, char **value, int force)
{
   i += 1;
   if (i >= argc) {
      if (force) {
         fprintf(stderr, "%s: missing string after option `%s'\n",
                 Program, argv[i - 1]);
         exit(EXIT_FAILURE);
      }
      return i - 1;
   }

   if (!force && argv[i][0] == '-')
      return i - 1;
   *value = argv[i];
   return i;
}

/**********************************************************************/

int getStringOpts(int argc, char **argv, int i, char ***values, int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
   int alloced, used;

   if (i + cmin >= argc) {
      fprintf(stderr,
              "%s: option `%s' wants at least %d parameters\n",
              Program, argv[i], cmin);
      exit(EXIT_FAILURE);
   }

   alloced = cmin + 4;

   *values = (char **) calloc((size_t) alloced, sizeof(char *));
   if (!*values) {
    outMem:
      fprintf(stderr,
              "%s: out of memory during parsing of option `%s'\n", Program, argv[i]);
      exit(EXIT_FAILURE);
   }

   for (used = 0; (cmax == -1 || used < cmax) && used + i + 1 < argc; used++) {
      if (used == alloced) {
         alloced += 8;
         *values = (char **) realloc(*values, alloced * sizeof(char *));
         if (!*values)
            goto outMem;
      }

      if (used >= cmin && argv[used + i + 1][0] == '-')
         break;
      (*values)[used] = argv[used + i + 1];
   }

   if (used < cmin) {
      fprintf(stderr,
              "%s: less than %d parameters for option `%s', only %d found\n",
              Program, cmin, argv[i], used);
      exit(EXIT_FAILURE);
   }

   return i + used;
}

/**********************************************************************/

void checkIntLower(char *opt, int *values, int count, int max)
{
   int i;

   for (i = 0; i < count; i++) {
      if (values[i] <= max)
         continue;
      fprintf(stderr,
              "%s: parameter %d of option `%s' greater than max=%d\n",
              Program, i + 1, opt, max);
      exit(EXIT_FAILURE);
   }
}

/**********************************************************************/

void checkIntHigher(char *opt, int *values, int count, int min)
{
   int i;

   for (i = 0; i < count; i++) {
      if (values[i] >= min)
         continue;
      fprintf(stderr,
              "%s: parameter %d of option `%s' smaller than min=%d\n",
              Program, i + 1, opt, min);
      exit(EXIT_FAILURE);
   }
}

/**********************************************************************/

void checkLongLower(char *opt, long *values, int count, long max)
{
   int i;

   for (i = 0; i < count; i++) {
      if (values[i] <= max)
         continue;
      fprintf(stderr,
              "%s: parameter %d of option `%s' greater than max=%ld\n",
              Program, i + 1, opt, max);
      exit(EXIT_FAILURE);
   }
}

/**********************************************************************/

void checkLongHigher(char *opt, long *values, int count, long min)
{
   int i;

   for (i = 0; i < count; i++) {
      if (values[i] >= min)
         continue;
      fprintf(stderr,
              "%s: parameter %d of option `%s' smaller than min=%ld\n",
              Program, i + 1, opt, min);
      exit(EXIT_FAILURE);
   }
}

/**********************************************************************/

void checkFloatLower(char *opt, float *values, int count, float max)
{
   int i;

   for (i = 0; i < count; i++) {
      if (values[i] <= max)
         continue;
      fprintf(stderr,
              "%s: parameter %d of option `%s' greater than max=%f\n",
              Program, i + 1, opt, max);
      exit(EXIT_FAILURE);
   }
}

/**********************************************************************/

void checkFloatHigher(char *opt, float *values, int count, float min)
{
   int i;

   for (i = 0; i < count; i++) {
      if (values[i] >= min)
         continue;
      fprintf(stderr,
              "%s: parameter %d of option `%s' smaller than min=%f\n",
              Program, i + 1, opt, min);
      exit(EXIT_FAILURE);
   }
}

/**********************************************************************/

void checkDoubleLower(char *opt, double *values, int count, double max)
{
   int i;

   for (i = 0; i < count; i++) {
      if (values[i] <= max)
         continue;
      fprintf(stderr,
              "%s: parameter %d of option `%s' greater than max=%f\n",
              Program, i + 1, opt, max);
      exit(EXIT_FAILURE);
   }
}

/**********************************************************************/

void checkDoubleHigher(char *opt, double *values, int count, double min)
{
   int i;

   for (i = 0; i < count; i++) {
      if (values[i] >= min)
         continue;
      fprintf(stderr,
              "%s: parameter %d of option `%s' smaller than min=%f\n",
              Program, i + 1, opt, min);
      exit(EXIT_FAILURE);
   }
}

/**********************************************************************/

static char *catArgv(int argc, char **argv)
{
   int i;
   size_t l;
   char *s, *t;

   for (i = 0, l = 0; i < argc; i++)
      l += (1 + strlen(argv[i]));
   s = (char *) malloc(l);
   if (!s) {
      fprintf(stderr, "%s: out of memory\n", Program);
      exit(EXIT_FAILURE);
   }
   strcpy(s, argv[0]);
   t = s;
   for (i = 1; i < argc; i++) {
      t = t + strlen(t);
      *t++ = ' ';
      strcpy(t, argv[i]);
   }
   return s;
}

/**********************************************************************/

void showOptionValues(void)
{
   int i;

   printf("Full command line is:\n`%s'\n", cmd.full_cmd_line);

  /***** -fwd: Force an forward FFT (sign=-1) to be performed */
   if (!cmd.forwardP) {
      printf("-fwd not found.\n");
   } else {
      printf("-fwd found:\n");
   }

  /***** -inv: Force an inverse FFT (sign=+1) to be performed */
   if (!cmd.inverseP) {
      printf("-inv not found.\n");
   } else {
      printf("-inv found:\n");
   }

  /***** -del: Delete the original file(s) when performing the FFT */
   if (!cmd.deleteP) {
      printf("-del not found.\n");
   } else {
      printf("-del found:\n");
   }

  /***** -disk: Force the use of the out-of-core memory FFT */
   if (!cmd.diskfftP) {
      printf("-disk not found.\n");
   } else {
      printf("-disk found:\n");
   }

  /***** -mem: Force the use of the in-core memory FFT */
   if (!cmd.memfftP) {
      printf("-mem not found.\n");
   } else {
      printf("-mem found:\n");
   }

  /***** -tmpdir: Scratch directory for temp file(s) in out-of-core FFT */
   if (!cmd.tmpdirP) {
      printf("-tmpdir not found.\n");
   } else {
      printf("-tmpdir found:\n");
      if (!cmd.tmpdirC) {
         printf("  no values\n");
      } else {
         printf("  value = `%s'\n", cmd.tmpdir);
      }
   }

  /***** -outdir: Directory where result file(s) will reside */
   if (!cmd.outdirP) {
      printf("-outdir not found.\n");
   } else {
      printf("-outdir found:\n");
      if (!cmd.outdirC) {
         printf("  no values\n");
      } else {
         printf("  value = `%s'\n", cmd.outdir);
      }
   }
   if (!cmd.argc) {
      printf("no remaining parameters in argv\n");
   } else {
      printf("argv =");
      for (i = 0; i < cmd.argc; i++) {
         printf(" `%s'", cmd.argv[i]);
      }
      printf("\n");
   }
}

/**********************************************************************/

void usage(void)
{
   fprintf(stderr, "usage: %s%s", Program, "\
 [-fwd] [-inv] [-del] [-disk] [-mem] [-tmpdir tmpdir] [-outdir outdir] [--] infiles ...\n\
    Perform a single-precision FFT of real data or its inverse\n\
     -fwd: Force an forward FFT (sign=-1) to be performed\n\
     -inv: Force an inverse FFT (sign=+1) to be performed\n\
     -del: Delete the original file(s) when performing the FFT\n\
    -disk: Force the use of the out-of-core memory FFT\n\
     -mem: Force the use of the in-core memory FFT\n\
  -tmpdir: Scratch directory for temp file(s) in out-of-core FFT\n\
           1 char* value\n\
  -outdir: Directory where result file(s) will reside\n\
           1 char* value\n\
  infiles: Input data file(s)\n\
           1...16 values\n\
version: 23Apr04\n\
");
   exit(EXIT_FAILURE);
}

/**********************************************************************/
Cmdline *parseCmdline(int argc, char **argv)
{
   int i;

   Program = argv[0];
   cmd.full_cmd_line = catArgv(argc, argv);
   for (i = 1, cmd.argc = 1; i < argc; i++) {
      if (0 == strcmp("--", argv[i])) {
         while (++i < argc)
            argv[cmd.argc++] = argv[i];
         continue;
      }

      if (0 == strcmp("-fwd", argv[i])) {
         cmd.forwardP = 1;
         continue;
      }

      if (0 == strcmp("-inv", argv[i])) {
         cmd.inverseP = 1;
         continue;
      }

      if (0 == strcmp("-del", argv[i])) {
         cmd.deleteP = 1;
         continue;
      }

      if (0 == strcmp("-disk", argv[i])) {
         cmd.diskfftP = 1;
         continue;
      }

      if (0 == strcmp("-mem", argv[i])) {
         cmd.memfftP = 1;
         continue;
      }

      if (0 == strcmp("-tmpdir", argv[i])) {
         int keep = i;
         cmd.tmpdirP = 1;
         i = getStringOpt(argc, argv, i, &cmd.tmpdir, 1);
         cmd.tmpdirC = i - keep;
         continue;
      }

      if (0 == strcmp("-outdir", argv[i])) {
         int keep = i;
         cmd.outdirP = 1;
         i = getStringOpt(argc, argv, i, &cmd.outdir, 1);
         cmd.outdirC = i - keep;
         continue;
      }

      if (argv[i][0] == '-') {
         fprintf(stderr, "\n%s: unknown option `%s'\n\n", Program, argv[i]);
         usage();
      }
      argv[cmd.argc++] = argv[i];
   }                            /* for i */


   /*@-mustfree */
   cmd.argv = argv + 1;
   /*@=mustfree */
   cmd.argc -= 1;

   if (1 > cmd.argc) {
      fprintf(stderr, "%s: there should be at least 1 non-option argument(s)\n",
              Program);
      exit(EXIT_FAILURE);
   }
   if (16 < cmd.argc) {
      fprintf(stderr, "%s: there should be at most 16 non-option argument(s)\n",
              Program);
      exit(EXIT_FAILURE);
   }
   /*@-compmempass */
   return &cmd;
}
