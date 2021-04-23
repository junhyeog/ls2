#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PATH_LEN 1024
#define BUF_SIZE 1024
char dir[PATH_LEN];
char path[PATH_LEN];
char buf[BUF_SIZE];

void usage() {
  perror("using : [-l] directory_name");
  return;
}

void printNum(char *str, int sz) {
  int len = strlen(str);
  int i = 0;
  for (; i < sz - len; i++) printf(" ");
  printf("%s", str);
}

void getMode(struct stat st) {
  /* https://stackoverflow.com/questions/8812959/how-to-read-linux-file-permission-programmatically-in-c-c */
  mode_t st_mode = st.st_mode;
  buf[0] = (st_mode & S_IFDIR) ? 'd' : '-';
  buf[1] = (st_mode & S_IRUSR) ? 'r' : '-';
  buf[2] = (st_mode & S_IWUSR) ? 'w' : '-';
  buf[3] = (st_mode & S_IXUSR) ? 'x' : '-';
  buf[4] = (st_mode & S_IRGRP) ? 'r' : '-';
  buf[5] = (st_mode & S_IWGRP) ? 'w' : '-';
  buf[6] = (st_mode & S_IXGRP) ? 'x' : '-';
  buf[7] = (st_mode & S_IROTH) ? 'r' : '-';
  buf[8] = (st_mode & S_IWOTH) ? 'w' : '-';
  buf[9] = (st_mode & S_IXOTH) ? 'x' : '-';
  buf[10] = '\0';
  return;
}

void getNlink(struct stat st) {
  int len;
  len = snprintf(NULL, 0, "%d", (int)st.st_nlink);
  buf[len] = '\0';
  snprintf(buf, len + 1, "%d", (int)st.st_nlink);
}

void getUid(struct stat st) {
  struct passwd *pw = getpwuid(st.st_uid);
  memcpy(buf, pw->pw_name, strlen(pw->pw_name) + 1);
}

void getGid(struct stat st) {
  struct group *gr = getgrgid(st.st_gid);
  memcpy(buf, gr->gr_name, strlen(gr->gr_name) + 1);
}

void getSize(struct stat st) {
  int len;
  len = snprintf(NULL, 0, "%d", (int)st.st_size);
  buf[len] = '\0';
  snprintf(buf, len + 1, "%d", (int)st.st_size);
}

void getMtime(struct stat st) {
  time_t cur = time(NULL);
  int curYear = localtime(&cur)->tm_year;
  struct tm *t = localtime(&st.st_mtime);
  strftime(buf, BUF_SIZE, t->tm_year == curYear ? "%b %d %R" : "%b %d  %Y", t);
}

void longListing(char *d_name) {
  struct stat st;
  memset(path, 0, sizeof(path));
  strcpy(path, dir);
  strcat(path, "/");
  strcat(path, d_name);

  if (stat(path, &st)) {
    perror("can't open directory_name");
    return;
  }

  getMode(st);
  printNum(buf, 10);
  getNlink(st);
  printNum(buf, 4);
  getUid(st);
  printNum(buf, 10);
  getGid(st);
  printNum(buf, 10);
  getSize(st);
  printNum(buf, 10);
  getMtime(st);
  printNum(buf, 20);
  printf("  %s\n", d_name);
  return;
}

int main(int argc, char *argv[]) {
  DIR *dp;
  struct dirent *dirp;
  int opt, mode;
  mode = 0;
  while ((opt = getopt(argc, argv, "l")) != -1) {
    switch (opt) {
      case 'l':
        mode = 1;
        break;
      default:
        usage();
        exit(EXIT_FAILURE);
    }
  }
  if (argc <= 1) {
    usage();
    exit(EXIT_FAILURE);
  }
  int i;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') continue;
    memset(dir, 0, sizeof(dir));
    memcpy(dir, argv[i], strlen(argv[i]));
    if ((dp = opendir(dir)) == NULL) {
      perror("can't open dir");
      continue;
    }

    if (argc - mode - 1 > 1) printf("%s:\n", dir);

    if (mode) {
      struct stat st;
      blkcnt_t cnt = 0;

      while ((dirp = readdir(dp)) != NULL) {
        memset(path, 0, sizeof(path));
        strcpy(path, dir);
        strcat(path, "/");
        strcat(path, dirp->d_name);
        if (stat(path, &st)) continue;
        cnt += st.st_blocks / 2;
      }

      printf("total %d\n", (int)cnt);
      dp = opendir(dir);
    }

    while ((dirp = readdir(dp)) != NULL) {
      if (!mode)
        printf("%s\n", dirp->d_name);
      else
        longListing(dirp->d_name);
    }
    printf("\n");
    closedir(dp);
  }
  exit(EXIT_SUCCESS);
}
