//by Jagger  [jagger@n0123.xyz]

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <tar.h>
#include <stdint.h>
#include <unistd.h>
#include <zlib.h>


#define BUFSIZE 4096

//ZIP/////////////////////////////////////////////////////////////////////////////

int is_zip_archive(const char* filename) {
  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    return 0;
  }

  unsigned char buffer[4];
  if (fread(buffer, 1, 4, fp) != 4) {
    fclose(fp);
    return 0;
  }

  unsigned int magic_number = *((unsigned int*) buffer);
  int result = magic_number == 0x04034b50;

  fclose(fp);
  return result;
}



//ende zip
//////////////////////////////////////////////////////////////////////////////////


//TAR////////////////////////////////////////////////////////////////////////////
int parseoct(const char *p, size_t n){
        int i = 0;
        while (*p < '0' || *p > '7') {
                ++p;
                --n;
        }
        while (*p >= '0' && *p <= '7' && n > 0) {
                i *= 8;
                i += *p - '0';
                ++p;
                --n;
        }
        return (i);
}

int is_end_of_archive(const char *p){
        int n;
        for (n = 511; n >= 0; --n)
                if (p[n] != '\0')
                        return (0);
        return (1);
}

void create_dir(char *pathname, int mode){
        char *p;
        int r;
        if (pathname[strlen(pathname) - 1] == '/')
                pathname[strlen(pathname) - 1] = '\0';
        r = mkdir(pathname, mode);
        if (r != 0) {
                p = strrchr(pathname, '/');
                if (p != NULL) {
                        *p = '\0';
                        create_dir(pathname, 0755);
                        *p = '/';
                        r = mkdir(pathname, mode);
                }
        }
        if (r != 0)
                fprintf(stderr, "Could not create directory %s\n", pathname);
}

FILE * create_file(char *pathname, int mode){
        FILE *f;
        f = fopen(pathname, "w+");
        if (f == NULL) {
                char *p = strrchr(pathname, '/');
                if (p != NULL) {
                        *p = '\0';
                        create_dir(pathname, 0755);
                        *p = '/';
                        f = fopen(pathname, "w+");
                }
        }
        return (f);
}

int verify_checksum(const char *p){
        int n, u = 0;
        for (n = 0; n < 512; ++n) {
                if (n < 148 || n > 155)
                        u += ((unsigned char *)p)[n];
                else
                        u += 0x20;

        }
        return (u == parseoct(p + 148, 8));
}

int is_tar(char* archive_name) {
  FILE* archive_file = fopen(archive_name, "rb");
  char buffer[1024];
  fread(buffer, 1, 1024, archive_file);
  (     strncmp(buffer, "ustar", 5) == 0 
         || (buffer[257] == 'u' 
              && buffer[258] == 's' 
              && buffer[259] == 't' 
              && buffer[260] == 'a' 
              && buffer[261] == 'r')) ? 1 : 0;
}

void untar(FILE *a, const char *path){
        char buff[512];
        FILE *f = NULL;
        size_t bytes_read;
        int filesize;

        printf("Extracting from %s\n", path);
        for (;;) {
                bytes_read = fread(buff, 1, 512, a);
                if (bytes_read < 512) {
                        fprintf(stderr,
                            "Short read on %s: expected 512, got %d\n",
                            path, bytes_read);
                        return;
                }
                if (is_end_of_archive(buff)) {
                        printf("End of %s\n", path);
                        return;
                }
                if (!verify_checksum(buff)) {
                        fprintf(stderr, "Checksum failure\n");
                        return;
                }
                filesize = parseoct(buff + 124, 12);
                switch (buff[156]) {
                case '1':
                        printf(" Ignoring hardlink %s\n", buff);
                        break;
                case '2':
                        printf(" Ignoring symlink %s\n", buff);
                        break;
                case '3':
                        printf(" Ignoring character device %s\n", buff);
                                break;
                case '4':
                        printf(" Ignoring block device %s\n", buff);
                        break;
                case '5':
                        printf(" Extracting dir %s\n", buff);
                        create_dir(buff, parseoct(buff + 100, 8));
                        filesize = 0;
                        break;
                case '6':
                        printf(" Ignoring FIFO %s\n", buff);
                        break;
                default:
                        printf(" Extracting file %s\n", buff);
                        f = create_file(buff, parseoct(buff + 100, 8));
                        break;
                }
                while (filesize > 0) {
                        bytes_read = fread(buff, 1, 512, a);
                        if (bytes_read < 512) {
                                fprintf(stderr,
                                    "Short read on %s: Expected 512, got %d\n",
                                    path, bytes_read);
                                return;
                        }
                        if (filesize < 512)
                                bytes_read = filesize;
                        if (f != NULL) {
                                if (fwrite(buff, 1, bytes_read, f)
                                    != bytes_read)
                                {
                                        fprintf(stderr, "Failed write\n");
                                        fclose(f);
                                        f = NULL;
                                }
                        }
                        filesize -= bytes_read;
                }
                if (f != NULL) {
                        fclose(f);
                        f = NULL;
                }
        }
}
//ende tar
/////////////////////////////////



/* 
 * This software shall extract every archive
 * help by expanding it.
 * 
 * compilecommand:
 * gcc extract.c -o extract -lz -lzip
 *
 *
 * usage:
 * ./extract archive 
 *
 *
 * */


//main/////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Error: Please provide an archive name as an argument.\n");
    return 1;
  }

  FILE* archive = fopen(argv[1], "rb");
  if (archive == NULL) {
    printf("Error: Unable to open archive file.\n");
    return 1;
  }

  char buffer[1024];
  fread(buffer, 1, 1024, archive);

  if (is_zip_archive(argv[1]))  {       //zip
        printf("extracting zip");







  } else if (is_tar(argv[1])) {         //tar
        printf("extracting tar");
        untar(archive, *argv);
        unlink("@LongLink");
  } else {
    printf("Error: Unsupported archive type.\n");
    printf("Help by extending extcats capabilitys: https://github.com .\n");
    return 1;
  }
  fclose(archive);
  return 0;
}

