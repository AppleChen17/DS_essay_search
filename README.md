# DS essay_search

This program is written in C++ and supports searching titles and contents in the provided data using different search types and logical operators. 

## Overview

### Search type
This program support exact, prefix, suffix, and wildcard search and some logical operand on titles and contents of the provided data. The below is the syntax of the supported search.

- exact search : `"search_word"`
- prefix : `search_word`
- suffix : `*search_word*`
- wildcard : `<search_pattern>` , where `*` can be empty, single or multiple characters

There are some sample data in `data` and `data-more` folder, and the titles of the articles which matched with the search would be recorded in `output.txt`.

### Logical operators
Also, this project also support the logical operator (**And / Or / Exclude**) between the searching command, in this program, all of these operators are handled as **left associative**. The below is the operator.

- And ( `+` ) : Returns results that match both conditions ( intersection )

      Ex : “graph” + decompos
    The answer would be the set intersection of exact search of graph and prefix search of decompos.

- Or ( `/` ) : Returns results that match either condition ( union )

      Ex : “graph” / *composition*
    The answer would be the union set of exact search of graph and suffix search of composition.

- Exclude ( `-` ) : Returns results that match the first condition but exclude the second condition ( difference )

      Ex : <com*on> - ”shaped”
    The answer would be the difference set of wildcard search of com*on ("common", "companion"...) and exact search of shaped.

## Usage

### Compile and Run
1. Enter the folder where `main.cpp` is in by terminal
2. Using `g++ -std=c++17 -O2 -o essay_search ./main.cpp` to compile the cpp file and get the .exe file
3. Run the file by command `./essay_search [input_folder_path] [query_file_path] [output_file_name]`. In the folder and data provided in this repository, the command would be like `./essay_search data query.txt output.txt` or `./essay_search data-more query_more.txt output.txt`.

## More detailed information
// the link
