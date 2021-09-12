#ifndef CONF_READER_H_
#define CONF_READER_H_

#include <stddef.h>

enum ConfReturn
{
	ConfReturnOK = 0,
	ConfReturnNULL_FILENAME,
	ConfReturnNULL_PARAMLIST,
	ConfReturnCANT_OPEN,
	ConfReturnBAD_FORMAT,
};

struct ConfParam
{
	const char *name;
	void *param;
	void *paramDefault;
	size_t paramLength;
	enum ConfType
	{
		ConfTypeINT,
		ConfTypeDOUBLE,
		ConfTypeBOOL,
		ConfTypeSTRING,
	} type;
	int _isDefault;
};

#define CONF_PARAM(TYPE, UNQUOTED_NAME, CONFIG, DEFAULT) \
{ \
	.name = #UNQUOTED_NAME, \
	.param = &CONFIG.UNQUOTED_NAME, \
	.paramDefault = &DEFAULT.UNQUOTED_NAME, \
	.paramLength = sizeof(CONFIG.UNQUOTED_NAME), \
	.type = TYPE, \
	._isDefault = 0, \
}

#define CONF_PARAM_END() {NULL, NULL, NULL, 0, 0, 0}

/**
 * Read parameters from a \a filename into \a paramList while first loading defaults
 * 
 * \param filename
 *    File to read, is nullable
 */
enum ConfReturn ConfParamRead(char *filename, struct ConfParam paramList[]);

/**
 * Same as \ref ConfParamRead but perform a callback when either the param is
 * successfully read or the default is used
 */
enum ConfReturn ConfParamReadFuncs(char *filename, struct ConfParam paramList[],
	void (*onSuccess)(const char *key, char *value),
	void (*onDefault)(const char *key, char *value));

/**
 * Write parameters from \a paramList into \a filename
 * 
 * \warning
 *    Performing this action will eat all comments within the file
 * 
 * \param filename
 *    File to write to, cannot be nullable
 */
enum ConfReturn ConfParamWrite(char *filename, struct ConfParam paramList[]);

/**
 * Stringify the return codes, no need to free
 */
const char *ConfReturnString(enum ConfReturn cr);

#endif /* CONF_READER_H_ */
