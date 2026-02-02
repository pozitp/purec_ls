#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>

struct passwd {
    char *pw_name;
    uid_t pw_uid;
    gid_t pw_gid;
    char *pw_dir;
    char *pw_shell;
};

char* strchr(const char* str, int ch) {
    while(*str != (char)ch) {
        if (!*str++) return 0;
    }
    return (char*)str;
}

int atoi(const char *nptr) {
    int res = 0, sign = 1;
    if (*nptr == '-') {
        sign = -1; 
        nptr++;
    }
    while (*nptr >= '0' && *nptr <= '9') res = res * 10 + (*nptr++ - '0');
    return res * sign;
}

char* strcpy(char* dest, const char* src) {
    char* temp = dest;
    while((*dest++ = *src++));
    return temp;
}

char* strcat(char* dest, const char* src) {
    char* temp = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++));
    return temp;
}

size_t strlen(const char *str) {
    const char *p = str;
    while (*p) p++;
    return p - str;
}

struct passwd *getpwuid(uid_t uid) {
    static struct passwd result;
    static char buffer[128*1024];
    int fd = open("/etc/passwd", O_RDONLY);
    
    read(fd, buffer, sizeof(buffer));
    close(fd);

    char *line_start = buffer;
    while (*line_start) {
        char *line_end = strchr(line_start, '\n');
        if (line_end) *line_end = '\0';
        
        char *p = line_start;
        char *fields[7];
        int i = 0;

        fields[i++] = p;
        while ((p = strchr(p, ':')) && i < 7) {
            *p = '\0';
            fields[i++] = ++p;
        }

        
        if (i >= 3 && atoi(fields[2]) == (int)uid) {
            result.pw_name = fields[0];
            result.pw_uid = (uid_t) atoi(fields[2]);
            result.pw_gid = (gid_t) atoi(fields[3]);
            result.pw_dir = (i > 5) ? fields[5] : "";
            result.pw_shell = (i > 6) ? fields[6] : "";
            return &result;
        }
        
        if (!line_end) break;
        line_start = line_end + 1;
    }
    return NULL;
}

void print_mode(mode_t mode) {
    printf("%c", S_ISDIR(mode) ? 'd' : '-');
    printf("%c", (mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (mode & S_IROTH) ? 'r' : '-');
    printf("%c", (mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (mode & S_IXOTH) ? 'x' : '-');
}

void list_directory(char* path) {
    int fd;
    char buf[PATH_MAX];
    struct dirent64 *d;
    struct stat statbuf;
    ssize_t nread;
    
    fd = open(path, O_DIRECTORY);
    while ((nread = getdents64(fd, buf, PATH_MAX)) > 0) {
        for (long bpos = 0; bpos < nread;) {
            d = (struct dirent64*) (buf + bpos);
            char full_path[PATH_MAX];
            strcpy(full_path, path);
            if (path[strlen(path) - 1] != '/')
                strcat(full_path, "/");
            strcat(full_path, d->d_name);
            stat(full_path, &statbuf);
            print_mode(statbuf.st_mode);
            printf(" %-10s %s\n", getpwuid(statbuf.st_uid)->pw_name, d->d_name);
            bpos += d->d_reclen;
        }
    }
    close(fd);
}

int main(int argc, char* argv[]) {
   if (argc > 1) {
       for (int i = 1; i < argc; i++) {
           printf("%s:\n", argv[i]);
           list_directory(argv[i]);
           if (i != argc - 1)
               printf("\n");
       }
   } else {
       list_directory(".");
   }
   return 0; 
}
