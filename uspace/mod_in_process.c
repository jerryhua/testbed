#include <stdio.h>
#include <elf.h>

asm(".balign 4096");
asm("_binary__module_start:");
asm(".incbin \"/home/develop/appexacce/accex.ko\"");
asm("_binary__module_end:");

extern char _binary__module_start[], _binary__module_end[];

main()
{
	char mod_params[128] = {0};
	int ret = 0;
	ret = init_module(_binary__module_start, _binary__module_end - _binary__module_start, mod_params);
	printf("ret =%d\n");
}


