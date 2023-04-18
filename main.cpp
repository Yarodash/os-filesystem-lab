#include <iostream>

#include "src/drive.hpp"

int main()
{
    void* drive_ptr = malloc(65536);
    FileSystem fs = FileSystem::mkfs(drive_ptr, 65536, 16, 64);

    fs.mkdir("foo");
    fs.mkdir("foo/bar");
    fs.mkdir("foo/bar/test");

    fs.cd("foo");
    fs.cd("bar");

    fs.create("file.txt");
    fs.link("./file.txt", "../../link.txt");

    uint8 text1[14] = "Hello, world!";
    uint8 text2[32] = "";

    uint32 fd1 = fs.open("/link.txt");
    uint32 fd2 = fs.open("/link.txt");

    fs.write(fd1, text1, 13);
    fs.seek(fd1, -6);
    fs.write(fd1, text1, 14);

    fs.read(fd2, text2, 32);

    printf("%s\n", text2);

    fs.close(fd1);
    fs.close(fd2);

    fs.truncate("./file.txt", 64);

    fs.unlink("./file.txt");
    fs.unlink("/link.txt");
    
    fs.cd("/");

    fs.rmdir("foo/bar/test");
    fs.rmdir("foo/bar");
    fs.rmdir("foo");

    free(drive_ptr);
    return 0;
}
