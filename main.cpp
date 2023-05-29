#include"header.h"
#include"zyg.h"
#include"zjh.h"

void readTokenFromFile()
{
    ifstream fin("token.txt");
    string line;
    while (getline(fin, line))
    {
        cout << line << endl;
    }
    fin.close();
}

int main()
{
    //词法分析
    GetNextChar();
    cout << "Token生成结束" << endl;


    //语法分析，生成抽象语法树
    TreeNode *root = DesParse();
    //TreeNode *root = Parse();
    cout << "语法分析结束" << endl;

    //语义分析
    Analyze(root);
    system("pause");

    return 0;
}
