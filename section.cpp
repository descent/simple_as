#include "elf.h"
#include "section.h"

#include <cstdio>

#include <map>
using namespace std;

const char *type_str[] =
{
  FOREACH_FRUIT(GENERATE_STRING)
};

namespace
{
  map<string, ElfSection> sections;
}

int write_section_to_file(const string &fn)
{
  FILE *fs;

  fs = fopen(fn.c_str(), "w");

  // e_ident: http://www.sco.com/developers/gabi/latest/ch4.eheader.html#elfid
  Elf32Ehdr elf_header{0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  elf_header.e_type = 1; // REL (Relocatable file)
  elf_header.e_machine = 3;
  elf_header.e_version = 1;
  elf_header.e_entry = 0;
  elf_header.e_phoff = 0;
  elf_header.e_shoff = sizeof(Elf32Ehdr);                /* section header 节头部偏移量 */
  elf_header.e_flags = 0;                /* 处理器特定标志 */
  elf_header.e_ehsize = sizeof(Elf32Ehdr);               /* elf头部长度 */
  elf_header.e_phentsize;            /* 程序头部中一个条目的长度 */
  elf_header.e_shentsize = sizeof(Elf32Shdr);            /* 节头部中一个条目的长度 */
  elf_header.e_shstrndx;  
  elf_header.e_shnum = sections.size();
  elf_header.e_phnum = 0;

  fwrite(&elf_header, 1, sizeof(Elf32Ehdr), fs);

  u32 cur_pos = sizeof(Elf32Ehdr) * elf_header.e_shnum;

  //for(int i=0 ; i < sections.size() ; ++i)
  u32 offset = 0;
  for(auto &i : sections)
  {
    Elf32Shdr section_header;

    //section_header.sh_name = (elf32_word)"abc";
    section_header.sh_name = 0x1234;
    section_header.sh_type = 1;
    section_header.sh_flags = 0x4;
    section_header.sh_addr = 0x0;
    section_header.sh_offset = cur_pos + offset;
    section_header.sh_size = i.second.length();
    section_header.sh_link = 0;
    section_header.sh_info = 0;
    section_header.sh_addralign = 0;
    section_header.sh_entsize = 0;

    cout << "sizeof section header: " << sizeof(Elf32Shdr) << endl;
    fwrite(&section_header, 1, sizeof(Elf32Shdr), fs);
    offset += section_header.sh_size;
  }

  for(auto &i : sections)
  {
    //cout << "section name: " << i.first << endl;
    auto buf = i.second.data();
    fwrite(buf, 1, i.second.length(), fs);
  }
  fclose(fs);
  return 0;
}

void dump_section()
{
  for(auto &i : sections)
  {
    //cout << "section name: " << i.first << endl;
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
