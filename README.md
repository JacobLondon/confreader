# Conf Reader
Read simple, non-nested configuration files with `key=value` pairs. Note there is some robustness in spacing: `key = value` is also acceptable. Note writing back the configuration is available, but will clobber comments left in the file.

Parameters will be read from the file specified, defaults will be used if: the file can't be read, the filename is NULL, a `key` is missing, a line is too long, the `value` is an invalid format, etc... Note Unicode will be read, but not processed properly. There SHALL be no bailout half-way through reading params.
```C

/* - header - */

/* types */
enum ConfType
{
	ConfTypeINT, /* int */
	ConfTypeDOUBLE, /* double */
	ConfTypeBOOL, /* int, reads first char: Y/N, y/n, 1/0, T/F, t/f */
	ConfTypeSTRING, /* char[N], where N is static */
};



/* - program - */

#include "confreader.h"

struct {
	int a;
	char b[32];
	int c;
	double real;
	int integer;
	int boolean;
} myConf, myConfDefault = {
	.a = 20,
	.b = "nothing",
	.c = 50,
	.real = 2.7182818,
	.integer = 5,
	.boolean = 1,
};

struct ConfParam paramList[] = {
	CONF_PARAM(ConfTypeINT,    a,       myConf, myConfDefault),
	CONF_PARAM(ConfTypeSTRING, b,       myConf, myConfDefault), /* out-param SHALL be NUL terminated */
	CONF_PARAM(ConfTypeINT,    c,       myConf, myConfDefault),
	CONF_PARAM(ConfTypeDOUBLE, real,    myConf, myConfDefault),
	CONF_PARAM(ConfTypeINT,    integer, myConf, myConfDefault),
	CONF_PARAM(ConfTypeBOOL,   boolean, myConf, myConfDefault),
	CONF_PARAM_END(),
};

int main(int argc, char *argv[])
{
	enum ConfReturn cr;
	cr = ConfParamRead("test.conf", paramList);
	if (cr != ConfReturnOK)
	{
		/* a paramter was NULL or no permission to access the file */
	}

	/* - snip - */

	return 0;
}

```
