#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

typedef unsigned char byte;

int line_len(byte *line) {
  int len = 0;
  while (line[len++])
    ;
  ;
  return len;
}

void cleanStr(byte *str) {
  byte *temp = str;
  int i = 0;
  while (temp[i]) {
    if (temp[i] == '\n') {
      temp[i] = '.';
    }
    i++;
  }
}

void hex_dump(FILE *file) {
  int n = 0, size = 16, ch;
  long offset;
  long manual_offset = 0;
  byte *line;
  line = malloc(size + 1);

  offset = ftell(file);
  printf("%08lx: ", offset != -1 ? offset : manual_offset);

  while ((ch = getc(file)) != EOF) {
    // print the hex
    printf("%02x", ch);
    if (n % 2 != 0) {
      printf(" ");
    }

    line[n++] = ch;

    if (n == size) {
      line[n] = '\0';
      cleanStr(line);
      printf(" %s\n", line);
      n = 0;

      offset = ftell(file);
      printf("%08lx: ", offset != -1 ? offset : (manual_offset += size));
    }
  }

  if (n > 0) {
    line[n] = '\0';

    for (int i = n; i < 16; i++) {
      printf("   ");
    }

    cleanStr(line);
    printf(" %s", line);
  }
}

int hex_to_int(char c) {
  if (c >= 97)
    c = c - 32;
  int first = c / 16 - 3;
  int second = c % 16;
  int result = first * 10 + second;
  if (result > 9)
    result--;
  return result;
}

int hex_to_ascii(char c, char d) {
  int high = hex_to_int(c) * 16;
  int low = hex_to_int(d);
  return high + low;
}

void bin_dump(FILE *file) {
  int ch, n = 0, size = 128;
  byte *line = malloc(size + 1);

  while ((ch = getc(file)) != EOF) {
    if (n == size) {
      size *= 2;
      line = realloc(line, size);
    }
    if (ch == '\n') {
      line[n] = '\0';

      int length = line_len(line);
      int i;
      char buf = 0;
      for (i = 0; i < length; i++) {
        if (i % 2 != 0) {
          printf("%c", hex_to_ascii(buf, line[i]));
        } else {
          buf = line[i];
        }
      }

      n = 0;
      size = 128;
      line = realloc(line, size + 1);
    } else if (ch == ' ') {
      continue;
    } else {
      line[n++] = ch;
    }
  }

  if (n == 0 && ch == EOF) {
    free(line);
  }
}

int main(int argc, char **argv) {
  int opt;
  int reverse = 0;
  char *seek = NULL;
  FILE *input = NULL;

  while ((opt = getopt(argc, argv, "rs:")) != -1) {
    switch (opt) {
    case 'r':
      reverse = 1;
      break;
    case 's':
      seek = optarg;
      break;
    default:
      fprintf(stdin, "Usage: %s [options] [infile [outfile]\n", argv[0]);
      return 1;
    }
  }

  if (optind == argc) {
    input = stdin;
  } else {
    char *file_name = argv[optind];
    if (file_name == NULL) {
      printf("no file name provided\n");
    }

    input = fopen(file_name, "r");
    if (!input) {
      perror("idk bro");
      exit(-1);
    }
  }

  if (seek) {
    int pos = 0;
    int whence = 0;
    int i = 2;

    if (seek[0] == '-') {
      i = 3;
    }

    while (seek[i]) {
      pos = pos * 16 + hex_to_int(seek[i++]);
    }

    if (seek[0] == '-') {
      whence = 2;
      pos = -pos;
    }

    int n = fseek(input, pos, whence);
    if (n != 0) {
      perror("error: ");
      exit(1);
    }
  }

  if (reverse) {
    bin_dump(input);
  } else {
    hex_dump(input);
  }

  if (input != stdin) {
    fclose(input);
  }
}
