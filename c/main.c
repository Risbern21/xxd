#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

struct args {
  char **options;
  int num_options;
  char *input;
};

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
  byte *line;
  line = malloc(size + 1);

  offset = ftell(file);
  printf("%08lx: ", offset);

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
      printf("%08lx: ", offset);
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
  if (argc < 2) {
    printf("Invalid usage!\nUsage: xxd [options] <filename>");
    exit(1);
  }

  struct args *parse_args = malloc(sizeof *parse_args);
  if (!parse_args) {
    perror("error: ");
    exit(1);
  }
  parse_args->input = NULL;
  parse_args->options = malloc(sizeof(*parse_args->options));
  if (!parse_args->options) {
    perror("error: ");
    exit(1);
  }

  parse_args->num_options = 0;

  int i = 1;
  while (argv[i]) {
    if (i + 1 == argc) {
      parse_args->input = argv[i];
    } else {
      parse_args->options[parse_args->num_options++] = argv[i];
      parse_args->options =
          realloc(parse_args->options,
                  (parse_args->num_options + 1) * sizeof(*parse_args->options));
      if (!parse_args->options) {
        perror("error: ");
        exit(1);
      }
    }
    i++;
  }

  const char *file_name = parse_args->input;

  FILE *file = fopen(file_name, "r");
  if (file == NULL) {
    perror("error");
    exit(-1);
  }

  if (parse_args->num_options == 0) {
    hex_dump(file);
  } else {
    bin_dump(file);
  }

  fclose(file);
}
