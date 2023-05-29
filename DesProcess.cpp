#include"header.h"

/*******************��������************************/
string arrayLexTypeD_hyf[] = {"ID", "IF", "BEGIN", "INTC", "END", "PLUS", "MINUS", "TIMES", "OVER", "EQ", "LT",
                              "LMIDPAREN", "RMIDPAREN", "DOT", "TYPE", "VAR", "PROCEDURE", "PROGRAM", "SEMI", "INTEGER",
                              "CHAR", "ARRAY", "RECORD", "UNDERANGE", "OF", "COMMA", "LPAREN", "RPAREN", "ENDWH",
                              "WHILE", "RETURN", "READ", "WRITE", "ASSIGN", "THEN", "FI", "ELSE", "DO"
};
string temp_name;
Token token;        // �洢��ǰToken
ifstream in("token.txt"); // ��ȡtoken.txt

/*******************��������************************/
TreeNode *DesParse()
{
    //����token
    ReadToken(); // �����һ��token
    TreeNode *t = Program(); // �﷨�����������﷨�����ڵ�
    if (in.eof()) // �����Ѿ���ȫ����
    {
        Input(t, "treeDes.txt");
    }
    else // ����program���������Ժ�in��û�����ļ���β��˵���ļ���ǰ�����ˣ����������Ϣ
    {
        string a = "�ļ���ǰ����";
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
    TreeNode *t = ProgramHead(); // �﷨��������ͷ
    TreeNode *q = DeclarePart(); // �﷨������������
    TreeNode *s = ProgramBody(); // �﷨����������
    if (t == NULL)
    {
        string a = "û�г���ͷ";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
    }
    else
    {
        (*root).child[0] = t;
    }
    if (q == NULL)
    {
        string a = "û������";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        //exit(0);
    }
    else
    {
        (*root).child[1] = q;
    }
    if (s == NULL)
    {
        string a = "û�г�����";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        //exit(0);
    }
    else
    {
        (*root).child[2] = s;
    }
    Match(DOT);
    return root;
}

// �������ͷ
TreeNode *ProgramHead()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = PheadK;
    (*t).lineno = token.linsShow; // ��ǰtoken��������
    Match(PROGRAM); // ��һ��tokenӦ����PROGRAM�����ĵ�
    if (token.lex == ID) // ���ĵ�PROGRAM�Ժ�Ͷ�ȡ��һ��token�ˣ������ǰtoken��һ����ʶ������
    {
        (*t).idnum++; // ��ǰ����ͷ�ı�ʶ����1
        (*t).name.push_back(token.sem); // ����ǰtoken������ֵ�������ͷ��name�����У������Ǳ����֣������±꣬Ҳ�����Ǳ�ʶ��
    }
    Match(ID); // ���ĵ���ʶ��������ȡ��һ��token
    return t;
}

// ������������
TreeNode *DeclarePart()
{
    // �������ֿ������������������������������������������
    // sibling�������ǽ���������������������γ�һ�����������Ϳ������﷨�������ֵܽڵ����ʽ�洢�����������ӽڵ����ʽ�洢
    // ����֮��Ĺ�ϵ�ǲ��е�
    TreeNode *typeP = NULL, *varP = NULL;
    int line = token.linsShow; // line��¼��ǰtoken�������������ڴ�����
    TreeNode *tp1 = TypeDec(); // ������������
    if (tp1 != NULL)
    {
        typeP = new TreeNode();
        (*typeP).nodekind = TypeK; // ����������־�ڵ�
        (*typeP).child[0] = tp1;
        (*typeP).lineno = line;
    }
    line = token.linsShow;
    TreeNode *tp2 = VarDec(); // ������������
    if (tp2 != NULL)
    {
        varP = new TreeNode();
        (*varP).nodekind = VarK; // ����������־�ڵ�
        (*varP).child[0] = tp2;
        (*varP).lineno = line;
    }
    TreeNode *s = ProcDec(); // ������������
    if (varP == NULL) // ���������������Ϊ��
    {
        if (typeP == NULL) // ���������������ҲΪ��
        {
            typeP = s; // ������������ֻ�й����������֡�
        }
        else
        {
            (*typeP).sibling = s; // �����������ֲ�Ϊ�գ�����������������Ϊ�����������ֵ��ֵܽڵ�
        }
    }
    else // ��������������ֲ�Ϊ�գ��б�����������
    {
        (*varP).sibling = s; // ����������������Ϊ�����������ֵ��ֵܽڵ�
        if (typeP == NULL)
        {
            // ������������ֻ�б����������ֺ͹�����������
            // �������������ֵĸ��ڵ� varP ��ֵ�� typeP��ʹ�������������ֵĸ��ڵ���� varP
            typeP = varP;
        }
        else
        {
            // ��� typeP ��Ϊ NULL����ʾ��������������
            // �������������ֵĸ��ڵ� varP ��Ϊ�����������ֵĸ��ڵ� typeP ���ֵܽڵ㣬����������������
            (*typeP).sibling = varP;
        }
    }
    return typeP;
}

// ����������������
TreeNode *TypeDec()
{
    TreeNode *t = NULL;
    // ��������������������������ص��﷨���ڵ�ָ��洢�� t �����С�
    if (token.lex == TYPE)
    {
        t = TypeDeclaration();
    }
    // �����ǰ�Ǻŵ������� VAR��PROCEDURE �� BEGIN�����ʾû�������������֣�ֱ��������
    else if (token.lex == VAR || token.lex == PROCEDURE || token.lex == BEGIN)
    {}
    // �����ǰ�Ǻŵ����Ͳ�������������������ʾ��ǰ�ǺŲ��Ϸ���������������һ��token
    else
    {
        ReadToken();
        string a = to_string(token.linsShow) + ":��ǰ����" + arrayLexTypeD_hyf[(int) token.lex] + "���ϸ�����";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
    }
    return t;
}

// ������������
TreeNode *TypeDeclaration()
{
    Match(TYPE); // �����ǰ������ TYPE�����ĵ�
    TreeNode *t = TypeDecList();
    if (t == NULL) // û�н�������Ч����������
    {
        // ��ʾ��ʾ��Ϣ
    }
    return t;
}

// �������Ͷ����б�
TreeNode *TypeDecList()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = DecK; // �����ڵ�
    (*t).lineno = token.linsShow;
    TypeId(t); // �������ͱ�ʶ������ΪSNL����д����������д�������ģ����������Ƚ������ͱ�ʶ��
    Match(EQ); // ƥ�䵱ǰ�Ǻ��Ƿ�Ϊ�Ⱥ�
    TypeDef(t); // ������������Ͷ���
    Match(SEMI); // ƥ�䵱ǰ�Ǻ��Ƿ�Ϊ�ֺ�
    TreeNode *p = TypeDecMore();
    if (p != NULL)
    { (*t).sibling = p; }
    return t;
}

// �������ܴ��ڵ��������Ͷ���
TreeNode *TypeDecMore()
{
    TreeNode *t = NULL;
    if (token.lex == ID) // �����ǰ�Ǻ��Ǳ�ʶ������ʾ���ܴ����������Ͷ���
    {
        t = TypeDecList(); // �����������Ͷ���
    }
    else if (token.lex == VAR || token.lex == PROCEDURE || token.lex == BEGIN)
    {}
    else
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
    }
    return t;
}

// �����������������ͣ�Ȼ�󽫾�������ͱ��浽�﷨���ڵ��У��������ʶ��
void TypeId(TreeNode *t)
{
    if (token.lex == ID && t != NULL)
    {
        (*t).idnum++; // ���ͱ�ʶ��������һ
        (*t).name.push_back(token.sem);
    }
    Match(ID); // ���ĵ���ǰ�Ǻ�
}

// �������嵽����ʲô����
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
            (*t).kind.dec = IdK; // �����ͱ�־����Ϊ����
            (*t).idnum++;
            (*t).name.push_back(token.sem);
            (*t).type_name = token.sem;
            Match(ID);
        }
        else
        {
            string a = to_string(token.linsShow) + "���д���";
            InputError(a, "error.txt");
            //cout << "�ļ���ǰ����";
            exit(0);
        }
    }
}

void BaseType(TreeNode *t)
{
    if (token.lex == INTEGER)
    {
        Match(INTEGER); // ���ĵ���ǰ������
        // ��¼�﷨���ڵ�ľ�������,����kind��������union Kind����¼�﷨���ڵ����������,��nodekind=DecKʱ��Ч
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
    Match(ARRAY); // ���ĵ���ǰ������
    Match(LMIDPAREN); // ƥ�䵱ǰ�Ǻ��Ƿ�Ϊ��������
    if (token.lex == INTC) // �����ǰ�Ǻ�����������ʾ��������½�
    {
        (*t).attr.arrayAttr.low = stoi(token.sem);
    }
    Match(INTC);
    Match(UNDERANGE); // ���ĵ�..
    if (token.lex == INTC) // �����ǰ�Ǻ�����������ʾ��������Ͻ�
    {
        (*t).attr.arrayAttr.up = stoi(token.sem);
    }
    Match(INTC); // ���ĵ���ǰ��ǰ������
    Match(RMIDPAREN); // ƥ�䵱ǰ�Ǻ��Ƿ�Ϊ��������
    Match(OF); // ƥ�䵱ǰ�Ǻ��Ƿ�ΪOF
    BaseType(t); // ������������
    // �﷨���ڵ���������
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������Ϣ��ʾ
    }
    Match(END);
}

// �����ṹ������
TreeNode *FieldDecList()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = DecK;
    (*t).lineno = token.linsShow;
    TreeNode *p = NULL;
    if (token.lex == INTEGER || token.lex == CHAR)
    {
        BaseType(t);
        IdList(t); // ������ʶ���б�
        Match(SEMI); // ƥ�䵱ǰ�Ǻ��Ƿ�Ϊ�ֺ�
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ�����ʣ��������ڶ�û��ƥ����ˣ�������һ�����ʲ������д�����
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
            string a = to_string(token.linsShow) + "���д���";
            InputError(a, "error.txt");
            //cout << "�ļ���ǰ����";
            exit(0);
            //������һ������
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
    return t;
}

TreeNode *VarDeclaration()
{
    Match(VAR); // ���ĵ���ǰ��VAR
    TreeNode *t = VarDecList();
    if (t == NULL)
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //�﷨������Ϣ��ʾ
    }
    return t;
}

TreeNode *VarDecList()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = DecK;
    (*t).lineno = token.linsShow;
    TreeNode *p = NULL;
    TypeDef(t); // �������ͱ�ʶ��
    VarIdList(t); // ����������ʶ���б�
    Match(SEMI); // ƥ�䵱ǰ�Ǻ��Ƿ�Ϊ�ֺ�
    p = VarDecMore(); // �����������ܴ��ڵ�������������
    (*t).sibling = p; // ����������������Ϊ�ֵܽڵ�
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������ʾ��������һ������
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
}

// ������������
TreeNode *ProcDec()
{
    TreeNode *t = NULL;
    if (token.lex == BEGIN)
    {}
    else if (token.lex == PROCEDURE)
    {
        t = ProcDeclaration(); // ������������
    }
    else
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
    return t;
}

// ����������������
TreeNode *ProcDeclaration()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = ProcDecK;
    (*t).lineno = token.linsShow;
    Match(PROCEDURE);
    if (token.lex == ID)
    {
        (*t).idnum++;
        (*t).name.push_back(token.sem); // ��¼������
        Match(ID);
    }
    Match(LPAREN); // ƥ�䵱ǰ�Ǻ��Ƿ�Ϊ�����ţ�
    ParamList(t); // ���������б�
    Match(RPAREN); // ƥ�䵱ǰ�Ǻ��Ƿ�Ϊ������
    Match(SEMI); // ƥ��ֺ�
    (*t).child[1] = ProcDecPart(); // ����������������
    (*t).child[2] = ProcBody(); // ����������
    (*t).sibling = ProcDecMore(); // ����������������
    return t;
}

// ����������������
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
    return t;
}

// ���������б�
void ParamList(TreeNode *t)
{
    TreeNode *p = NULL;
    if (token.lex == RPAREN)
    {}
    else if (token.lex == INTEGER || token.lex == CHAR || token.lex == ARRAY
             || token.lex == RECORD || token.lex == ID || token.lex == VAR)
    {
        p = ParamDecList(); // �������������б�
        (*t).child[0] = p;
    }
    else
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
}

// �������������б�
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

// ����������������
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
            //������ʾ
            string a = to_string(token.linsShow) + "���д���";
            InputError(a, "error.txt");
            //cout << "�ļ���ǰ����";
            exit(0);
        }
    }
    else
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
    return t;
}

// ��������
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
        TypeDef(t); // ��������
        FormList(t); // ���ұ�����
    }
    else
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
    return t;
}

// ������ʽ�����б�
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //��ȡ��һ������
    }
}

TreeNode *ProcDecPart()
{
    return DeclarePart();
}

TreeNode *ProcBody()
{
    TreeNode *t = ProgramBody(); // ����������
    if (t == NULL)
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������Ϣ��ʾ
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
    return t;
}

// ���
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //��ǰ����Ϊ�������ʣ������������﷨����
        //������ǰ���ʣ�������һ������
    }
    return t;
}

void AssCall(TreeNode *t)
{
    Match(ID);
    // ����Ǹ�ֵ���
    // ASSIGN: x := 1
    // LMIDPAREN: x[1] := 1
    // DOT: x.a := 1
    if (token.lex == ASSIGN || token.lex == LMIDPAREN || token.lex == DOT)
    {
        AssignmentRest(t); // ����������ֵ����ʣ�ಿ��
        t->kind.stmt = AssignK;
    }
    else if (token.lex == LPAREN) // ��ʶ�������Ժ��������ţ�˵���Ǻ�������
    {
        CallStmRest(t);
        t->kind.stmt = CallK;
    }
    else
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
}

// ��������
void AssignmentRest(TreeNode *t)
{
    VariMore(t->child[0]); // �Ǵ�������ĺ������֣��������������������ȵ����
    Match(ASSIGN);
    t->child[1] = Exp();
}

// �����������
TreeNode *ConditionalStm()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = StmtK;
    (*t).kind.stmt = IfK;
    (*t).lineno = token.linsShow;
    Match(IF); // ����if
    (*t).child[0] = Exp(); // �����������ʽ
    Match(THEN); // ����then
    (*t).child[1] = StmList(); // ����then��������
    if (token.lex == ELSE)
    {
        Match(ELSE);
        (*t).child[2] = StmList();
    }
    Match(FI);
    return t;
}

// ����ѭ�����
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

// �����������
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

// ����������
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

// ���������
TreeNode *ReturnStm()
{
    TreeNode *t = new TreeNode();
    (*t).nodekind = StmtK;
    (*t).kind.stmt = ReturnK;
    (*t).lineno = token.linsShow;
    Match(RETURN);
    return t;
}

// ����������ʣ�ಿ��
void CallStmRest(TreeNode *t)
{
    Match(LPAREN);
    (*t).child[1] = ActParamList();
    Match(RPAREN);
}

// ����ʵ���б�
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
            (*t).sibling = ActParamMore(); // �ݹ鴦��ʵ���б�
        }
    }
    else
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
    return t;
}

TreeNode *ActParamMore() // ʵ���б������ܻ���ʵ��
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ������
    }
    return t;
}

TreeNode *Exp() // ������ʽ
{
    int line = token.linsShow;
    TreeNode *t = Simple_exp(); // ����򵥱��ʽ
    if (token.lex == LT || token.lex == EQ) // �����С�ڻ��ߵ���
    {
        TreeNode *p = new TreeNode();
        (*p).lineno = line;
        (*p).nodekind = ExpK;
        (*p).kind.exp = OpK; // ���ʽ�������ǲ�����
        (*p).child[0] = t;
        (*p).attr.expAttr.op = token.lex; // ������������
        t = p;
        Match(token.lex);
        if (t != NULL)
        {
            (*t).child[1] = Simple_exp();
        }
    }
    return t;
}

TreeNode *Simple_exp() // ����򵥱��ʽ
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
    if (token.lex == INTC) // ��ǰ����������
    {
        t = new TreeNode();
        (*t).lineno = token.linsShow;
        (*t).nodekind = ExpK;
        (*t).kind.exp = ConstK;
        (*t).attr.expAttr.val = stoi(token.sem);
        Match(INTC);
    }
    else if (token.lex == ID) // ��ǰ��������һ����ʶ�������ñ�ʶ���Ĵ�����
    {
        t = Variable();
    }
    else if (token.lex == LPAREN) // ��ǰ��������һ�����ʽ
    {
        Match(LPAREN);
        t = Exp();
        Match(RPAREN);
    }
    else
    {
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //��ǰ����Ϊ�������ʣ�
        //���������ʴ���,��ʾ�����ʺ���Ϣ.��������һ����
    }
    return t;
}

TreeNode *Variable() // �����ʶ��
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ��token������ʾ������Ϣ
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
        string a = to_string(token.linsShow) + "���д���";
        InputError(a, "error.txt");
        //cout << "�ļ���ǰ����";
        exit(0);
        //������һ��token������ʾ������Ϣ
    }
}

// ��κ���������ƥ��token�ģ�ֻ�е�ǰȫ�ֱ���token.lex�ʹ���Ĳ���expected���ʱ���Ż����������һ��token������ͻᱨ��
void Match(LexType expected)
{
    if (token.lex == expected)
    {
        ReadToken(); // ������һ��token
    }
    else
    {
        string a = to_string(token.linsShow) + "���д���";
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