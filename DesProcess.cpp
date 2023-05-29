#include"header.h"

/*******************变量声明************************/
string arrayLexTypeD_hyf[] = {"ID", "IF", "BEGIN", "INTC", "END", "PLUS", "MINUS", "TIMES", "OVER", "EQ", "LT",
                              "LMIDPAREN", "RMIDPAREN", "DOT", "TYPE", "VAR", "PROCEDURE", "PROGRAM", "SEMI", "INTEGER",
                              "CHAR", "ARRAY", "RECORD", "UNDERANGE", "OF", "COMMA", "LPAREN", "RPAREN", "ENDWH",
                              "WHILE", "RETURN", "READ", "WRITE", "ASSIGN", "THEN", "FI", "ELSE", "DO"
};
string temp_name;
Token token;        // 存储当前Token
ifstream in("token.txt"); // 读取token.txt

/*******************函数声明************************/
TreeNode *DesParse()
{
    //读入token
    ReadToken(); // 读入第一个token
    TreeNode *t = Program(); // 语法分析，返回语法树根节点
    if (in.eof()) // 程序已经完全解析
    {
        Input(t, "treeDes.txt");
    }
    else // 否则program函数调用以后in还没到达文件结尾，说明文件提前结束了，输出错误信息
    {
        string a = "文件提前结束";
        InputError(a, "error.txt");
        exit(0);
    }
    return t;
}

TreeNode *Program()
{
    TreeNode *root = new TreeNode();
    (*root).nodekind = ProK;
    (*root).lineno = token.linsShow;
    TreeNode *t = ProgramHead(); // 语法分析程序头
    TreeNode *q = DeclarePart(); // 语法分析声明部分
    TreeNode *s = ProgramBody(); // 语法分析程序体
    if (t == NULL)
    {
        string a = "没有程序头";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
    }
    else
    {
        (*root).child[0] = t;
    }
    if (q == NULL)
    {
        string a = "没有声明";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        //exit(0);
    }
    else
    {
        (*root).child[1] = q;
    }
    if (s == NULL)
    {
        string a = "没有程序体";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        //exit(0);
    }
    else
    {
        (*root).child[2] = s;
    }
    Match(DOT);
    return root;
}

// 读入程序头
TreeNode *ProgramHead()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = PheadK;
    (*t).lineno = token.linsShow; // 当前token所在行数
    Match(PROGRAM); // 第一个token应该是PROGRAM，消耗掉
    if (token.lex == ID) // 消耗掉PROGRAM以后就读取下一个token了，如果当前token是一个标识符，就
    {
        (*t).idnum++; // 当前程序头的标识符加1
        (*t).name.push_back(token.sem); // 将当前token的语义值存入程序头的name数组中，可能是保留字，数组下标，也可能是标识符
    }
    Match(ID); // 消耗掉标识符，并读取下一个token
    return t;
}

// 读入声明部分
TreeNode *DeclarePart()
{
    // 声明部分可能有三种情况：类型声明，变量声明，过程声明
    // sibling的作用是将这三种情况串联起来，形成一个链表，这样就可以在语法树中以兄弟节点的形式存储，而不是以子节点的形式存储
    // 三者之间的关系是并列的
    TreeNode *typeP = NULL, *varP = NULL;
    int line = token.linsShow; // line记录当前token所在行数，用于错误处理
    TreeNode *tp1 = TypeDec(); // 类型声明部分
    if (tp1 != NULL)
    {
        typeP = new TreeNode();
        (*typeP).nodekind = TypeK; // 类型声明标志节点
        (*typeP).child[0] = tp1;
        (*typeP).lineno = line;
    }
    line = token.linsShow;
    TreeNode *tp2 = VarDec(); // 变量声明部分
    if (tp2 != NULL)
    {
        varP = new TreeNode();
        (*varP).nodekind = VarK; // 变量声明标志节点
        (*varP).child[0] = tp2;
        (*varP).lineno = line;
    }
    TreeNode *s = ProcDec(); // 过程声明部分
    if (varP == NULL) // 如果变量声明部分为空
    {
        if (typeP == NULL) // 如果类型声明部分也为空
        {
            typeP = s; // 整个声明部分只有过程声明部分。
        }
        else
        {
            (*typeP).sibling = s; // 类型声明部分不为空，将过程声明部分作为类型声明部分的兄弟节点
        }
    }
    else // 如果变量声明部分不为空，有变量声明部分
    {
        (*varP).sibling = s; // 将过程声明部分作为变量声明部分的兄弟节点
        if (typeP == NULL)
        {
            // 整个声明部分只有变量声明部分和过程声明部分
            // 将变量声明部分的根节点 varP 赋值给 typeP，使得整个声明部分的根节点就是 varP
            typeP = varP;
        }
        else
        {
            // 如果 typeP 不为 NULL，表示有类型声明部分
            // 将变量声明部分的根节点 varP 作为类型声明部分的根节点 typeP 的兄弟节点，将它们连接起来。
            (*typeP).sibling = varP;
        }
    }
    return typeP;
}

// 解析类型声明部分
TreeNode *TypeDec()
{
    TreeNode *t = NULL;
    // 解析具体的类型声明，并将返回的语法树节点指针存储在 t 变量中。
    if (token.lex == TYPE)
    {
        t = TypeDeclaration();
    }
    // 如果当前记号的类型是 VAR、PROCEDURE 或 BEGIN，则表示没有类型声明部分，直接跳过。
    else if (token.lex == VAR || token.lex == PROCEDURE || token.lex == BEGIN)
    {}
    // 如果当前记号的类型不是上述三种情况，则表示当前记号不合法，报错，并读入下一个token
    else
    {
        ReadToken();
        string a = to_string(token.linsShow) + ":当前单词" + arrayLexTypeD_hyf[(int) token.lex] + "不合格，跳过";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
    }
    return t;
}

// 类型声明部分
TreeNode *TypeDeclaration()
{
    Match(TYPE); // 如果当前类型是 TYPE，消耗掉
    TreeNode *t = TypeDecList();
    if (t == NULL) // 没有解析到有效的类型声明
    {
        // 显示提示信息
    }
    return t;
}

// 解析类型定义列表
TreeNode *TypeDecList()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = DecK; // 声明节点
    (*t).lineno = token.linsShow;
    TypeId(t); // 解析类型标识符，因为SNL是先写变量名，再写类型名的，所以这里先解析类型标识符
    Match(EQ); // 匹配当前记号是否为等号
    TypeDef(t); // 解析具体的类型定义
    Match(SEMI); // 匹配当前记号是否为分号
    TreeNode *p = TypeDecMore();
    if (p != NULL)
    { (*t).sibling = p; }
    return t;
}

// 解析可能存在的其他类型定义
TreeNode *TypeDecMore()
{
    TreeNode *t = NULL;
    if (token.lex == ID) // 如果当前记号是标识符，表示可能存在其他类型定义
    {
        t = TypeDecList(); // 解析其他类型定义
    }
    else if (token.lex == VAR || token.lex == PROCEDURE || token.lex == BEGIN)
    {}
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
    }
    return t;
}

// 解析基本的数据类型，然后将具体的类型保存到语法树节点中，即保存标识符
void TypeId(TreeNode *t)
{
    if (token.lex == ID && t != NULL)
    {
        (*t).idnum++; // 类型标识符个数加一
        (*t).name.push_back(token.sem);
    }
    Match(ID); // 消耗掉当前记号
}

// 解析具体到底是什么类型
void TypeDef(TreeNode *t)
{
    if (t != NULL)
    {
        if (token.lex == INTEGER || token.lex == CHAR)
        {
            BaseType(t);
        }
        else if (token.lex == ARRAY || token.lex == RECORD)
        {
            StructureType(t);
        }
        else if (token.lex == ID)
        {
            (*t).kind.dec = IdK; // 类类型标志符作为类型
            (*t).idnum++;
            (*t).name.push_back(token.sem);
            (*t).type_name = token.sem;
            Match(ID);
        }
        else
        {
            string a = to_string(token.linsShow) + "行有错误";
            InputError(a, "error.txt");
            //cout << "文件提前结束";
            exit(0);
        }
    }
}

void BaseType(TreeNode *t)
{
    if (token.lex == INTEGER)
    {
        Match(INTEGER); // 消耗掉当前的类型
        // 记录语法树节点的具体类型,其中kind的类型是union Kind，记录语法树节点的声明类型,当nodekind=DecK时有效
        (*t).kind.dec = IntegerK;
    }
    else if (token.lex == CHAR)
    {
        Match(CHAR);
        (*t).kind.dec = CharK;
    }
}

void StructureType(TreeNode *t)
{
    if (token.lex == ARRAY)
    {
        ArrayType(t);
    }
    else if (token.lex == RECORD)
    {
        (*t).kind.dec = RecordK;
        RecType(t);
    }
}

void ArrayType(TreeNode *t)
{
    Match(ARRAY); // 消耗掉当前的类型
    Match(LMIDPAREN); // 匹配当前记号是否为左中括号
    if (token.lex == INTC) // 如果当前记号是整数，表示是数组的下界
    {
        (*t).attr.arrayAttr.low = stoi(token.sem);
    }
    Match(INTC);
    Match(UNDERANGE); // 消耗掉..
    if (token.lex == INTC) // 如果当前记号是整数，表示是数组的上界
    {
        (*t).attr.arrayAttr.up = stoi(token.sem);
    }
    Match(INTC); // 消耗掉当前当前的整数
    Match(RMIDPAREN); // 匹配当前记号是否为右中括号
    Match(OF); // 匹配当前记号是否为OF
    BaseType(t); // 解析基本类型
    // 语法树节点其他属性
    (*t).attr.arrayAttr.childType = (*t).kind.dec;
    (*t).kind.dec = ArrayK;
}

void RecType(TreeNode *t)
{
    Match(RECORD);
    TreeNode *p = FieldDecList();
    if (p != NULL)
    {
        (*t).child[0] = p;
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //错误信息提示
    }
    Match(END);
}

// 解析结构体声明
TreeNode *FieldDecList()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = DecK;
    (*t).lineno = token.linsShow;
    TreeNode *p = NULL;
    if (token.lex == INTEGER || token.lex == CHAR)
    {
        BaseType(t);
        IdList(t); // 解析标识符列表
        Match(SEMI); // 匹配当前记号是否为分号
        p = FieldDecMore();
    }
    else if (token.lex == ARRAY)
    {
        ArrayType(t);
        IdList(t);
        Match(SEMI);
        p = FieldDecMore();
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词，但是现在都没有匹配的了，读入下一个单词不就是有错误吗
    }
    (*t).sibling = p;
    return t;
}

TreeNode *FieldDecMore()
{
    TreeNode *t = NULL;
    if (token.lex != END)
    {
        if (token.lex == INTEGER || token.lex == CHAR || token.lex == ARRAY)
        {
            t = FieldDecList();
        }
        else
        {
            string a = to_string(token.linsShow) + "行有错误";
            InputError(a, "error.txt");
            //cout << "文件提前结束";
            exit(0);
            //读入下一个单词
        }
    }
    return t;
}

void IdList(TreeNode *t)
{
    if (token.lex == ID)
    {
        (*t).idnum++;
        (*t).name.push_back(token.sem);
    }
    Match(ID);
    IdMore(t);
}

void IdMore(TreeNode *t)
{
    if (token.lex == SEMI)
    {}
    else if (token.lex == COMMA)
    {
        Match(COMMA);
        IdList(t);
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
}

TreeNode *VarDec()
{
    TreeNode *t = NULL;
    if (token.lex == PROCEDURE || token.lex == BEGIN)
    {}
    else if (token.lex == VAR)
    {
        t = VarDeclaration();
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

TreeNode *VarDeclaration()
{
    Match(VAR); // 消耗掉当前的VAR
    TreeNode *t = VarDecList();
    if (t == NULL)
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //语法错误信息提示
    }
    return t;
}

TreeNode *VarDecList()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = DecK;
    (*t).lineno = token.linsShow;
    TreeNode *p = NULL;
    TypeDef(t); // 解析类型标识符
    VarIdList(t); // 解析变量标识符列表
    Match(SEMI); // 匹配当前记号是否为分号
    p = VarDecMore(); // 继续解析可能存在的其他变量声明
    (*t).sibling = p; // 将其他变量声明作为兄弟节点
    return t;
}

TreeNode *VarDecMore()
{
    TreeNode *t = NULL;
    if (token.lex == PROCEDURE || token.lex == BEGIN)
    {}
    else if (token.lex == INTEGER || token.lex == CHAR ||
             token.lex == ARRAY || token.lex == RECORD || token.lex == ID)
    {
        t = VarDecList();
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

void VarIdList(TreeNode *t)
{
    if (token.lex == ID)
    {
        (*t).idnum++;
        (*t).name.push_back(token.sem);
        Match(ID);
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //错误提示，读入下一个单词
    }
    VarIdMore(t);
}

void VarIdMore(TreeNode *t)
{
    if (token.lex == SEMI)
    {}
    else if (token.lex == COMMA)
    {
        Match(COMMA);
        VarIdList(t);
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
}

// 解析过程声明
TreeNode *ProcDec()
{
    TreeNode *t = NULL;
    if (token.lex == BEGIN)
    {}
    else if (token.lex == PROCEDURE)
    {
        t = ProcDeclaration(); // 解析过程声明
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

// 解析过程声明部分
TreeNode *ProcDeclaration()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = ProcDecK;
    (*t).lineno = token.linsShow;
    Match(PROCEDURE);
    if (token.lex == ID)
    {
        (*t).idnum++;
        (*t).name.push_back(token.sem); // 记录过程名
        Match(ID);
    }
    Match(LPAREN); // 匹配当前记号是否为左括号，
    ParamList(t); // 解析参数列表
    Match(RPAREN); // 匹配当前记号是否为右括号
    Match(SEMI); // 匹配分号
    (*t).child[1] = ProcDecPart(); // 解析过程声明部分
    (*t).child[2] = ProcBody(); // 解析过程体
    (*t).sibling = ProcDecMore(); // 解析其他程序声明
    return t;
}

// 解析其他过程声明
TreeNode *ProcDecMore()
{
    TreeNode *t = NULL;
    if (token.lex == PROCEDURE || token.lex == BEGIN)
    {}
    else if (token.lex == INTEGER || token.lex == CHAR ||
             token.lex == ARRAY || token.lex == RECORD || token.lex == ID)
    {
        t = ProcDeclaration();
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

// 解析参数列表
void ParamList(TreeNode *t)
{
    TreeNode *p = NULL;
    if (token.lex == RPAREN)
    {}
    else if (token.lex == INTEGER || token.lex == CHAR || token.lex == ARRAY
             || token.lex == RECORD || token.lex == ID || token.lex == VAR)
    {
        p = ParamDecList(); // 解析参数声明列表
        (*t).child[0] = p;
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
}

// 解析参数声明列表
TreeNode *ParamDecList()
{
    TreeNode *t = Param();
    TreeNode *p = ParamMore();
    if (p != NULL)
    {
        (*t).sibling = p;
    }
    return t;
}

// 解析其他参数声明
TreeNode *ParamMore()
{
    TreeNode *t = NULL;
    if (token.lex == RPAREN)
    {}
    else if (token.lex == SEMI)
    {
        Match(SEMI);
        t = ParamDecList();
        if (t == NULL)
        {
            //错误提示
            string a = to_string(token.linsShow) + "行有错误";
            InputError(a, "error.txt");
            //cout << "文件提前结束";
            exit(0);
        }
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

// 解析参数
TreeNode *Param()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = DecK;
    (*t).lineno = token.linsShow;
    if (token.lex == INTEGER || token.lex == CHAR || token.lex == RECORD
        || token.lex == ARRAY || token.lex == ID)
    {
        (*t).attr.procAttr.paramt = Valparamtype;
        TypeDef(t);
        FormList(t);
    }
    else if (token.lex == VAR)
    {
        Match(VAR);
        (*t).attr.procAttr.paramt = Varparamtype;
        TypeDef(t); // 先找类型
        FormList(t); // 再找变量名
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

// 解析形式参数列表
void FormList(TreeNode *t)
{
    if (token.lex == ID)
    {
        (*t).idnum++;
        (*t).name.push_back(token.sem);
        Match(ID);
    }
    FidMore(t);
}

void FidMore(TreeNode *t)
{
    if (token.lex == SEMI || token.lex == RPAREN)
    {}
    else if (token.lex == COMMA)
    {
        Match(COMMA);
        FormList(t);
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读取下一个单词
    }
}

TreeNode *ProcDecPart()
{
    return DeclarePart();
}

TreeNode *ProcBody()
{
    TreeNode *t = ProgramBody(); // 解析程序体
    if (t == NULL)
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //错误信息提示
    }
    return t;
}

TreeNode *ProgramBody()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = StmLK;
    (*t).lineno = token.linsShow;
    Match(BEGIN);
    (*t).child[0] = StmList();
    Match(END);
    return t;
}

TreeNode *StmList()
{
    TreeNode *t = Stm();
    TreeNode *p = StmMore();
    (*t).sibling = p;
    return t;
}

TreeNode *StmMore()
{
    TreeNode *t = NULL;
    if (token.lex == END || token.lex == ENDWH || token.lex == ELSE || token.lex == FI)
    {}
    else if (token.lex = SEMI)
    {
        Match(SEMI);
        t = StmList();
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

// 语句
TreeNode *Stm()
{
    TreeNode *t = NULL;
    if (token.lex == IF)
    {
        t = ConditionalStm();
    }
    else if (token.lex == WHILE)
    {
        t = LoopStm();
    }
    else if (token.lex == RETURN)
    {
        t = ReturnStm();
    }
    else if (token.lex == READ)
    {
        t = InputStm();
    }
    else if (token.lex == WRITE)
    {
        t = OutputStm();
    }
    else if (token.lex == ID)
    {
        TreeNode *f = new TreeNode();
        (*f).nodekind = StmtK;
        (*f).lineno = token.linsShow;
        TreeNode *t1 = new TreeNode();
        t1->nodekind = ExpK;
        t1->lineno = token.linsShow;
        t1->kind.exp = IdEK;
        t1->attr.expAttr.varkind = IdV;
        t1->idnum++;
        t1->name.push_back(token.sem);
        f->child[0] = t1;
        temp_name = token.sem;
        AssCall(f);
        t = f;
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //当前单词为其他单词，非期望单词语法错误，
        //跳过当前单词，读入下一个单词
    }
    return t;
}

void AssCall(TreeNode *t)
{
    Match(ID);
    // 如果是赋值语句
    // ASSIGN: x := 1
    // LMIDPAREN: x[1] := 1
    // DOT: x.a := 1
    if (token.lex == ASSIGN || token.lex == LMIDPAREN || token.lex == DOT)
    {
        AssignmentRest(t); // 函数解析赋值语句的剩余部分
        t->kind.stmt = AssignK;
    }
    else if (token.lex == LPAREN) // 标识符完了以后是左括号，说明是函数调用
    {
        CallStmRest(t);
        t->kind.stmt = CallK;
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
}

// 函数调用
void AssignmentRest(TreeNode *t)
{
    VariMore(t->child[0]); // 是处理变量的后续部分，变量后面可能是运算符等等情况
    Match(ASSIGN);
    t->child[1] = Exp();
}

// 处理条件语句
TreeNode *ConditionalStm()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = StmtK;
    (*t).kind.stmt = IfK;
    (*t).lineno = token.linsShow;
    Match(IF); // 处理if
    (*t).child[0] = Exp(); // 处理条件表达式
    Match(THEN); // 处理then
    (*t).child[1] = StmList(); // 处理then后面的语句
    if (token.lex == ELSE)
    {
        Match(ELSE);
        (*t).child[2] = StmList();
    }
    Match(FI);
    return t;
}

// 处理循环语句
TreeNode *LoopStm()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = StmtK;
    (*t).kind.stmt = WhileK;
    (*t).lineno = token.linsShow;
    Match(WHILE);
    (*t).child[0] = Exp();
    Match(DO);
    (*t).child[1] = StmList();
    Match(ENDWH);
    return t;
}

// 处理输入语句
TreeNode *InputStm()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = StmtK;
    (*t).kind.stmt = ReadK;
    (*t).lineno = token.linsShow;
    Match(READ);
    Match(LPAREN);
    if (token.lex == ID)
    {
        (*t).idnum++;
        (*t).name.push_back(token.sem);
    }
    Match(ID);
    Match(RPAREN);
    return t;
}

// 处理输出语句
TreeNode *OutputStm()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = StmtK;
    (*t).kind.stmt = WriteK;
    (*t).lineno = token.linsShow;
    Match(WRITE);
    Match(LPAREN);
    (*t).child[0] = Exp();
    Match(RPAREN);
    return t;
}

// 处理返回语句
TreeNode *ReturnStm()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = StmtK;
    (*t).kind.stmt = ReturnK;
    (*t).lineno = token.linsShow;
    Match(RETURN);
    return t;
}

// 处理调用语句剩余部分
void CallStmRest(TreeNode *t)
{
    Match(LPAREN);
    (*t).child[1] = ActParamList();
    Match(RPAREN);
}

// 解析实参列表
TreeNode *ActParamList()
{
    TreeNode *t = NULL;
    if (token.lex == RPAREN)
    {}
    else if (token.lex == ID || token.lex == INTC)
    {
        t = Exp();
        if (t != NULL)
        {
            (*t).sibling = ActParamMore(); // 递归处理实参列表
        }
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

TreeNode *ActParamMore() // 实参列表后面可能还有实参
{
    TreeNode *t = NULL;
    if (token.lex == RPAREN)
    {}
    else if (token.lex == COMMA)
    {
        Match(COMMA);
        t = ActParamList();
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个单词
    }
    return t;
}

TreeNode *Exp() // 处理表达式
{
    int line = token.linsShow;
    TreeNode *t = Simple_exp(); // 处理简单表达式
    if (token.lex == LT || token.lex == EQ) // 如果是小于或者等于
    {
        TreeNode *p = new TreeNode();
        (*p).lineno = line;
        (*p).nodekind = ExpK;
        (*p).kind.exp = OpK; // 表达式的类型是操作符
        (*p).child[0] = t;
        (*p).attr.expAttr.op = token.lex; // 操作符的类型
        t = p;
        Match(token.lex);
        if (t != NULL)
        {
            (*t).child[1] = Simple_exp();
        }
    }
    return t;
}

TreeNode *Simple_exp() // 处理简单表达式
{
    int line = token.linsShow;
    TreeNode *t = Term();
    while (token.lex == PLUS || token.lex == MINUS)
    {
        TreeNode *p = new TreeNode();
        (*p).lineno = line;
        (*p).nodekind = ExpK;
        (*p).kind.exp = OpK;
        (*p).child[0] = t;
        (*p).attr.expAttr.op = token.lex;
        t = p;
        Match(token.lex);
        (*t).child[1] = Term();
    }
    return t;
}

TreeNode *Term()
{
    int line = token.linsShow;
    TreeNode *t = Factor();
    while (token.lex == TIMES || token.lex == OVER)
    {
        TreeNode *p = new TreeNode();
        (*p).lineno = line;
        (*p).nodekind = ExpK;
        (*p).kind.exp = OpK;
        (*p).child[0] = t;
        (*p).attr.expAttr.op = token.lex;
        t = p;
        Match(token.lex);
        (*t).child[1] = Factor();
    }
    return t;
}

TreeNode *Factor()
{
    TreeNode *t = NULL;
    if (token.lex == INTC) // 当前因子是整数
    {
        t = new TreeNode();
        (*t).lineno = token.linsShow;
        (*t).nodekind = ExpK;
        (*t).kind.exp = ConstK;
        (*t).attr.expAttr.val = stoi(token.sem);
        Match(INTC);
    }
    else if (token.lex == ID) // 当前的因子是一个标识符，调用标识符的处理函数
    {
        t = Variable();
    }
    else if (token.lex == LPAREN) // 当前的因子是一个表达式
    {
        Match(LPAREN);
        t = Exp();
        Match(RPAREN);
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //当前单词为其他单词，
        //非期望单词错误,显示出错单词和信息.并读入下一单词
    }
    return t;
}

TreeNode *Variable() // 处理标识符
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = ExpK;
    (*t).kind.exp = IdEK;
    (*t).lineno = token.linsShow;
    if (token.lex == ID)
    {
        (*t).idnum++;
        (*t).name.push_back(token.sem);
        Match(ID);
        VariMore(t);
    }
    return t;
}

void VariMore(TreeNode *t)
{
    if (token.lex == ASSIGN || token.lex == TIMES || token.lex == EQ || token.lex == LT
        || token.lex == PLUS || token.lex == MINUS || token.lex == OVER || token.lex == RPAREN
        || token.lex == SEMI || token.lex == COMMA || token.lex == THEN || token.lex == RMIDPAREN
        || token.lex == ELSE || token.lex == FI || token.lex == DO || token.lex == ENDWH || token.lex == END)
    {}
    else if (token.lex == LMIDPAREN)
    {
        Match(LMIDPAREN);
        (*t).child[0] = Exp();
        (*t).attr.expAttr.varkind = ArrayMembV;
        (*(*t).child[0]).attr.expAttr.varkind = IdV;
        Match(RMIDPAREN);
    }
    else if (token.lex == DOT)
    {
        Match(DOT);
        (*t).child[0] = FieldVar();
        (*t).attr.expAttr.varkind = FieldMembV;
        (*(*t).child[0]).attr.expAttr.varkind = IdV;
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个token，并提示错误信息
    }
}

TreeNode *FieldVar()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = ExpK;
    (*t).kind.exp = IdEK;
    (*t).lineno = token.linsShow;
    if (token.lex == ID)
    {
        (*t).idnum++;
        (*t).name.push_back(token.sem);
        Match(ID);
        FieldVarMore(t);
    }
    return t;
}

void FieldVarMore(TreeNode *t)
{
    if (token.lex == ASSIGN || token.lex == TIMES || token.lex == EQ || token.lex == LT
        || token.lex == PLUS || token.lex == MINUS || token.lex == OVER || token.lex == RPAREN
        || token.lex == SEMI || token.lex == COMMA || token.lex == THEN || token.lex == RMIDPAREN
        || token.lex == ELSE || token.lex == FI || token.lex == DO || token.lex == ENDWH || token.lex == END)
    {}
    else if (token.lex == LMIDPAREN)
    {
        Match(LMIDPAREN);
        (*t).child[0] = Exp();
        (*(*t).child[0]).attr.expAttr.varkind = ArrayMembV;
        Match(RMIDPAREN);
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
        //读入下一个token，并提示错误信息
    }
}

// 这段函数是用来匹配token的，只有当前全局变量token.lex和传入的参数expected相等时，才会继续读入下一个token，否则就会报错
void Match(LexType expected)
{
    if (token.lex == expected)
    {
        ReadToken(); // 读入下一个token
    }
    else
    {
        string a = to_string(token.linsShow) + "行有错误";
        InputError(a, "error.txt");
        exit(0);
    }
}

void ReadToken()
{
    string line;
    getline(in, line);
    istringstream inword(line);
    inword >> token.linsShow;
    int lex;
    inword >> lex;
    token.lex = (LexType) lex;
    inword >> token.sem;
    int lineno = token.linsShow;
}