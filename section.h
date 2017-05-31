#ifndef SECTION_H
#define SECTION_H

#include <set>
#include <map>
#include <vector>
#include <iostream>

#include "mytype.h"
#include "elf.h"

#define MAX_MACHINE_SIZE 512

using namespace std;

// ref: http://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
// thanks for coldnew

#define FOREACH_FRUIT(FRUIT) \
        FRUIT(TEXT)   \
        FRUIT(DATA)  \
        FRUIT(BSS)   \
        FRUIT(UNKNOWN_SECTION) 

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum Section
{
  FOREACH_FRUIT(GENERATE_ENUM)
};


extern map<string, Elf32Sym> elf_symbol;
extern set<string> elf_string;
//extern const char *type_str[];

//extern set<string> section_string;

class ElfSection
{
  public:
    ElfSection(const string &sec_name);
    int write(const u8 *buf, u32 len);
    int init_text_section();
    int init_shstrtab_section();
    int init_symtab_section();
    int init_other_section();
    int init_null_section();
    int init_bss_section();
    int init_data_section();
    int init_rel_section();
    int len_add(u32 len)
    {
      len_ += len;
      return 0;
    }
    void print()
    {
      int idx=0;
      cout << "section name: " << sec_name_ << endl; 
      cout << "len_: " << data_.size() << endl; // can use cout??
      for (auto &i : data_)
      {
        if ((idx % 16) == 0)
          cout << endl;
        cout << hex << (u32)i << ' ';
        ++idx;
      }
      cout << dec << endl;
    }
    void print_sec_name()
    {
      cout << sec_name_ << endl;
    }
    const u8 *data() const
    {
      return &data_[0];
    }
    auto length() const
    {
      return data_.size();
    }
    void set_name_index(u32 index)
    {
      name_index_ = index;
    }
    auto name_index() const
    {
      return name_index_;
    }
    const string &sec_name() const
    {
      return sec_name_;
    }
    Elf32Shdr section_header_;
  private:
    u32 len_;
    u32 name_index_;
    vector<u8> data_;
    string sec_name_; // section name
};

ElfSection *get_section(const string &section_name);
Elf32Sym *get_symbol(const string &symbol_name);
void dump_section();
int write_section_to_file(const string &fn);

#endif
