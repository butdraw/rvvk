#include "include/rvvk.h"

int main (int argc, char *argv[])
{
    char *prog_path;
    char **prog_argv;
    char **prog_envp;
    char prog_real_path[PATH_MAX];
    int progfd;
    struct target_pt_regs regs_struct, *regs = &regs_struct;
    struct linux_binprm binprm, *bprm = &binprm;
    struct image_info info_struct, *info = &info_struct;
    state_t env_struct, *env = &env_struct;
    task_t task_struct, *task = &task_struct;
    errinfo_t errinfo, *errp = &errinfo;

    memset(bprm, 0, sizeof *bprm);
    memset(info, 0, sizeof *info);
    memset(regs, 0, sizeof *regs);
    memset(errp, 0, sizeof *errp);
    memset(env, 0, sizeof *env);
    memset(task, 0, sizeof *task);

    if (parse_options(&argv[1], &prog_path, &prog_argv, &prog_envp, errp)) {
        goto fail;
    }

    if (get_real_path(prog_path, prog_real_path, errp) == NULL) {
        goto fail;
    }
    prog_path = prog_real_path;

    progfd = open(prog_path, O_RDONLY);
    if (progfd < 0) {
        fprintf(stderr, "Failed to open %s: %s.\n", prog_path, strerror(errno));
        exit(1);
    }

    if (elf_load(progfd, prog_path, prog_argv, prog_envp,
                 bprm, info, regs, errp) < 0) {
        goto fail;
    }

    target_set_brk(info->brk);
    target_cpu_copy_regs(env, task, info, regs);

    cpu_loop(env);

    return 0;
fail:
    fprintf(stderr, "rvvk: %s:%u: %s: %s\n",
        errp->file, errp->line, errp->func, errp->info);
    return 1;
}

