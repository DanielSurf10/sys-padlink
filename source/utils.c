#include "utils.h"

void	print_to_file(FILE *arq, const char *format, ...)
{
#ifdef DEBUG
	va_list	args;

	if (arq == NULL)
		return ;

	va_start(args, format);
	vfprintf(arq, format, args);
	va_end(args);
#endif
}
