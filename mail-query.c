#define _GNU_SOURCE

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
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

static int address_cmp(const void *a1, const void *a2) {
    address_t *address1 = *(address_t**)a1;
    address_t *address2 = *(address_t**)a2;

    return (strcmp(address1->email, address2->email));
}

static char *strtrim(char *str) {
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

static void add_address(address_t *address) {
    if (!(entries = realloc(entries, ++entry_count * sizeof *entries))) {
        fprintf(stderr, "realloc failed");
        exit(EXIT_FAILURE);
    }

    entries[entry_count - 1] = address;
}

static address_t *parse_from(char *line) {
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

static int parse_mailfile(FILE *fp) {
    char line[PATH_MAX];
    address_t *entry;

    while (fgets(line, PATH_MAX, fp) != NULL) {
        if (strncmp(line, "From: ", 6) == 0) {
            break;
        }
    }

    entry = parse_from(&line[6]);
    if (entry) {
        add_address(entry);
    }

    return(0);
}

static void print_entries() {
    int i;
    char *prev_email = NULL;
    address_t **entry;

    if (entry_count == 0) {
        return;
    }

    printf("\n");
    for (i = 0, entry = entries; i < entry_count; i++, entry++) {
        if (prev_email && strcmp(prev_email, (*entry)->email) == 0) {
            continue;
        }

        printf("%s\t%s\n", (*entry)->email, (*entry)->name);
        prev_email = (*entry)->email;
    }
}

static int walk_maildir(const char *path) {
    DIR *dirp;
    FILE *fp;
    struct dirent *dentry;
    char *subdir;
    char filename[PATH_MAX];

    dirp = opendir(path);
    if (!dirp) {
        fprintf(stderr, "opendir: %s: %s\n", strerror(errno), path);
        return(1);
    }

    while ((dentry = readdir(dirp)) != NULL) {
        if (dentry->d_type == DT_DIR) {
            if (strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0) {
                continue;
            }

            asprintf(&subdir, "%s/%s", path, dentry->d_name);
            walk_maildir(subdir);
            free(subdir);
        } else if (dentry->d_type == DT_REG) {
            snprintf(filename, PATH_MAX, "%s/%s", path, dentry->d_name);

            fp = fopen(filename, "r");
            if (!fp) {
                fprintf(stderr, "fopen: %s: %s: ", filename, strerror(errno));
                continue;
            }

            parse_mailfile(fp);
            fclose(fp);
        }
    }

    closedir(dirp);

    return(0);
}

int main(int argc, char *argv[]) {
    int i, ret;
    char errbuf[PATH_MAX];

    if (argc < 3) {
        printf("usage: %s <regex> </path/to/mdir> ...\n", argv[0]);
        return(EXIT_FAILURE);
    }

    ret = regcomp(&regex, argv[1], REGEX_OPTS);
    if (ret != 0) {
        regerror(ret, &regex, errbuf, PATH_MAX);
        fprintf(stderr, "failed to compile regex: %s: %s\n", errbuf, argv[1]);
        return(EXIT_FAILURE);
    }

    ret = regcomp(&emailverifier, EMAIL_VERIFY_REGEX, REGEX_OPTS);
    if (ret != 0) {
        regerror(ret, &regex, errbuf, PATH_MAX);
        fprintf(stderr, "failed to compile regex: %s: %s\n", errbuf, EMAIL_VERIFY_REGEX);
        return(EXIT_FAILURE);
    }

    for (i = 2; i < argc; i++) {
        walk_maildir(argv[i]);
    }

    regfree(&regex);
    regfree(&emailverifier);

    qsort(entries, entry_count, sizeof *entries, address_cmp); 
    print_entries();

    return(EXIT_SUCCESS);
}

