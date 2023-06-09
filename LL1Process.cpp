#include"header.h"
#include <map>

/*******************变量声明************************/
stack<string> signStack;                    // 符号栈，用于进行SNL的LL(1)语法分析，一开始只压栈当前节点的儿子或者兄弟节点，并不是直接压入节点，因为节点的生成是在语法分析过程中完成的
stack<TreeNode *> opSignStack;              // 操作符栈，生成声明部分和语句部分的语法树;
stack<TreeNode *> opNumStack;               // 操作数栈，用于生成表达式部分的语法树
stack<TreeNode **> syntaxTreeStack;         // 语法树栈，用于生成表达式部分的语法树
vector<vector<string>> productSet;          // 存储所有的产生式，每个产生式是一个vector<string>，vector<string>中的每个元素是一个终极符或者非终极符
vector<vector<string>> predictSet;          // 存储所有产生式的predict集，predict集中每个元素是一个终极符
vector<vector<int>> LL1Table;               // LL1矩阵，作用是对于当前非终极符和当前输入符确定应该选择的语法规则，其值对应产生式的编号或者错误编号，产生式编号从1开始，错误编号为0，产生式是行号，predict集中每个元素是列号
set<string> VT;                             // 存储所有终极符
set<string> VN;                             // 存储所有非终极符
string S;                                   // 开始符
TreeNode *currentP;                         // 存储当前节点
TreeNode *saveP;                            // 保存当前指向记录类型声明节点的指针
TreeNode *saveFuncP;                        // 保存指向某一节点的指针
DeckEnum *temp;
bool getExpResult = true;
bool getExpResult2;                         //用于数组
int expflag = 0;                            //用于计算括号
extern Token token;
string arrayLexType_hyf[] = {"ID", "IF", "BEGIN", "INTC", "END", "PLUS", "MINUS", "TIMES", "OVER", "EQ", "LT",
                             "LMIDPAREN", "RMIDPAREN", "DOT", "TYPE", "VAR", "PROCEDURE", "PROGRAM", "SEMI", "INTEGER",
                             "CHAR", "ARRAY", "RECORD", "UNDERANGE", "OF", "COMMA", "LPAREN", "RPAREN", "ENDWH",
                             "WHILE", "RETURN", "READ", "WRITE", "ASSIGN", "THEN", "FI", "ELSE", "DO"
};

// 创建映射表，将枚举类型和对应的类型名关联起来，打印到控制台的时候会用到，在LL1Process.cpp的151行
std::map<NodeKindEnum, std::string> nodeKindMapForOutput = {
        {ProK,     "ProK"},
        {PheadK,   "PheadK"},
        {TypeK,    "TypeK"},
        {VarK,     "VarK"},
        {ProcDecK, "ProcDecK"},
        {StmLK,    "StmLK"},
        {DecK,     "DecK"},
        {StmtK,    "StmtK"},
        {ExpK,     "ExpK"}
};


/*******************函数声明************************/
TreeNode *Parse()
{
    CreatLL1Table();                                // 创建LL1预测分析表
    S = productSet[0][0];                           // 开始符赋值，即第一个产生式的左部，这里是Program
    signStack.push(S);                           // 将开始符压入符号栈
    TreeNode *proK;                                 // 根节点
    proK = new TreeNode();
    (*proK).nodekind = ProK;
    (*proK).lineno = 1;
    // 将根节点的三个孩子节点指针压入语法树栈，因为根节点没有兄弟节点，所以不用压入兄弟节点
    syntaxTreeStack.push(&(*proK).child[2]);    // 将根节点的儿子指针压入语法树栈，这里是程序体部分
    syntaxTreeStack.push(&(*proK).child[1]);    // 将根节点的儿子指针压入语法树栈，这里是声明部分
    syntaxTreeStack.push(&(*proK).child[0]);    // 将根节点的儿子指针压入语法树栈，这里是程序头部分

    //读入一个Token，用lineno记录单词行号
    ifstream in("token.txt");
    string line;
    getline(in, line);
    istringstream inword(line);
    inword >> token.linsShow;                   // 读入行号
    int lex;                                    // 用于存储词法信息
    inword >> lex;                              // 读入词法信息
    token.lex = (LexType) lex;                  // 将词法信息转换为枚举类型
    inword >> token.sem;
    int lineno = token.linsShow;                // 记录当前行号
    while (!signStack.empty())                  // 当栈不空的时候，说明还没有分析完毕
    {
        string top = signStack.top();           // 取出栈顶元素
        int num = GetIndex(VN, top);      // 若top是终极符会返回-1;
        if (num != -1)                          // 不是终极符
        {
            int x = GetIndex(VN, top);    // 栈顶非终极符做为行号
            int y = GetIndex(VT, arrayLexType_hyf[(int) token.lex]); // Token的词法信息为列号
            int pnum = LL1Table[x][y];          // 栈顶非终极符做为行号，Token的词法信息为列号，取LL(1)分析表内容，得到产生式编号
            if (pnum == 0)                      // 产生式编号为0，说明不属于该非终极符的predict集
            {
                string a = to_string(token.linsShow) + ":" + arrayLexType_hyf[(int) token.lex] + "不属于" + top +
                           "的predict集";
                InputError(a, "error.txt");
                exit(0);
            }
            signStack.pop();                    // 弹出栈顶元素
            Predict(pnum);                      // 根据不同的产生式编号，选择不同的产生式，然后将产生式的右部逆序压入栈中
        }
        else                                    // 是终极符
        {
            if (top == arrayLexType_hyf[(int) token.lex])           // 如果栈顶元素和Token匹配
            {
                signStack.pop();                                    // 弹出栈顶元素
                //读入一个Token，并用lineon记录行号
                getline(in, line);                            // 读入下一个Token
                istringstream inword(line);
                inword >> token.linsShow;
                int lex;
                inword >> lex;
                token.lex = (LexType) lex;
                inword >> token.sem;
                lineno = token.linsShow;
            }
            else
            {
                string a = to_string(token.linsShow) + ":栈顶(" + top + ")不与当前token(" +
                           arrayLexType_hyf[(int) token.lex] + ")匹配";
                InputError(a, "error.txt");
                exit(0);
            }
        }
    }
    if (in.eof())                                                   // 文件结束
    {
        Input(proK, "treeLL1.txt");                         // 将语法树输出到文件
    }
    else
    {
        string a = "文件提前结束";                                    // 文件提前结束
        InputError(a, "error.txt");
        //cout << "文件提前结束";
        exit(0);
    }
    in.close();
    return proK;
}

void PrintTreeNode(TreeNode *node, int depth) // 打印语法树节点
{
    for (int i = 0; i < depth; i++)
    {
        string str = "│   ";
        std::cout << str; // 每级深度增加四个空格的缩进，并添加竖线分隔符
    }

    if (node->idnum > 0)
    {
        std::cout << "├── " << nodeKindMapForOutput[node->nodekind] << "  " ;
        if (nodeKindMapForOutput[node->nodekind] == "DecK")
        {
            switch (node->kind.dec)
            {
                case 0:
                    cout << "ArrayK  ";
                    break;
                case 1:
                    cout << "CharK  ";
                    break;
                case 2:
                    cout << "IntegerK  ";
                    break;
                case 3:
                    cout << "RecordK  ";
                    break;
                case 4:
                    cout << "IdK  ";
                    break;
            }
        }
        else if (nodeKindMapForOutput[node->nodekind] == "StmLK")
        {
            switch (node->kind.dec)
            {
                case 0:
                    cout << "IfK  ";
                    break;
                case 1:
                    cout << "WhileK  ";
                    break;
                case 2:
                    cout << "AssignK";
                    break;
                case 3:
                    cout << "ReadK";
                    break;
                case 4:
                    cout << "WriteK";
                    break;
                case 5:
                    cout << "CallK";
                    break;
                case 6:
                    cout << "ReturnK";
                    break;
            }

            cout << "  ";
        }
        else if (nodeKindMapForOutput[node->nodekind] == "ExpK")
        {
            switch (node->kind.exp)
            {
                case 0:
                    cout << "OpK";
                    break;
                case 1:
                    cout << "ConstK";
                    break;
                case 2:
                    cout << "IdEK";
                    break;
            }
            cout << "  ";
        }
        cout <<"ID(";
        for (int i = 0; i < node->idnum; i++)
        {
            std::cout << node->name[i];
            if (i < node->idnum - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << ")" << std::endl;
    }
    else
    {
        std::cout << "├── " << nodeKindMapForOutput[node->nodekind] << endl;
    }

    for (int i = 0; i <= 2; i++)
    {
        if (node->child[i] != nullptr)
        {
            PrintTreeNode(node->child[i], depth + 1); // 递归打印子节点
        }
    }

    if (node->sibling != nullptr)
    {
        PrintTreeNode(node->sibling, depth); // 继续打印兄弟节点
    }
}

void Input(TreeNode *root, string path)
{
    ofstream ouput;
    ouput.open(path);
    stack<TreeNode *> inputSort;
    inputSort.push(root);
    while (!inputSort.empty())
    {
        TreeNode *t = inputSort.top();
        inputSort.pop();
        ouput << t->child[0] << " " << t->child[1] << " " << t->child[2] << " ";
        ouput << t->sibling << " ";
        ouput << t->lineno << " ";
        ouput << t->nodekind << " ";
        ouput << t->kind.dec << " ";
        ouput << t->idnum << " ";
        for (int i = 0; i < t->idnum; i++)
        {
            ouput << t->name[i] << " ";
        }
        ouput << t->type_name << " ";
        ouput << t->attr.arrayAttr.childType << " " << t->attr.arrayAttr.low << " " << t->attr.arrayAttr.up << " ";
        ouput << t->attr.expAttr.op << " " << t->attr.expAttr.type << " " << t->attr.expAttr.val << " "
              << t->attr.expAttr.varkind << " ";
        ouput << t->attr.procAttr.paramt << endl;

        // 打印节点内容到控制台
        PrintTreeNode(t, 0);

        if (t->sibling != NULL)
        {
            inputSort.push(t->sibling);
        }
        for (int i = 2; i >= 0; i--)
        {
            if (t->child[i] != NULL)
            {
                inputSort.push(t->child[i]);
            }
        }
    }
    ouput.close();
}

void InputError(string errorInfo, string path)
{
    ofstream ouput;
    ouput.open(path);
    ouput << errorInfo;
    ouput.close();
}

void Predict(int pnum) // 根据产生式编号，选择相应的语义动作
{
    if (pnum == 1)
    { process1(); }
    else if (pnum == 2)
    { process2(); }
    else if (pnum == 3)
    { process3(); }
    else if (pnum == 4)
    { process4(); }
    else if (pnum == 5)
    { process5(); }
    else if (pnum == 6)
    { process6(); }
    else if (pnum == 7)
    { process7(); }
    else if (pnum == 8)
    { process8(); }
    else if (pnum == 9)
    { process9(); }
    else if (pnum == 10)
    { process10(); }
    else if (pnum == 11)
    { process11(); }
    else if (pnum == 12)
    { process12(); }
    else if (pnum == 13)
    { process13(); }
    else if (pnum == 14)
    { process14(); }
    else if (pnum == 15)
    { process15(); }
    else if (pnum == 16)
    { process16(); }
    else if (pnum == 17)
    { process17(); }
    else if (pnum == 18)
    { process18(); }
    else if (pnum == 19)
    { process19(); }
    else if (pnum == 20)
    { process20(); }
    else if (pnum == 21)
    { process21(); }
    else if (pnum == 22)
    { process22(); }
    else if (pnum == 23)
    { process23(); }
    else if (pnum == 24)
    { process24(); }
    else if (pnum == 25)
    { process25(); }
    else if (pnum == 26)
    { process26(); }
    else if (pnum == 27)
    { process27(); }
    else if (pnum == 28)
    { process28(); }
    else if (pnum == 29)
    { process29(); }
    else if (pnum == 30)
    { process30(); }
    else if (pnum == 31)
    { process31(); }
    else if (pnum == 32)
    { process32(); }
    else if (pnum == 33)
    { process33(); }
    else if (pnum == 34)
    { process34(); }
    else if (pnum == 35)
    { process35(); }
    else if (pnum == 36)
    { process36(); }
    else if (pnum == 37)
    { process37(); }
    else if (pnum == 38)
    { process38(); }
    else if (pnum == 39)
    { process39(); }
    else if (pnum == 40)
    { process40(); }
    else if (pnum == 41)
    { process41(); }
    else if (pnum == 42)
    { process42(); }
    else if (pnum == 43)
    { process43(); }
    else if (pnum == 44)
    { process44(); }
    else if (pnum == 45)
    { process45(); }
    else if (pnum == 46)
    { process46(); }
    else if (pnum == 47)
    { process47(); }
    else if (pnum == 48)
    { process48(); }
    else if (pnum == 49)
    { process49(); }
    else if (pnum == 50)
    { process50(); }
    else if (pnum == 51)
    { process51(); }
    else if (pnum == 52)
    { process52(); }
    else if (pnum == 53)
    { process53(); }
    else if (pnum == 54)
    { process54(); }
    else if (pnum == 55)
    { process55(); }
    else if (pnum == 56)
    { process56(); }
    else if (pnum == 57)
    { process57(); }
    else if (pnum == 58)
    { process58(); }
    else if (pnum == 59)
    { process59(); }
    else if (pnum == 60)
    { process60(); }
    else if (pnum == 61)
    { process61(); }
    else if (pnum == 62)
    { process62(); }
    else if (pnum == 63)
    { process63(); }
    else if (pnum == 64)
    { process64(); }
    else if (pnum == 65)
    { process65(); }
    else if (pnum == 66)
    { process66(); }
    else if (pnum == 67)
    { process67(); }
    else if (pnum == 68)
    { process68(); }
    else if (pnum == 69)
    { process69(); }
    else if (pnum == 70)
    { process70(); }
    else if (pnum == 71)
    { process71(); }
    else if (pnum == 72)
    { process72(); }
    else if (pnum == 73)
    { process73(); }
    else if (pnum == 74)
    { process74(); }
    else if (pnum == 75)
    { process75(); }
    else if (pnum == 76)
    { process76(); }
    else if (pnum == 77)
    { process77(); }
    else if (pnum == 78)
    { process78(); }
    else if (pnum == 79)
    { process79(); }
    else if (pnum == 80)
    { process80(); }
    else if (pnum == 81)
    { process81(); }
    else if (pnum == 82)
    { process82(); }
    else if (pnum == 83)
    { process83(); }
    else if (pnum == 84)
    { process84(); }
    else if (pnum == 85)
    { process85(); }
    else if (pnum == 86)
    { process86(); }
    else if (pnum == 87)
    { process87(); }
    else if (pnum == 88)
    { process88(); }
    else if (pnum == 89)
    { process89(); }
    else if (pnum == 90)
    { process90(); }
    else if (pnum == 91)
    { process91(); }
    else if (pnum == 92)
    { process92(); }
    else if (pnum == 93)
    { process93(); }
    else if (pnum == 94)
    { process94(); }
    else if (pnum == 95)
    { process95(); }
    else if (pnum == 96)
    { process96(); }
    else if (pnum == 97)
    { process97(); }
    else if (pnum == 98)
    { process98(); }
    else if (pnum == 99)
    { process99(); }
    else if (pnum == 100)
    { process100(); }
    else if (pnum == 101)
    { process101(); }
    else if (pnum == 102)
    { process102(); }
    else if (pnum == 103)
    { process103(); }
    else if (pnum == 104)
    { process104(); }
    else
    {}
}

void CreatLL1Table()
{
    string line;                    // 记录每一行字符串
    vector<string> words;           // 存储每一行字符串里的所有单词
    // 将product.txt文件读入到productSet，该文件内存储所有的产生式,通过该文件可以获得所有的非终极符，保存到VN中
    ifstream in("Product.txt");  // 从Product.txt文件中读取产生式
    if (in)
    {
        while (getline(in, line))
        {
            //cout << line << endl;
            istringstream inword(line);
            string word;            // 记录单词
            int i = 0;              // 记录单词数，方便对第二个单词":="舍弃，并将第一个单词加入集合VN
            while (inword >> word)
            {
                i++;
                if (i == 1)
                {
                    VN.insert(word); // 将产生式左部加入到VN中，即得到所有的非终极符
                    words.push_back(word);
                }
                else if (i > 2)
                {
                    words.push_back(word);
                }
            }
            productSet.push_back(words); // 将每一行的产生式加入到productSet中
            words.clear();
        }
    }
    else
    {
        cerr << "该文件不存在";
    }
    in.close();
    //将predict.txt文件读入到predictSet，该文件内存储所有的产生式的predict集,通过该文件可以获得所有的终极符，保存到VT中
    // predict集和product中的每一条产生式一一对应
    in.open("predict.txt");
    if (in)
    {
        while (getline(in, line))
        {
            //cout << line << endl;
            istringstream inword(line);
            string word;    //记录单词
            int i = 0;        //记录单词数，将第一个单词加入集合VT
            while (inword >> word)
            {
                i++;
                words.push_back(word);
                if (i != 1) // 第一个单词是产生式的序号，不是终极符
                {
                    VT.insert(word);
                }
            }
            predictSet.push_back(words); // 序号也要加入到predictSet中，一行对应一个产生式
            words.clear();
        }
    }
    else
    {
        cerr << "该文件不存在";
    }
    in.close();
    // 利用上面获取到的所有非终极符和终极符构建LL1矩阵，然后通过predictSet来对矩阵进行赋值
    int m = VN.size(), n = VT.size();
    // 初始化LL1Table，将所有的元素初始化为0，是一个m*n的矩阵，元素为产生式的序号，表示应该去找哪一条产生式
    LL1Table = vector<vector<int>>(m, vector<int>(n, 0));
    for (int i = 0; i < predictSet.size(); i++) // 遍历每一个predict集的中的每一条
    {
        int num = stoi(predictSet[i][0]);   // 得到这个predict集对应的产生式序号
        string s = productSet[num - 1][0];      // 获取产生式左部的非终极符，因为非终极符就是通过序号来获取的
        int x = GetIndex(VN, s);             // 获取该非终极符在VN中的下标
        for (int j = 1; j < predictSet[i].size(); j++) // 遍历每一行
        {
            int y = GetIndex(VT, predictSet[i][j]); // 获取第i条predict集中的第j个终极符在VT中的下标
            LL1Table[x][y] = num;                  // 将该产生式的序号填入LL1Table[x][y]
        }
    }
}

int Priosoty(LexType op) // 传入一个运算符，返回该运算符的优先级
{
    if (op == END)
    {
        return 0;
    }
    else if (op == LT || op == EQ)
    {
        return 1;
    }
    else if (op == PLUS || op == MINUS)
    {
        return 2;
    }
    else if (op == TIMES || op == OVER)
    {
        return 3;
    }
    else
    {
        string a = to_string(token.linsShow) + ":没有" + arrayLexType_hyf[(int) token.lex] + "运算符";
        InputError(a, "error.txt");
        //cout << "没有" << arrayLexType_hyf[(int)token.lex] << "运算符";
        exit(0);
    }
}

int GetIndex(set<string> &A, string s) // 获取字符串s在集合A中的下标
{
    int i = 0;
    for (set<string>::iterator begin = A.begin(); begin != A.end(); begin++, i++)
    {
        if (*begin == s)
        {
            return i;
        }
    }
    return -1;    //当字符串s不在集合内时返回-1
}

// Program ::= ProgramHead DeclarePart ProgramBody DOT
void process1() // 处理产生式1，将ProgramHead DeclarePart ProgramBody DOT从右至左压入栈中
{
    for (int i = productSet[0].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[0][i]); // 栈顶元素是ProgramHead
    }
}

// ProgramHead ::= PROGRAM ProgramName
void process2() // 处理产生式2，将ProgramHead压入栈中，并且将栈顶的指针指向该节点
{
    for (int i = productSet[1].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[1][i]);    // 经过下面的操作以后，栈顶元素是ProgramName
    }
    currentP = new TreeNode();                  // 创建一个新的节点，表示ProgramHead
    (*currentP).nodekind = PheadK;              // 程序头标志节点
    (*currentP).lineno = token.linsShow;        // 行号
    TreeNode **t = syntaxTreeStack.top();       // 获取栈顶的指针，树的节点要么是终结符，要么是非终结符，所以用指针来指向这些节点
    syntaxTreeStack.pop();                      // 弹出栈顶的指针
    *t = currentP;                              // 将栈顶的指针指向新创建的节点
    // 程序头节点是语法树根节点的儿子节点
    //currentP = t;
}

// ProgramName ::= ID
void process3()
{
    for (int i = productSet[2].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[2][i]); // 栈顶元素是ID
    }
    (*currentP).idnum++; // 程序头中的标识符个数加1，因为产生式3中有一个标识
    (*currentP).name.push_back(token.sem); // 将标识符的名字加入到程序头的名字中
}

// DeclarePart ::= TypeDec VarDec ProcDec
void process4()
{
    for (int i = productSet[3].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[3][i]); // 栈顶元素是TypeDec
    }
}

// TypeDecpart ::=
void process5()
{}

// TypeDecpart ::= TypeDec
void process6()
{
    for (int i = productSet[5].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[5][i]); // 栈顶元素是TypeDec
    }
}

// TypeDec ::= TYPE TypeDecList
void process7() // 处理类型声明
{
    for (int i = productSet[6].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[6][i]);
    }
    currentP = new TreeNode();                  // 生成类型声明标志节点
    (*currentP).nodekind = TypeK;               // 类型声明标志节点
    (*currentP).lineno = token.linsShow;        // 行号
    TreeNode **t = syntaxTreeStack.top();       // 获取栈顶的指针，此时的栈顶指针指向的是TypeDec，他也作为根节点的儿子节点
    syntaxTreeStack.pop();                      // 弹出栈顶的指针
    *t = currentP;                              // 将栈顶的指针指向新创建的节点，即类型声明标志节点TypeDec

    syntaxTreeStack.push(
            &(*currentP).sibling); // 当前类型声明节点的兄弟节点应该指向变量声明标识节点，函数声明节点或者语句序列节点(因为声明完类型可能没有变量声明，函数声明，所以可能是语句序列节点)
    syntaxTreeStack.push(&(*currentP).child[0]); // 子节点应该指向具体的声明节点
}

// TypeDecList ::= TypeId EQ TypeDef SEMI TypeDecMore
void process8()
{
    for (int i = productSet[7].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[7][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = DecK; // 声明节点
    (*currentP).lineno = token.linsShow;
    TreeNode **t = syntaxTreeStack.top();
    // 若是第一个声明节点，则是 Type类型的子节点指向当前节点
    // 否则，作为上一个类型声明的兄弟节点出现。并将此节点的兄弟节点压入语法树栈，以便处理下一个类型声明。
    // 即表示成多个类型声明的链表
    syntaxTreeStack.pop(); // 弹出栈顶的指针，正好是TypeK节点的child[0]节点，表示类型声明的具体类型
    *t = currentP;
    syntaxTreeStack.push(&(*currentP).sibling);
}

// TypeDecMore ::=
void process9()
{
    // 没有其它的类型声明，这时语法树栈顶存放的是最后一个类型声明节点的兄弟节点,弹出，完成类型部分的语法树节点生成。
    syntaxTreeStack.pop();
}

// TypeDecMore ::= TypeDecList
void process10()
{
    for (int i = productSet[9].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[9][i]); // 栈顶元素是TypeDecList，右部符号入栈
    }
}

// TypeId ::= ID
void process11()
{
    for (int i = productSet[10].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[10][i]); // 栈顶元素是ID，当前节点是类型声明节点的子节点
    }
    (*currentP).idnum++; // 类型声明中的标识符个数加1
    (*currentP).name.push_back(token.sem); // 因为可能会保存多个标识符，所以将标识符的名字保存在vector中
}

// TypeDef ::= BaseType
void process12()
{
    for (int i = productSet[11].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[11][i]);
    }
    // 处理声明节点的标识符的类型部分。基本类型可以是整型和字符型，
    // 这里用变量temp记录节点上填写标识符类型信息的部分的地址,
    // 在下面的产生式处理对 temp里的内容进行赋值，就完成了类型部分的填写。
    // 即temp指向的是类型声明节点的类型部分，要完成类型部分的填写，就要对temp进行赋值
    temp = &((*currentP).kind.dec);
}

// TypeDef ::= StructureType
void process13()
{
    for (int i = productSet[12].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[12][i]);
    }
    temp = &((*currentP).kind.dec);
}

// TypeDef ::= ID 类型定义
void process14()
{
    for (int i = productSet[13].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[13][i]);
    }
    (*currentP).kind.dec = IdK; // 类类型标志符作为类型
    (*currentP).type_name = token.sem;
    (*currentP).idnum++;
    (*currentP).name.push_back(token.sem);
}

// BaseType ::= INTEGER
void process15()
{
    for (int i = productSet[14].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[14][i]);
    }
    (*temp) = IntegerK; // 整数类型
}

// BaseType ::= CHAR
void process16()
{
    for (int i = productSet[15].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[15][i]);
    }
    // temp一定都是指向类型声明节点的类型部分，所以这里对temp进行赋值，就完成了类型部分的填写
    // 只要temp被赋值为&((*currentP).kind.dec); 就一定要紧接着对temp进行赋值，否则会出错，造成temp变量的值被覆盖
    (*temp) = CharK;
}

// StructureType ::= ArrayType
void process17()
{
    for (int i = productSet[16].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[16][i]);
    }
}

void process18()
{
    for (int i = productSet[17].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[17][i]);
    }
}

void process19()
{
    for (int i = productSet[18].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[18][i]);
    }
    (*temp) = ArrayK; // 数组类型
    temp = &((*currentP).attr.arrayAttr.childType);
}

void process20()
{
    for (int i = productSet[19].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[19][i]);
    }
    (*currentP).attr.arrayAttr.low = stoi(token.sem);
}

void process21()
{
    for (int i = productSet[20].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[20][i]);
    }
    (*currentP).attr.arrayAttr.up = stoi(token.sem);
}

void process22()
{
    for (int i = productSet[21].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[21][i]);
    }
    // 声明的类型部分赋值为记录类型 RecordK,用变量saveP保存当前指向记录类型声明节点的指针，
    // 以便处理完记录的各个域以后能够回到记录类型节点处理没有完成的信息，
    // 并压入指向记录的第一个域的指针进语法树栈，在后面对指针赋值。
    (*temp) = RecordK;
    saveP = currentP;
    syntaxTreeStack.push(&(*currentP).child[0]);
}

void process23()
{
    // 生成记录类型的一个域，节点为声明类型的节点，不添加任何信息;
    // 类型属于基类型，用 temp记录填写类型信息的成员地址，以待以后填写是整数类型还是字符类型
    // 弹语法树栈，令指针指向这个节点。
    // 若是第一个，则是 record类型的子节点指向当前节点;
    // 否则，是上一个记录域声明的兄弟节点。
    // 最后，压入指向记录类型下一个域的指针，以处理多个域。
    for (int i = productSet[22].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[22][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = DecK; // 具体类型声明节点，而不是标志节点
    (*currentP).lineno = token.linsShow;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    temp = &((*currentP).kind.dec);
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process24()
{
    for (int i = productSet[23].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[23][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = DecK;
    (*currentP).lineno = token.linsShow;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

// 没有记录类型的下一个域了,弹出栈顶保存的最后一个域的兄弟节点
// 表示记录的域全部处理完;并利用saveP恢复当前记录类型节点的指针。
// 兄弟指针可能是空指针，但是也同样压入栈中
// 所有的连接操作都不是在process中完成的，而是在各种process以后完成的，延迟连接，从而构建语法树
void process25()
{
    syntaxTreeStack.pop();
    currentP = saveP;
}

void process26()
{
    for (int i = productSet[25].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[25][i]);
    }
}

void process27()
{
    for (int i = productSet[26].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[26][i]);
    }
    (*currentP).idnum++;
    (*currentP).name.push_back(token.sem);
}

void process28()
{}

void process29()
{
    for (int i = productSet[28].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[28][i]);
    }
}

void process30()
{}

void process31()
{
    for (int i = productSet[30].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[30][i]);
    }
}

/**
 * 处理变量声明，产生式右部压入符号栈;
 * 语法树部分，生成变量声明标志节点，弹语法树栈，得到指针的地址，令指针指向此声明节点，
 * 使得此节点作为根节点的儿子节点或者类型标识节点的兄弟节点出现。
 * 当前变量声明节点的兄弟节点应该指向函数声明节点或语句序列节点，而子节点则应指向具体的声明节点，
 * 故将当前节点的兄弟节点和第一个儿子节点压入语法树栈，以待以后处理。
 */
void process32()
{
    for (int i = productSet[31].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[31][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = VarK;
    (*currentP).lineno = token.linsShow;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;

    syntaxTreeStack.push(&(*currentP).sibling);
    syntaxTreeStack.push(&(*currentP).child[0]);
}

void process33()
{
    for (int i = productSet[32].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[32][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = DecK;
    (*currentP).lineno = token.linsShow;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process34()
{
    syntaxTreeStack.pop();
}

void process35()
{
    for (int i = productSet[34].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[34][i]);
    }
}

void process36()
{
    for (int i = productSet[35].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[35][i]);
    }
    (*currentP).idnum++;
    (*currentP).name.push_back(token.sem);
}

void process37()
{}

void process38()
{
    for (int i = productSet[37].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[37][i]);
    }
}

void process39()
{}

void process40()
{
    for (int i = productSet[39].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[39][i]);
    }
}

void process41()
{
    for (int i = productSet[40].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[40][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = ProcDecK;
    (*currentP).lineno = token.linsShow;
    TreeNode **t = syntaxTreeStack.top();
    saveFuncP = currentP;
    syntaxTreeStack.pop();
    *t = currentP;
    syntaxTreeStack.push(&(*currentP).sibling);
    syntaxTreeStack.push(&(*currentP).child[2]); // 函数体
    syntaxTreeStack.push(&(*currentP).child[1]); // 声明部分
    syntaxTreeStack.push(&(*currentP).child[0]); // 形参，形参最后入栈，所以是栈顶元素
}

void process42()
{}

void process43()
{
    for (int i = productSet[42].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[42][i]);
    }
}

void process44()
{
    for (int i = productSet[43].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[43][i]);
    }
    (*currentP).idnum++;
    (*currentP).name.push_back(token.sem);
}

void process45()
{
    syntaxTreeStack.pop();
}

void process46()
{
    for (int i = productSet[45].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[45][i]);
    }
}

void process47()
{
    for (int i = productSet[46].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[46][i]);
    }
}

void process48()
{
    syntaxTreeStack.pop();
    currentP = saveFuncP;
    temp = &((*currentP).kind.dec);
}

void process49()
{
    for (int i = productSet[48].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[48][i]);
    }
}

void process50()
{
    for (int i = productSet[49].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[49][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = DecK;
    (*currentP).lineno = token.linsShow;
    (*currentP).attr.procAttr.paramt = Valparamtype;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process51()
{
    for (int i = productSet[50].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[50][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = DecK;
    (*currentP).lineno = token.linsShow;
    (*currentP).attr.procAttr.paramt = Varparamtype;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process52()
{
    for (int i = productSet[51].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[51][i]);
    }
    (*currentP).idnum++;
    (*currentP).name.push_back(token.sem);
}

void process53()
{}

void process54()
{
    for (int i = productSet[53].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[53][i]);
    }
}

void process55()
{
    for (int i = productSet[54].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[54][i]);
    }
}

void process56()
{
    for (int i = productSet[55].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[55][i]);
    }
}

void process57()
{
    for (int i = productSet[56].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[56][i]);
    }
    /*TreeNode* p = proK->child[1], * q = proK;
    while (p->sibling != NULL) {
        q = p;
        p = p->sibling;
    }
    delete p;
    if (q == proK)q->child[1] = NULL;
    else q->sibling = NULL;*/

    syntaxTreeStack.pop();
    currentP = new TreeNode();
    (*currentP).nodekind = StmLK;
    (*currentP).lineno = token.linsShow;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).child[0] = new TreeNode();
    syntaxTreeStack.push(&(*currentP).child[0]);
}

void process58()
{
    for (int i = productSet[57].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[57][i]);
    }
}

void process59()
{
    syntaxTreeStack.pop();
}

void process60()
{
    for (int i = productSet[59].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[59][i]);
    }
}

void process61()
{
    for (int i = productSet[60].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[60][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = StmtK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.stmt = IfK;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process62()
{
    for (int i = productSet[61].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[61][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = StmtK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.stmt = WhileK;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process63()
{
    for (int i = productSet[62].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[62][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = StmtK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.stmt = ReadK;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process64()
{
    for (int i = productSet[63].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[63][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = StmtK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.stmt = WriteK;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process65()
{
    for (int i = productSet[64].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[64][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = StmtK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.stmt = ReturnK;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
    //currentP = t;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

//不确定
void process66()
{
    for (int i = productSet[65].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[65][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = StmtK;
    (*currentP).lineno = token.linsShow;
    TreeNode *t = new TreeNode();
    (*t).nodekind = ExpK;
    (*t).kind.exp = IdEK;
    (*t).attr.expAttr.varkind = IdV;
    (*t).lineno = token.linsShow;
    (*t).idnum++;
    (*t).name.push_back(token.sem);
    (*currentP).child[0] = t;
    TreeNode **t1 = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t1 = currentP;
    //currentP = t1;
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process67()
{
    for (int i = productSet[66].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[66][i]);
    }
    (*currentP).kind.stmt = AssignK;
}

void process68()
{
    for (int i = productSet[67].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[67][i]);
    }
    (*((*currentP).child[0])).attr.expAttr.varkind = IdV;
    (*currentP).kind.stmt = CallK;
}

//不清楚
void process69()
{
    for (int i = productSet[68].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[68][i]);
    }
    //(*currentP).child[1] = new TreeNode();
    syntaxTreeStack.push(&(*currentP).child[1]);
    currentP = (*currentP).child[0];
    TreeNode *t = new TreeNode();
    (*t).nodekind = ExpK;
    (*t).lineno = token.linsShow;
    (*t).kind.exp = OpK;
    (*t).attr.expAttr.op = END;
    opSignStack.push(t);
}

void process70()
{
    for (int i = productSet[69].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[69][i]);
    }
    //(*currentP).child[0] = new TreeNode();
    //(*currentP).child[1] = new TreeNode();
    //(*currentP).child[2] = new TreeNode();
    syntaxTreeStack.push(&(*currentP).child[2]);
    syntaxTreeStack.push(&(*currentP).child[1]);
    syntaxTreeStack.push(&(*currentP).child[0]);
}

void process71()
{
    for (int i = productSet[70].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[70][i]);
    }
    //(*currentP).child[1] = new TreeNode();
    //(*currentP).child[0] = new TreeNode();
    syntaxTreeStack.push(&(*currentP).child[1]);
    syntaxTreeStack.push(&(*currentP).child[0]);
}

void process72()
{
    for (int i = productSet[71].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[71][i]);
    }
}

void process73()
{
    for (int i = productSet[72].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[72][i]);
    }
    (*currentP).idnum++;
    (*currentP).name.push_back(token.sem);
}

void process74()
{
    for (int i = productSet[73].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[73][i]);
    }
    //(*currentP).child[0] = new TreeNode();
    syntaxTreeStack.push(&(*currentP).child[0]);
    TreeNode *t = new TreeNode;
    (*t).nodekind = ExpK;
    (*t).lineno = token.linsShow;
    (*t).kind.exp = OpK;
    (*t).attr.expAttr.op = END;
    opSignStack.push(t);
}

void process75()
{
    for (int i = productSet[74].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[74][i]);
    }
}

void process76()
{
    //(*currentP).child[0] = new TreeNode();
    for (int i = productSet[75].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[75][i]);
    }
    syntaxTreeStack.push(&(*currentP).child[1]);
}

void process77()
{
    syntaxTreeStack.pop();
}

void process78()
{
    for (int i = productSet[77].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[77][i]);
    }
    TreeNode *t = new TreeNode;
    (*t).nodekind = ExpK;
    (*t).lineno = token.linsShow;
    (*t).kind.exp = OpK;
    (*t).attr.expAttr.op = END;
    opSignStack.push(t);
}

void process79()
{}

void process80()
{
    for (int i = productSet[79].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[79][i]);
    }
    //(*currentP).sibling = new TreeNode();
    syntaxTreeStack.push(&(*currentP).sibling);
}

void process81()
{
    for (int i = productSet[80].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[80][i]);
    }
    TreeNode *t = new TreeNode;
    (*t).nodekind = ExpK;
    (*t).lineno = token.linsShow;
    (*t).kind.exp = OpK;
    (*t).attr.expAttr.op = END;
    opSignStack.push(t);
    getExpResult = false;
}

void process82()
{
    for (int i = productSet[81].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[81][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = ExpK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.exp = OpK;
    (*currentP).attr.expAttr.op = token.lex;
    LexType top = (*opSignStack.top()).attr.expAttr.op;
    while (Priosoty(top) >= Priosoty(token.lex))
    {
        TreeNode *t = opSignStack.top();
        opSignStack.pop();
        TreeNode *Rnum = opNumStack.top();
        opNumStack.pop();
        TreeNode *Lnum = opNumStack.top();
        opNumStack.pop();
        (*t).child[0] = Lnum;
        (*t).child[1] = Rnum;
        opNumStack.push(t);
        top = (*opSignStack.top()).attr.expAttr.op;
    }
    opSignStack.push(currentP);
    getExpResult = true;
}

void process83()
{
    for (int i = productSet[82].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[82][i]);
    }
}

void process84()
{
    if (token.lex == RPAREN && expflag > 0)
    {
        LexType top = (*opSignStack.top()).attr.expAttr.op;
        while (top != LPAREN)
        {
            TreeNode *t = opSignStack.top();
            opSignStack.pop();
            TreeNode *Rnum = opNumStack.top();
            opNumStack.pop();
            TreeNode *Lnum = opNumStack.top();
            opNumStack.pop();
            (*t).child[0] = Lnum;
            (*t).child[1] = Rnum;
            opNumStack.push(t);
            top = (*opSignStack.top()).attr.expAttr.op;
        }
        opSignStack.pop();
        expflag--;
    }
    else
    {
        if (getExpResult || getExpResult2)
        {
            LexType top = (*opSignStack.top()).attr.expAttr.op;
            while (top != END)
            {
                TreeNode *t = opSignStack.top();
                opSignStack.pop();
                TreeNode *Rnum = opNumStack.top();
                opNumStack.pop();
                TreeNode *Lnum = opNumStack.top();
                opNumStack.pop();
                (*t).child[0] = Lnum;
                (*t).child[1] = Rnum;
                opNumStack.push(t);
                top = (*opSignStack.top()).attr.expAttr.op;
            }
            opSignStack.pop();
            currentP = opNumStack.top();
            opNumStack.pop();
            TreeNode **t = syntaxTreeStack.top();
            syntaxTreeStack.pop();
            *t = currentP;
            getExpResult2 = false;
        }
    }
}

void process85()
{
    for (int i = productSet[84].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[84][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = ExpK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.exp = OpK;
    (*currentP).attr.expAttr.op = token.lex;
    LexType top = (*opSignStack.top()).attr.expAttr.op;
    while (top != LPAREN && Priosoty(top) >= Priosoty(token.lex))
    {
        TreeNode *t = opSignStack.top();
        opSignStack.pop();
        TreeNode *Rnum = opNumStack.top();
        opNumStack.pop();
        TreeNode *Lnum = opNumStack.top();
        opNumStack.pop();
        (*t).child[0] = Lnum;
        (*t).child[1] = Rnum;
        opNumStack.push(t);
        top = (*opSignStack.top()).attr.expAttr.op;
    }
    opSignStack.push(currentP);
}

void process86()
{
    for (int i = productSet[85].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[85][i]);
    }
}

void process87()
{}

void process88()
{
    for (int i = productSet[87].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[87][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = ExpK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.exp = OpK;
    (*currentP).attr.expAttr.op = token.lex;
    LexType top = (*opSignStack.top()).attr.expAttr.op;
    while (top != LPAREN && Priosoty(top) >= Priosoty(token.lex))
    {
        TreeNode *t = opSignStack.top();
        opSignStack.pop();
        TreeNode *Rnum = opNumStack.top();
        opNumStack.pop();
        TreeNode *Lnum = opNumStack.top();
        opNumStack.pop();
        (*t).child[0] = Lnum;
        (*t).child[1] = Rnum;
        opNumStack.push(t);
        top = (*opSignStack.top()).attr.expAttr.op;
        opSignStack.push(currentP);
    }
}

void process89()
{
    for (int i = productSet[88].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[88][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = ExpK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.exp = OpK;
    (*currentP).attr.expAttr.op = LPAREN;
    opSignStack.push(currentP);
    expflag++;
}

void process90()
{
    for (int i = productSet[89].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[89][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = ExpK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.exp = ConstK;
    (*currentP).attr.expAttr.val = stoi(token.sem);
    opNumStack.push(currentP);
}

void process91()
{
    for (int i = productSet[90].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[90][i]);
    }
}

void process92()
{
    for (int i = productSet[91].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[91][i]);
    }
    currentP = new TreeNode();
    (*currentP).nodekind = ExpK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.exp = IdEK;
    (*currentP).idnum++;
    (*currentP).name.push_back(token.sem);
    opNumStack.push(currentP);
}

void process93()
{
    (*currentP).attr.expAttr.varkind = IdV;
}

void process94()
{
    for (int i = productSet[93].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[93][i]);
    }
    (*currentP).attr.expAttr.varkind = ArrayMembV;
    //(*currentP).child[0] = new TreeNode();
    syntaxTreeStack.push(&(*currentP).child[0]);
    TreeNode *t = new TreeNode();
    (*t).nodekind = ExpK;
    (*t).lineno = token.linsShow;
    (*t).kind.exp = OpK;
    (*t).attr.expAttr.op = END;
    opSignStack.push(t);
    getExpResult2 = true;
}

void process95()
{
    for (int i = productSet[94].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[94][i]);
    }
    (*currentP).attr.expAttr.varkind = FieldMembV;
    //(*currentP).child[0] = new TreeNode();
    syntaxTreeStack.push(&(*currentP).child[0]);
}

void process96()
{
    for (int i = productSet[95].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[95][i]);
    }
    currentP = new TreeNode();
    (*currentP).idnum++;
    (*currentP).name.push_back(token.sem);
    (*currentP).nodekind = ExpK;
    (*currentP).lineno = token.linsShow;
    (*currentP).kind.exp = IdEK;
    TreeNode **t = syntaxTreeStack.top();
    syntaxTreeStack.pop();
    *t = currentP;
}

void process97()
{
    (*currentP).attr.expAttr.varkind = IdV;
}

void process98()
{
    for (int i = productSet[97].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[97][i]);
    }
    (*currentP).attr.expAttr.varkind = ArrayMembV;
    //(*currentP).child[0] = new TreeNode();
    syntaxTreeStack.push(&(*currentP).child[0]);
    TreeNode *t = new TreeNode();
    (*t).nodekind = ExpK;
    (*t).lineno = token.linsShow;
    (*t).kind.exp = OpK;
    (*t).attr.expAttr.op = END;
    opSignStack.push(t);
    getExpResult2 = true;
}

void process99()
{
    for (int i = productSet[98].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[98][i]);
    }
}

void process100()
{
    for (int i = productSet[99].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[99][i]);
    }
}

void process101()
{
    for (int i = productSet[100].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[100][i]);
    }
}

void process102()
{
    for (int i = productSet[101].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[101][i]);
    }
}

void process103()
{
    for (int i = productSet[102].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[102][i]);
    }
}

void process104()
{
    for (int i = productSet[103].size() - 1; i >= 1; i--)
    {
        signStack.push(productSet[103][i]);
    }
}