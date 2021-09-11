#include <stdio.h>
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
	.real = 2.718281828,
	.integer = 5,
	.boolean = 1,
};

struct ConfParam paramList[] = {
	CONF_PARAM(ConfTypeINT,    a,        myConf, myConfDefault),
	CONF_PARAM(ConfTypeSTRING, b,        myConf, myConfDefault),
	CONF_PARAM(ConfTypeINT,    c,        myConf, myConfDefault),
	CONF_PARAM(ConfTypeDOUBLE, real,     myConf, myConfDefault),
	CONF_PARAM(ConfTypeINT,    integer,  myConf, myConfDefault),
	CONF_PARAM(ConfTypeBOOL,    boolean, myConf, myConfDefault),
	CONF_PARAM_END(),
};

int main(void)
{
	enum ConfReturn cr;

	cr = ConfParamRead("test.conf", paramList);
	printf("Result: %s\n", ConfReturnString(cr));

	printf("a=%d\n", myConf.a);
	printf("b=%s\n", myConf.b);
	printf("c=%d\n", myConf.c);
	printf("real=%lf\n", myConf.real);
	printf("integer=%d\n", myConf.integer);
	printf("boolean=%d\n", myConf.boolean);

	return 0;
}
