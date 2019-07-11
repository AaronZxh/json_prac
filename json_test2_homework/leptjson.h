#ifndef LEPTJSON_H__//incldue防范重复，如果LEPTJSON_H__没有定义，则执行直到#endif之间的代码
#define LEPTJSON_H__

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;   //枚举

typedef struct {  //表示一个节点
	double n;//选择double来储存json数字，仅当type==LEPT_NUMBER时，n才表示JSON数字的数值
    lept_type type;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG
};

int lept_parse(lept_value* v, const char* json);  //解析函数声明，其中v为根节点，json为json文本（c风格的字符串）

lept_type lept_get_type(const lept_value* v);

double lept_get_number(const lept_value* v);

#endif /* LEPTJSON_H__ */
