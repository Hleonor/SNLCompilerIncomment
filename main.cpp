#include"hyf.h"
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
    //�ʷ�����
    GetNextChar();
    // ��token.txt��һ��һ�ж�ȡtoken���
    // readTokenFromFile();


    //�﷨���������ɳ����﷨��
     //TreeNode *root = DesParse();
    TreeNode *root = Parse();
    cout << "�﷨��������" << endl;

    //�������
    Analyze(root);
    system("pause");
    return 0;
}
