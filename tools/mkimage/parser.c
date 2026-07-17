#include "parser.h"
#include "tree.h"
#include <stdio.h>
#include <string.h>

#define MAX_BUF_SIZE ((FS_PATH_MAX * 2) + 3)

fs_node_t *parse_disk_files(FILE *fp) {
  fs_node_t *root = fs_node_create_directory("");

  char line_buf[MAX_BUF_SIZE]; // host_path:image_path\n\0

  while (fgets(line_buf, sizeof(line_buf), fp)) {
    line_buf[strcspn(line_buf, "\n")] = '\0'; // remove newline

    char *sep = strchr(line_buf, ':');

    if (!sep)
      continue;

    *sep = '\0';

    char *host_path = line_buf;
    char *image_path = sep + 1;

    // :image_path/ -> syntax for creating an empty directory
    size_t len = strlen(image_path);
    int is_directory = (len > 0 && image_path[len - 1] == '/');

    fs_node_t *node = NULL;

    if (is_directory) {
      // remove the last '/'. split_path should not interpet it as a separator
      image_path[len - 1] = '\0';

      node = tree_insert_directory(root, image_path);
    } else {
      // file insertion
      node = tree_insert_node(root, image_path, host_path);
    }

    if (!node) {
      fprintf(stderr, "mkimage: parser: cannot insert %s\n", image_path);
    }
  }

  return root;
}