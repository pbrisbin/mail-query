#define _GNU_SOURCE

#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define REGEX_OPTS            REG_ICASE|REG_EXTENDED|REG_NOSUB|REG_NEWLINE
#define EMAIL_VERIFY_REGEX    "^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}$"

typedef struct __address_t {
  char *name;
  char *email;
} address_t;

static regex_t regex;
static regex_t emailverifier;
static address_t **entries = NULL;
static int entry_count = 0;

static const char *subdirs[] = { "new", "cur", NULL };

static int address_cmp(const void *a1, const void *a2) {
  address_t *address1 = *(address_t**)a1;
  address_t *address2 = *(address_t**)a2;

  return (strcmp(address1->email, address2->email));
}

char *strtrim(char *str) {
  char *pch = str;

  if (!str || *str == '\0') {
    return(str);
  }

  while (isspace((unsigned char)*pch)) {
    pch++;
  }
  if (pch != str) {
    memmove(str, pch, (strlen(pch) + 1));
  }

  if (*str == '\0') {
    return(str);
  }

  pch = (str + (strlen(str) - 1));
  while (isspace((unsigned char)*pch)) {
    pch--;
  }
  *++pch = '\0';

  return(str);
}

void add_address(address_t *address) {
  if (!(entries = realloc(entries, ++entry_count * sizeof *entries))) {
    fprintf(stderr, "realloc failed");
    exit(EXIT_FAILURE);
  }

  entries[entry_count - 1] = address;
}

address_t *parse_from(char *line) {
  char *name, *email, *ptr;
  address_t *address;

  ptr = strchr(line, '<');
  if (!ptr) {
    return(NULL);
  }

  *ptr = '\0';
  name = line;
  email = ptr + 1;
  *(ptr + 1 + strcspn(email, ">")) = '\0';

  strtrim(name);
  strtrim(email);

  if (strlen(name) == 0 || strlen(email) == 0) {
    return(NULL);
  }

  if (regexec(&emailverifier, email, 0, 0, 0) == REG_NOMATCH) {
    return(NULL);
  }

  if (regexec(&regex, name, 0, 0, 0) == REG_NOMATCH &&
      regexec(&regex, email, 0, 0, 0) == REG_NOMATCH) {
    return(NULL);;
  }

  address = malloc(sizeof *address);
  address->name = strdup(name);
  address->email = strdup(email);

  return(address);
}

int parse_mailfile(FILE *fp) {
  char line[PATH_MAX];
  address_t *entry;

  while (fgets(line, PATH_MAX, fp) != NULL) {
    if (strncmp(line, "From: ", 6) != 0) {
      continue;
    }

    entry = parse_from(&line[6]);
    if (entry) {
      add_address(entry);
    }
  }

  return(0);
}

int read_maildir(const char *path, DIR *dirp) {
  struct dirent *dentry;
  char *filename;
  FILE *fp;

  while ((dentry = readdir(dirp)) != NULL) {
    if (dentry->d_type != DT_REG) {
      continue;
    }

    asprintf(&filename, "%s/%s", path, dentry->d_name);

    fp = fopen(filename, "r");
    if (!fp) {
      fprintf(stderr, "%s: ", filename);
      perror("fopen");
      continue;
    }

    free(filename);
    parse_mailfile(fp);
    fclose(fp);
  }

  return(0);
}

void print_entries() {
  int i;
  char *prev_email = NULL;
  address_t **entry;

  printf("\n");
  for (i = 0, entry = entries; i < entry_count; i++, entry++) {
    if (prev_email && strcmp(prev_email, (*entry)->email) == 0) {
      continue;
    }

    printf("%s\t%s\n", (*entry)->email, (*entry)->name);
    prev_email = (*entry)->email;
  }
}

int main(int argc, char *argv[]) {
  char *path;
  const char **subdir;
  DIR *dirp;
  int i;

  if (argc < 3) {
    printf("usage: %s <regex> </path/to/mdir> ...\n", argv[0]);
    return(EXIT_FAILURE);
  }

  if (regcomp(&regex, argv[1], REGEX_OPTS) != 0) {
    fprintf(stderr, "failed to compile regex: %s\n", argv[1]);
    return(1);
  }

  if (regcomp(&emailverifier, EMAIL_VERIFY_REGEX, REGEX_OPTS) != 0) {
    fprintf(stderr, "failed to compile regex: %s\n", EMAIL_VERIFY_REGEX);
    return(1);
  }

  for (i = 2; i < argc; i++) {
    for (subdir = subdirs; *subdir; subdir++) {
      asprintf(&path, "%s/%s", argv[i], *subdir);
      dirp = opendir(path);
      if (!dirp) {
        fprintf(stderr, "error: unable to read from path: %s: ", path);
        perror("");
        continue;
      }
      read_maildir(path, dirp);
      free(path);
      closedir(dirp);
    }
  }

  regfree(&regex);
  regfree(&emailverifier);

  qsort(entries, entry_count, sizeof *entries, address_cmp); 
  print_entries();

  return(EXIT_SUCCESS);
}

