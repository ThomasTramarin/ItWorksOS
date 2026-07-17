#include "tree.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

fs_node_t *fs_node_create_directory(char *name) {

  fs_node_t *node = calloc(1, sizeof(fs_node_t));

  if (!node)
    return NULL;

  strncpy(node->name, name, FS_NAME_MAX - 1);
  node->name[FS_NAME_MAX - 1] = '\0';

  node->type = NODE_DIRECTORY;

  node->dir.children = NULL;
  node->dir.child_count = 0;
  node->first_cluster = 0;

  node->parent = NULL;

  return node;
}

fs_node_t *fs_node_add_child(fs_node_t *parent, fs_node_t *child) {
  fs_node_t **tmp =
      realloc(parent->dir.children,
              sizeof(fs_node_t *) * (parent->dir.child_count + 1));

  if (!tmp)
    return NULL;

  parent->dir.children = tmp;

  parent->dir.children[parent->dir.child_count] = child;

  parent->dir.child_count++;

  child->parent = parent;

  return child;
}

fs_node_t *fs_node_find_child(fs_node_t *dir, char *name) {
  for (uint32_t i = 0; i < dir->dir.child_count; i++) {
    fs_node_t *child = dir->dir.children[i];

    if (strcmp(child->name, name) == 0)
      return child;
  }

  return NULL;
}

fs_node_t *fs_node_create_file(char *filename, char *host_path) {
  fs_node_t *file = calloc(1, sizeof(fs_node_t));

  if (!file)
    return NULL;

  file->type = NODE_FILE;
  file->first_cluster = 0;
  file->file.size = 0;

  strncpy(file->name, filename, FS_NAME_MAX - 1);
  file->name[FS_NAME_MAX - 1] = '\0';

  strncpy(file->file.host_path, host_path, FS_PATH_MAX - 1);
  file->file.host_path[FS_PATH_MAX - 1] = '\0';

  // determine file size
  FILE *fp = fopen(host_path, "rb");
  if (!fp) {
    free(file);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);

  file->file.size = ftell(fp);

  fclose(fp);

  file->parent = NULL;

  return file;
}

int split_path(char *path, char *parts[]) {
  int count = 0;

  char *token = strtok(path, "/");

  while (token != NULL && count < MAX_PATH_COMPONENTS) {
    parts[count++] = token;

    token = strtok(NULL, "/");
  }

  return count;
}

fs_node_t *tree_insert_node(fs_node_t *root_dir, char *path, char *host_path) {
  char path_copy[FS_PATH_MAX];

  strncpy(path_copy, path, FS_PATH_MAX);
  path_copy[FS_PATH_MAX - 1] = '\0';

  char *parts[MAX_PATH_COMPONENTS];

  int count = split_path(path_copy, parts);

  if (count == 0)
    return NULL;

  fs_node_t *current = root_dir;

  // create intermidiate directories
  for (int i = 0; i < count - 1; i++) {
    fs_node_t *child = fs_node_find_child(current, parts[i]);

    if (child) {
      if (child->type != NODE_DIRECTORY)
        return NULL;
    } else {
      child = fs_node_create_directory(parts[i]);

      if (!child)
        return NULL;

      fs_node_add_child(current, child);
    }

    current = child;
  }

  // create the file
  if (fs_node_find_child(current, parts[count - 1]))
    return NULL;

  fs_node_t *file = fs_node_create_file(parts[count - 1], host_path);

  if (!file)
    return NULL;

  fs_node_add_child(current, file);

  return file;
}

fs_node_t *tree_insert_directory(fs_node_t *root_dir, char *path) {
  char path_copy[FS_PATH_MAX];

  strncpy(path_copy, path, FS_PATH_MAX);
  path_copy[FS_PATH_MAX - 1] = '\0';

  char *parts[MAX_PATH_COMPONENTS];

  int count = split_path(path_copy, parts);

  if (count == 0)
    return NULL;

  fs_node_t *current = root_dir;

  // iterate over all path components (even the last), to create them as
  // directories
  for (int i = 0; i < count; i++) {
    fs_node_t *child = fs_node_find_child(current, parts[i]);

    if (child) {
      if (child->type != NODE_DIRECTORY)
        return NULL; // error: there is already a file with the same name
    } else {
      child = fs_node_create_directory(parts[i]);

      if (!child)
        return NULL;

      fs_node_add_child(current, child);
    }

    current = child;
  }

  return current;
}

void fs_node_free(fs_node_t *node) {
  if (!node)
    return;

  // if directory, first free children
  if (node->type == NODE_DIRECTORY) {
    for (uint32_t i = 0; i < node->dir.child_count; i++) {
      fs_node_free(node->dir.children[i]);
    }

    // free the children pointer array
    free(node->dir.children);
  }

  // free the node itself
  free(node);
}