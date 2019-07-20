#ifndef LEPTJSON_H__//incldue防范重复，如果LEPTJSON_H__没有定义，则执行直到#endif之间的代码
#define LEPTJSON_H__
#include <stddef.h> /* size_t */

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;   //枚举

typedef struct {  //表示一个节点
	union {
		struct { char*s; size_t len; }s; /*string*/
		double n;//选择double来储存json数字，仅当type==LEPT_NUMBER时，n才表示JSON数字的数值
	}u;
    lept_type type;/*该节点的类型由枚举类型表示*/
}lept_value;

enum {
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG,
	LEPT_PARSE_MISS_QUOTATION_MARK,
	LEPT_PARSE_INVALID_STRING_ESCAPE,
	LEPT_PARSE_INVALID_STRING_CHAR
};

#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)/*初始化值*/

int lept_parse(lept_value* v, const char* json);  //解析函数声明，其中v为根节点，json为json文本（c风格的字符串）

void lept_free(lept_value* v);/*释放内存，并且将节点类型设置为NULL*/

lept_type lept_get_type(const lept_value* v);

#define lept_set_null(v) lept_free(v)

 /*读取一个json节点的值(只针对于数字，bool值和字符串)和向一个json节点写入值的API*/
int lept_get_boolean(const lept_value* v);   
void lept_set_boolean(lept_value* v, int b);

double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);

#endif /* LEPTJSON_H__ */
