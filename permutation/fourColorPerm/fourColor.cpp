#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <assert.h>
#include <algorithm>
#include "fourColor.h"

using namespace std;

#define COMBO_NUM 4


vector<vector<int> > g_fourPerms;
ofstream g_ofs;

int g_nTotal = 0;
int g_nK4=0;
int g_nTwoFold=0;
int g_nUnsatisfy=0;

int dump(const vector<int> &vecInt, int n)
{
	g_ofs << "<";
	for(int i = 1; i <= n; ++ i )
		g_ofs << vecInt[i] << ' ';
	g_ofs << ">";
	return 0;
}

int readFourColor(string szFilename)
{
	ifstream inf;
	inf.open(szFilename.c_str());

	string szLine;
	
	while(getline(inf, szLine) )
	{
		// ��ȡ���ݿ�ʼ��־
		if(szLine.find("��ɫ����Ҫ3��ͼ") != string::npos)
		{
			break;
		}
	}

	while(getline(inf, szLine) )
	{
		// ��ȡ���ݽ�����־
		if(szLine.find("��ɫ����Ҫ4��ͼ") != string::npos)
		{
			break;
		}
		else
		{
			// ������ɫ��Ϊ4������
			istringstream ss(szLine);
			int num;
			vector<int> vecInt;
			vecInt.push_back(0);
			for(int i = 0;i < 8; i ++ )
			{
				ss >> num;
				vecInt.push_back(num);
			}
			g_fourPerms.push_back(vecInt);
		}
	}

	inf.close();

	return 0;
}

int isNormalDiamond(vector<int> &vecInt, map<int, set<int> > &graph, int &num1, int &num2)
{
	
	int one = vecInt[1], two = vecInt[2], three = vecInt[3], four = vecInt[4];
	// �ҵ�K4ͼ��
	if(graph[one].find(two) != graph[one].end()
		&& graph[one].find(three) != graph[one].end()
		&& graph[one].find(four) != graph[one].end() 
		&& graph[two].find(three) != graph[two].end() 
		&& graph[two].find(four) != graph[two].end()
		&& graph[three].find(four) != graph[three].end() )		
	{
		return 0;
	}
	// ������ȱ(1-2)
	if(graph[one].find(two) == graph[one].end()
		&& graph[one].find(three) != graph[one].end()
		&& graph[one].find(four) != graph[one].end() 
		&& graph[two].find(three) != graph[two].end() 
		&& graph[two].find(four) != graph[two].end()
		&& graph[three].find(four) != graph[three].end() )		
	{
		num1 = one;
		num2 = two;
		return 1;
	}
	//(1-3)
	if(graph[one].find(two) != graph[one].end()
		&& graph[one].find(three) == graph[one].end()
		&& graph[one].find(four) != graph[one].end() 
		&& graph[two].find(three) != graph[two].end() 
		&& graph[two].find(four) != graph[two].end()
		&& graph[three].find(four) != graph[three].end() )		
	{
		num1 = one;
		num2 = three;
		return 1;
	}
	// (1-4)
	if(graph[one].find(two) != graph[one].end()
		&& graph[one].find(three) != graph[one].end()
		&& graph[one].find(four) == graph[one].end() 
		&& graph[two].find(three) != graph[two].end() 
		&& graph[two].find(four) != graph[two].end()
		&& graph[three].find(four) != graph[three].end() )		
	{
		num1 = one;
		num2 = four;
		return 1;
	}
	// (2-3)
	if(graph[one].find(two) != graph[one].end()
		&& graph[one].find(three) != graph[one].end()
		&& graph[one].find(four) != graph[one].end() 
		&& graph[two].find(three) == graph[two].end() 
		&& graph[two].find(four) != graph[two].end()
		&& graph[three].find(four) == graph[three].end() )		
	{
		num1 = two;
		num2 = three;
		return 1;
	}
	// (2-4)
	if(graph[one].find(two) != graph[one].end()
		&& graph[one].find(three) != graph[one].end()
		&& graph[one].find(four) != graph[one].end() 
		&& graph[two].find(three) != graph[two].end() 
		&& graph[two].find(four) == graph[two].end()
		&& graph[three].find(four) != graph[three].end() )		
	{
		num1 = two;
		num2 = four;
		return 1;
	}
	// (3-4)
	if(graph[one].find(two) != graph[one].end()
		&& graph[one].find(three) != graph[one].end()
		&& graph[one].find(four) != graph[one].end() 
		&& graph[two].find(three) != graph[two].end() 
		&& graph[two].find(four) != graph[two].end()
		&& graph[three].find(four) == graph[three].end() )		
	{
		num1 = three;
		num2 = four;
		return 1;
	}

	return 1;
}


int main()
{
	readFourColor("8.txt");
	g_nTotal = g_fourPerms.size();
	g_ofs.open("out.txt");
	// ����ÿһ��4ɫ����
	vector<vector<int> >::iterator perm_p = g_fourPerms.begin(), perm_e = g_fourPerms.end();
	for(; perm_p != perm_e; ++ perm_p )
	{
		const vector<int> &vecInt = *perm_p;
		// 1. �����ཻͼ
		int n = 8;
		map<int, set<int> > graph;
		for( int i = 1; i <= n; i ++ )
		{
			int num = vecInt[i];
			if( num > 1 )
				graph[num].insert(num - 1);
			if( num < n )
				graph[num].insert(num + 1);
			if( i > 1 )
				graph[num].insert(vecInt[i-1]);
			if( i < n  )
				graph[num].insert(vecInt[i+1]);
		}

		// ���һ����������������K4��ȫͼ
		bool bFound = false;
		Combo combo(8,4);
		combo.init();
		do
		{
			int num1, num2;			
			vector<int> value;
			combo.setValue(vecInt, value);
			int nType = isNormalDiamond(value, graph, num1, num2);
			if( nType == 0 )
			{
				dump(vecInt, 8);
				g_ofs << " is K4 at ";
				dump(combo._fourCombo, 4);
				g_ofs << endl;
				++ g_nK4;	
				bFound = true;
				break;
				combo.init();
			}
		}
		while(!combo.next(4));
		if( bFound )
			continue;
		
		// 2. ���������������		
		int v1, v2;	
		int v11, v22;
		combo.init();
		do
		{
			vector<int> value;
			combo.setValue(vecInt, value);
					
			int nType = isNormalDiamond(value, graph, v1, v2);
			if( nType == 1 )  //�ҵ�һ����������
			{
				// 2. �ҵڶ�����������
				// 2.1 �ҳ��ڶ����������ڵķ�Χ
				vector<int> vecInt2(1,0);
				for(int i = 1; i <= combo.getSuper(); ++ i )
				{
					if(std::find(value.begin(), value.end(), vecInt[i]) == value.end() )
						vecInt2.push_back(vecInt[i]);
				}				

				// 2.2 �ҵڶ�������
				Combo combo2(4,3);
				combo2.init();
				do
				{		
					vector<int> value2;
					combo2.setValue(vecInt2, value2);
					// 2.2.1 v1 �غ�ʱ
					value2.push_back(v1);  
					
					int nType2 = isNormalDiamond(value2, graph, v11, v22);
					
					if( nType2 == 1 
						&& v1 == v11 
						&& graph[v2].find(v22) != graph[v2].end() )  // �ҵ���2������
					{
						bFound = true;
						break;
					}
					// 2.2.2 v2�غ�ʱ
					value2.pop_back();
					value2.push_back(v2);
					nType2 = isNormalDiamond(value2, graph, v11, v22);
					if( nType2 == 1
						&& v2 == v22
						&& graph[v1].find(v11) != graph[v1].end() )
					{
						bFound = true;
						break;
					}
				}while( !combo2.next(3));
			}
		}while(!bFound && !combo.next(4));
		
		if( bFound )
		{
			dump(vecInt, 8);
			++ g_nTwoFold;
			g_ofs << " is two-fold at (" << v1 << "," << v2 << ") and (" << v11 << "," << v2 <<")" << endl;
		}
		else
		{
			++ g_nUnsatisfy;
			dump(vecInt, 8);
			g_ofs << " cannot satisfy the model!" << endl;
		}
	}
	
	g_ofs << "��Ҫ������ɫ����Ŀ��\t" << g_nTotal << endl;
	g_ofs << "һ�������������У�\t" << g_nK4 << endl;
	g_ofs << "���������������У�\t" << g_nTwoFold << endl;
	g_ofs << "��������ģ�͵��У�\t" << g_nUnsatisfy << endl;
	g_ofs.close();
	return 0;
}



