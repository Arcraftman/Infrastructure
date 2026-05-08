#include "stk_string.h"

STK_API void StrInit(String* s) {
	if (!s) return;
	s->data = NULL;
	s->length = 0;
}

STK_API void StrInitFrom(String* s, const char* cstr) {
	if (!s) return;

	if (!cstr || cstr[0] == '\0') {
		s->data = NULL;
		s->length = 0;
		return;
	}

	s->length = strlen(cstr);
	s->data = (char*)malloc(s->length + 1);
	if (s->data) {
		strcpy(s->data, cstr);  
	}
	else {
		s->length = 0;
	}
}

STK_API void StrDestroy(String* s) {
	if (!s) return;
	if (s->data) {
		free(s->data);
		s->data = NULL;
	}
	s->length = 0;
}

STK_API const char* StrCStr(const String* s) {
	return s && s->data ? s->data : "";
}

STK_API size_t StrLength(const String* s) {
	return s ? s->length : 0;
}

STK_API int StrEmpty(const String* s) {
	return s ? s->length == 0 : 1;
}

STK_API void StrClear(String* s) {
	if (!s) return;
	if (s->data) {
		s->data[0] = '\0';
	}
	s->length = 0;
}

STK_API void StrPushBack(String* s, char ch) {
	if (!s) return;

	char* new_data = (char*)realloc(s->data, s->length + 2);
	if (!new_data) return;

	s->data = new_data;
	s->data[s->length++] = ch;
	s->data[s->length] = '\0';
}

STK_API void StrPopBack(String* s) {
	if (!s || s->length == 0) return;

	s->length--;
	s->data[s->length] = '\0';

	if (s->length == 0) {
		free(s->data);
		s->data = NULL;
	}
}

STK_API void StrAppend(String* s, const char* cstr) {
	if (!s || !cstr) return;

	size_t len = strlen(cstr);
	if (len == 0) return;

	char* new_data = (char*)realloc(s->data, s->length + len + 1);
	if (!new_data) return;

	s->data = new_data;
	memcpy(s->data + s->length, cstr, len + 1);
	s->length += len;
}

STK_API void StrAppendString(String* s, const String* other) {
	if (!s || !other) return;
	StrAppend(s, other->data);
}

STK_API void StrAssign(String* s, const char* cstr) {
	if (!s) return;

	StrClear(s);
	if (!cstr || cstr[0] == '\0') return;

	size_t len = strlen(cstr);
	char* new_data = (char*)realloc(s->data, len + 1);
	if (!new_data) return;

	s->data = new_data;
	strcpy(s->data, cstr);  
	s->length = len;
}

STK_API void StrAssignString(String* s, const String* other) {
	if (!s || !other) return;
	StrAssign(s, other->data);
}

STK_API int StrFindChar(const String* s, char ch, size_t start) {
	if (!s || !s->data || start >= s->length) return -1;

	for (size_t i = start; i < s->length; i++) {
		if (s->data[i] == ch) return (int)i;
	}
	return -1;
}

STK_API int StrFind(const String* s, const char* substr, size_t start) {
	if (!s || !s->data || !substr || start >= s->length) return -1;

	char* pos = strstr(s->data + start, substr);
	if (!pos) return -1;

	return (int)(pos - s->data);
}

STK_API int StrCompare(const String* s1, const String* s2) {
	if (!s1 && !s2) return 0;
	if (!s1) return -1;
	if (!s2) return 1;

	const char* str1 = s1->data ? s1->data : "";
	const char* str2 = s2->data ? s2->data : "";
	return strcmp(str1, str2);
}

STK_API int StrCompareCStr(const String* s, const char* cstr) {
	if (!s && !cstr) return 0;
	if (!s) return -1;
	if (!cstr) return 1;

	const char* str = s->data ? s->data : "";
	return strcmp(str, cstr);
}

STK_API void StrToUpper(String* s) {
	if (!s || !s->data) return;

	for (size_t i = 0; i < s->length; i++) {
		s->data[i] = (char)toupper((unsigned char)s->data[i]);
	}
}

STK_API void StrToLower(String* s) {
	if (!s || !s->data) return;

	for (size_t i = 0; i < s->length; i++) {
		s->data[i] = (char)tolower((unsigned char)s->data[i]);
	}
}

STK_API String* StrSubstring(const String* s, size_t start, size_t end) {
	String* result = (String*)malloc(sizeof(String));
	if (!result) return NULL;
	StrInit(result);

	if (!s || !s->data || start >= s->length || end <= start) {
		return result;
	}

	if (end > s->length) end = s->length;

	size_t len = end - start;
	result->data = (char*)malloc(len + 1);
	if (!result->data) {
		free(result);
		return NULL;
	}

	memcpy(result->data, s->data + start, len);
	result->data[len] = '\0';
	result->length = len;

	return result;
}

STK_API void StrPrint(const String* s) {
	if (s && s->data) {
		printf("%s", s->data);
	}
}

STK_API void StrPrintln(const String* s) {
	StrPrint(s);
	printf("\n");
}

STK_API void StrDeepCopy(String* dest, const String* src) {
	if (!dest || !src) return;
	StrInitFrom(dest, src->data);
}