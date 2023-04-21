#include <iostream>

#include "src/drive.hpp"

int main()
{
    void* drive_ptr = malloc(65536);
    FileSystem fs = FileSystem::mkfs(drive_ptr, 1024, 16, 64);

    fs.mkdir("/foo");
    fs.mkdir("foo/bar");

    fs.cd("foo/bar");
    fs.mkdir("test");

    fs.mkdir("/qwerty");
    fs.create("/qwerty/uiop.dat");

    fs.cd("..");

    fs.symlink("/foo/bar/test", "../symlink");

    fs.create("/symlink/file.txt");
    fs.link("bar/test/file.txt", "/link.txt");
    fs.link("/foo/bar/test/file.txt", "/link2.txt");

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

    fs.truncate("/symlink/file.txt", 256);

    fs.ls("/");

    printf("rmdir result %d\n", fs.rmdir("/symlink/../../../qwerty"));
    fs.unlink("/qwerty/uiop.dat");
    printf("delete qwerty/uiop.dat\n");
    printf("rmdir result %d\n", fs.rmdir("/symlink/../../../qwerty"));
    fs.unlink("/link2.txt");

    fs.ls("/");

    fs.stat("/");
    fs.stat("/link.txt");
    fs.stat("/symlink/file.txt");

    free(drive_ptr);
    return 0;
}
