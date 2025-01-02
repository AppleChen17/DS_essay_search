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

class TrieNode
{
    public:
        TrieNode* child[26]; // Fixed size for lowercase English letters
        bool isEnd;

        TrieNode() :isEnd(false)
        {
            for(int i = 0;i < 26;i++) child[i] = nullptr;
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
        if(prefix) return true;

        return p->isEnd;
    }
    
    bool prefix(string pre) 
    {
        return search(pre,true);
    }
};

vector <vector <Trie>> trie;// for different doc => 2 trie (for prefix && suffix)
vector <string> Titles;

// Utility Func

// string parser : output vector of strings (words) after parsing
vector<string> word_parse(vector<string> tmp_string){
	vector<string> parse_string;
	for(auto& word : tmp_string){
		string new_str;
    	for(auto &ch : word){
			if(isalpha(ch))
				new_str.push_back(ch);
		}
		parse_string.emplace_back(new_str);
	}
	return parse_string;
}

vector<string> split(const string& str, const string& delim) {
	vector<string> res;
	if("" == str) return res;

	char * strs = new char[str.length() + 1] ;
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

// unordered_set<int> executeQuery(const vector<string>& tokens, Trie& trie) {
//     stack<unordered_set<int>> operands;
//     for (const string& token : tokens) {
//         if (token == "+" || token == "/" || token == "-") {
//             // 運算符
//             auto set2 = operands.top(); operands.pop();
//             auto set1 = operands.top(); operands.pop();
//             unordered_set<int> result;

//             if (token == "+") { // 交集
//                 for (int doc : set1) {
//                     if (set2.count(doc)) result.insert(doc);
//                 }
//             } else if (token == "/") { // 並集
//                 result = set1;
//                 result.insert(set2.begin(), set2.end());
//             } else if (token == "-") { // 差集
//                 result = set1;
//                 for (int doc : set2) result.erase(doc);
//             }
//             operands.push(result);
//         } else {
//             // 操作數：從 Trie 中搜尋
//             operands.push(unordered_set<int>(trie.search(token).begin(), trie.search(token).end()));
//         }
//     }
//     return operands.top();
// }

void processQueries(const string& queryFile) 
{
    ifstream file(queryFile);
    string line;
    while (getline(file, line))
	{
        vector<string> tokens = tokenize(line);
		vector<pair<string,int>> v; // 0 exact | 1 prefix | 2 suffix | 3 wildcard
		// for(auto t : tokens)
		// {
		// 	cout << t << "\n";
		// }
		// cout << "\n";

		// auto result = executeQuery(tokens, trie);
		// for (int doc : result) {
		//     cout << doc << ' ';
		// }
		// cout << endl;
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
	std::ofstream ofs;
	ofs.open("output.txt");
	vector<string> tmp_string;

	trie.assign(100, vector<Trie>());

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

		if(trie[i][0].prefix("reflect"))
		{
			cout << "i = " << i << "\n";
			ofs << title_name << "\n";
			// ofs << "\n";
		}

		// CLOSE FILE
		fi.close();
	}
	processQueries(query);
	ofs.close();

	auto end = high_resolution_clock::now(); // 結束計時
    auto duration = duration_cast<milliseconds>(end - start); // 計算時間
    cout << "Execution time: " << duration.count() << " ms\n";
	// string file = data_dir + "0" + FILE_EXTENSION;
	// fi.open("data/0.txt", ios::in);

    // // GET TITLENAME
	// getline(fi, title_name);

    // // GET TITLENAME WORD ARRAY
    // tmp_string = split(title_name, " ");

	// vector<string> title = word_parse(tmp_string);

	// // for(auto &word : title){
	// // 	cout << word << endl;
	// // }

    // // GET CONTENT LINE BY LINE
	// while(getline(fi, tmp)){

    //     // GET CONTENT WORD VECTOR
	// 	tmp_string = split(tmp, " ");

	// 	// PARSE CONTENT
	// 	vector<string> content = word_parse(tmp_string);

	// 	for(auto &word : content){
	// 		cout << word << endl;
	// 	}
	// 	//......
	// }

    // // CLOSE FILE
	// fi.close();
}


// 1. UPPERCASE CHARACTER & LOWERCASE CHARACTER ARE SEEN AS SAME.
// 2. FOR SPECIAL CHARACTER OR DIGITS IN CONTENT OR TITLE -> PLEASE JUST IGNORE, YOU WONT NEED TO CONSIDER IT.
//    EG : "AB?AB" WILL BE SEEN AS "ABAB", "I AM SO SURPRISE!" WILL BE SEEN AS WORD ARRAY AS ["I", "AM", "SO", "SURPRISE"].
// 3. THE OPERATOR IN "QUERY.TXT" IS LEFT ASSOCIATIVE
//    EG : A + B / C == (A + B) / C

//

//////////////////////////////////////////////////////////
