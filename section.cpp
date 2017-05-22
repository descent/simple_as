#include "section.h"

#include <map>

const char *type_str[] =
{
  FOREACH_FRUIT(GENERATE_STRING)
};

namespace
{
  map<string, ElfSection> sections;
}

void dump_section()
{
  for(auto &i : sections)
  {
    cout << "section name: " << i.first << endl;
    i.second.print();
  }
}

ElfSection *get_section(const string &section_name)
{
  auto it = sections.find(section_name);
  if (it == sections.end()) // not found
  {
    ElfSection section{section_name};
    sections.insert({section_name, section});
    auto new_it = sections.find(section_name);
    cout << "yy add new section" << endl;
    if (new_it == sections.end()) // not found
    {
      cout << "something error, should not go here" << endl;
      return nullptr;
    }
    else
    {
      return &(new_it->second);
    }
  }

  return &(it->second);
}

ElfSection::ElfSection(const string &sec_name):len_(0), sec_name_{sec_name}
{
}

int ElfSection::write(const u8 *buf, u32 len)
{
  for (u32 i=0 ; i < len ; ++i)
  {
    data_.push_back(buf[i]); 
  }
  len_ += len;
  return 0;
}
