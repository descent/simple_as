#ifndef SECTION_H
#define SECTION_H

#include <vector>
#include <iostream>

#include "mytype.h"

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


extern const char *type_str[];

class ElfSection
{
  public:
    ElfSection(const string &sec_name);
    int write(const u8 *buf, u32 len);
    int len_add(u32 len)
    {
      len_ += len;
      return 0;
    }
    void print()
    {
      int idx=0;
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
  private:
    u32 len_;
    vector<u8> data_;
    string sec_name_; // section name
};

ElfSection *get_section(const string &section_name);
void dump_section();

#endif
