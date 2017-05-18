#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <map>

#include "mytype.h"

using namespace std;

typedef vector<string> Line;

void usage(const char *cmd)
{
  cout << cmd << endl;
  cout << cmd << " file_name (ex.s)" << endl;
}

ofstream ofs("r.o");
FILE *fs;

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

#define UNKNOWN  0x00000000
#define R32      0x00000001
#define IMM32    0x00000002
#define R16      0x00000004

int check_operand_type(const string &str)
{
  if ('%' == str[0]) // current only support 32 register
  {
    if (str.length() == 4) // %eax
      return R32;
    else // %ax
      return R16;
  }
  if ('$' == str[0]) // current only support imm32
    return IMM32;
  return UNKNOWN;
}

int reg_to_val(const string &str)
{
  string lower_str;
  for (auto &c : str)
  {
    lower_str.push_back(tolower(c));
  }

  static map<string, int> reg_table
                   {
                     {"eax", 0},
                     {"ecx", 1},
                     {"edx", 2},
                     {"ebx", 3},
                     {"esp", 4},
                     {"ebp", 5},
                     {"esi", 6},
                     {"edi", 7}
                   };

  auto it = reg_table.find(lower_str);
  if (it != reg_table.end())
  {
    return it->second;
  }
  else
  {
    return -1;
  }
}


// ref: http://x86.renejeschke.de/html/file_module_x86_id_5.html
int add_func(const Line &l)
{
  int op1_type;
  int op2_type;

  cout << "handle add" << endl;

  if (l.size() != 3)
  {
    cout << "syntax error" << endl;
  }

  op1_type = check_operand_type(l[1]);
  op2_type = check_operand_type(l[2]);

  cout << l[1] << " op type: " << op1_type << endl;
  cout << l[2] << " op type: " << op2_type << endl;

  if ((op1_type | op2_type) == 0x3)
  {
    cout << "handle r, imm8" << endl;
    int reg_val = reg_to_val(l[2].substr(1));

    //string imm_str = string{"$0xf"}.substr(1);
    string imm_str = l[1].substr(1);
    cout << "imm_str: " << imm_str << endl;
    u32 imm_num = std::stoul(imm_str, 0, 0);
    cout << "imm_num: " << imm_num << endl;
    //u32 imm_num = std::stoul("0xabcdef12", 0, 16);

    u8 op = 0x83;
    u8 imm_size = 1; // 1 byte
    if (imm_num <= 0xff)
    {
      cout << imm_num << ": imm8" << endl;

    }
    #if 0
    else if ((imm_num <=  0xffff) != 0)
         {
           cout << imm_num << ": imm16" << endl;
           imm_size = 2;
           op = 0x81;
         }
    #endif
         else
         {
           cout << imm_num << ": imm32" << endl;
           imm_size = 4;
           op = 0x81;
         }

    u8 mod_rm = 0xc0 | reg_val;

    #if 0
    cout << "machine code: ";
    cout << hex << op;
    cout << imm32_num << endl;
    cout << dec;

    ofs << hex << 0xb8 + reg_val;
    ofs << imm32_num << endl;
    ofs << dec;
    #endif

    fwrite(&op, 1, 1, fs);
    fwrite(&mod_rm, 1, 1, fs);
    fwrite(&imm_num, 1, imm_size, fs);
  }
  return 0;
}

int sub_func(const Line &l)
{
  int op1_type;
  int op2_type;

  cout << "handle add" << endl;

  if (l.size() != 3)
  {
    cout << "syntax error" << endl;
  }

  op1_type = check_operand_type(l[1]);
  op2_type = check_operand_type(l[2]);

  cout << l[1] << " op type: " << op1_type << endl;
  cout << l[2] << " op type: " << op2_type << endl;

  u8 mod_rm=0xe8;
  u8 rm;
  if ((op1_type | op2_type) == 0x3)
  { // subl $2, %esp
    cout << "handle r, imm8" << endl;
    int reg_val = reg_to_val(l[2].substr(1));
    cout << "  reg_val: " << reg_val << endl;
    if (4 == reg_val) 
    {
      rm = 4;
    }

    mod_rm |= rm;
    cout << "\tmod_rm: " << hex << (u32)mod_rm << dec << endl;

    //string imm_str = string{"$0xf"}.substr(1);
    string imm_str = l[1].substr(1);
    cout << "imm_str: " << imm_str << endl;
    u32 imm_num = std::stoul(imm_str, 0, 0);
    cout << "imm_num: " << imm_num << endl;
    //u32 imm_num = std::stoul("0xabcdef12", 0, 16);

    u8 op = 0x83;
    u8 imm_size = 1; // 1 byte
    if (imm_num <= 0xff)
    {
      cout << imm_num << ": imm8" << endl;

    }
    #if 0
    else if ((imm_num <=  0xffff) != 0)
         {
           cout << imm_num << ": imm16" << endl;
           imm_size = 2;
           op = 0x81;
         }
    #endif
         else
         {
           cout << imm_num << ": imm32" << endl;
           imm_size = 4;
           op = 0x81;
         }

    fwrite(&op, 1, 1, fs);
    fwrite(&mod_rm, 1, 1, fs);
    fwrite(&imm_num, 1, imm_size, fs);
  }
  return 0;
}

// b8 +rd r32, imm32
int mov_func(const Line &l)
{
  int op1_type;
  int op2_type;

  cout << "handle mov" << endl;

  if (l.size() != 3)
  {
    cout << "syntax error" << endl;
  }

  op1_type = check_operand_type(l[1]);
  op2_type = check_operand_type(l[2]);

  cout << l[1] << " op type: " << op1_type << endl;
  cout << l[2] << " op type: " << op2_type << endl;


  u8 op = 0x89;
  u8 mod_rm = 0xc0;
  if ((op1_type | op2_type) == 0x1)
  {
    // mov %esp, %ebp
    int reg_val_1 = reg_to_val(l[1].substr(1)); // %esp << 3
    int reg_val_2 = reg_to_val(l[2].substr(1)); // %ebp

    mod_rm |= (reg_val_1 << 3);
    mod_rm |= reg_val_2;
    fwrite(&op, 1, 1, fs);
    fwrite(&mod_rm, 1, 1, fs);
  }

  if ((op1_type | op2_type) == 0x3)
  {
    cout << "handle R32, IMM32" << endl;
    int reg_val = reg_to_val(l[2].substr(1));

    string imm32_str = l[1].substr(1);
    u32 imm32_num = std::stoul(imm32_str, 0, 0);

    cout << "machine code: ";
    cout << hex << 0xb8 + reg_val;
    cout << imm32_num << endl;
    cout << dec;

    op = 0xb8 + reg_val;
    #if 0
    ofs << hex << 0xb8 + reg_val;
    ofs << imm32_num << endl;
    ofs << dec;
    #endif

    fwrite(&op, 1, 1, fs);
    fwrite(&imm32_num, 1, 4, fs);
  }



  for (auto &i : l)
  {
    cout << i << endl;
  }
  return 0;
}

int leave_func(const Line &l)
{
  cout << "handle leave" << endl;

  if (l.size() != 1)
  {
    cout << "syntax error" << endl;
  }

  u8 op=0xc9;
  fwrite(&op, 1, 1, fs);
  return 0;
}

int push_func(const Line &l)
{
  cout << "handle push" << endl;

  if (l.size() != 2)
  {
    cout << "syntax error" << endl;
  }

  int op1_type = check_operand_type(l[1]);
  u8 op=0x50;
  int reg_val = reg_to_val(l[1].substr(1));

  if (op1_type == R32 || op1_type == R16)
  {
    op = 0x50 + reg_val;
    fwrite(&op, 1, 1, fs);
  }

  return 0;
}

int ret_func(const Line &l)
{
  cout << "\thandle ret" << endl;

  if (l.size() > 2 || l.size() <= 0)
  {
    cout << "syntax error" << endl;
  }

  u8 op=0xc3; // 0xc3: Near return to calling procedure. or 0xcb: Far return to calling procedure.
  if (l.size() == 1)
  {
    fwrite(&op, 1, 1, fs);
  }

  if (l.size() == 2)
  {
  #if 0
  int op1_type = check_operand_type(l[1]);
  int reg_val = reg_to_val(l[1].substr(1));

  if (op1_type == R32 || op1_type == R16)
  {
    op = 0x50 + reg_val;
  }
  #endif
  }



  return 0;
}

map<string, Fp> obj_handle;



void gen_obj(const vector<Line> &tokens)
{
  for (auto &line : tokens)
  {
    string inst = trim_inst_size(line[0]);

    string lower_str; 
    for (auto &c : inst)
    { 
      lower_str.push_back(tolower(c));
    }
    cout << "lower_str: " << lower_str << endl;
    auto it = obj_handle.find(lower_str);
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

  obj_handle.insert({"mov", mov_func});
  obj_handle.insert({"add", add_func});
  obj_handle.insert({"sub", sub_func});
  obj_handle.insert({"leave", leave_func});
  obj_handle.insert({"push", push_func});
  obj_handle.insert({"ret", ret_func});

  char *pos = strrchr(argv[1], '.');
  string obj_fn;
  if (pos)
  {
    obj_fn = string{&argv[1][0], pos+1};
    obj_fn.push_back('o');
  }

  ifstream ifs(argv[1]);
  cout << "open obj_fn: " << obj_fn << endl;

  fs = fopen(obj_fn.c_str(), "wb");

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

  fclose(fs);
  cout << "write obj_fn: " << obj_fn << endl;

  return 0;
}
