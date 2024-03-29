#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL ,strtod() */


#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')


typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;     //如果出现ws空白，将指针跳过空白，最后指向不是空白后的第一个字符
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
	size_t i;
	EXPECT(c, literal[0]); //EXPECT宏中，将c->json往后移动了一个位置
	for ( i= 0; literal[i+1]; i++) {
		if (c->json[i] != literal[i + 1])
			return LEPT_PARSE_INVALID_VALUE;
		}
	c->json += i;
	v->type = type;
	return LEPT_PARSE_OK;
}

/*
static int lept_parse_null(lept_context* c, lept_value* v) {  //解析null
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;  //测试后将指针指向值的下一个位置
    v->type = LEPT_NULL;
	return LEPT_PARSE_OK;  
}

static int lept_parse_true(lept_context*c, lept_value* v) {  //解析true
	EXPECT(c, 't');
	if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = LEPT_TRUE;
	return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context*c, lept_value* v) {  //解析false
	EXPECT(c, 'f');
	if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's'||c->json[3]!='e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 4;
	v->type = LEPT_FALSE;
	return LEPT_PARSE_OK;
} 
*/

static int lept_parse_number(lept_context* c, lept_value* v) {  //解析数字，c视为传入的文本，v为根节点，此数字解析函数还未实现LEPT_PARSE_ROOT_NOT_SINGULLAR
	const char*p = c->json;
	if (*p == '-') p++;     //数字检验
	if (*p == '0') p++;
	else {
		if (!ISDIGIT1TO9(*p))return LEPT_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	if (*p == '.') {
		p++;
		if (!ISDIGIT(*p))return LEPT_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-') p++;
		if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	errno = 0;
	v->n = strtod(c->json, NULL);
	if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
		return LEPT_PARSE_NUMBER_TOO_BIG;
	v->type = LEPT_NUMBER;
	c->json = p;//将数字后的第一个char赋给json。
	return LEPT_PARSE_OK;
}


static int lept_parse_value(lept_context* c, lept_value* v) {  //根据不同的case选择不同的解析函数
	switch (*c->json) {
	case 't':  return lept_parse_literal(c, v,"true",LEPT_TRUE);
	case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
	case 'n':  return lept_parse_literal(c, v,"null",LEPT_NULL);
	default:   return lept_parse_number(c, v);
	case '\0': return LEPT_PARSE_EXPECT_VALUE;
	}
}
int lept_parse(lept_value* v, const char* json) {
	int ret;
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
	if ((ret=lept_parse_value(&c, v)) == LEPT_PARSE_OK) { //如果已经保证ws value ws中的value是正确的，再处理ws
		lept_parse_whitespace(&c); //处理值后面的ws
		if (*c.json != '\0')
			return LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->n;
}
