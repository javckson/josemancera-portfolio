#include<stdio.h>
#include<elf.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>

void print_symbol_type(unsigned char type)
{
	switch (type){
       case STT_NOTYPE: printf("NOTYPE"); break;
       case STT_OBJECT: printf("OBJECT"); break;
       case STT_FUNC: printf("func"); break;
       case STT_SECTION: printf("SECTION"); break;
       case STT_FILE: printf("FILE"); break;
       default: printf("OTHER"); break;
	}
}


int main(int argc, char *argv[])
{
	FILE *fp;
	long file_size;
	unsigned char *buffer;

	Elf64_Ehdr *ehdr;
	Elf64_Shdr *shdr_table;
	char *section_names;

if (argc != 2 ){
	printf("Usage:%s <elf_file>\n", argv[0]);
	return 1;
}

fp = fopen(argv[1], "rb");
if (fp == NULL){
	perror("fopen");
	return 1;
}


fseek(fp, 0,SEEK_END);
file_size = ftell(fp);
rewind(fp);

buffer = malloc(file_size);
if (buffer == NULL){
	perror("malloc");
	fclose(fp);
	return 1;
}

if (fread(buffer,1 , file_size, fp) != (size_t )file_size){
	perror("fread");
	free(buffer);
	fclose(fp);
	return 1;
	}

fclose(fp);

ehdr = (Elf64_Ehdr *) buffer;

if(memcmp(ehdr->e_ident,ELFMAG, SELFMAG)!= 0){
	printf("Not an ELF file.\n");
	free(buffer);
	return 1;
}

if(ehdr->e_ident[EI_CLASS] != ELFCLASS64){
	printf("Only ELF64 is supported.\n");
	free(buffer);
	return 1;
}

printf("ELF Header\n");
printf("Entry point: 0x%lx\n", ehdr->e_entry);
printf("Section header offset: %lu\n", ehdr->e_shoff);
printf("Number of sections: %u\n", ehdr->e_shnum);
printf("Section header string table index: %u\n", ehdr->e_shstrndx);

shdr_table = (Elf64_Shdr *)(buffer + ehdr->e_shoff);
section_names = (char *)(buffer + shdr_table[ehdr->e_shstrndx].sh_offset);

printf("\nsections\n");

for(int i = 0; i < ehdr->e_shnum; i++){

	printf("[%2d] %-20s type=%u offset=0x%lx size = 0x%lx\n",
			i,
			section_names + shdr_table[i].sh_name,
			shdr_table[i].sh_type,
			shdr_table[i].sh_offset,
			shdr_table[i].sh_size);
}

printf("\nSymbol Tables\n");

for( int i = 0; i < ehdr->e_shnum; i++){
	

		if(shdr_table[i].sh_type == SHT_SYMTAB ||
				shdr_table[i].sh_type == SHT_DYNSYM){

                         Elf64_Shdr *sym_section = &shdr_table[i];
			 Elf64_Shdr  *str_section = &shdr_table[sym_section->sh_link];
			 

			Elf64_Sym *symbols = (Elf64_Sym *)(buffer + sym_section->sh_offset);
			char *symbol_names = (char *)(buffer + str_section->sh_offset);

			size_t symbol_count = sym_section->sh_size / sym_section->sh_entsize;


     printf("\nSymbols table: %s \n", section_names + sym_section->sh_name);
     printf("Number of symbols: %zu\n\n" ,symbol_count);

     printf("%-5s %-18s %-10s %-10s %s %s\n",
		     "Num", "value", "Size", "Type" , "Section" , "Name");


     for( size_t j = 0; j < symbol_count; j++){
	     unsigned char type = ELF64_ST_TYPE(symbols[j].st_info);

	     printf("%-5zu 0x%-16lx  %-10lu", j, 
			     symbols[j].st_value,
			     symbols[j].st_size);

	     print_symbol_type(type);

	     printf("    %-10u%s\n",
			     symbols[j].st_shndx,
			     symbol_names + symbols[j].st_name);

     }
      
		}

		 }

	free(buffer);
	return 0;
}
