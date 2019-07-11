#ifndef LEPTJSON_H__//incldue�����ظ������LEPTJSON_H__û�ж��壬��ִ��ֱ��#endif֮��Ĵ���
#define LEPTJSON_H__

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;   //ö��

typedef struct {  //��ʾһ���ڵ�
	double n;//ѡ��double������json���֣�����type==LEPT_NUMBERʱ��n�ű�ʾJSON���ֵ���ֵ
    lept_type type;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG
};

int lept_parse(lept_value* v, const char* json);  //������������������vΪ���ڵ㣬jsonΪjson�ı���c�����ַ�����

lept_type lept_get_type(const lept_value* v);

double lept_get_number(const lept_value* v);

#endif /* LEPTJSON_H__ */
