#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cutils/properties.h>

#define err printf

int
read_soc_props(void)
{
    int fd;
    ssize_t len;
    char buf[256];
    char model[256];
    char baseband[256];
    char* p;

    fd = open("/sys/devices/system/soc/soc0/build_id", O_RDONLY);
    if (fd < 0) {
        err("Cannot open soc file: %s\n", strerror(errno));
        return -1;
    }
    memset(buf, 0, sizeof(buf));
    len = read(fd, buf, sizeof(buf)-1);
    if (len < 5) {
        err("Cannot read soc file\n");
        return -1;
    }
    close(fd);

    p = strchr(buf, '\n');
    if (p) {
        *p = '\0';
    }

    strcpy(baseband, buf);
    memcpy(model, buf, 5);
    model[5] = '\0';

    printf("Baseband: %s\nModel: %s\n", baseband, model);
    property_set("install.baseband", baseband);
    property_set("install.model", model);

    return 0;
}

int
read_install_props(void)
{
    int fd;
    int rc;
    char buf[1024];
    char* nextline;
    char* key;
    char* val;

    fd = open("/sdcard/.install.prop", O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    memset(buf, 0, sizeof(buf));
    rc = read(fd, buf, sizeof(buf)-1);
    close(fd);
    if (rc <= 0) {
        err("Cannot read install props\n");
        return -1;
    }

    nextline = buf;
    while (nextline != NULL) {
        key = nextline;
        nextline = strchr(key, '\n');
        if (nextline != NULL) {
            *nextline = '\0';
            ++nextline;
        }
        val = strchr(key, '=');
        if (val != NULL) {
            *val = '\0';
            ++val;
            printf("Install prop: %s=%s\n", key, val);
            property_set(key, val);
        }
    }

    return 0;
}

int
main(int argc, char** argv)
{
    read_soc_props();
    read_install_props();
    return 0;
}
