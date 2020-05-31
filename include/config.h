#ifndef _VPND_CONFIG
#define _VPND_CONFIG

char *join_paths(const char *path1, const char *path2);
const char *daemon_config_dir(void);
void free_config_dir(void);
const char *set_config_dir(const char *dir);

#endif