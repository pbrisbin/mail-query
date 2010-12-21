#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <regex.h>
#include <stdlib.h>

#define REGEX_OPTS            REG_ICASE|REG_EXTENDED|REG_NOSUB|REG_NEWLINE

static regex_t regex;
static char **entries = NULL;
static size_t count = 0;

/* parse the name and email from a string */
void add_entries(char *str) {
  char name[51];
  char email[51];
  char *entry;

  int rc = sscanf(str, "%50[^<]<%50[^>]", name, email);

  /* good parse, and search string found in email or name */
  if (rc != 2) {
    return;
  }

  /* if (strcasestr(email, search_string) || strcasestr(name, search_string)) */
  if (regexec(&regex, name, 0, 0, 0) == REG_NOERROR ||
      regexec(&regex, email, 0, 0, 0) == REG_NOERROR) {
    asprintf(&entry, "%s\t%s", email, name);

    /* allocate memory for the new entries */
    if (!(entries = realloc(entries, ++count * sizeof *entries))) {
      fprintf(stderr, "malloc failed");
      return;
    }

    /* add entries to the array */
    if (!(entries[count-1] = strdup(entry))) {
      fprintf(stderr, "strdup failed");
      return;
    }

    free(entry);
  }

  free(str);
}

void find_from(char *fn) {
  FILE *file;
  char line[1000];

  file = fopen(fn, "r");
  if (!file) {
    fprintf(stderr, "%s: failed to open file.\n", fn);
    return;
  }

  while ((fgets(line, 1000, file) != NULL)) {
    if (strncmp("From: ", line, 6) == 0) {
      /* trim leading "From: " and trailing "\n" */
      add_entries(strndup(line + 6, strlen(line)-1));
      break;
    }
  }

  fclose(file);
}

/* read a directory's contents and operate on each file */
int read_from_dir(char *path) {
  DIR *dir;
  struct dirent *dit;

  char *fullpath, *filename;

  if ((dir = opendir(path)) == NULL) {
    fprintf(stderr, "%s: unable to open directory.\n", dir);
    return(1);
  }

  /* main loop */
  while ((dit = readdir(dir)) != NULL) {
    filename = dit->d_name;

    /* skip . and .. */
    if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0) {
      asprintf(&fullpath, "%s/%s", path, filename);
      find_from(fullpath);
      free(fullpath);
    }
  }

  if (closedir(dir) == -1) {
    fprintf(stderr, "%s: unable to close directory.\n", dir);
    return(1);
  }
}

/* sorting/uniquing code bogarted from dmenu_path */
int qstrcmp(const void *a, const void *b) {
  return(strcmp(*(const char **)a, *(const char **)b));
}

void sort_and_print_entries(void) {
  size_t i;

  qsort(entries, count, sizeof *entries, qstrcmp);

  for (i = 0; i < count; i++) {
    if (i > 0 && !strcmp(entries[i], entries[i-1]))
      continue;

    printf("%s\n", entries[i]);
  }
}

int main(int argc, char *argv[]) {
  char *path;
  int i;

  /* invalid usage */
  if (argc < 3) {
    printf("usage: %s <search string> </path/to/mdir> ...\n", argv[0]);
    return(1);
  }

  if (regcomp(&regex, argv[1], REGEX_OPTS) != 0) {
    fprintf(stderr, "failed to compile regex: %s\n", argv[1]);
    return(1);
  }

  for (i = 2; i < argc; i++) {
    /* check the new subdir */
    asprintf(&path, "%s/%s", argv[i], "new");

    if (read_from_dir(path) == 1) {
      fprintf(stderr, "%s: unable to read from path.\n", path);
    }

    free(path);

     /* check the cur subdir */
    asprintf(&path, "%s/%s", argv[i], "cur");

    if (read_from_dir(path) == 1) {
      fprintf(stderr, "%s: unable to read from path.\n", path);
    }

    free(path);
  }

  regfree(&regex);

  /* output */
  printf("\n");
  sort_and_print_entries();
  return(0);
}
