#include <base/sections.h>
#include <boot/boot_info.h>
#include <device/platform.h>
#include <device/resource.h>
#include <kernel/boot.h>
#include <kernel/error.h>
#include <kernel/initcall.h>

static struct resource vga_text_resources[] = {
    RESOURCE_MMIO(0xB8000, 0xB8F9F), /* 4000 bytes */
    RESOURCE_PORT(0x3D4, 0x3D5)};

struct platform_device vga_device;

static int32_t __init vga_device_init(void) {

  vga_device.dev.id = 0;

  /*
   * Choose at runtime the name, based on the BIOS vga mode to match the correct
   * driver.
   */
  struct boot_state *state = boot_get_state();

  if (state->video.type == BOOT_VIDEO_TYPE_TEXT) {
    vga_device.dev.name = "vga-text";
    vga_device.resources = vga_text_resources;
    vga_device.resource_count =
        sizeof(vga_text_resources) / sizeof(vga_text_resources[0]);
  } else
    return -KERR_NOSUP;

  return platform_device_register(&vga_device);
}

INITCALL(INIT_DEVDRV, vga_device_init);