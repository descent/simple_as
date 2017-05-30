#include "section.h"

#include <cstdio>

using namespace std;

const char *type_str[] =
{
  FOREACH_FRUIT(GENERATE_STRING)
};



#if 0
need create the section
  [ 5] .symtab           SYMTAB          00000000 000048 000060 10      6   5  4
  [ 6] .strtab           STRTAB          00000000 0000a8 000006 00      0   0  1
  [ 7] .shstrtab  


use readelf -x dump section content

descent@debian64:test_pattern$ readelf -x .shstrtab op_1.gcc.o 

Hex dump of section '.shstrtab':
  0x00000000 002e7379 6d746162 002e7374 72746162 ..symtab..strtab
  0x00000010 002e7368 73747274 6162002e 74657874 ..shstrtab..text
  0x00000020 002e6461 7461002e 62737300 61627800 ..data..bss.abx.

#endif

//set<string> section_string{".symtab", ".strtab", ".shstrtab"};
//set<string> section_string{".shstrtab"};
set<string> section_string;
set<string> elf_string;
map<string, Elf32Sym> elf_symbol;

namespace
{
  map<string, ElfSection> sections;
}

int write_section_to_file(const string &fn)
{
  ElfSection *symbol_section = get_section(".symtab");

  cout << "zz symbol_section->length(): " << symbol_section->length() << endl;

  get_section("");
  u8 null_char = '\0';

  map<string, u32> str_to_index; // .strtab section index
  {
    u32 str_index = 1;

    ElfSection *section = get_section(".strtab");
    section->write(&null_char, 1);
    for (auto &i : elf_string)
    {
      section->write((const u8*)i.c_str(), i.length());
      section->write(&null_char, 1);

      // generate string, index pair
      str_to_index.insert({i, str_index});
      str_index += i.length() + 1;
    }

  }

  for(auto &i : elf_symbol)
  {
#if 1
    if (i.second.is_rel_ == true)
    {
      string rel_section_name = ".rel" + i.second.which_section_;
      ElfSection *sec = get_section(rel_section_name);
      sec->write((u8 *)&i.second.rel_, sizeof(i.second.rel_));
    }
#endif

  }

  ElfSection *section = get_section(".shstrtab");

  u32 sec_name_index = 1;
  section->write(&null_char, 1);

  for (auto &i : section_string)
  {
    ElfSection *sec = get_section(i);
    if (sec)
      sec->set_name_index(sec_name_index);

    section->write((const u8*)i.c_str(), i.length());
    section->write(&null_char, 1);
    sec_name_index += i.length() + 1;
  }

  u32 sec_index=0;
  map<string, u32> sec_to_index;

  // generate section name, index pair
  for(auto &i : sections)
  {
    sec_to_index.insert({i.second.sec_name(), sec_index}); 
    ++sec_index;
  }

  for(auto &i : sec_to_index)
  {
    cout << i.first << ", " << i.second << endl;
  }

#if 1
  symbol_section->section_header_.sh_link = sec_to_index[".strtab"];

  for(auto &i : elf_symbol)
  {
    Elf32Sym symbol = i.second;

    symbol.symbol.st_name = str_to_index[i.second.symbol_str_];
    symbol.symbol.st_shndx = sec_to_index[i.second.which_section_];

    //symbol.symbol.st_info = 0;
    cout << "rr symbol.symbol.st_info: " << (u32)symbol.symbol.st_info << endl;

    symbol_section->write((const u8 *)&symbol.symbol, sizeof(Symbol));
    //cout << "  symbol.st_name: " << symbol.st_name << endl;
  }
#endif

  // note ref: elf document 1-13, should greate last local symbol index
  symbol_section->section_header_.sh_info = elf_symbol.size()+2; // work around
  //symbol_section->section_header_.sh_info = 100;

  //symbol_section = get_section(".symtab");


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
  elf_header.e_phentsize = 0;            /* 程序头部中一个条目的长度 */
  elf_header.e_shentsize = sizeof(Elf32Shdr);            /* 节头部中一个条目的长度 */
  elf_header.e_shstrndx = sec_to_index[".shstrtab"];
  elf_header.e_shnum = sections.size();
  elf_header.e_phnum = 0;

  fwrite(&elf_header, 1, sizeof(Elf32Ehdr), fs);

  u32 cur_pos = sizeof(Elf32Ehdr) + sizeof(Elf32Shdr) * elf_header.e_shnum;

  //for(int i=0 ; i < sections.size() ; ++i)
  u32 offset = 0;
  // write section header to a file
  for(auto &i : sections)
  {
    Elf32Shdr section_header = i.second.section_header_;

    //section_header.sh_name = (elf32_word)"abc";
    section_header.sh_name = i.second.name_index();
    cout << "i: " << i.second.sec_name() << endl;;
    cout << "section_header.sh_name: " << section_header.sh_name << endl;
    section_header.sh_offset = cur_pos + offset;
    section_header.sh_size = i.second.length();
    #if 0
    section_header.sh_type = 1;
    section_header.sh_flags = 0x4;
    section_header.sh_addr = 0x0;
    section_header.sh_link = 0;
    section_header.sh_info = 0;
    section_header.sh_addralign = 0;
    section_header.sh_entsize = 0;
    #endif

    cout << "sizeof section header: " << sizeof(Elf32Shdr) << endl;
    fwrite(&section_header, 1, sizeof(Elf32Shdr), fs);
    offset += section_header.sh_size;
  }

  // write section content to a file
  for(auto &i : sections)
  {
    cout << "write section name: " << i.first << ", size: " << i.second.length() << endl;
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

Elf32Sym *get_symbol(const string &symbol_name)
{
  auto it = elf_symbol.find(symbol_name);
  if (it == elf_symbol.end()) // not found
  {
    Elf32Sym symbol{0};
    symbol.is_rel_ = false;
    elf_symbol.insert({symbol_name, symbol});
    it = elf_symbol.find(symbol_name);
    return &(it->second);
  }
  else
  {
    return &(it->second);
  }
}

ElfSection *get_section(const string &section_name)
{
  auto it = sections.find(section_name);
  bool add_to_symbol_section = true;

  if (it == sections.end()) // not found
  {
    ElfSection section{section_name};

    cout << "oo section_name: " << section_name << endl;
    if (section_name == ".text")
      section.init_text_section();
    else if (section_name == ".shstrtab")
         {
           section.init_shstrtab_section();
           add_to_symbol_section = false;
         }
         else if (section_name == ".strtab")
              {
                section.init_shstrtab_section();
                add_to_symbol_section = false;
              }
              else if (section_name == ".symtab")
                   {
                     section.init_symtab_section();
                     add_to_symbol_section = false;
                   }
                   else if (section_name == "") // null section
                        {
                          section.init_null_section();
                          add_to_symbol_section = false;
                        }
                        else if (section_name == ".bss") 
                             {
                               section.init_bss_section();
                             }
                        else
                        {
                          regex re("^.rel.*");
                          bool found = regex_match(section_name, re);
                          if (found)
                          {
                            add_to_symbol_section = false;
                            cout << "find rel" << endl;
                            section.init_rel_section();
                          }
                          else
                            section.init_other_section();
                        }

    sections.insert({section_name, section});
    auto new_it = sections.find(section_name);
    cout << "yy add new section: " << section_name << endl;
    section_string.insert(section_name);

    if (add_to_symbol_section)
    {
      Elf32Sym *sym = get_symbol(section_name);

      u8 b = 0;
      u8 t = STT_SECTION;
      sym->symbol.st_info |= ELF32_ST_INFO(b, t);
      sym->which_section_ = section_name;
    }
    if (new_it == sections.end()) // not found
    {
      cout << "something error, should not go here" << endl;
      return nullptr;
    }
    else
    {
      cout << "zz find section: " << section_name << endl;
      return &(new_it->second);
    }
  }

  cout << "pp find section: " << section_name << endl;
  return &(it->second);
}

ElfSection::ElfSection(const string &sec_name):
  len_(0), name_index_(0), sec_name_{sec_name}
{
#if 0
  if (sec_name == ".text")
    init_text_section();
  else if (sec_name == ".shstrtab")
         init_shstrtab_section();
       else
         init_other_section();
#endif
}

int ElfSection::init_text_section()
{
  section_header_.sh_name = 0;
  section_header_.sh_type = 1;
  section_header_.sh_flags = 0x2 | 0x4;
  section_header_.sh_addr = 0x0;
  //section_header_.sh_offset = cur_pos + offset;
  //section_header_.sh_size = i.second.length();
  section_header_.sh_link = 0;
  section_header_.sh_info = 0;
  section_header_.sh_addralign = 0;
  section_header_.sh_entsize = 0;
  return 0;
}

int ElfSection::init_shstrtab_section()
{
  section_header_.sh_type = 3;
  section_header_.sh_flags = 0;
  section_header_.sh_addr = 0x0;
  //section_header_.sh_offset = cur_pos + offset;
  //section_header_.sh_size = i.second.length();
  section_header_.sh_link = 0;
  section_header_.sh_info = 0;
  section_header_.sh_addralign = 0;
  section_header_.sh_entsize = 0;
  return 0;
}

int ElfSection::init_symtab_section()
{
  Elf32Sym symbol{0};
  write((const u8 *)&symbol.symbol, sizeof(Symbol));

  section_header_.sh_type = 2;
  section_header_.sh_flags = 0;
  section_header_.sh_addr = 0x0;
  //section_header_.sh_offset = cur_pos + offset;
  //section_header_.sh_size = i.second.length();
  section_header_.sh_link = 0; // note ref: elf document 1-13, need to set .strtab index
  section_header_.sh_info = 0; // note ref: elf document 1-13, should greate last local symbol index
  section_header_.sh_addralign = 0;
  section_header_.sh_entsize = sizeof(Symbol);




  return 0;
}

int ElfSection::init_null_section()
{
  section_header_.sh_type = 0;
  section_header_.sh_flags = 0;
  section_header_.sh_addr = 0x0;
  //section_header_.sh_offset = cur_pos + offset;
  //section_header_.sh_size = i.second.length();
  section_header_.sh_link = 0;
  section_header_.sh_info = 0;
  section_header_.sh_addralign = 0;
  section_header_.sh_entsize = 0;
  return 0;
}

int ElfSection::init_other_section()
{
  section_header_.sh_type = 1;
  section_header_.sh_flags = 0;
  section_header_.sh_addr = 0x0;
  //section_header_.sh_offset = cur_pos + offset;
  //section_header_.sh_size = i.second.length();
  section_header_.sh_link = 0;
  section_header_.sh_info = 0;
  section_header_.sh_addralign = 0;
  section_header_.sh_entsize = 0;
  return 0;
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
