#define FILE_EXTENSION ".txt"
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <chrono>
#include <unordered_set>
#include <stack>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace chrono;

const int filecount = 100;

class TrieNode
{
    public:
        vector<TrieNode*> child{vector<TrieNode*>(26, nullptr)};
		// Fixed size for lowercase English letters
        // TrieNode* child[26];
		bool isEnd;

        TrieNode() :isEnd(false)
        {
			// child.resize(26);
            // for(int i = 0;i < 26;i++) child[i] = nullptr;
        }
};

class Trie 
{
    TrieNode* root; // root would NOT store char !!
public:
    
    Trie() 
    {
        root = new TrieNode();
    } // Trie => root would NOT put any word
    
    void insert(string word) 
    {
        TrieNode* p = root;
        //recursive => iterative !!! (較少 stack 儲存 recursion call)

        for(auto s:word)
        {
            int i = tolower(s) - 'a';
            // no this path
            if(p->child[i] == nullptr) p->child[i] = new TrieNode();
            p = p->child[i];
        }
        p->isEnd = true;
    }
    
    bool search(string word, bool prefix = false)
    // could use the same function when setting prefix with initial value
    {
        TrieNode* p = root;
        for(auto s : word)
        {
            int i = tolower(s) - 'a';
            if(p ->child[i] == nullptr) return false;
            p = p->child[i];
        }
        if(prefix) return true; // for prefix match !!

        return p->isEnd;
    }
    
    // bool prefix(string pre) 
    // {
    //     return search(pre,true);
    // }
};

vector <vector <Trie>> trie;// for different doc => 2 trie (for prefix && suffix)
vector <string> Titles;

// Utility Func

// string parser : output vector of strings (words) after parsing
vector<string> word_parse(vector<string> tmp_string)
{
	vector<string> parse_string;
	for(auto& word : tmp_string)
	{
		string new_str;
    	for(auto &ch : word)
		{
			if(isalpha(ch))
				new_str.push_back(ch);
		}
		parse_string.emplace_back(new_str);
	}
	return parse_string;
}

vector<string> split(const string& str, const string& delim) 
{
	vector<string> res;
	if("" == str) return res;

	char * strs = new char[str.length() + 1];
	strcpy(strs, str.c_str());

	char * d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());

	char *p = strtok(strs, d);
	while(p) {
		string s = p; 
		res.push_back(s);
		p = strtok(NULL, d);
	}

	return res;
}

vector<string> tokenize(const string& query) 
{
    vector<string> tokens;
    string token;
    stringstream ss(query);
    while (ss >> token)
	{
        tokens.push_back(token);
    }
    return tokens;
}

std::ofstream ofs;

vector<int> executeQuery(pair<string,int>& query) 
{
	vector<int> ans;
	auto [ask,type] = query;
	
	// 0 exact
	if(type == 0)
	{
		for(int i = 0;i < filecount;i++)
		{
			if(trie[i][0].search(ask))
			{
				ans.push_back(i);
			}
		}
	}

	// 1 prefix
	else if(type == 1)
	{
		for(int i = 0;i < filecount;i++)
		{
			if(trie[i][0].search(ask,true))
			{
				ans.push_back(i);
			}
		}
	}

	// 2 suffix => inverse prefix !!!
	else if(type == 2)
	{
		reverse(ask.begin(),ask.end());
		for(int i = 0;i < filecount;i++)
		{
			if(trie[i][1].search(ask,true))
			{
				ans.push_back(i);
			}
		}
	}

	// 3 wildcard
	// else if(type == 3)
	// {
	// }
	return ans;
}

void processQueries(const string& queryFile)
{
    ifstream file(queryFile);
    string line;

	// a single line query
	int cnt = 0;
    while (getline(file, line))
	{
		ofs << "Query " << cnt++ << ":\n";
        vector<string> tokens = tokenize(line);
		vector<pair<string,int>> v; // 0 exact | 1 prefix | 2 suffix | 3 wildcard
		// deque<char> op; // operator
		vector<char> op;
		for(auto t : tokens)
		{
			// cout << t << "\n";
			if(t == "+" || t == "/" || t == "-") op.emplace_back(t[0]);

			// prefix
			else if(isalpha(t[0])) v.push_back({t,1});
			// exact
			else if(t[0] == '\"' && t[t.size()-1] == '\"') v.push_back({t.substr(1,t.size()-2),0}); // cut the middle part!
			// suffix
			else if(t[0] == '*' && t[t.size()-1] == '*') v.push_back({t.substr(1,t.size()-2),2});
			// wildcard
			else if(t[0] == '<' && t[t.size()-1] == '>') v.push_back({t.substr(1,t.size()-2),3});
		}
		// cout << "\n";

		// cout << "queries\n";
		// for(auto qq:v)
		// {
		// 	cout << qq.first << " " << qq.second << "\n";
		// }

		// cout << "operator\n";
		// for(auto o:op)
		// {
		// 	cout << o << "\n";
		// }
		// cout << "\n";

		vector ans = executeQuery(v[0]);
		ofs << "now = 0: \n";
		for(auto a : ans)
		{
			ofs << Titles[a] << "\n";
		}
		vector<int> tmp;
		int now = 1;
		for(auto o:op)
		{
			ofs << "now = " << now << ":\n";
			tmp = executeQuery(v[now++]);
			for(auto a : tmp)
			{
				ofs << Titles[a] << "\n";
			}

			if(o == '+') //both need to have !
			{
				for (auto it = ans.begin(); it != ans.end();) 
				{
					if (!binary_search(tmp.begin(), tmp.end(), *it)) 
					{
						it = ans.erase(it); // Erase returns the iterator to the next element.
					} 
					else 
					{
						++it;
					}
				}
			}

			else if(o == '/')
			{
				for(auto a:tmp)
				{
					if(!binary_search(ans.begin(),ans.end(),a))
					{
						ans.push_back(a);
					}
				}
				sort(ans.begin(),ans.end());
			}

			else if(o == '-')
			{
				for(auto a:tmp)
				{
					if(binary_search(ans.begin(),ans.end(),a))
					{
						auto it = find(ans.begin(),ans.end(),a);
						ans.erase(it);
					}
				}
			}
		}

		ofs << "Final ans:\n";
		if(ans.size()==0) ofs << "Not Found!\n\n";
		else
		{
			// by the order
			sort(ans.begin(),ans.end());
			for(auto a : ans)
			{
				ofs << Titles[a] << "\n";
			}
			ofs << "\n";
		}

    }
}


int main(int argc, char *argv[])
{
    // INPUT :
	// 1. data directory in data folder
	// 2. number of txt files
	// 3. output route

	auto start = high_resolution_clock::now(); // start time

    string data_dir = argv[1] + string("/");
	string query = string(argv[2]);
	string output = string(argv[3]);

	// Read File & Parser Example

	string file, title_name, tmp;
	fstream fi;
	ofs.open("output.txt");
	vector<string> tmp_string;

	trie.assign(filecount+2, vector<Trie>());

	// from data_dir get file ....
	// eg : use 0.txt in data directory

	for(int i = 0;i < 100;i++)
	{
		file = data_dir + to_string(i) + FILE_EXTENSION;
		fi.open(file, ios::in);

		// GET TITLENAME
		getline(fi, title_name);

		Titles.emplace_back(title_name);

		// CREATE TRIE
		Trie trie1 = Trie();
		Trie trie2 = Trie();
		trie[i].push_back(trie1);
		trie[i].push_back(trie2);

		// GET TITLENAME WORD ARRAY
		tmp_string = split(title_name, " ");

		vector<string> title = word_parse(tmp_string);

		for(auto &word : title)
		{
			trie[i][0].insert(word);
			reverse(word.begin(), word.end());
			trie[i][1].insert(word);
		}

		// if find => use this to output !!
		// for(auto &word : tmp_string)
		// {
		// 	ofs << word << " ";
		// }
		// ofs << "\n";

		// GET CONTENT LINE BY LINE
		while(getline(fi, tmp))
		{
			// GET CONTENT WORD VECTOR
			tmp_string = split(tmp, " ");

			// PARSE CONTENT
			vector<string> content = word_parse(tmp_string);

			for(auto &word : content)
			{
				trie[i][0].insert(word);
				reverse(word.begin(), word.end());
				trie[i][1].insert(word);
			}
		}

		// string key = "shaped";
		// // reverse(key.begin(), key.end());
		// if(trie[i][0].search(key,true))
		// {
		// 	cout << "i = " << i << "\n";
		// 	ofs << title_name << "\n";
		// 	// ofs << "\n";
		// }

		// CLOSE FILE
		fi.close();
	}
	processQueries(query);
	ofs.close();

	auto end = high_resolution_clock::now(); // 結束計時
    auto duration = duration_cast<milliseconds>(end - start); // 計算時間
    cout << "Execution time: " << duration.count() << " ms\n";
}


// 1. UPPERCASE CHARACTER & LOWERCASE CHARACTER ARE SEEN AS SAME.
// 2. FOR SPECIAL CHARACTER OR DIGITS IN CONTENT OR TITLE -> PLEASE JUST IGNORE, YOU WONT NEED TO CONSIDER IT.
//    EG : "AB?AB" WILL BE SEEN AS "ABAB", "I AM SO SURPRISE!" WILL BE SEEN AS WORD ARRAY AS ["I", "AM", "SO", "SURPRISE"].
// 3. THE OPERATOR IN "QUERY.TXT" IS LEFT ASSOCIATIVE
//    EG : A + B / C == (A + B) / C

//

//////////////////////////////////////////////////////////