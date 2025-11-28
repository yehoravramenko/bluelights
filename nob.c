#define NOB_IMPLEMENTATION
#include "nob.h"

#include <stdio.h>
#include <string.h>

#define BUILD_DIR "build\\"
#define OBJ_DIR   "obj\\"
#define BIN_DIR   "bin\\"
#define SRC_DIR   "src\\"
#define EXE_NAME  "bluelights.exe"

#define CARGS                                                                  \
    "/nologo", "/W4", "/WX", "/Yupch.hpp", "/Fp" BUILD_DIR OBJ_DIR "pch.pch",  \
        "/std:c++latest", "/EHsc", "/c"

#define LARGS "/nologo"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (argc > 1 && strcmp(argv[1], "pch") == 0)
    {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "cl", "/Ycpch.hpp", "/std:c++latest", "/EHsc",
                       "/c", "/W4", "/Fp" BUILD_DIR OBJ_DIR "pch.pch",
                       "/Fo:build\\obj\\pch.obj", SRC_DIR "pch.cpp");

        if (!nob_cmd_run(&cmd))
            return 1;

        return 0;
    }

    const char *files[] = {"main", "\0"};

    if (!nob_file_exists(BUILD_DIR OBJ_DIR "pch.pch"))
    {
        printf("run with pch arg\n");
        return 1;
    }

    if (nob_file_exists("nob.obj"))
        nob_delete_file("nob.obj");

    const char *ROOT_DIR = nob_get_current_dir_temp();

    nob_mkdir_if_not_exists(BUILD_DIR);

    nob_set_current_dir(BUILD_DIR);
    nob_mkdir_if_not_exists(OBJ_DIR);
    nob_mkdir_if_not_exists(BIN_DIR);

    nob_set_current_dir(ROOT_DIR);

    Nob_Cmd cmd = {};

    for (int i = 0; strcmp(files[i], "\0") != 0; ++i)
    {
        char src_buf[MAX_PATH] = {0};
        char obj_buf[MAX_PATH] = {0};

        snprintf(src_buf, MAX_PATH, "%s%s.cpp", SRC_DIR, files[i]);
        snprintf(obj_buf, MAX_PATH, "/Fo:%s%s%s.obj", BUILD_DIR, OBJ_DIR,
                 files[i]);

        nob_cmd_append(&cmd, "cl", CARGS, obj_buf, src_buf);

        if (!nob_cmd_run(&cmd))
            return 1;
    }

    nob_cmd_append(&cmd, "link", LARGS, "build\\obj\\*.obj",
                   "/OUT:build\\bin\\" EXE_NAME);

    if (!nob_cmd_run(&cmd))
        return 1;

    return 0;
}
