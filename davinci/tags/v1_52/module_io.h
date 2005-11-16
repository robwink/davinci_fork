#ifndef __MODULE_IO_H__
#define __MODULE_IO_H__

typedef void *dvModuleHandle;

dvModuleHandle open_dv_module_file(char *fname);
void close_dv_module_file(dvModuleHandle emh);
void *locate_dv_module_func_in_slib(dvModuleHandle emh, char *func_name);

#endif /* __MODULE_IO_H__ */
