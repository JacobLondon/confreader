#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "confreader.h"

static int paramReadHelper(struct ConfParam *param, char *value);
static void paramValueIntoBuffer(struct ConfParam *param, char *valout, size_t outsize);
static char *stringFindStart(char *string);
static void stringEatFirstWhitespace(char *string);

enum ConfReturn ConfParamRead(char *filename, struct ConfParam paramList[])
{
	return ConfParamReadFuncs(filename, paramList, NULL, NULL);
}

enum ConfReturn ConfParamReadFuncs(char *filename, struct ConfParam paramList[],
	void (*onSuccess)(const char *key, char *value),
	void (*onDefault)(const char *key, char *value))
{
	char line[1024];
	int lineLen;
	FILE *fp;
	char *equals;
	char *start;
	char *value;
	size_t i;

	if (paramList == NULL)
	{
		return ConfReturnNULL_PARAMLIST;
	}

	/* load defaults */
	for (i = 0; paramList[i].name != NULL; i++)
	{
		(void)paramReadHelper(&paramList[i], NULL);
	}

	if (filename == NULL)
	{
		return ConfReturnNULL_FILENAME;
	}

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		return ConfReturnCANT_OPEN;
	}

	while (fgets(line, sizeof(line), fp) != NULL)
	{
		lineLen = (int)strlen(line) - 1;
		if (lineLen <= 0)
		{
			continue;
		}

		if (line[0] == '\r' || line[0] == '\n')
		{
			continue;
		}

		if ((line[lineLen] != '\n' && !feof(fp))) {
			/* line too long, skip it */
			while (line[lineLen] != '\n' && !feof(fp))
			{
				(void)fgets(line, sizeof(line), fp);
				lineLen = (int)strlen(line) - 1;
			}

			if (feof(fp))
			{
				break;
			}

			continue;
		}

		/* get ridda CRLF */
		if ((lineLen - 1 >= 0) && (line[lineLen - 1] == '\r'))
		{
			line[lineLen - 1] = '\0';
		}

		start = stringFindStart(line);

		/* end */
		if (start[0] == '\0')
		{
			continue;
		}
		/* comment */
		if (start[0] == '#')
		{
			continue;
		}

		equals = strchr(start, '=');
		if (equals == NULL || equals == start)
		{
			continue;
		}
		value = stringFindStart(&equals[1]);
		stringEatFirstWhitespace(start);

		for (i = 0; paramList[i].name != NULL; i++)
		{
			if (strncmp(start, paramList[i].name, (size_t)(equals - start)) == 0)
			{
				(void)paramReadHelper(&paramList[i], value);
				break;
			}
		}
	}

	/* perform callbacks on params in given order, note the above skips default params */
	if (onDefault != NULL || onSuccess != NULL)
	{
		for (i = 0; paramList[i].name != NULL; i++)
		{
			if (paramList[i]._isDefault && onDefault != NULL)
			{
				paramValueIntoBuffer(&paramList[i], line, sizeof(line));
				onDefault(paramList[i].name, line);
			}
			else if (!paramList[i]._isDefault && onSuccess != NULL)
			{
				paramValueIntoBuffer(&paramList[i], line, sizeof(line));
				onSuccess(paramList[i].name, line);
			}
		}
	}

	(void)fclose(fp);
	return ConfReturnOK;
}

enum ConfReturn ConfParamWrite(char *filename, struct ConfParam paramList[])
{
	size_t i;
	FILE *fp;

	if (filename == NULL)
	{
		return ConfReturnNULL_FILENAME;
	}

	if (paramList == NULL)
	{
		return ConfReturnNULL_PARAMLIST;
	}

	fp = fopen(filename, "w");
	if (fp == NULL)
	{
		return ConfReturnCANT_OPEN;
	}

	for (i = 0; paramList[i].name != NULL; i++)
	{
		switch (paramList[i].type)
		{
		case ConfTypeINT:
			(void)fprintf(fp, "%s=%d\n", paramList[i].name, *(int *)paramList[i].param);
			break;
		case ConfTypeDOUBLE:
			(void)fprintf(fp, "%s=%lf\n", paramList[i].name, *(double *)paramList[i].param);
			break;
		case ConfTypeBOOL:
			(void)fprintf(fp, "%s=%s\n", paramList[i].name,
				(*(int *)paramList[i].param) ? "True" : "False"
			);
			break;
		case ConfTypeSTRING:
			(void)fprintf(fp, "%s=%s\n", paramList[i].name, (char *)paramList[i].param);
			break;
		default:
			(void)fclose(fp);
			return ConfReturnBAD_FORMAT;
		}
		(void)fflush(fp);
	}

	(void)fclose(fp);
	return ConfReturnOK;
}

const char *ConfReturnString(enum ConfReturn cr)
{
	switch (cr)
	{
	case ConfReturnOK:             return "Ok";
	case ConfReturnNULL_FILENAME:  return "Null filename";
	case ConfReturnNULL_PARAMLIST: return "Null parameter list";
	case ConfReturnCANT_OPEN:      return "Can't open file";
	case ConfReturnBAD_FORMAT:     return "Bad format definition";
	}
	return "Unknown";
}

/**
 * Copy the param as a string into \a valout
 */
static void paramValueIntoBuffer(struct ConfParam *param, char *valout, size_t outsize)
{
	if (!param || !valout)
	{
		return;
	}

	switch (param->type)
	{
	case ConfTypeINT:
		(void)snprintf(valout, outsize, "%d", *(int *)param->param);
		break;
	case ConfTypeDOUBLE:
		(void)snprintf(valout, outsize, "%lf", *(double *)param->param);
		break;
	case ConfTypeBOOL:
		(void)snprintf(valout, outsize, "%d", *(int *)param->param);
		break;
	case ConfTypeSTRING:
		(void)snprintf(valout, outsize, "%s", (char *)param->param);
		break;
	default:
		valout[0] = '\0';
		break;
	}
}

/**
 * \param value
 *    The string value to attempt setting the param to, nullable = default value
 * 
 * \return
 *    >0 failure
 *     0 loaded parameter
 *    -1 loaded default parameter
 *    -2 loaded string but truncated
 */
static int paramReadHelper(struct ConfParam *param, char *value)
{
	union
	{
		int integer;
		double real;
		char boolean[8];
		/* string already taken care of */
	} tmp;
	int rv;

	if (param == NULL)
	{
		return 1;
	}

	if (value == NULL)
	{
	default_param:
		(void)memcpy(param->param, param->paramDefault, param->paramLength);
		param->_isDefault = 1;
		return -1;
	}

	switch (param->type)
	{
	case ConfTypeINT:
		rv = sscanf(value, "%d", &tmp.integer);
		if (rv != 1)
		{
			goto default_param;
		}
		*(int *)param->param = tmp.integer;
		break;

	case ConfTypeDOUBLE:
		rv = sscanf(value, "%lf", &tmp.real);
		if (rv != 1)
		{
			goto default_param;
		}
		*(double *)param->param = tmp.real;
		break;

	case ConfTypeBOOL:
		rv = sscanf(value, "%8s", tmp.boolean);
		if (rv != 1)
		{
			goto default_param;
		}
		tmp.boolean[7] = '\0';
		rv = (
			tmp.boolean[0] == '1' ||
			tmp.boolean[0] == 't' ||
			tmp.boolean[0] == 'T' ||
			tmp.boolean[0] == 'y' ||
			tmp.boolean[0] == 'Y'
		);
		if (rv)
		{
			*(int *)param->param = 1;
			break;
		}

		rv = (
			tmp.boolean[0] == '0' ||
			tmp.boolean[0] == 'f' ||
			tmp.boolean[0] == 'F' ||
			tmp.boolean[0] == 'n' ||
			tmp.boolean[0] == 'N'
		);
		if (rv)
		{
			*(int *)param->param = 0;
			break;
		}
		goto default_param;

	case ConfTypeSTRING:
		(void)snprintf((char *)param->param, (size_t)param->paramLength, "%s", value);
		break;

	default:
		return 2;
	}

	param->_isDefault = 0;
	return 0;
}

static char *stringFindStart(char *string)
{
	char *p;

	if (string == NULL)
	{
		return NULL;
	}

	p = string;
	while (isspace(*p))
	{
		p++;
	}

	return p;
}

static void stringEatFirstWhitespace(char *string)
{
	char *p;

	if (string == NULL)
	{
		return;
	}

	p = string;
	while (!isspace(*p))
	{
		p++;
		if (*p == '\0')
		{
			break;
		}
	}

	if (isspace(*p))
	{
		*p = '\0';
	}
}
