#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <assert.h>

using namespace std;

class Combo
{
public:
	vector<int> _fourCombo;
private:
	int _super;
	int _sub;
public:

	Combo(int super, int sub) { _super = super; _sub = sub;}
	int init();
	int next(int nLevel);

	int setSuper(int n) { _super = n; }
	int setSub(int n ) { _sub = n; }
	int getSuper() { return _super; }
	int getSub() { return _sub;}

	int setValue(const vector<int> &vec, vector<int> &value);
};


//生成8中取4组合数
int Combo::init()
{
	_fourCombo.assign(_sub+1,0);
	for(int i =0; i <= _sub; ++ i )
		_fourCombo[i] = i;	
	return 0;
}

int Combo::next(int nLevel)
{
	assert(nLevel > 0 && nLevel <= _sub);
	int max = _super-_sub+nLevel;	

	if( _fourCombo[nLevel] < max )
	{
		++ _fourCombo[nLevel];
		for(int i = nLevel; i < _sub; ++ i )
			_fourCombo[i+1] = _fourCombo[i]+1;
#ifdef PERM_DEBUG
		for(int i = 0; i <= COMBO_NUM; ++ i )
		{
			cout << g_fourCombo[i] << ' ';
		}
		cout << endl;
#endif
		return 0;
	}
	else
	{
		if( nLevel == 1 )
		{			
			return 1;
		}
		else
			return next(nLevel-1);
	}
}

int Combo::setValue(const vector<int> &vec, vector<int> &value)
{
	value.assign(1,0);
	for(int i = 1; i <= this->_sub; ++ i)
	{
		int index = this->_fourCombo[i];
		value.push_back(vec[index]);
	}
	
	return 0;
}