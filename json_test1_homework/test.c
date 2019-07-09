
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;   //�ֲ���̬����static
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)
//ע�⣺�˴���format��EXPECT_EQ_INT���Ǵ����ŵ�""��printf�п����ж�����š�
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

static void test_parse_null() {   //���null�ı�
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));//�����û�аѸ��ڵ�����Ϊnull
}

static void test_parse_true() {   //���true�ı�
	lept_value v;
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));//�����û�аѸ��ڵ�����Ϊtrue

}

static void test_parse_false() {    //���false�ı�
	lept_value v;
	v.type = LEPT_TRUE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));//�����û�аѸ��ڵ�����Ϊfalse
}

static void test_parse_expect_value() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));//һ��jsonֻ���пհף�����LEPT_PARSE_EXPECT_VALUE
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));//��lept_parse�У������ڵ���������ΪNULL
	
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, " "));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}//�ò���ͨ��

static void test_parse_invalid_value() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));//����NULL��FALSE��TRUE������LEPT_PARSE_INVALID_VALUE
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));//��lept_parse�У������ڵ���������ΪNULL

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "?"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}//�ò���ͨ��

static void test_parse_root_not_singular() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));//��ws value ws���������ַ�������LEPT_PARSE_ROOT_NOT_SINGULAR���ò��Բ�ͨ��
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));//�ò���ͨ��
}


static void test_parse() {
    test_parse_null();
	test_parse_true();   //�ڲ��Ժ����е���test_parse_null��test_parse_true
	test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}
