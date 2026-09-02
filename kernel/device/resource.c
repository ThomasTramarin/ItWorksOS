#include <device/resource.h>
#include <kernel/error.h>

struct resource *resource_get(struct resource *resources, size_t count,
                              enum resource_type type, size_t index) {

  if (!resources)
    return KERR_PTR(-KERR_INVAL);

  struct resource *res;
  size_t current_type_idx = 0;

  for (size_t i = 0; i < count; i++) {
    res = &resources[i];

    if (res->type != type)
      continue;

    if (current_type_idx == index)
      return res;

    current_type_idx++;
  }

  return KERR_PTR(-KERR_NOENT);
}