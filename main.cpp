#define FILE_EXTENSION ".txt"
#define MAX_NUM 11000
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <bitset>

using namespace std;
using namespace chrono;

int filecount = 0;

class TrieNode
{
    public:
        vector<TrieNode*> child{vector<TrieNode*>(26, nullptr)};
		// bitset<MAX_NUM> index;
		bitset<MAX_NUM> index_End;
		bitset<MAX_NUM> index_prefix;
		// Fixed size for lowercase English letters
		bool isEnd;

        TrieNode() :isEnd(false){}
};

class Trie {
public:
    TrieNode* root;

    Trie() 
	{
        root = new TrieNode();
    }

    void insert(const string& word, int idx) 
	{
        TrieNode* p = root;
        for (char s : word) 
		{
            int i = s - 'a';
            if (p->child[i] == nullptr) 
			{
                p->child[i] = new TrieNode();
            }
            p = p->child[i];
            p->index_prefix.set(idx); // Set prefix index
        }
        p->isEnd = true;
        p->index_End.set(idx); // Set end index
    }

    void collectIndexes(TrieNode* node, bitset<MAX_NUM>& ans, bool collectEnd = false) 
	{
        if (node == nullptr) return;
        if (collectEnd) 
		{
            ans |= node->index_End; // Collect end indexes
        } 
		else 
		{
            ans |= node->index_prefix; // Collect prefix indexes
        }
    }

    void wildcard(TrieNode* node, const string& pattern, int len, int now, bitset<MAX_NUM>& ans) {
        if (node == nullptr) return;

        // If the entire pattern is matched
        if (now == len) 
		{
            ans |= node->index_End;
            return;
        }

        char c = pattern[now];

        if (c == '*') 
		{
            // Try matching zero or more characters
            wildcard(node, pattern, len, now + 1, ans); // Match zero characters
            for (int i = 0; i < 26; ++i) 
			{
                if (node->child[i] != nullptr) 
				{
                    wildcard(node->child[i], pattern, len, now, ans); // Match one or more characters
                }
            }
        } 
		else 
		{
            // Match the specific character
            int i = c - 'a';
            if (node->child[i] != nullptr) 
			{
                wildcard(node->child[i], pattern, len, now + 1, ans);
            }
        }
    }

    void search(const string& word, bitset<MAX_NUM>& ans, bool prefix = false) 
	{
        TrieNode* p = root;
        for (char s : word) 
		{
            int i = s - 'a';
            if (p->child[i] == nullptr) return;
            p = p->child[i];
        }

        if (prefix) 
		{
            collectIndexes(p, ans, false); // collect prefix indexes
        } 
		else 
		{
            ans |= p->index_End; // end indexes
        }
    }

    void Write_Into_File(TrieNode* node, const string& prefix, ofstream& file) 
	{
        if (node->isEnd) 
		{
            file << prefix << " [";
            for (int i = 0; i < MAX_NUM; i++) 
			{
                if (node->index_End[i]) file << i << ", ";
            }
            file << "]\n";
        }

        for (int i = 0; i < 26; i++) 
		{
            if (node->child[i] != nullptr) 
			{
                Write_Into_File(node->child[i], prefix + char('a' + i), file);
            }
        }
    }
};


// vector <vector <Trie>> trie;// for different doc => 2 trie (for prefix && suffix)
vector <string> Titles;

Trie* trie1 = new Trie();
Trie* trie2 = new Trie(); // for suffix => build the trie reversely

// Utility Func

// string parser : output vector of strings (words) after parsing
inline vector<string> word_parse(vector<string> tmp_string)
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

inline vector<string> split(const string& str, const string& delim) 
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
		res.emplace_back(s);
		p = strtok(NULL, d);
	}

	return res;
}

inline vector<string> tokenize(const string& query)
{
    vector<string> tokens;
    string token;
    stringstream ss(query);
    while (ss >> token)
	{
        tokens.emplace_back(token);
    }
    return tokens;
}

std::ofstream ofs;

inline void executeQuery(pair<string,int>& query,bitset<MAX_NUM>& ans) 
{
	// auto [ask,type] = query;
	string ask = query.first;
	int type = query.second;
	
	// 0 exact
	if(type == 0)
	{
		trie1->search(ask,ans,false);
	}

	// 1 prefix
	else if(type == 1)
	{
		trie1->search(ask,ans,true);
	}

	// 2 suffix => inverse prefix !!!
	else if(type == 2)
	{
		reverse(ask.begin(),ask.end());
		// cout << "ask in type 2 = " << ask << "\n";
		trie2->search(ask,ans,true);
	}

	// 3 wildcard
	else if(type == 3)
	{
		int len = ask.size();
		trie1->wildcard(trie1->root,ask,len,0,ans);
	}
}

inline void processQueries(const string& queryFile)
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
			std::transform(t.begin(), t.end(), t.begin(), 
						[](unsigned char c){return std::tolower(c);});
	
			// cout << t << "\n";

			switch(t[0])
			{
				case '+':
				case '/':
				case '-':
				{
					op.emplace_back(t[0]);
					break;
				}

				case '\"':
				{
					v.emplace_back(t.substr(1,t.size()-2),0);
					break;
				}

				case '*':
				{
					v.emplace_back(t.substr(1,t.size()-2),2);
					break;
				}

				case '<':
				{
					int len = t.size();
					string tmp = "";
					for(int i = 1;i < len-1;i++)
					{
						if(t[i] == '*') // only put in one "*" if there are continuous "*"
						{
							tmp += t[i];
							while((i+1<len) && t[i+1] == '*') i++;
						}
						else tmp += t[i];
					}
					// cout << "push in tmp = " << tmp << "\n";
					v.emplace_back(tmp,3);
					break;
				}
				
				default:
				{
					v.emplace_back(t,1);
					break;
				}
			}
		}

		// default initialize => all 0 !!
		bitset<MAX_NUM> ans;
		executeQuery(v[0],ans);
		
		int now = 1;
		for(auto o:op)
		{
			bitset<MAX_NUM> tmp;
			// ofs << "now = " << now << ":\n";
			executeQuery(v[now++],tmp);

			if(o == '+') //both need to have !
			{
				ans &= tmp;
			}

			else if(o == '/')
			{
				ans |= tmp;
			}

			else if(o == '-')
			{
				ans &= (~tmp);
			}
		}

		// ofs << "Final ans:\n";

		if(ans.none()) ofs << "Not Found!\n";
		else
		{
			for(int i = 0;i < filecount;i++)
			{
				if(ans[i]) ofs << Titles[i] << "\n";
			}
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

		// GET TITLENAME WORD ARRAY
		tmp_string = split(title_name, " ");

		vector<string> title = word_parse(tmp_string);

		for(auto &word : title)
		{
			std::transform(word.begin(), word.end(), word.begin(), 
							[](unsigned char c){return std::tolower(c);});
			trie1->insert(word,filecount);
			reverse(word.begin(), word.end());
			// cout << "word = " << word << "\n";
			// if(word == "noitisopmoced") cout << "decomposition in trie2\n";
			// if(word == "decomposition") cout << "composition in trie1\n";
			trie2->insert(word,filecount);
		}

		// GET CONTENT LINE BY LINE
		while(getline(fi, tmp))
		{
			// GET CONTENT WORD VECTOR
			tmp_string = split(tmp, " ");

			// PARSE CONTENT
			vector<string> content = word_parse(tmp_string);

			for(auto &word : content)
			{
				std::transform(word.begin(), word.end(), word.begin(), 
								[](unsigned char c){return std::tolower(c);});
				trie1->insert(word,filecount);
				reverse(word.begin(), word.end());
				trie2->insert(word,filecount);
			}
		}

		filecount++;
		// CLOSE FILE
		fi.close();
	}
	
	// bitset<MAX_NUM> ans;
	// string key = "composition";
	// reverse(key.begin(), key.end());
	// cout << "key in trie2 search = " << key << "\n";
	// trie2->search(key,ans,true);

	// for(int i = 0;i < filecount;i++)
	// {
	// 	// if(ans[i])
	// 	// {
	// 		ofs << "i = " << i << "\n";
	// 		ofs << Titles[i] << "\n";
	// 	// }
	// }
	processQueries(query);

	// trie1->Write_Into_File(trie1->root,"",ofs);
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