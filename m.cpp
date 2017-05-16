#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <map>

using namespace std;

typedef vector<string> Line;

void usage(const char *cmd)
{
  cout << cmd << endl;
  cout << cmd << " file_name (ex.s)" << endl;
}

string trim_inst_size(const string &s)
{
  regex re("[blw]$");
  string str = regex_replace(s, re, "");
  return str;
}

string trim(const string &s)
{
  //regex re("^[ \t] +|[ \t]+$");
  regex re("^[ ] +|[ ]+$");
  string str = regex_replace(s, re, "");
  return str;
}

typedef int (*Fp)(const Line &l);

int mov_fp(const Line &l)
{
  cout << "handle mov" << endl;

  if (l.size() != 3)
  {
    cout << "syntax error" << endl;
  }
  
  for (auto &i : l)
  {
    cout << i << endl;
  }
  return 0;
}

map<string, Fp> obj_handle;



void gen_obj(const vector<Line> &tokens)
{
  for (auto &line : tokens)
  {
    string inst = trim_inst_size(line[0]);

    auto it = obj_handle.find(inst);
    Fp fp;
    if (it != obj_handle.end())
    {
      fp = it->second;
      (*fp)(line);
    }
    else
    {
      cout << inst << " found no handle" << endl;
    }
  }
}


int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    usage(argv[0]);
    return 5;
  }
  obj_handle.insert({"mov", mov_fp});

  ifstream ifs(argv[1]);

  vector<string> lines;  
  string str;

  vector<Line> tokens;

  while (getline(ifs, str)) 
  {
    lines.push_back(str);
  }

  int num=0;
  //regex begin_space("^[ \\t\\n]*");
  //regex begin_space("\\n");
  regex comment("#.*");
  regex sep("[ ]*[ \t,][ \t\n]*");
  for (auto &i : lines)
  {
    cout << num << ": " << i << endl;

    #if 0
    if (i[0] == '\n')
      continue;
    #endif
    //string remove_str = regex_replace(regex_replace(i, begin_space, "$1"), comment, "");
    string remove_str = trim(regex_replace(i, comment, ""));

    #if 1
    if (remove_str.empty())
    {
      ++num;
      continue;
    }
    #endif
    cout << num << " (remove_str): " << remove_str << endl;
    ++num;
    //sregex_token_iterator p(remove_str.cbegin(), remove_str.cend(), sep, {0, 2});
    sregex_token_iterator p(remove_str.cbegin(), remove_str.cend(), sep, -1);
    sregex_token_iterator e;

    Line line{p, e};

    #if 0
    for_each(p, e, [](const smatch &m) 
                   {
                     cout << "match: " << m.str() << endl;
                   }
            );
    #endif
    #if 0
    for (; p!=e ; ++p)
    {
      //p->empty();
      cout << "n: " << *p << endl;
    }
    #endif
    for (auto &j : line)
    {
      cout << "j: " << j << endl;
    }
    tokens.push_back(line);

  }

  gen_obj(tokens);

  return 0;
}
