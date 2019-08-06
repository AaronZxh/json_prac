#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h> /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>  /* memcpy() */

#ifndef LEPT_PARSE_STACK_INIT_SIZE  /*默认缓冲区堆栈的大小*/
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)         do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)


typedef struct {
    const char* json;
	char* stack;  /*指向堆栈起始位置的指针*/
	size_t size, top;/*size 是当前的堆栈容量，top 是栈顶的位置*/
}lept_context;

static void* lept_context_push(lept_context* c, size_t size) {  /*栈压入*/
	void* ret;
	assert(size > 0);
	if (c->top + size >= c->size) {
		if (c->size == 0)
			c->size = LEPT_PARSE_STACK_INIT_SIZE;
		while (c->top + size >= c->size)
			c->size += c->size >> 1;  /* c->size * 1.5 */
		c->stack = (char*)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;
	c->top += size;
	return ret;
}

static void* lept_context_pop(lept_context* c, size_t size) { /*栈弹出*/
	assert(c->top >= size);
	return c->stack + (c->top -= size);
}

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
	v->u.n = strtod(c->json, NULL);
	if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
		return LEPT_PARSE_NUMBER_TOO_BIG;
	v->type = LEPT_NUMBER;
	c->json = p;//将数字后的第一个char赋给json。
	return LEPT_PARSE_OK;
}

static const char* lept_parse_hex4(const char* p, unsigned* u) {/*把字符串表示的十六进制数表示为16进制的数字u*/
	int i;
	*u = 0;
	for (i = 0; i < 4; i++) {
		char ch = *p++;
		*u <<= 4;
		if (ch >= '0' && ch <= '9')* u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F')* u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f')* u |= ch - ('a' - 10);
		else return NULL;
	}
	return p;
}

static void lept_encode_utf8(lept_context* c, unsigned u) {
	if (u <= 0x7F)
		PUTC(c, u & 0xFF);
	else if (u <= 0x7FF) {
		PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else if (u <= 0xFFFF) {
		PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else {
		assert(u <= 0x10FFFF);
		PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
}

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

static int lept_parse_string(lept_context* c, lept_value* v) {/*将字符串压入在lept_contest的栈中，然后用lept_set_string把栈中的所有元素设置在lept_value中*/
	unsigned u,u2; /*调用lept_parse_hex4()储存为码点u*/
	size_t head = c->top, len;    /*head备份栈顶top*/
	const char* p;
	EXPECT(c, '\"');
	p = c->json;
	for (;;) {
		char ch = *p++;
		switch (ch) {
		case '\"':
			len = c->top - head;
			lept_set_string(v, (const char*)lept_context_pop(c, len), len);
			c->json = p;
			return LEPT_PARSE_OK;
		case '\\':
			switch (*p++) {
			case '\"': PUTC(c, '\"'); break;
			case '\\': PUTC(c, '\\'); break;
			case '/':  PUTC(c, '/'); break;
			case 'b':  PUTC(c, '\b'); break;
			case 'f':  PUTC(c, '\f'); break;
			case 'n':  PUTC(c, '\n'); break;
			case 'r':  PUTC(c, '\r'); break;
			case 't':  PUTC(c, '\t'); break;
			case 'u':
				if (!(p = lept_parse_hex4(p, &u)))
					STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
				if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
					if (*p++ != '\\')
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					if (*p++ != 'u')
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					if (!(p = lept_parse_hex4(p, &u2)))
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
					if (u2 < 0xDC00 || u2 > 0xDFFF)
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
				}
				lept_encode_utf8(c, u);
				break;
			default:
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
			}
			break;
		case '\0':
			STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
		default:
			if ((unsigned char)ch < 0x20)
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
			PUTC(c, ch);
		}
	}
}


static int lept_parse_value(lept_context* c, lept_value* v) {  //根据不同的case选择不同的解析函数
	switch (*c->json) {
	case 't':  return lept_parse_literal(c, v,"true",LEPT_TRUE);
	case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
	case 'n':  return lept_parse_literal(c, v,"null",LEPT_NULL);
	default:   return lept_parse_number(c, v);
	case '"':  return lept_parse_string(c, v);
	case '\0': return LEPT_PARSE_EXPECT_VALUE;
	}
}

int lept_parse(lept_value* v, const char* json) {
	int ret;
    lept_context c;
    assert(v != NULL);
    c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;
	lept_init(v);
    lept_parse_whitespace(&c);
	if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
		lept_parse_whitespace(&c);
		if (*c.json != '\0') {
			v->type = LEPT_NULL;
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	assert(c.top == 0);
	free(c.stack);  /*每次都释放内存。使得栈重复使用*/
	return ret;
}

void lept_free(lept_value* v) {
	assert(v != NULL);
	if (v->type == LEPT_STRING)/*当类型为字符串数组或者对象时，才会释放内存*/
		free(v->u.s.s);
	v->type = LEPT_NULL;

}/*以下为对节点的访问或储存API定义*/
lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

int lept_get_boolean(const lept_value* v) {
	assert(v != NULL && (v->type == LEPT_FALSE || v->type == LEPT_TRUE));
	return v->type==LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b) {
	lept_free(v); /*设置节点前首先清空节点可能存在的内存，因为该节点有可能是字符串，数组或者对象*/
	v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}

void lept_set_number(lept_value* v, double n) {
	lept_free(v);
	v->u.n = n;               /*改  值*/
	v->type = LEPT_NUMBER;   /*改类型*/
}

const char* lept_get_string(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.s;
}

size_t lept_get_string_length(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.len;
}

void lept_set_string(lept_value* v, const char* s, size_t len) {
	assert(v != NULL && (s != NULL || len == 0));
	lept_free(v);/*清空v原有的内存*/
	v->u.s.s = (char*)malloc(len + 1);/*分配内存多出一个字节，用于储存空字符*/
	memcpy(v->u.s.s, s, len);
	v->u.s.s[len] = '\0';
	v->u.s.len = len;
	v->type = LEPT_STRING;
}