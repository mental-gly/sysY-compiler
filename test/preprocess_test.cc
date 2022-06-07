/*
    宏定义示例：
    #define MAX(a,b) (a>b?a:b)
    #define N 1000
    #define A(x) (x+1)
    #define MUL(a,b,c) (a*b*c)
    使用宏时，宏名内部不允许空格，前后需要空格
*/

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>
 
using namespace std;

string temp;
struct{
    string name;
    vector<string> params;
    string target;
}def[10];

vector<string> split(const string& str, const string& delim) {
    vector<string> res;
    if("" == str) return res;
    //先将要切割的字符串从string类型转换为char*类型
    char * strs = new char[str.length() + 1] ; 
    strcpy(strs, str.c_str());
  
    char * d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());
  
    char *p = strtok(strs, d);
    while(p) {
        string s = p; //分割得到的字符串转换为string类型
        res.push_back(s); //存入结果数组
        p = strtok(NULL, d);
    }
    return res;
}

void ReadDataFromFileLBLIntoString(string in, string out)
{
    ifstream fin(in);
    ofstream fout(out);  
    string s;
    int k = 0;
    while( getline(fin,s) )
    {    
        if(s.find("#define") != s.npos){
            std::vector<string> res = split(s, " ");
            for (int i = 0; i < res.size(); ++i)
            {
                cout << res[i] <<endl;
            }
            if(res[1].find("(") != res[1].npos){
                std::vector<string> r = split(res[1], "(");
                def[k].name = r[0];
                cout << k <<endl;
                r[1].replace(r[1].find(")"),1,"");
                std::vector<string> rr = split(r[1], ",");
                def[k].params = rr;
                def[k].target = res[2];
            }else{
                cout << k <<endl;
                def[k].name = res[1];
                def[k].target = res[2];
            }
            k++;
            continue;
        }
        fout << s << endl;
    }
    fin.close();
    fout.close();
}

string& replace_all(string& src, const string& old_value, const string& new_value) {
    // 每次重新定位起始位置，防止上轮替换后的字符串形成新的old_value
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = src.find(old_value, pos)) != string::npos) {
			src.replace(pos, old_value.length(), new_value);
		}
		else break;
	}
	return src;
}

void ReplaceDefine(string in, string out)
{
    ifstream fin(in);
    ofstream fout(out);  
    string s;
    while( fin >> s )
    {    
        temp  = s;
        if(s.find("(")!= s.npos&&s.find(")")!= s.npos){
            std::vector<string> r = split(s, "(");
            temp = r[0];
            for(int i = 0; i < 10; i++){
                if(def[i].name == temp){
                    string strValue = def[i].target;
                    cout << strValue << " "; 
                    r[1].replace(r[1].find(")"),1,"");
                    std::vector<string> rr = split(r[1], ",");
                    for(int j = 0; j < rr.size();j++){
                        replace_all(strValue, def[i].params[j], rr[j]);
                    }
                    fout << strValue << " ";
                }
            }
        }else{
            int flag = 1;
            for(int i = 0; i < 10; i++){
                if(def[i].name == temp){
                    flag = 0;
                    fout << def[i].target << " "; 
                    break;
                }
            }
            if(flag == 1){
                fout << s << " ";
            }
        }
    }
    fin.close();
    fout.close();
}

void preprocess(const char* input){
    string in = input;
    string mid = "mid.txt";
    ReadDataFromFileLBLIntoString(in, mid);
    ReplaceDefine(mid, in);
}

int main(){
    auto input = "data.txt";
    preprocess(input);//处理好的input在原文件中
    return 0;
}