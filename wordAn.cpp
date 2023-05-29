// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
#include"zyg.h"

string arrayLexType_zyg[] =
        {"id", "if", "begin", "intc", "end", "+", "-", "*", "/", "=", "<", "[", "]", ".", "type",
                             "var", "procedure", "program", ";", "integer", "char", "array", "record", "..", "of", ",",
                             "(", ")", "endwh", "while", "return", "read", "write", ":=", "then", "fi", "else", "do"

};

//id, if, begin, intc, end, plus, minus, times, over, eq, lt, lmidparen, rmidparen, dot
//, type, var, procedure, program, semi, integer, char, array, record, underange
//, of, comma, lparen, rparen, endwh, while, return, read, write, assign, then, fi
//, else, do
// 结果保存到token.txt中，其中三个字段分别为行号，词法类型，语义信息

void GetNextChar()
{
    //string imbuffer;
    string query; // 当前读取的一行
    int linenum = 0; // 行号
    state sta = START; // 状态
    int longquery; // 一行的长度
    int i = 0; // 一行的下标
    Token tok; // 词法单元，一个tok对应一个词法单元
    //ifstream fin("s5.txt");
    ifstream fin("semanticError/e7.txt");
    ofstream fout("token.txt");

    if (!fin && !fout)
    {
        string a = "文件不能打开";
        InputError(a, "error.txt");
        exit(0);
    }
    else
    {
        while ((getline(fin, query))) // 对于每一行进行处理
        {
            linenum++;
            /*if (linenum == 8)
            {
                cout << "debug" << endl;
            }*/
            i = 0; // 追踪当前处理的字符在行中的位置。
            string imbuffer; // 暂存正在处理的单词或字符。
            //cout << query<<endl;
            longquery = size(query); // 获取当前行 query 的长度
            //cout << longquery << endl;
            if (sta == FINISH) // 如果当前状态为 FINISH，则说明词法分析已经结束，可以退出循环。
            {
                break;
            }
            while (i < longquery) // 如果当前行还没有结束，则继续处理。
            {
                char ch = query[i]; // 获取当前字符
               /* if (i == 14)
                {
                    cout << "debug" << endl;
                }*/
                if (sta == FINISH) // 读取到句点，说明词法分析结束，将token序列写入文件并退出循环。
                {
                    tok.linsShow = linenum; // 将当前行号赋值给词法单元结构体 tok 中的 linsShow 字段
                    tok.lex = DOT; // 将当前词法单元的类型赋值为 DOT
                    tok.sem = "Finish"; // 将当前词法单元的语义信息赋值为 Finish
                    //cout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                    fout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                    imbuffer.clear(); // 清空暂存的单词或字符
                    break;
                }
                while ((sta == INCOMMENT || sta == START) && i < longquery) // 如果在开始阶段或者注释阶段
                {
                    if (query[i] == ' ')
                    {
                        while (query[i] == ' ') // 读取到空格，跳过空格
                        {
                            i++;
                        }
                    }
                    if (query[i] >= 'a' && query[i] <= 'z') // 读取到英文字母
                    {
                        // 读取到字母的时候，说明已经开始分析，此时将状态置为INID; 进入循环可能是因为当前状态是 START，也可能是因为当前状态是 INCOMMENT
                        // 如果是因为当前状态是 START，则说明是第一次读取到字母，将状态置为 INID
                        // 如果是因为当前状态是 INCOMMENT，则不修改状态，继续读取字母
                        if (sta == START)
                        {
                            sta = INID;
                        }
                        // 英文字母可能构成标识符或者保留字，先统一当作标识符处理，如果后面发现是保留字，则修改词法单元的类型和语义信息
                        while ((query[i] >= 'a' && query[i] <= 'z') || (query[i] >= '0' && query[i] <= '9'))
                        {
                            imbuffer = imbuffer + query[i];
                            i++;
                            if (i == longquery) // 如果读取到行尾，则退出循环
                            {
                                break;
                            }
                        }
                        // 如果当前状态是INID，说明在上面的if中是因为当前状态是start而修改的，进而说明进入第二层循环时的状态并不是INCOMMENT
                        if (sta == INID)
                        {
                            tok.linsShow = linenum;
                            tok.lex = ID;
                            tok.sem = imbuffer;
                            for (int a = 0; a <= 37; a++)
                            {
                                if (arrayLexType_zyg[a] == imbuffer) // 如果是保留字，则修改词法单元的类型和语义信息
                                {
                                    tok.lex = LexType(a);
                                    tok.sem = "ReseverWord";
                                    break;
                                }
                            }
                            fout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                            imbuffer.clear();
                            sta = START; // 词法单元处理完毕，将状态置为 START
                            break;
                        }
                    }
                    if (query[i] >= '0' && query[i] <= '9') // 处理数字
                    {
                        if (sta == START) // 如果当前状态是 START不是 INCOMMENT，则说明是第一次读取到数字，将状态置为 INNUM
                        {
                            sta = INNUM;
                        }
                        while (query[i] >= '0' && query[i] <= '9')
                        {
                            imbuffer = imbuffer + query[i];
                            i++;
                            if (i == longquery) // 任何时候读取到行尾都退出循环
                            {
                                break;
                            }
                        }
                        // 如果遇到非数字字符（如小写字母），说明数字读取结束，则将状态重新置为 START，并跳出循环
                        while (query[i] >= 'a' && query[i] <= 'z')
                        {
                            sta = START;
                            i++; // 跳过非数字字符，继续读取
                            if (i == longquery)
                            {
                                break;
                            }
                        }
                        if (sta == INNUM) // 如果读取数字的过程中没有遇到字母，则说明是整数，将词法单元的类型和语义信息修改为整数
                        {
                            tok.linsShow = linenum;
                            tok.lex = INTC;
                            tok.sem = imbuffer;
                            //cout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                            fout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                            imbuffer.clear();
                            sta = START;
                            break;
                        }
                        if (sta == START) // 如果读取数字的过程中遇到字母，说明是非法的
                        {
                            string a = to_string(linenum) + "行有错误";
                            InputError(a, "error.txt");
                            exit(0);
                            break;
                        }
                    }
                    // 处理单字符分界符
                    if (query[i] == '+' || query[i] == '-' || query[i] == '*' || query[i] == '/' || query[i] == '(' ||
                        query[i] == ')' || query[i] == ';' || query[i] == '[' || query[i] == ']' || query[i] == '=' ||
                        query[i] == '<')
                    {
                        if (sta == START) // 如果当前状态是 START 不是 INCOMMENT，则说明是第一次读取到单字符分界符，将状态置为 DONE
                        {
                            sta = DONE;
                        }
                        if (sta == DONE)
                        {
                            tok.linsShow = linenum;

                            imbuffer = query[i];
                            for (int a = 0; a <= 37; a++)
                            {
                                if (arrayLexType_zyg[a] == imbuffer)
                                {
                                    tok.lex = LexType(a);
                                    tok.sem = "SingleSeparator";
                                    break;
                                }
                            }
                            i++;
                            fout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                            imbuffer.clear();
                            sta = START;
                            break;
                        }
                    }
                    if (query[i] == ':')
                    {
                        if (i < longquery - 1 && query[i + 1] == '=')
                        {
                            if (sta == START) // 如果当前状态是 START 不是 INCOMMENT，说明是第一次读取到 := ，将状态置为 INASSIGN
                            {
                                sta = INASSIGN;
                            }
                        }
                        if (sta == INASSIGN)
                        {
                            tok.linsShow = linenum;
                            tok.lex = ASSIGN;
                            tok.sem = "DobleSeparator";
                            i = i + 2;
                            fout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                            imbuffer.clear();
                            sta = START;
                            break;
                        }

                    }
                    if (query[i] == ',')
                    {
                        if (sta == START) // 如果当前状态是 START 不是 INCOMMENT，说明是第一次读取到','将状态置为 INCHAR，表示读取到了字符
                        {
                            sta = INCHAR;
                        }
                        if (sta == INCHAR)
                        {
                            tok.linsShow = linenum;
                            tok.lex = COMMA;
                            tok.sem = "SingleSeparator";
                            i = i + 1;
                            fout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                            imbuffer.clear();
                            sta = START;
                            while (query[i] == ' ')
                            {
                                i++;
                            }
                            /**
                             * 这个条件判断当前位置是否在字符串范围内，
                             * 并且下一个字符是小写字母或数字。
                             * 如果满足条件，表示还有有效字符需要处理，继续循环。
                             * 否则，表示逗号后面缺少有效字符，输出错误信息。
                             * 例子：a,b,c
                             */
                            if (i < longquery &&
                                ((query[i] >= 'a' && query[i] <= 'z') || (query[i] >= '0' && query[i] <= '9')))
                            {}
                            else
                            {
                                string a = to_string(linenum) + "行有错误";
                                InputError(a, "error.txt");
                                exit(0);
                            }

                            break;
                        }
                    }
                    if (query[i] == '.')
                    {
                        if (i < longquery - 1 && query[i + 1] == '.') // 如果是 .. ，说明正在处理范围，将状态置为 INRANGE
                        {
                            if (sta == START)
                            {
                                sta = INRANGE;
                            }
                        }
                        else
                        {
                            if (sta == START && tok.lex == END) // .可能是结束符，也可能是数组下标的点
                            {
                                sta = FINISH;
                            }
                            else if (sta == START)
                            {
                                i++;
                                tok.linsShow = linenum;
                                tok.lex = DOT;
                                tok.sem = "Dot";
                                fout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                                imbuffer.clear();
                                break;
                            }
                        }
                        if (sta == INRANGE)
                        {
                            tok.linsShow = linenum;
                            tok.lex = UNDERANGE;
                            tok.sem = "ArrayIndex";
                            i = i + 2;
                            fout << tok.linsShow << ' ' << tok.lex << ' ' << tok.sem << endl;
                            imbuffer.clear();
                            sta = START;
                            break;
                        }
                    }
                    if (query[i] == '{')
                    {
                        if (sta == START)
                        {
                            sta = INCOMMENT;
                        }
                    }
                    if (sta == INCOMMENT)
                    {
                        //cout << "注释开始" << endl;
                        while (i < longquery && query[i] != '}')
                        {
                            i++;
                        }
                        if (query[i] == '}')
                        {
                            i++;
                            sta = START;
                            //cout << "注释结束" << endl;
                            break;
                        }
                    }
                    // 上面全部的情况都已经判断完，但是程序还是没有结束，说明是非法字符，输出错误信息
                    if (sta != INCOMMENT && sta != FINISH && i < longquery)
                    {
                        string a = to_string(linenum) + "行有错误";
                        InputError(a, "error.txt");
                        exit(0);
                        i++;
                    }
                }
            }
        }
    }
    fin.close();
    fout.close();
}
