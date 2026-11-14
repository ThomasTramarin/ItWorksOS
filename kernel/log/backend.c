#include <base/stddef.h>
#include <kernel/error.h>
#include <klib/list.h>
#include <log/backend.h>

static struct list log_backend_list = LIST_INIT(log_backend_list);

int32_t log_backend_register(struct log_backend *backend) {
  if (!backend)
    return -KERR_INVAL;

  list_insert_back(&log_backend_list, &backend->log_backend_node);

  return KERR_OK;
}

int32_t log_backend_dispatch(const struct log_record *record) {
  if (!record)
    return -KERR_INVAL;

  int32_t result = KERR_OK;

  struct list_node *node;

  list_for_each(node, &log_backend_list) {
    struct log_backend *backend = (struct log_backend *)container_of(
        node, struct log_backend, log_backend_node);

    int32_t ret = backend->write(record);

    if (ret < 0 && result == KERR_OK)
      result = ret;
  }

  return result;
}