#include <algorithm>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <list>
#include <assert.h>
#include <fstream>

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

int CheckColor(vector<int> &vecInt, int n)
{
	map<int, int> num2color;


	// 1. 构建相交图
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
	
	// 2. 基于栈启发式的图着色
	map<int, set<int> > tmpGraph(graph.begin(), graph.end() );
	list<int> numStack;
	// 2.1 push into stack
	list<int> tmps(vecInt.begin(), vecInt.begin()+n);	
	while( !tmps.empty() )
	{
		// 找出度最小的结点
		unsigned minDegree = 4;
		int minNum = 1;
		list<int>::iterator i_p = tmps.begin(), i_e = tmps.end();
		for(; i_p != i_e; ++ i_p )
		{
			int num = *i_p;
			if( tmpGraph[num].size() < minDegree )
			{
				minDegree = graph[num].size();
				minNum = num;
			}
		}
		// 移除该结点并压栈
		tmps.remove(minNum);
		i_p = tmps.begin(), i_e = tmps.end();
		for(; i_p != i_e; ++ i_p )
		{
			int num = *i_p;
			tmpGraph[num].erase(minNum);
		}
		numStack.push_front(minNum);
	}

	// 2.2 pop off stack and coloring
	int maxColor = 1;
	while(!numStack.empty())
	{
		int num = numStack.front();
		numStack.pop_front();
		int color = 1;
		for( ; color < 5; ++ color)
		{
			bool conflict = false;
			set<int>::iterator s_p = graph[num].begin(), s_e = graph[num].end();
			for(; s_p != s_e; ++s_p )
			{
				if( num2color[*s_p] == color )
				{
					conflict = true;
					break;
				}
			}

			if( conflict ) // 如果冲突，试下一个颜色
				continue;
			else
				break;
		}
		assert(color < 5);
		if( color > maxColor )
			maxColor = color;
		num2color[num] = color;
	}


	// 输出着色信息
#ifdef DUMP
	dump(vecInt, n, 1);
	for( int i = 0; i < n; i ++ )
	{
		int num = vecInt[i];
		cout << num2color[num];
	}
	cout << '(';
	for( int i = 0; i < n; i ++ )
	{
		int num = vecInt[i];
		if( i != 0 )
			cout << ',';
		cout << num << "->" << num2color[num];
	}
	cout << ")\t" << maxColor << endl;
#else
	g_perms[maxColor].push_back(vecInt);
	vector<int> colors;
	for( int i = 0; i < n; i ++ )
	{
		int num = vecInt[i];
		colors.push_back(num2color[num]);
	}
	g_colors[maxColor].push_back(colors);
#endif
	return maxColor;
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

	std::vector<int> vecInt(MAX);
	for(int i = 0; i < n; ++ i )
	{
		vecInt[i]=i+1;
	}

	do
	{	
		CheckColor(vecInt, n);
	}
	while(std::next_permutation(vecInt.begin(), vecInt.begin()+n));
	
	//输出结果
	char digits[16];
	_itoa(n, digits, 10);
	string szFile(digits);
	szFile +=  ".txt";
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
		g_ofs << "------------颜色种需要" << perm_p->first << "的图有" << perms.size() << "个，具体统计如上----------" << endl;
	}
	int total = 1;
	for( int i = 1; i <= n; ++ i )
	{
		total = total * i;
	}
	g_ofs << "总共有: " << total << " 个" << endl;
	g_ofs << "需要颜色种数为：2 的个数有：" << g_perms[2].size() << " 个" << endl;
	g_ofs << "需要颜色种数为：3 的个数有：" << g_perms[3].size() << " 个" << endl;
	g_ofs << "需要颜色种数为：4 的个数有：" << g_perms[4].size() << " 个" << endl;
	g_ofs.close();
	return 0;
}