#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>

/* email parsing from
 * http://bytes.com/topic/c/answers/215251-regular-expressions-c
 */
void print_email(char str[])
{
  char name[51];
  char email[51];

  int rc = sscanf(str, "%50[^<]<%50[^>]", name, email);

  if (rc == 2)
  {
    printf("%s\t%s\n", email, name);
  }

  free(str);
}
 
/* find the From: line in a file */
void find_from(char *fn)
{
  FILE *file;
  char line[1000];
  bool found = false;

  /* a hack for now */
  int i     = 1;
  int limit = 100; /* look for From: in the first n lines */

  if (file = fopen(fn, "r"))
  {
    while (!found && i <= limit)
    {
      i++;

      fgets(line, 1000, file);
      if (strncmp("From: ", line, 6) == 0)
      {
        /* trim leading "From: " and trailing "\n" */
        print_email(strndup(line + 6, strlen(line)-1));
        found = true;
      }
    }

    fclose(file);
  }
  else
  {
    fprintf(stderr, "%s: failed to open file.\n", fn);
  }
}

int read_from_dir(char *path)
{
  DIR *dir;
  struct dirent *dit;

  char *fullpath;
  char *filename;

  if ((dir = opendir(path)) == NULL)
  {
    fprintf(stderr, "%s: unable to open directory.\n", dir);
    return 1;
  }

  /* main loop */
  while ((dit = readdir(dir)) != NULL)
  {
    filename = dit->d_name;

    /* skip . and .. */
    if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0)
    {
      asprintf(&fullpath, "%s/%s", path, filename);
      find_from(fullpath);
      free(fullpath);
    }
  }

  if (closedir(dir) == -1)
  {
    fprintf(stderr, "%s: unable to close directory.\n", dir);
    return 1;
  }
}
 
/* directory code from
 * http://www.metalshell.com/source_code/116/Read_Directory.html
 */
int main(int argc, char *argv[])
{
  char *path;
  int i;

  /* invalid usage */
  if (argc < 2)
  {
    printf("usage: %s </path/to/mdir> ...\n", argv[0]);
    return 1;
  }

  for (i = 1; i < argc; i++)
  {
    /* check the new subdir */
    asprintf(&path, "%s/%s", argv[i], "new");
    
    if (read_from_dir(path) == 1)
    {
      fprintf(stderr, "%s: unable to read from path.\n", path);
    }

    free(path);

     /* check the cur subdir */
    asprintf(&path, "%s/%s", argv[i], "cur");
    
    if (read_from_dir(path) == 1)
    {
      fprintf(stderr, "%s: unable to read from path.\n", path);
    }

    free(path);
  }
}
