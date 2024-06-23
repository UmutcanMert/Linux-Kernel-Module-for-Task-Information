#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 256 * 100 /* module deki ile ayni yapmaya calistim size kismini */

int main() {
    int fd;
    char buffer[BUFFER_SIZE];

    
    fd = open("/proc/mytaskinfo", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open the file");
        return -1;
    }

    /* Dosyayi okur */
    ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE);
    if (bytesRead < 0) {
        perror("Error reading from file");
        close(fd);
        return -1;
    }

    /* Okunan veriyi ekrana yazdirir */
    printf("%.*s\n", (int)bytesRead, buffer);

    close(fd);

    return 0;
}
