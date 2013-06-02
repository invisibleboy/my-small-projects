#include <algorithm>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <list>
#include <assert.h>
#include <fstream>

//#define DUMP
#define MAX 16
using namespace std;

ofstream g_ofs;

map<int, vector<vector<int> > > g_perms;
map<int, vector<vector<int> > > g_colors;
map<int, vector<int> > g_numColors;
int dump(ostream &os, vector<int> &vecInt, int n, char delimiter1=' ', char delimiter2 = '\n')
{
	for( int i = 0; i < n; i ++ )
	{
		os << vecInt[i] << delimiter1;
	}
	os << delimiter2;
	return 0;
}





bool CheckColorbyEnum(vector<int> &vecInt, int n, int nColors)
{
	map<int, int> num2color;


	// 1. �����ཻͼ
	map<int, set<int> > graph;
	for( int i = 0; i < n; i ++ )
	{		

		int num = vecInt[i];
		if( num > 1 )
			graph[num].insert(num - 1);
		if( num < n )
			graph[num].insert(num + 1);
		if( i > 0 )
			graph[num].insert(vecInt[i-1]);
		if( i < n - 1 )
			graph[num].insert(vecInt[i+1]);
	}
	
	// 2. ���ڻ��ݵ�ͼ��ɫ
	map<int, set<int> > tmpGraph(graph.begin(), graph.end() );
	list<int> numStack;
    int i = 1;
	bool bSucc = false;
	while( i >= 1 )
	{
		int j = num2color[i] + 1;
		bool bColor = false;
		while( j <= nColors && !bColor)
		{
			set<int> &ajacents = graph[i];
			set<int>::iterator a_p = ajacents.begin(), a_e = ajacents.end();
			for(; a_p!= a_e; ++ a_p ) // �����ɫj�Ƿ���ڽڵ�i����
			{
				if(num2color[*a_p] == j )
					break;
			}
			if( a_p == a_e ) // �ҵ�����ɫj
			{
				num2color[i] = j;
				bColor = true;
			}
			++ j;
		}
		if( bColor ) // ��ɫ�ɹ�
		{
			if( i == n )
			{
				bSucc = true;
				break;
			}
			else
				++ i;
		}
		else  // ����
		{
			num2color[i] = 0;
			-- i;			
		}
	}
	
	if( !bSucc )
		return bSucc;


	
	// �����ɫ��Ϣ
#ifdef DUMP
	dump(g_ofs, vecInt, n, ' ');
	for( int i = 0; i < n; i ++ )
	{
		int num = vecInt[i];
		g_ofs << num2color[num];
	}
	g_ofs << '(';
	for( int i = 0; i < n; i ++ )
	{
		int num = vecInt[i];
		if( i != 0 )
			g_ofs << ',';
		g_ofs << num << "->" << num2color[num];
	}
	g_ofs << ")\t" << nColors << endl;
#else
	g_perms[nColors].push_back(vecInt);
	vector<int> colors;
	for( int i = 0; i < n; i ++ )
	{
		int num = vecInt[i];
		colors.push_back(num2color[num]);
	}
	g_colors[nColors].push_back(colors);
#endif
	return bSucc;
}

int main(int argc, char ** argv)
{
	if( argc < 2 )
	{
		cerr << "Need one argument (n) to specifiy the numerial array to permutate!" << endl;
		getchar();
		return -1;
	}

	istringstream ss(argv[1]);
	int n = 0;
	ss >> n;
	if( n == 0 || n > MAX )
	{
		cerr << "Argument n can not either be zero or be greater than " << MAX <<" !" << endl;
		getchar();
		return -1;
	}

	std::vector<int> vecInt;
	vecInt.assign(n,0);
	for(int i = 0; i < n; ++ i )
	{
		vecInt[i]=i+1;
	}

	do
	{	
		int nColors = 1;
		while( !CheckColorbyEnum(vecInt, n, ++nColors) );
		
	}
	while(std::next_permutation(vecInt.begin(), vecInt.begin()+n));
	
	//������
	char digits[16];
	_itoa(n, digits, 10);
	string szFile(digits);
#ifdef DUMP
	szFile += ".err";
#else
	szFile +=  ".txt";
#endif
	g_ofs.open(szFile.c_str());
	map<int, vector<vector<int> > >::iterator perm_p = g_perms.begin(), perm_e = g_perms.end();
	for(; perm_p != perm_e; ++ perm_p )
	{
		vector<vector<int> > &perms = perm_p->second;
		for( int i = 0; i < perms.size(); ++ i)
		{
			vector<int> &perm = perms.at(i);
			vector<int> &color = g_colors[perm_p->first].at(i);

			dump(g_ofs, perm, n, ' ', '\t');
			dump(g_ofs, color, n, ' ', ' ');
			g_ofs << '(';
			for( int i = 0; i < n; i ++ )
			{
				if( i != 0 )
					g_ofs << ',';
				g_ofs << perm[i] << "->" << color[i];
			}
			g_ofs << ")\t" << perm_p->first << endl;				
		}
		g_ofs << "------------��ɫ����Ҫ" << perm_p->first << "��ͼ��" << perms.size() << "��������ͳ������----------" << endl;
	}
	int total = 1;
	for( int i = 1; i <= n; ++ i )
	{
		total = total * i;
	}
	g_ofs << "�ܹ���: " << total << " ��" << endl;
	g_ofs << "��Ҫ��ɫ����Ϊ��2 �ĸ����У�" << g_perms[2].size() << " ��" << endl;
	g_ofs << "��Ҫ��ɫ����Ϊ��3 �ĸ����У�" << g_perms[3].size() << " ��" << endl;
	g_ofs << "��Ҫ��ɫ����Ϊ��4 �ĸ����У�" << g_perms[4].size() << " ��" << endl;
	g_ofs.close();
	return 0;
}