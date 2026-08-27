#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ELAN_VENDOR_ID  0x04f3
#define ELAN_PRODUCT_ID 0x0c4b
#define ELAN_INTERFACE  0
#define ELAN_BULK_OUT   0x01

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s on|off\n", name);
}

int main(int argc, char **argv)
{
    const uint8_t command_on[] = {0x40, 0x31};
    const uint8_t command_off[] = {0x00, 0x0b};
    const uint8_t *command;
    const char *state;
    int detached = 0;
    int transferred = 0;
    int rc;
    libusb_context *ctx = NULL;
    libusb_device_handle *handle = NULL;

    if (argc != 2 ||
        (strcmp(argv[1], "on") != 0 && strcmp(argv[1], "off") != 0)) {
        usage(argv[0]);
        return 2;
    }

    if (strcmp(argv[1], "on") == 0) {
        command = command_on;
        state = "on";
    } else {
        command = command_off;
        state = "off";
    }

    rc = libusb_init(&ctx);
    if (rc != 0) {
        fprintf(stderr, "libusb_init: %s\n", libusb_error_name(rc));
        return 1;
    }

    handle = libusb_open_device_with_vid_pid(ctx, ELAN_VENDOR_ID,
                                             ELAN_PRODUCT_ID);
    if (handle == NULL) {
        fprintf(stderr,
                "ELAN %04x:%04x not found (stop fprintd first)\n",
                ELAN_VENDOR_ID, ELAN_PRODUCT_ID);
        libusb_exit(ctx);
        return 1;
    }

    rc = libusb_kernel_driver_active(handle, ELAN_INTERFACE);
    if (rc == 1) {
        rc = libusb_detach_kernel_driver(handle, ELAN_INTERFACE);
        if (rc != 0) {
            fprintf(stderr, "detach interface: %s\n", libusb_error_name(rc));
            libusb_close(handle);
            libusb_exit(ctx);
            return 1;
        }
        detached = 1;
    } else if (rc < 0 && rc != LIBUSB_ERROR_NOT_SUPPORTED) {
        fprintf(stderr, "kernel driver check: %s\n", libusb_error_name(rc));
    }

    rc = libusb_claim_interface(handle, ELAN_INTERFACE);
    if (rc != 0) {
        fprintf(stderr, "claim interface: %s\n", libusb_error_name(rc));
        if (detached)
            libusb_attach_kernel_driver(handle, ELAN_INTERFACE);
        libusb_close(handle);
        libusb_exit(ctx);
        return 1;
    }

    rc = libusb_bulk_transfer(handle, ELAN_BULK_OUT, (unsigned char *)command,
                              2, &transferred, 1000);
    if (rc == 0 && transferred == 2)
        printf("sent %02x %02x (%s)\n", command[0], command[1], state);
    else
        fprintf(stderr, "bulk transfer: %s (sent %d bytes)\n",
                libusb_error_name(rc), transferred);

    libusb_release_interface(handle, ELAN_INTERFACE);
    if (detached)
        libusb_attach_kernel_driver(handle, ELAN_INTERFACE);
    libusb_close(handle);
    libusb_exit(ctx);

    return rc == 0 && transferred == 2 ? 0 : 1;
}
