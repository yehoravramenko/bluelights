#define NOB_IMPLEMENTATION
#include "nob.h"

#include <stdio.h>
#include <string.h>

#define BUILD_DIR "build\\"
#define OBJ_DIR   "obj\\"
#define BIN_DIR   "bin\\"
#define SRC_DIR   "src\\"
#define EXE_NAME  "bluelights.exe"

#define CARGS "/nologo", "/Wall", "/std:c++latest", "/EHsc", "/c"
#define LARGS "/nologo"

int main(int argc, char **argv)
{
    const char *files[] = {"main", "\0"};

    NOB_GO_REBUILD_URSELF(argc, argv);

    if (nob_file_exists("nob.obj"))
        nob_delete_file("nob.obj");

    const char *ROOT_DIR = nob_get_current_dir_temp();

    nob_mkdir_if_not_exists(BUILD_DIR);

    nob_set_current_dir(BUILD_DIR);
    nob_mkdir_if_not_exists(OBJ_DIR);
    nob_mkdir_if_not_exists(BIN_DIR);

    nob_set_current_dir(ROOT_DIR);

    Nob_Cmd cmd                  = {};
    Nob_String_Builder obj_files = {0};

    for (int i = 0; strcmp(files[i], "\0") != 0; ++i)
    {
        char src_buf[MAX_PATH] = {0};
        char obj_buf[MAX_PATH] = {0};

        snprintf(src_buf, MAX_PATH, "%s%s.cpp", SRC_DIR, files[i]);
        snprintf(obj_buf, MAX_PATH, "/Fo:%s%s%s.obj", BUILD_DIR, OBJ_DIR,
                 files[i]);

        nob_cmd_append(&cmd, "cl", CARGS, obj_buf, src_buf);

        nob_sb_appendf(&obj_files, "%s%s%s.obj ", BUILD_DIR, OBJ_DIR, files[i]);

        if (!nob_cmd_run(&cmd))
            return 1;
    }

    nob_sb_append_null(&obj_files);

    char link_buf[1024] = {0};
    snprintf(link_buf, 1024, "/OUT:%s%s%s", BUILD_DIR, BIN_DIR, EXE_NAME);

    nob_cmd_append(&cmd, "link", LARGS, obj_files.items, link_buf);
    if (!nob_cmd_run(&cmd))
        return 1;

    return 0;
}
