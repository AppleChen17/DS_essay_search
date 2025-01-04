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
#include <filesystem>
#include <algorithm>

using namespace std;
using namespace chrono;

int filecount = 0;

class TrieNode
{
    public:
        vector<TrieNode*> child{vector<TrieNode*>(26, nullptr)};
		// Fixed size for lowercase English letters
        // TrieNode* child[26];
		bool isEnd;
		int remain_depth = 0;// initial value == 0

        TrieNode() :isEnd(false)
        {
			// child.resize(26);
            // for(int i = 0;i < 26;i++) child[i] = nullptr;
        }
};

class Trie 
{
public:
    TrieNode* root; // root would NOT store char !!
	bool wildcard_search = false; // default false

    Trie() 
    {
        root = new TrieNode();
		wildcard_search = false;
    } // Trie => root would NOT put any word

	// only need to calculate once !!! (because it is built and NOT changed)
	int cal_depth(TrieNode* p)
	{
		if(p == nullptr) return 0;
		// if(p->remain_depth != 0) return p->remain_depth;

		int depth = 0;
		for(int i = 0;i < 26;i++)
		{
			depth = std::max(depth,cal_depth(p->child[i]));
		}
		return (p->remain_depth = depth + 1);
	}
    
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

	bool wildcard(TrieNode* p,string& pattern,int len,int now)
	{
		if(wildcard_search) return true;
		// now => the one NEED to be matched !!!
		if(now == len)
		{
			if(p->isEnd)
			{
				wildcard_search = true;
				// cout << "now == len TRUE\n";
				return true;
			}
			else return false;
		}
		if(p == nullptr) return false;

		if(pattern[now] == '*')
		{
			if(now + 1 == len) return true; // * at the end => match all
			else if(now+1 < len && pattern[now+1] != '*')
			{
				// can match further pattern
				char c = pattern[now+1];
				// cout << "look further c = " << c << "\n";
				if(p->child[tolower(c)-'a'])
				{
					if(wildcard(p->child[tolower(c)-'a'],pattern,len,now+2))
					{
						// cout << "TRUE\n";
						return true;
					}
				}
				// if couldn't match here => * may match more chars
			}
			for(int i = 0;i < 26;i++)
			{
				if(wildcard(p->child[i],pattern,len,now))
					return true;
			}
			return false; // cannot match
		}

		// is "char"
		char c = pattern[now];
		if(p->child[tolower(c)-'a'])
		{
			// cout << "c = " << c << "\n";
			return wildcard(p->child[tolower(c)-'a'],pattern,len,now+1);
		}
		else return false;
	}
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
	else if(type == 3)
	{
		int len = ask.size();
		// cout << "ask = " << ask << "\n";
		for(int i = 0;i < filecount;i++)
		{
			trie[i][0].wildcard_search = false;
			if(trie[i][0].wildcard(trie[i][0].root,ask,len,0))
			{
				ans.push_back(i);
			}
			trie[i][0].wildcard_search = false;
		}
	}
	return ans;
}

void processQueries(const string& queryFile)
{
	// open up query from file !!!
    ifstream file(queryFile);
    string line;

	// a single line => a query
	int cnt = 0;
    while (getline(file, line))
	{
		// ofs << "Query " << cnt++ << ":\n";
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
			else if(t[0] == '<' && t[t.size()-1] == '>')
			{
				int len = t.size();
				string tmp = "";
				for(int i = 1;i < len-1;i++)
				{
					if(t[i] == '*') // only put in one "*" if there are continuous "*"
					{
						tmp += t[i];
						while(t[i+1] == '*') i++;
					}
					else tmp.push_back(t[i]);
				}

				// as "prefix" match => so could understand as the last one would 
				// NOT match anything case to deal with ! 
				/// => cannot !!! since some of them need to match absolutely with the end
				// if(tmp[tmp.size()-1] == '*') tmp.pop_back();
				// cout << "push in tmp = " << tmp << "\n";
				v.push_back({tmp,3});
			}
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
		// ofs << "now = 0: \n";
		// for(auto a : ans)
		// {
		// 	ofs << a << ": ";
		// 	ofs << Titles[a] << "\n";
		// }
		vector<int> tmp;
		int now = 1;
		for(auto o:op)
		{
			// ofs << "now = " << now << ":\n";
			tmp = executeQuery(v[now++]);
			// for(auto a : tmp)
			// {
			// 	ofs << a << ": ";
			// 	ofs << Titles[a] << "\n";
			// }

			if(o == '+') //both need to have !
			{
				// tmp would be in ascending order since push in in order !!
				for (auto it = ans.begin(); it != ans.end();) 
				{
					if (!binary_search(tmp.begin(), tmp.end(), *it)) 
					{
						it = ans.erase(it); // Erase returns the iterator to the next element.
					}
					else
					{
						it++;
					}
				}
			}

			else if(o == '/')
			{
				sort(ans.begin(),ans.end());
				for(auto a:tmp)
				{
					if(!binary_search(ans.begin(),ans.end(),a))
					{
						ans.push_back(a);
					}
				}
				sort(ans.begin(), ans.end());
				ans.erase(unique(ans.begin(), ans.end()), ans.end());
				// for (auto a : tmp) 
				// {
				// 	auto it = std::lower_bound(ans.begin(), ans.end(), a); // Find insertion point.
				// 	if (it == ans.end() || *it != a) 
				// 	{
				// 		ans.insert(it, a); // Insert `a` in sorted position.
				// 	}
				// }
			}

			else if(o == '-')
			{
				// sort(ans.begin(),ans.end());
				for(auto a:tmp)
				{
					// if(binary_search(ans.begin(),ans.end(),a))
					// {
					// 	auto it = find(ans.begin(),ans.end(),a);
					// 	ans.erase(it);
					// }
					ans.erase(std::remove(ans.begin(), ans.end(), a), ans.end());
				}
			}
		}

		// ofs << "Final ans:\n";

		if(ans.size()==0) ofs << "Not Found!\n";
		else
		{
			// by the order
			sort(ans.begin(),ans.end());

			// unique
			// auto it = std::unique(ans.begin(), ans.end());
			// remove repeated one
			// ans.erase(it, ans.end());

			for(auto a : ans)
			{
				ofs << Titles[a] << "\n";
			}
			// ofs << "\n";
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

	// trie.assign(filecount+2, vector<Trie>());

	// from data_dir get file ....
	// eg : use 0.txt in data directory

	// get_total_files(data_dir);

	while(1)
	{
		// cout << "filecount = " << filecount << "\n";
		file = data_dir + to_string(filecount) + FILE_EXTENSION;
		fi.open(file, ios::in);

		// if(fi.fail()) cout << "file " << filecount << " fail\n";
		if(!fi.is_open())
		{
			cout << "total num of files = " << filecount << "\n";
			break;
		}

		// GET TITLENAME
		getline(fi, title_name);

		Titles.emplace_back(title_name);

		// CREATE TRIE
		Trie trie1 = Trie();
		Trie trie2 = Trie();
		trie.push_back(vector<Trie>());
		trie[filecount].push_back(trie1);
		trie[filecount].push_back(trie2);

		// GET TITLENAME WORD ARRAY
		tmp_string = split(title_name, " ");

		vector<string> title = word_parse(tmp_string);

		for(auto &word : title)
		{
			trie[filecount][0].insert(word);
			reverse(word.begin(), word.end());
			trie[filecount][1].insert(word);
		}

		// trie[filecount][0].root->remain_depth = trie[filecount][0].cal_depth(trie[filecount][0].root);
		// only the [0] need to do wildcard
		// cout << "root depth = " << trie[filecount][0].root->remain_depth << "\n";

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
				trie[filecount][0].insert(word);
				reverse(word.begin(), word.end());
				trie[filecount][1].insert(word);
			}
		}

		filecount++;

		// CLOSE FILE
		fi.close();

		// string key = "shaped";
		// // reverse(key.begin(), key.end());
		// if(trie[i][0].search(key,true))
		// {
		// 	cout << "i = " << i << "\n";
		// 	ofs << title_name << "\n";
		// 	// ofs << "\n";
		// }
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