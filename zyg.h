#ifndef DEFINED
#include <iostream>
#include<string>
#include<fstream>
using namespace std;

/*******************枚举声明************************/
enum LexType{
    ID, IF, BEGIN, INTC, END, PLUS, MINUS, TIMES, OVER, EQ, LT, LMIDPAREN, RMIDPAREN, DOT
    , TYPE, VAR, PROCEDURE, PROGRAM, SEMI, INTEGER, CHAR, ARRAY, RECORD, UNDERANGE
    , OF, COMMA, LPAREN, RPAREN, ENDWH, WHILE, RETURN, READ, WRITE, ASSIGN, THEN, FI
    , ELSE, DO
};
struct Token
{
    int linsShow;		//记录单词行号
    LexType lex;		//记录该单词的词法信息，其中LexType为单词的类型枚举
    string sem;		//记录该单词的语义信息
};
#define DEFINED
#endif
enum state
{
    START, INASSIGN, INCOMMENT, INNUM, INID, INCHAR, INRANGE, DONE, FINISH
};
/**
 *  START: 表示词法分析器的起始状态。 "START" 阶段是词法分析器的初始状态。在这个阶段，词法分析器还没有进入任何特定的词法状态，它等待读取输入字符并根据输入的字符决定下一步的处理。
    INASSIGN: 表示正在处理赋值操作符 := 的状态。
    INCOMMENT: 表示正在处理注释的状态。
    INNUM: 表示正在处理数字的状态。
    INID: 表示正在处理标识符（变量名）的状态。
    INCHAR: 表示正在处理字符的状态。
    INRANGE: 表示正在处理范围操作符 .. 的状态。
    DONE: 表示已完成词法分析，准备生成词法单元。
    FINISH: 表示词法分析的终止状态。
 */


/*******************函数声明************************/
void GetNextChar();

void InputError(string errorInfo, string path);
