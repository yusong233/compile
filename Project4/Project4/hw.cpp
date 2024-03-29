﻿#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include "useType.h"


//重置id
void idInitial() {
	memset(id_name, '\0', sizeof(id_name));
	id_type = NONE;
	id_kind = NONE;
	id_size = 1;
	id_value = 0;
}
//重置func
void funcInitial() {
	memset(func_name, '\0', sizeof(id_name));
	func_type = NONE;
	func_index = -1;
	memset(func_arrValue, 0, sizeof(func_arrValue));
	func_return = FALSE;
	id_before = NULL;
}

void addID() {

	struct symbol* iden;
	iden = (struct symbol*)malloc(sizeof(struct symbol));
	iden->before = NULL;
	iden->next = NULL;
	iden->type = id_type;
	iden->kind = id_kind;
	iden->value = id_value;
	iden->size = id_size;
	strcpy(iden->name, id_name);


	if (id_before == NULL) {
		funcTable[funcTable_index]->begin = iden;
	}
	else {
		id_before->next = iden;
	}
	iden->before = id_before;
	id_before = iden;
	idInitial();
}

int checkRedefinition(struct word* now) { // 检查标识符重定义,TRUE为没有重定义，FALSE为重定义
	int judge = TRUE;
	struct symbol* p;
	if (funcTable_index > 0) {
		p = funcTable[funcTable_index]->begin;
		while (p != NULL) {
			if (strcmp(p->name, id_name) == 0) {
				error(now, b);//名字重定义
				idInitial();
				return FALSE;
			}
			p = p->next;
		}
	}

	p = funcTable[0]->begin;
	while (p != NULL) {
		if (strcmp(p->name, id_name) == 0) {
			error(now, b);//名字重定义
			idInitial();
			return FALSE;
		}
		p = p->next;
	}

	return judge;
}

int checkRedefinition2(struct word* now) { //检查函数重定义，TRUE为没有重定义，FALSE为重定义
	int judge = TRUE;
	int i;
	for (i = funcTable_index; i >= 0; i--) {
		if (strcmp(funcTable[i]->name, func_name) == 0) {
			error(now, b);//名字重定义
			idInitial();
			return FALSE;
		}
	}
	return judge;
}

struct symbol* checkUndefined(struct word* now) {//检查名字未定义，返回NULL为未定义，返回值为对应的symbol
	int judge = FALSE;
	struct symbol* p;
	if (funcTable_index > 0) {
		p = funcTable[funcTable_index]->begin;
		while (p != NULL) {
			if (strcmp(p->name, now->string) == 0) {
				return p;
			}
			p = p->next;
		}
	}

	p = funcTable[0]->begin;
	while (p != NULL) {
		if (strcmp(p->name, now->string) == 0) {
			return p;
		}
		p = p->next;
	}
	if (judge == FALSE) {
		error(now, c);//名字未定义
		return NULL;
	}
}

struct func* checkUndefined2(struct word* now) {//检查函数未定义，返回NULL为未定义，否则返回已定义函数
	int i;
	int judge = FALSE;
	for (i = funcTable_index; i >= 0; i--) {
		if (strcmp(funcTable[i]->name, now->string) == 0) {
			return funcTable[i];
		}
	}
	if (judge == FALSE) {
		error(now, c);//名字未定义
		return NULL;
	}
}

struct word* getSym() {
	struct word* nowWord = it;
	pre = it;
	it = it->next;
	return nowWord;
}

void back() {
	if (it == NULL) {
		it = pre;
	}
	else {
		it = it->before;
		pre = it->before;
	}
}

void error(struct word* word, int index) {

	if (index == k || index == l || index == m) {
		back();
	}
	fprintf(fp2, "%d %c\n", word->colNum, charact[index]);
	if (index == n) {
		back();
	}
}
//<关系运算符>
int relationOp() {
	struct word* now = getSym();
	int type = now->type;
	if (type == LSS || type == LEQ || type == GRE || type == GEQ || type == NEQ || type == EQL) {
		return TRUE;
	}
	else {
		back();
		return FALSE;
	}
}

//<无符号整数>
int unsignedInteger() {
	struct word* nowWord = getSym();
	if (nowWord->type == INTCON) {
		if (strcmp(id_name,"'\0'") != 0) {
			id_value = atoi(nowWord->string);
		}
		return TRUE;
	}
	else {
		back();
		return FALSE;
	}
}

//<整数>
int integer() {
	struct word* nowWord = getSym();
	int type = nowWord->type;
	if (type == MINU || type == PLUS) {
		if (unsignedInteger() == TRUE) {
			if (type == MINU) {
				id_value = -id_value;
			}
			return TRUE;
		}
	}
	else {
		back();
		if (unsignedInteger() == TRUE) {
			return TRUE;
		}
	}
	return FALSE;
}

//<声明头部>
int headState() {
	struct word* did;
	struct word* now = getSym();
	if (now->type == INTTK) {
		func_type = INT;
		if ((did = getSym())->type == IDENFR) {
			strcpy(func_name, did->string);
			checkRedefinition2(did);
			return TRUE;
		}
		else {
			back();//返回did
			back();//返回now
			return FALSE;
		}
	}
	else if (now->type == CHARTK) {
		func_type = CHAR;
		if ((did = getSym())->type == IDENFR) {
			strcpy(func_name, did->string);
			checkRedefinition2(did);
			return TRUE;
		}
		else {
			back();//返回did
			back();//返回now
			return FALSE;
		}
	}
	else {
		back();//返回now
		return FALSE;
	}
}

//<类型标识符>不需要输出
int typeIden() {
	struct word* now = getSym();
	if (now->type == INTTK || now->type == CHARTK) {
		back();
		return TRUE;
	}
	else {
		back();
		return FALSE;
	}
}

//<变量定义>
int varDef() {
	struct word* did, * did1, * did2;
	int judge = FALSE;
	int type;
	if (typeIden() == TRUE) {
		did = getSym();
		type = did->type;

		if ((did1 = getSym())->type == IDENFR) {
			strcpy(id_name, did1->string);
			id_kind = VAR;
			if ((did2 = getSym())->type == SEMICN || did2->type == COMMA) {//int/char v1
				switch (type) {
				case INTTK:
					id_type = INT;
					break;
				case CHARTK:
					id_type = CHAR;
					break;
				default:
					break;
				}
				if (checkRedefinition(did1) == TRUE) {
					addID();
				}
				else idInitial();
				judge = TRUE;
			}
			if (did2->type == LBRACK) {// int/char v1[length]
				switch (type) {
				case INTTK:
					id_type = ARRAY;
					break;
				case CHARTK:
					id_type = STRING;
					break;
				default:
					break;
				}
				if (unsignedInteger() == TRUE) {
					back();
					if ((id_size = atoi((did = getSym())->string)) >= 0) {
						//这个无符号整数已经输出
						if (checkRedefinition(did1) == TRUE) {
							addID();
						}
						else idInitial();
						if ((did = getSym())->type == RBRACK) {
							judge = TRUE;
							while ((did = getSym())->type == COMMA) {
								if ((did1 = getSym())->type == IDENFR) {
									id_kind = VAR;
									strcpy(id_name, did1->string);
									if ((did = getSym())->type == LBRACK) {
										switch (type) {
										case INTTK:
											id_type = ARRAY;
											break;
										case CHARTK:
											id_type = STRING;
											break;
										default:
											break;
										}

										if (unsignedInteger() == TRUE) {
											back();
											if ((id_size = atoi((did = getSym())->string)) >= 0) {
												if (checkRedefinition(did1) == TRUE) {
													addID();
												}
												else idInitial();
												if ((did = getSym())->type == RBRACK) {
													judge = TRUE;
												}
												else {
													error(did, m);//缺少]
													judge = TRUE;
													break;
												}
											}
											else {
												judge = FALSE;
												break;
											}
										}
										else {
											judge = FALSE;
											break;
										}
									}
									else {
										back();
										switch (type) {
										case INTTK:
											id_type = INT;
											break;
										case CHARTK:
											id_type = CHAR;
											break;
										default:
											break;
										}
										if (checkRedefinition(did1) == TRUE) {
											addID();
										}
										else {
											idInitial();
										}
									}
								}
								else {
									judge = FALSE;
									break;
								}
							}
							back();
						}
						else {
							error(did, m);//缺少]
							judge = TRUE;
						}
					}
					else {
						judge = FALSE;
					}
				}
				else {
					judge = FALSE;
				}
			}
			else {
				back();
				if (judge == TRUE) {
					while ((did = getSym())->type == COMMA) {
						if ((did = getSym())->type == IDENFR) {
							id_kind = VAR;
							strcpy(id_name, did->string);
							judge = TRUE;
							if ((did = getSym())->type == LBRACK) {
								switch (type) {
								case INTTK:
									id_type = ARRAY;
									break;
								case CHARTK:
									id_type = STRING;
									break;
								default:
									break;
								}
								if (unsignedInteger() == TRUE) {
									back();
									if ((id_size = atoi((did = getSym())->string)) >= 0) {
										if ((did = getSym())->type == RBRACK) {
											addID();
											judge = TRUE;
										}
										else {
											judge = TRUE;
											error(did, m);//缺少]
											break;
										}
									}
									else {
										judge = FALSE;
										//数组长度<0
										break;
									}
								}
								else {
									judge = FALSE;
									//[]中不是无符号整数
									break;
								}
							}
							else {
								switch (type) {
								case INTTK:
									id_type = INT;
									break;
								case CHARTK:
									id_type = CHAR;
									break;
								default:
									break;
								}
								addID();
								back();
							}
						}
						else {
							judge = FALSE;
							break;
						}
					}
					back();
				}
				else {
					back();
					back();
				}

			}
		}
		else {
			back();
			judge = FALSE;
		}
	}
	else {
		judge = FALSE;
	}
	if (judge == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<变量说明>
int varSpe() {
	struct word* now;
	int judge = FALSE;
	while (varDef() == TRUE) {
		if ((now = getSym())->type == SEMICN) {
			judge = TRUE;
		}
		else {
			error(now, k);//缺少;
			judge = TRUE;
		}
	}
	if (judge == TRUE) {
		return TRUE;
	}
	else
		return FALSE;

}

//<常量定义>
int conDef() {
	struct word* did, * did1;
	int judge = TRUE;
	struct word* now = getSym();
	if (now->type == INTTK) {//const int
		id_type = INT;//type
		id_kind = CONST;//kind
		if ((did1 = getSym())->type == IDENFR) {//const int iden
			strcpy(id_name, did1->string);//name
			if ((did = getSym())->type == ASSIGN) {//const int iden =
				if (integer() == TRUE) {//const int iden = value
					if (checkRedefinition(did1) == TRUE) {
						addID();
					}
					else idInitial();
					while ((did = getSym())->type == COMMA) {//const int iden1 = v1,
						id_type = INT;
						id_kind = CONST;
						if ((did1 = getSym())->type == IDENFR) {
							strcpy(id_name, did1->string);
							if ((did = getSym())->type == ASSIGN) {
								if (integer() == TRUE) {
									if (checkRedefinition(did1) == TRUE) {
										addID();
									}
									else idInitial();
									judge = TRUE;
								}
								else {
									error(did1, o);//常量定义后应该为整型常量
								}
							}
						}
					}
					back();
					if ((did = getSym())->type == SEMICN && judge == TRUE) {
						back();
						return TRUE;
					}
				}
				else {
					error(did1, o);//常量定义后应该为整型常量
					while ((did = getSym())->type == COMMA) {
						id_type = INT;
						id_kind = CONST;
						if ((did1 = getSym())->type == IDENFR) {
							strcpy(id_name, did1->string);
							if ((did = getSym())->type == ASSIGN) {
								if (integer() == TRUE) {
									if (checkRedefinition(did1) == TRUE) {
										addID();
									}
									else idInitial();
									judge = TRUE;
								}
								else {
									error(did1, o);//常量定义后应该为整型常量
									judge = TRUE;
								}
							}
						}
					}
					back();
					if ((did = getSym())->type == SEMICN && judge == TRUE) {
						back();
						return TRUE;
					}
				}
			}
		}
	}
	else if (now->type == CHARTK) {
		id_type = CHAR;
		id_kind = CONST;
		if ((did1 = getSym())->type == IDENFR) {
			strcpy(id_name, did1->string);
			if ((did = getSym())->type == ASSIGN) {
				if ((did = getSym())->type == CHARCON) {
					id_value = did->string[0];
					if (checkRedefinition(did1) == TRUE) {
						addID();
					}
					else idInitial();
					while ((did = getSym())->type == COMMA) {
						id_kind = CONST;
						id_type = CHAR;
						if ((did1 = getSym())->type == IDENFR) {
							strcpy(id_name, did1->string);
							if ((did = getSym())->type == ASSIGN) {
								if ((did = getSym())->type == CHARCON) {
									id_value = did->string[0];
									if (checkRedefinition(did1) == TRUE) {
										addID();
									}
									else idInitial();
									judge = TRUE;
								}
								else {
									error(did1, o);//常量定义后应该为字符型常量
									judge = TRUE;
								}
							}
						}
					}
					back();
					if ((did = getSym())->type == SEMICN && judge == TRUE) {
						back();
						return TRUE;
					}
				}
				else {
					error(did1, o);//常量定义后应该为字符型常量
					while ((did = getSym())->type == COMMA) {
						id_type = CHAR;
						id_kind = CONST;
						if ((did1 = getSym())->type == IDENFR) {
							strcpy(id_name, did1->string);
							if ((did = getSym())->type == ASSIGN) {
								if ((did = getSym())->type == CHARCON) {
									id_value = did->string[0];
									if (checkRedefinition(did1) == TRUE) {
										addID();
									}
									else idInitial();
									judge = TRUE;
								}
								else {
									error(did1, o);//常量定义后应该为字符型常量
								}
							}
						}
					}
					back();
					if ((did = getSym())->type == SEMICN && judge == TRUE) {
						back();
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

//<常量说明>
int conSpe() {
	struct word* now;
	int judge = FALSE;
	while ((now = getSym())->type == CONSTTK) {
		if (conDef() == TRUE) {
			if ((now = getSym())->type == SEMICN) {
				judge = TRUE;
			}
			else {//缺少分号；
				judge = TRUE;
				error(now, k);
			}
		}
		else {
			judge = FALSE;
		}
	}
	back();
	if (judge == TRUE) {
		return TRUE;
	}
	else
		return FALSE;
}

//<因子>
int factor() {
	struct word* did;
	struct word* now = getSym();
	struct symbol* symbol1;
	struct func* symbol2;
	int judge = FALSE;
	nowFactor = INT;
	if (now->type == IDENFR) {
		if ((did = getSym())->type == LBRACK) {
			symbol1 = checkUndefined(now);
			if (symbol1 == NULL) {
				error(now, c);//名字未定义
			}
			if (expre() == TRUE) {
				if (nowExpre != INT) {
					error(did, ii);//数组下标都应为整型表达式
				}
				if ((did = getSym())->type == RBRACK) {//<标识符>[<表达式>]
					if (symbol1 != NULL && symbol1->type == STRING) {
						nowFactor = CHAR;
					}
					judge = TRUE;
					struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
					initialMid(mid);
					strcpy(mid->x, now->string);
					strcpy(mid->y, mid_now->z);
					strcpy(mid->z, mid_now->z);
					mid->op = DEVIATION;
					mid_now->next = mid;
					mid_now = mid;
				}
				else {
					error(did, m);//无]
					judge = TRUE;
				}

			}
			else judge = FALSE;
		}
		else {
			if (did->type == LPARENT) {
				back();
				back();
				symbol2 = checkUndefined2(now);
				if (symbol2 == NULL) {
					error(now, c);//名字未定义
				}
				if (reFuncState() == TRUE) {//有返回值函数调用语句 

					struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
					initialMid(mid);
					strcpy(mid->x, "ret");
					strcpy(mid->z, getVarIndex());
					mid_now->next = mid;
					mid_now = mid;

					judge = TRUE;
				}
			}
			else { //<标识符>
				back();
				symbol1 = checkUndefined(now);
				if (symbol1 == NULL) {
					error(now, c);//名字未定义
				}
				judge = TRUE;
				if (symbol1 != NULL && (symbol1->type == CHAR)) {
					nowFactor = CHAR;
				}

				struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
				initialMid(mid);
				strcpy(mid->x, now->string);
				strcpy(mid->z, getVarIndex());
				mid->op = ASS;
				mid_now->next = mid;
				mid_now = mid;

			}
		}
	}
	else if (now->type == LPARENT) {//(<表达式>)
		if (expre() == TRUE) {
			if (nowExpre == CHAR) {
				nowFactor = INT;
				nowItem = INT;
				nowExpre = INT;
			}
			if ((did = getSym())->type == RPARENT) {
				judge = TRUE;
			}
			else {
				error(did, l);//缺少）
				judge = TRUE;
			}
		}
		else judge = FALSE;
	}
	else {
		back();
		if (integer() == TRUE) {//<整数> 将整数的值存放在id_value中

			char s[1024] = { '\0' };
			sprintf(s, "%d", id_value);
			struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
			initialMid(mid);
			strcpy(mid->x, s);
			strcpy(mid->z, getVarIndex());
			mid->op = ASS;
			mid_now->next = mid;
			mid_now = mid;

			judge = TRUE;
		}
		else if ((now = getSym())->type == CHARCON) {//<字符>

			struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
			initialMid(mid);
			strcpy(mid->x, now->string);
			strcpy(mid->z, getVarIndex());
			mid->op = ASS;
			mid_now->next = mid;
			mid_now = mid;

			nowFactor = CHAR;
			judge = TRUE;
		}
		else {
			back();
			judge = FALSE;
		}
	}
	if (judge == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<项>
int item() {
	struct word* did;
	int judge = FALSE;
	nowItem = INT;
	if (factor() == TRUE) {
		struct mid* mid1, * mid2;
		mid1 = mid_now;
		judge = TRUE;
		while ((did = getSym())->type == MULT || did->type == DIV) {
			if (factor() == TRUE) {
				mid2 = mid_now;

				struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
				initialMid(mid);
				strcpy(mid->x, mid1->z);
				strcpy(mid->y, mid2->z);
				strcpy(mid->z, mid1->z);
				mid->op = did->type;
				mid_now->next = mid;
				mid_now = mid;

				backVarIndex();
				mid1 = mid_now;
				judge = TRUE;
			}
			nowFactor = INT;
		}
		back();
	}
	if (nowFactor == CHAR) {
		nowItem = CHAR;
	}
	if (judge == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<表达式>
int expre() {
	struct word* now = getSym();
	int judge = FALSE;
	nowExpre = INT;
	int add = 0;
	struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
	initialMid(mid);
	if (now->type == PLUS || now->type == MINU) {
		if (now->type == MINU) {
			add = 1;
			mid->op = MINU;
			strcpy(mid->x, "0");
		}
		judge = FALSE;
	}
	else back();
	struct mid* mid1, * mid2;
	if (item() == TRUE) {
		judge = TRUE;
		if (add == 1) {
			strcpy(mid->y, mid_now->z);
			strcpy(mid->z, mid_now->z);
			mid_now->next = mid;
			mid_now = mid;
		}
		mid1 = mid_now;
		while ((now = getSym())->type == PLUS || now->type == MINU) {
			judge = FALSE;
			if (item() == TRUE) {
				mid2 = mid_now;

				struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
				initialMid(mid);
				strcpy(mid->x, mid1->z);
				strcpy(mid->y, mid2->z);
				strcpy(mid->z, mid1->z);
				mid->op = now->type;
				mid_now->next = mid;
				mid_now = mid;

				backVarIndex();

				mid1 = mid_now;
				judge = TRUE;
			}
			else judge = FALSE;
			nowItem = INT;
		}
		back();
	}
	if (nowItem == CHAR) {
		nowExpre = CHAR;
	}
	if (judge == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<读语句>
int readState() {//scanf(标识符{，标识符})
	struct word* now, * did;
	int flag = TRUE;
	if ((now = getSym())->type == SCANFTK) {
		if ((did = getSym())->type == LPARENT) {
			if ((did = getSym())->type == IDENFR) {

				struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
				initialMid(mid);
				strcpy(mid->x, did->string);
				mid->op = READ;
				mid_now->next = mid;
				mid_now = mid;

				checkUndefined(did);
				while ((did = getSym())->type == COMMA) {
					if ((did = getSym())->type == IDENFR) {

						struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
						initialMid(mid);
						strcpy(mid->x, did->string);
						mid->op = READ;
						mid_now->next = mid;
						mid_now = mid;

						checkUndefined(did);
						flag = TRUE;
					}
					else flag = FALSE;
				}
				back();
				if ((did = getSym())->type == RPARENT && flag == TRUE) {
					if (did->type != RPARENT) {
						error(did, l);//缺少）
						flag = TRUE;
					}
					return TRUE;
				}
			}
		}
	}
	else back();
	return FALSE;
}

//<写语句>
int writeState() {
	struct word* now, * did;
	int flag = FALSE;
	if ((now = getSym())->type == PRINTFTK) {
		if ((did = getSym())->type == LPARENT) {
			if ((did = getSym())->type == STRCON) {
				struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
				initialMid(mid);
				strcpy(string[text_index], did->string);
				char s[1024] = "\0";
				sprintf(s, "string%d", text_index);
				strcpy(mid->x, s);
				text_index++;
				mid->op = WRITE;
				
				if ((did = getSym())->type == COMMA) {
					if (expre() == TRUE) {
						if ((did = getSym())->type == RPARENT) {
							strcpy(mid->y, mid_now->z);
							flag = TRUE;
						}
						else {
							error(did, l);//缺少）
							flag = TRUE;
						}
					}
				}
				else if (did->type == RPARENT) {

					flag = TRUE;
				}
				else {
					error(did, l);//缺少）
					flag = TRUE;
				}
				mid_now->next = mid;
				mid_now = mid;
				
			}
			else {
				back();
				if (expre() == TRUE) {

					struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
					initialMid(mid);
					strcpy(mid->x, mid_now->z);
					mid->op = WRITE;
					mid_now->next = mid;
					mid_now = mid;

					if ((did = getSym())->type == RPARENT) {
						flag = TRUE;
					}
					else {
						error(did, l);//缺少）
						flag = TRUE;
					}
				}
			}

		}
	}
	else back();

	if (flag == TRUE) {
		return TRUE;
	}
	return FALSE;
}

//<返回语句>
int returnState() {
	int flag = FALSE;
	struct word* now, * did;
	if ((now = getSym())->type == RETURNTK) {
		flag = TRUE;

		if (func_type == NONE) {
			if (now->next->type != SEMICN) {
				error(now, g);//无返回值函数多余的return语句
			}
		}
		else if (func_type == INT || func_type == CHAR) {
			if (now->next->type != SEMICN) {
				func_return = TRUE;
			}
			else {
				func_return = FALSE;
				return TRUE;
			}
		}

		if ((did = getSym())->type == LPARENT) {
			flag = FALSE;
			if (expre() == TRUE) {
				if (func_type != NONE) {
					if (nowExpre != func_type) {
						error(did, h);//有返回值函数存在不匹配的返回类型
					}
					else {
						func_return = TRUE;
					}
				}
				if ((did = getSym())->type == RPARENT) {
					flag = TRUE;
				}
				else {
					error(did, l);//缺少）
					flag = TRUE;
				}
			}

			struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
			initialMid(mid);
			strcpy(mid->x, mid_now->z);
			mid->op = RET;
			mid_now->next = mid;
			mid_now = mid;

		}
		else back();
	}
	else back();

	if (flag == TRUE) {
		return TRUE;
	}
	return FALSE;
}

//<赋值语句>
int assignState() {
	struct word* now = getSym();
	struct word* did, * did1;
	int flag = FALSE;
	struct symbol* symbol1;
	if (now->type == IDENFR) {
		symbol1 = checkUndefined(now);
		if ((did = getSym())->type == ASSIGN) {
			if (symbol1 != NULL && symbol1->kind == CONST) {
				error(did, j);//不能改变常量的值
			}
			if (expre() == TRUE) {
				struct mid* mid = (struct mid*)malloc(sizeof(struct mid*));
				initialMid(mid);
				strcpy(mid->z, now->string);
				strcpy(mid->x, mid_now->z);
				mid->op = ASS;
				mid_now->next = mid;
				mid_now = mid;
				backVarIndex();//归还expre使用的寄存器
				flag = TRUE;
			}
		}
		else if (did->type == LBRACK) {
			if (expre() == TRUE) {
				if (nowExpre != INT) {
					error(did, ii);//数组下标必须是整型
				}

				struct mid* mid1 = (struct mid*)malloc(sizeof(struct mid));
				initialMid(mid1);
				mid1->op = DEVIATION;
				strcpy(mid1->x, now->string);
				strcpy(mid1->y, mid_now->z);
				strcpy(mid1->z, mid_now->z);
				mid_now->next = mid1;
				mid_now = mid1;

				if ((did1 = getSym())->type == RBRACK) {
					if ((did = getSym())->type == ASSIGN) {
						if (expre() == TRUE) {
							struct mid* mid = (struct mid*)malloc(sizeof(struct mid*));
							initialMid(mid);
							strcpy(mid->z, mid1->z);
							strcpy(mid->x, mid_now->z);
							mid->op = ASS;
							mid_now->next = mid;
							mid_now = mid;
							backVarIndex();//归还expre使用的寄存器
							flag = TRUE;
						}
					}
				}
				else {
					back();
					error(getSym(), m);//缺少]
					if ((did = getSym())->type == ASSIGN) {
						if (expre() == TRUE) {
							flag = TRUE;
						}
					}
					flag = TRUE;
				}
			}
		}
		else flag = FALSE;
	}
	else back();
	if (flag == TRUE) {
		return TRUE;
	}
	else return FALSE;

}

//<条件>
int condition() {
	int type1, type2;
	if (expre() == TRUE) {
		type1 = nowExpre;
		if (relationOp() == TRUE) {
			if (expre() == TRUE) {
				type2 = nowExpre;
				if (type1 == CHAR || type2 == CHAR || type1 != type2) {
					back();
					error(getSym(), f);//条件判断中出现不合法的类型，两个表达式类型必须相同且为整型
				}
				return TRUE;
			}
			else {
				back();
				error(getSym(), f);
				return TRUE;
			}
		}
		else {
			if (type1 != INT) {
				back();
				error(getSym(), f);//条件判断中出现不合法的类型，条件中如果是单个表达式必须是整型
			}
			return TRUE;
		}
	}
	else {
		back();
		error(getSym(), f);
		return TRUE;
	}
	return FALSE;
}

//<条件语句>
int conditionState() {
	struct word* now = getSym();
	struct word* did, * did1;
	int flag = FALSE;
	if (now->type == IFTK) {
		if ((did = getSym())->type == LPARENT) {
			if (condition() == TRUE) {
				if ((did1 = getSym())->type == RPARENT) {
					if (state() == TRUE) {
						if ((did = getSym())->type == ELSETK) {
							if (state() == TRUE) {
								flag = TRUE;
							}
							else {
								flag = FALSE;
							}
						}
						else {
							back();
							flag = TRUE;
						}
					}
				}
				else {
					error(did1, l);//缺少）
					if (state() == TRUE) {
						if ((did = getSym())->type == ELSETK) {
							if (state() == TRUE) {
								flag = TRUE;
							}
							else {
								flag = FALSE;
							}
						}
						else {
							back();
							flag = TRUE;
						}
					}
					flag = TRUE;
				}
			}
		}
	}
	else back();
	if (flag == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<步长>
int step() {
	if (unsignedInteger() == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<循环语句>
int loopState() {
	struct word* now = getSym();
	struct word* did, * did1;
	int flag = FALSE;
	if (now->type == WHILETK) {
		if ((did = getSym())->type == LPARENT) {
			if (condition() == TRUE) {
				if ((did = getSym())->type == RPARENT) {
					if (state() == TRUE) {
						flag = TRUE;
					}
				}
				else {
					error(did, l);//缺少）
					if (state() == TRUE) {
						flag = TRUE;
					}
					flag = TRUE;
				}
			}
		}
	}
	else if (now->type == DOTK) {
		if (state() == TRUE) {
			if ((did1 = getSym())->type == WHILETK) {
				if ((did = getSym())->type == LPARENT) {
					if (condition() == TRUE) {
						if ((did = getSym())->type == RPARENT) {
							flag = TRUE;
						}
						else {
							error(did, l);//缺少）
							flag = TRUE;
						}
					}
				}
			}
			else {
				error(did1, n);//缺少while,报错在当前单词
				flag = TRUE;
				if ((did = getSym())->type == LPARENT) {
					if (condition() == TRUE) {
						if ((did = getSym())->type == RPARENT) {
							flag = TRUE;
						}
						else {
							error(did, l);//缺少）
							flag = TRUE;
						}
					}
				}
			}
		}
	}
	else if (now->type == FORTK) {
		struct symbol* symbol1;
		if ((did = getSym())->type == LPARENT) {
			if ((did = getSym())->type == IDENFR) {
				symbol1 = checkUndefined(did);
				if (symbol1 != NULL && symbol1->kind == CONST) {
					error(did, j);//不能改变常量的值
				}
				if ((did = getSym())->type == ASSIGN) {
					if (expre() == TRUE) {
						if ((did = getSym())->type == SEMICN) {
							if (condition() == TRUE) {
								if ((did = getSym())->type == SEMICN) {
									if ((did = getSym())->type == IDENFR) {
										symbol1 = checkUndefined(did);
										if (symbol1 != NULL && symbol1->kind == CONST) {
											error(did, j);//不能改变常量的值
										}
										if ((did = getSym())->type == ASSIGN) {
											if ((did = getSym())->type == IDENFR) {
												checkUndefined(did);
												if ((did = getSym())->type == PLUS || did->type == MINU) {
													if (step() == TRUE) {
														if ((did = getSym())->type == RPARENT) {
															if (state() == TRUE) {
																flag = TRUE;
															}
														}
														else {
															error(did, l);//缺少）
															if (state() == TRUE) {
																flag = TRUE;
															}
															flag = TRUE;
														}
													}
												}
											}
										}
									}
								}
							}

						}
					}
				}
			}
		}
	}
	else back();

	if (flag == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<值参数表>
int valueParaTable() {
	struct word* did1;
	int flag = FALSE;
	int i = 0;
	if (expre() == TRUE) {
		flag = TRUE;
		if (nowFunc->arrValue[i] != nowExpre) {
			back();
			error(getSym(), e);//函数参数类型不匹配
		}
		i++;
		struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
		initialMid(mid);
		mid->op = PUSH;
		strcpy(mid->x, mid_now->z);
		mid_now->next = mid;
		mid_now = mid;

		while ((did1 = getSym())->type == COMMA) {
			flag = FALSE;
			if (expre() == TRUE) {
				if (nowFunc->arrValue[i] != nowExpre) {
					back();
					error(getSym(), e);//函数参数类型不匹配
				}
				i++;
				struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
				initialMid(mid);
				mid->op = PUSH;
				strcpy(mid->x, mid_now->z);
				mid_now->next = mid;
				mid_now = mid;
				flag = TRUE;
			}
		}
		back();
	}
	else {
		flag = TRUE;
	}
	if (i != nowFunc->valueNum) {
		back();
		error(getSym(), d);//函数参数个数不匹配
	}
	if (flag == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<有返回值函数调用语句>
int reFuncState() {
	struct word* did1, * did2, * did3;
	int judge = FALSE;
	struct func* function;
	if ((did1 = getSym())->type == IDENFR) {
		if ((did2 = getSym())->type == LPARENT) {
			function = checkUndefined2(did1);
			if (function == NULL) {
				error(did1, c);
				judge = TRUE;
			}
			else if (function != NULL && function->type != NONE) {
				judge = TRUE;
				nowFunc = function;
				func_test = TRUE;
				nowFactor = nowFunc->type;
			}
			if (judge == TRUE) {
				if (valueParaTable() == TRUE) {
					if ((did3 = getSym())->type == RPARENT) {

						struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
						initialMid(mid);
						strcpy(mid->x, did1->string);
						mid->op = CALL;
						mid_now->next = mid;
						mid_now = mid;

						return TRUE;
					}
					else {
						error(did3, l);//缺少）
						return TRUE;
					}
				}
			}
			else {
				back();
				back();
			}
		}
		else {
			back();
			back();
		}

	}
	else back();
	return FALSE;
}

//<无返回值函数调用语句>
int nonFuncState() {
	struct word* did1, * did2, * did3;
	int judge = FALSE;
	struct func* function;
	if ((did1 = getSym())->type == IDENFR) {
		if ((did2 = getSym())->type == LPARENT) {
			function = checkUndefined2(did1);
			if (function != NULL) {
				judge = TRUE;
				nowFunc = function;
				func_test = TRUE;
				nowFactor = nowFunc->type;
			}
			else {
				error(did1, c);
				judge = TRUE;
			}
			if (judge == TRUE) {
				if (valueParaTable() == TRUE) {
					if ((did3 = getSym())->type == RPARENT) {
						struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
						initialMid(mid);
						mid->op = CALL;
						strcpy(mid->x, did1->string);//call func_name
						mid_now->next = mid;
						mid_now = mid;
						return TRUE;
					}
					else {
						error(did3, l);//缺少)
						return TRUE;
					}
				}
			}
			else {
				back();
				back();
			}
		}
		else {
			back();
			back();
		}
	}
	else back();

	return FALSE;
}

//<语句>
int state() {
	struct word* now;
	struct word* did;
	int judge = FALSE;

	if (conditionState() == TRUE) {
		judge = TRUE;
	}
	else if (loopState() == TRUE) {
		judge = TRUE;
	}
	else if ((now = getSym())->type == LBRACE) {
		if (stateColumn() == TRUE) {
			if ((did = getSym())->type == RBRACE) {
				judge = TRUE;
			}
		}
	}
	else {
		back();//回退上一个getSym
		if (reFuncState() == TRUE || nonFuncState() == TRUE || assignState() == TRUE) {
			if ((now = getSym())->type == SEMICN) {
				judge = TRUE;
			}
			else {
				error(now->before, k);//缺少；
				judge = TRUE;
			}
		}
		else if (readState() == TRUE || writeState() == TRUE || returnState() == TRUE) {
			if ((now = getSym())->type == SEMICN) {
				judge = TRUE;
			}
			else {
				error(now->before, k);//缺少；
				judge = TRUE;
			}
		}
		else if ((now = getSym())->type == SEMICN) {
			judge = TRUE;
		}
		else {
			back();
			judge = FALSE;
		}
	}

	if (judge == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<语句列>
int stateColumn() {
	while (state() == TRUE);
	return TRUE;
}

//<复合语句>
int comState() {
	conSpe();//常量说明
	varSpe();//变量说明
	if (stateColumn() == TRUE) {
		return TRUE;
	}
	return FALSE;
}

//<参数表>
int paraTable() {
	struct word* did;
	int judge = FALSE;
	int type;
	if (typeIden() == TRUE) {
		type = getSym()->type;
		id_kind = PARA;
		struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
		initialMid(mid);
		mid_now->next = mid;
		mid_now = mid;
		switch (type)
		{
		case INTTK:
			func_index++;
			func_arrValue[func_index] = INT;
			id_type = INT;
			strcpy(mid_now->x, "para int ");
			break;
		case CHARTK:
			func_index++;
			func_arrValue[func_index] = CHAR;
			id_type = CHAR;
			strcpy(mid_now->x, "para char ");
			break;
		default:
			break;
		}
		if ((did = getSym())->type == IDENFR) {
			judge = TRUE;
			strcpy(id_name, did->string);
			strcat(mid_now->x, id_name);
			addID();
			strcat(mid_now->x, id_name);
			while ((did = getSym())->type == COMMA) {
				if (typeIden() == TRUE) {
					type = getSym()->type;
					id_kind = PARA;
					struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
					initialMid(mid);
					mid_now->next = mid;
					mid_now = mid;
					switch (type)
					{
					case INTTK:
						func_index++;
						func_arrValue[func_index] = INT;
						id_type = INT;
						strcpy(mid_now->x, "para int ");
						break;
					case CHARTK:
						func_index++;
						func_arrValue[func_index] = CHAR;
						id_type = CHAR;
						strcpy(mid_now->x, "para char ");
						break;
					default:
						break;
					}

					if ((did = getSym())->type == IDENFR) {
						judge = TRUE;
						strcpy(id_name, did->string);
						strcat(mid_now->x, id_name);
						addID();
					}
					else {
						judge = FALSE;
						break;
					}
				}
				else {
					judge = FALSE;
					break;
				}
			}
			back();
		}
		else {
			judge = FALSE;
		}
	}
	else judge = TRUE;//参数表为<空>

	if (judge == TRUE) {
		return TRUE;
	}
	else return FALSE;
}

//<有返回值函数定义>
int reFunc() {
	struct word* did;
	if (headState() == TRUE) { // headState（）只有有返回值函数定义使用
		if ((did = getSym())->type == LPARENT) {
			/* addFunction()  begin*/
			struct func* function = (struct func*)malloc(sizeof(struct func));
			strcpy(function->name, func_name);
			function->begin = NULL;
			function->end = NULL;
			function->type = func_type;
			if (checkRedefinition2(did) == TRUE) {
				funcTable_index++;
				funcTable[funcTable_index] = function;
			}
			else {
				error(did, b);//重定义
			}
			/* addFunction()  end*/

			struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
			initialMid(mid);
			mid_now->next = mid;
			mid_now = mid;
			mid_now->op = FUNC;
			//type name()
			char s[1024] = { '\0' };
			switch (func_type) {
			case INT:
				strcpy(mid_now->y, "int");
				strcpy(mid_now->x, func_name);
				break;
			case CHAR:
				strcpy(mid_now->y, "char");
				strcpy(mid_now->x, func_name);
				break;
			default:
				break;
			}

			if (paraTable() == TRUE) {

				funcTable[funcTable_index]->valueNum = func_index + 1;
				memcpy(funcTable[funcTable_index]->arrValue, func_arrValue, sizeof(func_arrValue));

				if ((did = getSym())->type == RPARENT) {
					if ((did = getSym())->type == LBRACE) {
						if (comState() == TRUE) {
							if ((did = getSym())->type == RBRACE) {
								if (func_return == FALSE) {
									error(did, h);//有返回值函数缺少return语句，报错在当前函数}位置
								}
								funcTable[funcTable_index]->end = id_before;
								funcInitial();
								return TRUE;
							}
						}
					}
				}
				else {
					error(did, l);//缺少)
					if ((did = getSym())->type == LBRACE) {
						if (comState() == TRUE) {
							if ((did = getSym())->type == RBRACE) {
								if (func_return == FALSE) {
									error(did, h);//有返回值函数缺少return语句，报错在当前函数}位置
								}
								funcTable[funcTable_index]->end = id_before;
								funcInitial();
								return TRUE;
							}
						}
					}
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

//<无返回值函数定义>
int nonFunc() {
	struct word* did;
	struct word* now = getSym();
	if (now->type == VOIDTK) {
		func_type = NONE;
		if ((did = getSym())->type == IDENFR) {
			strcpy(func_name, did->string);
			if ((did = getSym())->type == LPARENT) {
				/* addFunction() begin*/
				struct func* function = (struct func*)malloc(sizeof(struct func));
				strcpy(function->name, func_name);
				function->begin = NULL;
				function->end = NULL;
				function->type = func_type;
				if (checkRedefinition2(did) == TRUE) {
					funcTable_index++;
					funcTable[funcTable_index] = function;
				}
				else {
					error(did, b);//重定义
				}

				struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
				initialMid(mid);
				mid_now->next = mid;
				mid_now = mid;
				//void name()
				strcpy(mid_now->y, "void");
				strcpy(mid_now->x, func_name);
				mid_now->op = FUNC;
				/* addFunction() end*/
				if (paraTable() == TRUE) {
					funcTable[funcTable_index]->valueNum = func_index + 1;
					memcpy(funcTable[funcTable_index]->arrValue, func_arrValue,
						sizeof(func_arrValue));
					if ((did = getSym())->type == RPARENT) {
						if ((did = getSym())->type == LBRACE) {
							if (comState() == TRUE) {
								if ((did = getSym())->type == RBRACE) {
									funcTable[funcTable_index]->end = id_before;
									funcInitial();
									return TRUE;
								}
							}
						}
					}
					else {
						error(did, l);//缺少）
						if ((did = getSym())->type == LBRACE) {
							if (comState() == TRUE) {
								if ((did = getSym())->type == RBRACE) {
									funcTable[funcTable_index]->end = id_before;
									funcInitial();
									return TRUE;
								}
							}
						}
						return TRUE;
					}
				}
			}
		}
		else {
			back();
			back();
		}
	}
	else back();
	return FALSE;
}

//<主函数>
int mainFunc() {
	struct word* did;
	struct word* nowWord = getSym();
	if (nowWord->type == VOIDTK) {
		if ((did = getSym())->type == MAINTK) {
			struct mid* mid = (struct mid*)malloc(sizeof(struct mid));
			initialMid(mid);
			mid_now->next = mid;
			mid_now = mid;
			strcpy(mid_now->x, "main");
			strcpy(mid_now->y, "void");
			mid_now->op = FUNC;
			if ((did = getSym())->type == LPARENT) {
				if ((did = getSym())->type == RPARENT) {
					struct func* mainFunc = (struct func*)malloc(sizeof(struct func));
					mainFunc->begin = NULL;
					mainFunc->end = NULL;
					strcpy(mainFunc->name, "main");
					mainFunc->type = NONE;
					funcTable_index++;
					funcTable[funcTable_index] = mainFunc;

					if ((did = getSym())->type == LBRACE) {
						if (comState() == TRUE) {
							if ((did = getSym())->type == RBRACE) {
								funcTable[funcTable_index]->end = id_before;
								return TRUE;
							}
						}
					}
				}
				else {
					error(did, l);//无)
					if ((did = getSym())->type == LBRACE) {
						if (comState() == TRUE) {
							if ((did = getSym())->type == RBRACE) {
								funcTable[funcTable_index]->end = id_before;
								return TRUE;
							}
						}
					}
					return TRUE;
				}
			}
		}
	}
	else {
		back();
	}
	return FALSE;

}

//<程序>
void program() {
	it = head;
	int flag = FALSE;

	struct func* func0 = (struct func*)malloc(sizeof(struct func));//全局
	func0->begin = NULL;
	func0->end = NULL;
	strcpy(func0->name, "main0");
	func0->type = NONE;
	funcTable_index++;
	funcTable[funcTable_index] = func0;//funcTable[0]是全局

	midCode = (struct mid*)malloc(sizeof(struct mid));
	initialMid(midCode);
	mid_now = midCode;
	conSpe();
	varSpe();
	funcTable[0]->end = id_before;
	funcInitial();

	while (reFunc() == TRUE || nonFunc() == TRUE);
	if (mainFunc() == TRUE) {
		flag = TRUE;
	}
}

void initialLink() {
	head = (struct word*)malloc(sizeof(struct word));
	head->before = NULL;
	pre = head;
}

void addType(int type) {
	struct word* one;
	one = (struct word*)malloc(sizeof(struct word));
	one->type = type;
	strcpy(one->string, text[type]);

	one->before = pre;
	pre->next = one;
	pre = one;
}

void addType2(int type, char* str) {
	struct word* one;
	one = (struct word*) malloc(sizeof(struct word));
	one->type = type;
	one->colNum = numCol;
	one->before = NULL;
	one->next = NULL;
	strcpy(one->string, str);
	if (type == CHARCON) {
		char c = str[0];
		if ((c >= 47 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == 42 || c == 43 || c == 45 || c == 95) {

		}
	}

	one->before = pre;
	pre->next = one;
	pre = one;
}

int testINTCON(char* str, int i) {
	int j;
	int judge = 1;
	for (j = 0; j < i; j++) {
		if (str[j] < 48 || str[j] > 57) {
			judge = 0;
			break;
		}

	}
	return judge;

}

void testKeyWords(char* str, int i) {
	if (str[0] == 'c' && str[1] == 'o' && str[2] == 'n' && str[3] == 's' && str[4] == 't' && i == 5) {
		addType(CONSTTK);
	}
	else if (str[0] == 'i' && str[1] == 'n' && str[2] == 't' && i == 3) {
		addType(INTTK);
	}
	else if (str[0] == 'c' && str[1] == 'h' && str[2] == 'a' && str[3] == 'r' && i == 4) {
		addType(CHARTK);
	}
	else if (str[0] == 'v' && str[1] == 'o' && str[2] == 'i' && str[3] == 'd' && i == 4) {
		addType(VOIDTK);
	}
	else if (str[0] == 'm' && str[1] == 'a' && str[2] == 'i' && str[3] == 'n' && i == 4) {
		addType(MAINTK);
	}
	else if (str[0] == 'i' && str[1] == 'f' && i == 2) {
		addType(IFTK);
	}
	else if (str[0] == 'e' && str[1] == 'l' && str[2] == 's' && str[3] == 'e' && i == 4) {
		addType(ELSETK);
	}
	else if (str[0] == 'd' && str[1] == 'o' && i == 2) {
		addType(DOTK);
	}
	else if (str[0] == 'w' && str[1] == 'h' && str[2] == 'i' && str[3] == 'l' && str[4] == 'e' && i == 5) {
		addType(WHILETK);
	}
	else if (str[0] == 'f' && str[1] == 'o' && str[2] == 'r' && i == 3) {
		addType(FORTK);
	}
	else if (str[0] == 's' && str[1] == 'c' && str[2] == 'a' && str[3] == 'n' && str[4] == 'f' && i == 5) {
		addType(SCANFTK);
	}
	else if (str[0] == 'p' && str[1] == 'r' && str[2] == 'i' && str[3] == 'n' && str[4] == 't' && str[5] == 'f' && i == 6) {
		addType(PRINTFTK);
	}
	else if (str[0] == 'r' && str[1] == 'e' && str[2] == 't' && str[3] == 'u' && str[4] == 'r' && str[5] == 'n' && i == 6) {
		addType(RETURNTK);
	}
	else {
		if (i > 0) {
			if (testINTCON(str, i) == 1) {
				addType2(INTCON, str);
			}
			else {
				addType2(IDENFR, str);
			}
		}
	}
}

void test(char* str, int i) {
	testKeyWords(str, i);
	memset(str, '\0', sizeof(char) * 1024);
}

int testExit(char ch) {

	if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == ';'
		|| ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']'
		|| ch == '{' || ch == '}') {
		return 1;
	}
	return 0;
}

void testSingleSymbol(char ch) {
	switch (ch)
	{
	case '+':
		addType(PLUS);
		break;
	case '-':
		addType(MINU);
		break;
	case '*':
		addType(MULT);
		break;
	case '/':
		addType(DIV);
		break;
	case ';':
		addType(SEMICN);
		break;
	case ',':
		addType(COMMA);
		break;
	case '(':
		addType(LPARENT);
		break;
	case ')':
		addType(RPARENT);
		break;
	case '[':
		addType(LBRACK);
		break;
	case ']':
		addType(RBRACK);
		break;
	case '{':
		addType(LBRACE);
		break;
	case '}':
		addType(RBRACE);
		break;
	default:
		break;
	}
}

void readChar() {
	char ch1;
	char str[2] = { '\0' };
	if ((ch1 = fgetc(fp1)) != EOF) {
		str[0] = ch1;
		addType2(CHARCON, str);
	}
	if ((ch1 = fgetc(fp1)) != EOF) {//读字符常量的第二个'
		return;
	}
}

void readString() {
	char ch1;
	char str[1024] = { '\0' };
	int i = 0;
	while ((ch1 = fgetc(fp1)) != EOF) {
		if (ch1 == '\"') break;
		str[i] = ch1;
		i++;
	}
	addType2(STRCON, str);
}

int testDoubleSymbol(char ch, char* arr, int i) {
	char ch1 = '\0';
	if (i > 0) {
		test(arr, i);
		i = 0;
	}
	if ((ch1 = fgetc(fp1)) != EOF) {
		if (ch1 == '=') {
			switch (ch)
			{
			case '>':
				addType(GEQ);
				break;
			case '=':
				addType(EQL);
				break;
			case '<':
				addType(LEQ);
				break;
			case '!':
				addType(NEQ);
				break;
			default:
				break;
			}
		}
		else {
			switch (ch)
			{
			case '>':
				addType(GRE);
				break;
			case '=':
				addType(ASSIGN);
				break;
			case '<':
				addType(LSS);
				break;
			default:
				break;
			}

			if (isspace(ch1) || testExit(ch1) == 1) {
				i = 0;
				if (testExit(ch1) == 1) testSingleSymbol(ch1);
			}
			else if (ch1 == '\'') {
				readChar();
			}
			else if (ch1 == '\"') {
				readString();
			}
			else {
				arr[i] = ch1;
				i++;
			}
		}
	}
	return i;
}

void readWord() {
	char ch = '*';
	char string[1024] = { '\0' };
	int i = 0;
	while ((ch = fgetc(fp1)) != EOF) {
		if (isspace(ch) || testExit(ch) == 1) {
			test(string, i);
			i = 0;
			if (testExit(ch) == 1) testSingleSymbol(ch);
			if (ch == '\n') numCol++;
		}
		else if (ch == '\'') {
			readChar();
		}
		else if (ch == '"') {
			readString();
		}
		else if (ch == '=' || ch == '<' || ch == '>' || ch == '!') {
			i = testDoubleSymbol(ch, string, i);
		}
		else {
			string[i] = ch;
			i++;
		}
	}

	pre->next = NULL;
	head = head->next;
	head->before = NULL;
}

void printTable() {
	struct mid* m = midCode;
	while (m != NULL) {
		if (m->op == NONE && strlen(m->z) != 0) {
			printf("%s = %s\n", m->z, m->x);
		}
		else if (m->op == NONE && strlen(m->z) == 0) {
			printf("%s\n", m->x);
		}
		else if (m->op != NONE && strlen(m->z) == 0 && strlen(m->y) == 0) {
			printf("%s %s\n", text[m->op], m->x);
		}
		else if (m->op != NONE && strlen(m->z) == 0) {
			printf("%s %s %s\n", text[m->op], m->x, m->y);
		}
		else {
			printf("%s = %s %s %s\n", m->z, m->x, text[m->op], m->y);
		}
		m = m->next;
	}

}

void wholeMake() {
	struct func* func0 = funcTable[0];
	struct symbol* b = func0->begin;
	fprintf(fp2, ".data\n");
	while (b != NULL) {
		if (b->kind == CONST) {
			if (b->type == CHAR)
				fprintf(fp2, "%s: .word '%c'\n", b->name, (char)b->value);
			else
				fprintf(fp2, "%s: .word %d\n", b->name, b->value);
		}
		else {
			if (b->type == INT || b->type == CHAR)
				fprintf(fp2, "%s: .space 4\n", b->name);
			else if (b->type == ARRAY || b->type == STRING)
				fprintf(fp2, "%s: .space %d\n", b->name, b->size * 4);
		}
		b = b->next;
	}
	int i;
	for (i = 0; i < text_index; i++) {
		fprintf(fp2, "string%d : .asciiz \"%s\"\n", i, string[i]);
	}
	fprintf(fp2, "huanhang : .asciiz \"\\n\"\n");
}

void funcMake() {
	int i;
	struct func* func = funcTable[0];
	for (i = 0; i <= funcTable_index; i++) {
		if (strcmp(mid_now->x, funcTable[i]->name) == 0) {
			func = funcTable[i];
			fprintf(fp2, "%s:\n", func->name);
			break;
		}
	}

	struct symbol* symbol = func->begin;
	while (symbol != NULL) {
		switch (symbol->type) {
		case INT:
			symbol->address = sp;
			sp = sp - 4; 
			break;
		case CHAR:
			symbol->address = sp;
			sp = sp - 4;
			break;
		case STRING:
			sp = sp - 4 * (symbol->size - 1);
			symbol->address = sp;
			sp = sp - 4;
			break;
		case ARRAY:
			sp = sp - 4 * (symbol->size - 1);
			symbol->address = sp;
			sp = sp - 4;
			break;
		default: break;
		}
		if (symbol->kind == CONST) {
			fprintf(fp2, "li $8,%d\n", symbol->value);
			fprintf(fp2, "sw $8,%d\n", symbol->address);
		}
		symbol = symbol->next;
	}
	mid_now = mid_now->next;
	while (1) {
		if (mid_now == NULL) {
			break;
		}
		stateMake(mid_now, func);
		mid_now = mid_now->next;
		if (mid_now == NULL || mid_now->op == RET) {
			break;
		}
	}
}

struct symbol* findWhole(struct mid* mid, int i) {
	struct func* func = funcTable[0];
	struct symbol* p = func->begin;
	char s[1024] = "\0";
	switch (i) {
	case 1:
		strcpy(s, mid->x);
		break;
	case 2:
		strcpy(s, mid->y);
		break;
	case 3:
		strcpy(s, mid->z);
		break;
	default: break;
	}
	while (p != NULL) {
		if (strcmp(s, p->name) == 0) {
			return p;
		}
		p = p->next;
	}
	return NULL;
}

struct symbol* findFunc(struct func* func, struct mid* mid, int i) {
	struct symbol* p = func->begin;
	char s[1024] = "\0";
	switch (i) {
	case 1:
		strcpy(s, mid->x);
		break;
	case 2:
		strcpy(s, mid->y);
		break;
	case 3:
		strcpy(s, mid->z);
		break;
	default: break;
	}
	while (p!= NULL) {
		if (strcmp(s, p->name) == 0) {
			return p;
		}
		p = p->next;
	}
	return NULL;
}

void stateMake(struct mid* mid, struct func* func) {
	struct symbol* sym;
	switch (mid->op) {
	case ASS:
		if (mid->z[0] == '$') { //#8 = iden;
			if ((sym = findWhole(mid, 1)) != NULL) {
				fprintf(fp2, "lw %s, %s\n", mid->z, sym->name);
			}
			else if ((sym = findFunc(func, mid, 1)) != NULL) {
				fprintf(fp2, "lw %s, %d\n", mid->z,sym->address);
			}
			else {
				fprintf(fp2, "li %s,%s\n", mid->z, mid->x);
			}
			
		}
		else {//#iden = #8;
			if ((sym = findWhole(mid, 3)) != NULL) {
				fprintf(fp2, "sw %s, %s\n", mid->x, sym->name);
			}
			else if ((sym = findFunc(func, mid, 3)) != NULL) {
				fprintf(fp2, "sw %s, %d\n", mid->x, sym->address);
			}
		}
		break;
	case PLUS:
		fprintf(fp2, "add %s,%s,%s\n", mid->z, mid->x, mid->y);
		break;
	case MINU:
		fprintf(fp2, "sub %s,%s,%s\n", mid->z, mid->x, mid->y);
		break;
	case MULT:
		fprintf(fp2, "mul %s,%s,%s,\n", mid->z, mid->x, mid->y);
		break;
	case DIV:
		fprintf(fp2, "div %s,%s,%s,\n", mid->z, mid->x, mid->y);
		break;
	case DEVIATION:
		if ((sym = findWhole(mid, 1)) != NULL) {
			fprintf(fp2, "la %s, %s\n", mid->z, sym->name);
		}
		else if ((sym = findFunc(func, mid, 1)) != NULL) {
			fprintf(fp2, "li %s, %d\n", mid->z, sym->address);
		}
		fprintf(fp2, "move $25,%s\n", mid->y);
		fprintf(fp2, "sll $25,$25,2\n");
		fprintf(fp2, "add %s,%s,$25\n", mid->z, mid->z);
		break;
	case READ:
		fprintf(fp2, "li $v0,5\n");
		fprintf(fp2, "syscall\n");
		if ((sym = findWhole(mid, 1)) != NULL) {
			fprintf(fp2, "sw $v0, %s\n", sym->name);
		}
		else if ((sym = findFunc(func, mid, 1)) != NULL) {
			fprintf(fp2, "sw $v0, %d\n", sym->address);
		}
		break;
	case WRITE:
		
		if (mid->x[0] == '$') {
			fprintf(fp2, "li $v0,1\n");
			fprintf(fp2, "move $a0,%s\n", mid->x);
			fprintf(fp2, "syscall\n");
			fprintf(fp2, "li $v0,4\n");
			fprintf(fp2, "la $a0,huanhang\n");
			fprintf(fp2, "syscall\n");
		}
		else {
			fprintf(fp2, "li $v0,4\n");
			fprintf(fp2, "la $a0,%s\n", mid->x);
			fprintf(fp2, "syscall\n");
			if (strcmp(mid->y, "") != 0) {
				if (mid->y[0] == '$') {
					fprintf(fp2, "li $v0,1\n");
					fprintf(fp2, "move $a0,%s\n", mid->y);
					fprintf(fp2, "syscall\n");
					fprintf(fp2, "li $v0,4\n");
					fprintf(fp2, "la $a0,huanhang\n");
					fprintf(fp2, "syscall\n");
				}
			}
			else {
				fprintf(fp2, "li $v0,4\n");
				fprintf(fp2, "la $a0,huanhang\n");
				fprintf(fp2, "syscall\n");
			}
		}
		break;
	default: break;
	}

}

void finalCode() {
	wholeMake();
	fprintf(fp2, ".text\n");
	fprintf(fp2, "j main\n");
	mid_now = midCode;
	while (mid_now != NULL) {
		funcMake();
	}
}

int main() {

	if ((fp1 = fopen("testfile.txt", "r")) == NULL) {
		printf("File cannot be opened\n");
	}
	if ((fp2 = fopen("mips.txt", "w+")) == NULL) {
		printf("File cannot be opened\n");
	}
	initialLink();
	readWord();

	program();
	midCode = midCode->next;
	printTable();
	finalCode();
	
	fclose(fp1);
	fclose(fp2);

	return 0;

}