#ifndef TREE_H
#define TREE_H

#include <stdint.h>

#define FS_NAME_MAX 64
#define FS_PATH_MAX 256
#define MAX_PATH_COMPONENTS 32

typedef enum {
  NODE_FILE,
  NODE_DIRECTORY,
} node_type_t;

typedef struct fs_node {
  char name[FS_NAME_MAX];

  node_type_t type;

  uint32_t first_cluster;
  union {
    struct {
      char host_path[FS_PATH_MAX];
      uint32_t size;
    } file;

    struct {
      struct fs_node **children;
      uint32_t child_count;
    } dir;
  };
  struct fs_node *parent;
} fs_node_t;

fs_node_t *fs_node_create_directory(char *name);
fs_node_t *fs_node_add_child(fs_node_t *parent, fs_node_t *child);
fs_node_t *fs_node_find_child(fs_node_t *dir, char *name);
fs_node_t *fs_node_create_file(char *filename, char *host_path);
int split_path(char *path, char *parts[]);
fs_node_t *tree_insert_node(fs_node_t *root_dir, char *path, char *host_path);
fs_node_t *tree_insert_directory(fs_node_t *root_dir, char *path);

void fs_node_free(fs_node_t *node);

#endif