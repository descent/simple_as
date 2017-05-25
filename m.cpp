#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <map>

#include "mytype.h"
#include "section.h"

using namespace std;

bool is_num(const char *str)
{
  const char *p = str;
  while (*p)
  {
    if (isdigit(*p) == 0)
      return false;
    ++p;
  }
  return true;
}

//enum Section {TEXT, DATA, BSS, UNKNOWN_SECTION};

ElfSection *cur_elf_section;

typedef vector<string> Line;

void usage(const char *cmd)
{
  cout << cmd << endl;
  cout << cmd << " file_name (ex.s)" << endl;
}

bool is_label(const string &label)
{
  auto rit = label.rbegin();
  if (':' == *rit)
    return true;
  else
    return false;
}

//ofstream ofs("r.o");
FILE *fs;

string trim_inst_size(const string &s)
{
  if (s == "call")
    return s;
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
#define SYMBOL   0x00000008

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
  {
    const char *c_str = str.c_str();

    if (is_num(c_str+1)) // c_str[0]: '$'
      return IMM32;
    else
      return SYMBOL;
  }
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
  int gen_len = -1; // generate how many machine code byte
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
    cur_elf_section->write(&op, 1);
    fwrite(&mod_rm, 1, 1, fs);
    cur_elf_section->write(&mod_rm, 1);
    fwrite(&imm_num, 1, imm_size, fs);
    cur_elf_section->write((u8 *)&imm_num, imm_size);
    gen_len = imm_size + 2;
  }
  return gen_len;
}

// ref: http://x86.renejeschke.de/html/file_module_x86_id_308.html
// intel manual vol 2: 2a 2-6
// Intel® 64 and IA-32 Architectures Software Developer’s Manual Volume 2 (2A, 2B, 2C & 2D): Instruction Set Reference, A-Z
int sub_func(const Line &l)
{
  int gen_len = -1; // generate how many machine code byte
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
  if ((op1_type | op2_type) == 0x3)
  { // subl $2, %esp
    cout << "handle r, imm8" << endl;
    int reg_val = reg_to_val(l[2].substr(1));
    cout << "  reg_val: " << reg_val << endl;

    if (-1 != reg_val) 
    {
      mod_rm |= reg_val;
    }

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
    cur_elf_section->write(&op, 1);
    fwrite(&mod_rm, 1, 1, fs);
    cur_elf_section->write(&mod_rm, 1);
    fwrite(&imm_num, 1, imm_size, fs);
    cur_elf_section->write((u8 *)&imm_num, imm_size);
    gen_len = imm_size + 2;
  }
  return gen_len;
}

int gas_section_func(const Line &l)
{
  cout << "handle .section: " << l[0] << endl;

  // remove ^" 
  // remove "$
  regex re("^\"|\"$");
  string str = regex_replace(l[1], re, "");

  cur_elf_section = get_section(str);
  cout << "xx sec name: ";
  cur_elf_section->print_sec_name();


  //section_string.insert(str);
  return 0;
}

int gas_text_func(const Line &l)
{
  cout << "handle .text: " << l[0] << endl;
  cur_elf_section = get_section(l[0]);
  cout << "xx sec name: ";
  cur_elf_section->print_sec_name();
  //section_string.insert(".text");
  return 0;
}

int gas_global_func(const Line &l)
{
  cout << "handle .global: " << l[0] << endl;
  Elf32Sym *symbol = get_symbol(l[1]);

  u8 b = STB_GLOBAL;
  u8 t = 0;
  symbol->symbol.st_info |= ELF32_ST_INFO(b, t);
  cout << "gg symbol->symbol.st_info: " << (u32)symbol->symbol.st_info << endl;
  return 0;
}

int label_func(const Line &l)
{
  cout << "handle label: " << l[0] << endl;

  regex re(":$");
  string str = regex_replace(l[0], re, "");

  auto ret = elf_string.insert(str);
  if (ret.second == false)
  {
    cout << "symbol '" << str << "' is already defined" << endl;
    exit(1);
  }

  Elf32Sym *symbol = get_symbol(str);

  symbol->symbol_str_ = str;
  symbol->which_section_ = cur_elf_section->sec_name();
  //elf_symbol.insert({cur_elf_section->sec_name(), symbol});
  //elf_symbol.insert({str, symbol});

  return 0;
}

int gas_type_func(const Line &l)
{
  cout << "handle .type" << l[0] << endl;
  Elf32Sym *symbol = get_symbol(l[1]);
  if (l[2] == "@function")
  {
    u8 b = 0;
    u8 t = STT_FUNC;
    symbol->symbol.st_info |= ELF32_ST_INFO(b, t);
    cout << "qq symbol->symbol.st_info: " << (u32)symbol->symbol.st_info << endl;
  }
  return 0;
}

// b8 +rd r32, imm32
int mov_func(const Line &l)
{
  vector<u8> machine_code;

  int op1_type;
  int op2_type;
  int gen_len = -1; // generate how many machine code byte

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
    cur_elf_section->write(&op, 1);
    fwrite(&mod_rm, 1, 1, fs);
    cur_elf_section->write(&mod_rm, 1);
    gen_len = 2;
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
    cur_elf_section->write(&op, 1);
    fwrite(&imm32_num, 1, 4, fs);
    cur_elf_section->write((u8 *)&imm32_num, 4);
    gen_len = 5;
  }



  for (auto &i : l)
  {
    cout << i << endl;
  }
  return gen_len;
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
  cur_elf_section->write(&op, 1);
  return 1;
}

int push_func(const Line &l)
{
  int gen_len = -1; // generate how many machine code byte

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
    cur_elf_section->write(&op, 1);
    gen_len = 1;
  }

  if (op1_type == SYMBOL)
  {
    int imm32 = 0;
    op = 0x68;
    cur_elf_section->write(&op, 1);
    cur_elf_section->write((u8*)&imm32, sizeof(imm32));
    gen_len = 5;
  }

  return gen_len;
}

int pop_func(const Line &l)
{
  int gen_len = -1; // generate how many machine code byte

  cout << "handle push" << endl;

  if (l.size() != 2)
  {
    cout << "syntax error" << endl;
  }

  int op1_type = check_operand_type(l[1]);
  u8 op=0x58;
  int reg_val = reg_to_val(l[1].substr(1));

  if (op1_type == R32 || op1_type == R16)
  {
    op = 0x58 + reg_val;
    fwrite(&op, 1, 1, fs);
    cur_elf_section->write(&op, 1);
    gen_len = 1;
  }

#if 0
  if (op1_type == SYMBOL)
  {
    int imm32 = 0;
    op = 0x68;
    cur_elf_section->write(&op, 1);
    cur_elf_section->write((u8*)&imm32, sizeof(imm32));
    gen_len = 5;
  }
#endif
  return gen_len;
}

int call_func(const Line &l)
{
  int gen_len = -1; // generate how many machine code byte

  cout << "handle call" << endl;

  s32 rel32 = -4;
  s8 op = 0xe8;
  cur_elf_section->write((u8*)&op, 1);
  cur_elf_section->write((u8*)&rel32, sizeof(rel32));

  gen_len = 5;

  return gen_len;
}

int ret_func(const Line &l)
{
  int gen_len = -1; // generate how many machine code byte
  cout << "\thandle ret" << endl;

  if (l.size() > 2 || l.size() <= 0)
  {
    cout << "syntax error" << endl;
  }

  u8 op=0xc3; // 0xc3: Near return to calling procedure. or 0xcb: Far return to calling procedure.
  if (l.size() == 1)
  {
    fwrite(&op, 1, 1, fs);
    cur_elf_section->write(&op, 1);
    gen_len = 1;
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



  return gen_len;
}

map<string, Fp> obj_handle;



void gen_obj(const vector<Line> &tokens)
{
  for (auto &line : tokens)
  {
    string inst = line[0];
    if ('.' == line[0][0]) // pseudo inst
    {
    }
    else
    {
      inst = trim_inst_size(line[0]);
    }

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
      int result;
      fp = it->second;
      result = (*fp)(line);
      if (result != -1)
      {
      }
      else
      {
        cout << "handle error" << endl;
      }
    }
    else if (is_label(line[0]))
         {
           label_func(line);
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
  obj_handle.insert({"pop", pop_func});
  obj_handle.insert({"call", call_func});
  obj_handle.insert({"ret", ret_func});
  obj_handle.insert({".text", gas_text_func});
  obj_handle.insert({".global", gas_global_func});
  obj_handle.insert({".type", gas_type_func});
  obj_handle.insert({".section", gas_section_func});

  char *pos = strrchr(argv[1], '.');
  string obj_fn;
  if (pos)
  {
    obj_fn = string{&argv[1][0], pos+1};
    obj_fn.push_back('o');
  }

  ifstream ifs(argv[1]);
  cout << "open obj_fn: " << obj_fn << endl;

  fs = fopen("/tmp/tt.o", "wb");

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

#if 0
  for (auto &i : section_string)
  {
    cout << "section string: " << i << endl;
  }
#endif

  write_section_to_file(obj_fn);

  dump_section();
  return 0;
}
