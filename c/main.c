#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

typedef unsigned char byte;

int line_len(byte *line) {
  int len = 0;
  while (line[len])
    len++;
  ;
  return len;
}

void clean_str(byte *str) {
  byte *temp = str;
  int i = 0;
  while (temp[i]) {
    if (temp[i] == '\n' || temp[i] == '\t') {
      temp[i] = '.';
    }
    i++;
  }
}

void hex_dump(FILE *in_file, FILE *out_file) {
  int n = 0, size = 16, ch;
  long offset;
  byte *line;
  line = malloc(size + 1);

  while ((ch = getc(in_file)) != EOF) {
    if (n == 0) {
      offset = ftell(in_file) - 1;
      fprintf(out_file, "%08lx: ", offset);
    }
    // print the hex
    fprintf(out_file, "%02x", ch);
    if (n % 2 != 0) {
      fprintf(out_file, " ");
    }

    line[n++] = ch;

    if (n == size) {
      line[n] = '\0';
      clean_str(line);
      fprintf(out_file, " %s\n", line);
      n = 0;
    }
  }

  if (n > 0) {
    line[n] = '\0';

    for (int i = n; i < 16; i++) {
      fprintf(out_file, "   ");
    }

    clean_str(line);
    fprintf(out_file, " %s", line);
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

void bin_dump(FILE *in_file, FILE *out_file) {
  int ch, n = 0, size = 128;
  byte *line = malloc(size + 1);

  while ((ch = getc(in_file)) != EOF) {
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
          fprintf(out_file, "%c", hex_to_ascii(buf, line[i]));
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

#ifndef DOING_UNIT_TESTS
int main(int argc, char **argv) {
  int opt;
  int reverse = 0;
  char *seek = NULL;
  FILE *input = NULL, *output = NULL;

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
    FILE *temp_file = fopen("/tmp/temp_xxd", "w");
    if (!temp_file) {
      perror("error: ");
      exit(1);
    }
    int ch;

    while ((ch = getchar()) != EOF) {
      putc(ch, temp_file);
    }

    rewind(temp_file);
    fclose(temp_file);

    input = fopen("/tmp/temp_xxd", "r");
    if (!input) {
      perror("error: ");
      exit(1);
    }
  } else {
    char *in_file = (optind < argc) ? argv[optind] : NULL;
    char *out_file = (optind + 1 < argc) ? argv[optind + 1] : NULL;

    if (in_file == NULL) {
      printf("no file name provided\n");
    }

    input = fopen(in_file, "r");
    output = out_file ? fopen(out_file, "w+") : NULL;
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

  switch (reverse) {
  case 0:
    hex_dump(input, output ? output : stdout);
    break;
  case 1:
    bin_dump(input, output ? output : stdout);
    break;
  }

  if (input != stdin) {
    fclose(input);
  }
  if (output && output != stdout) {
    fclose(output);
  }
}
#endif
