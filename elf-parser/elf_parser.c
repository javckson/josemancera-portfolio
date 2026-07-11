#include<stdio.h>
#include<elf.h>
#include<stdlib.h>
#include<stdint.h>

static void print_magic( const unsigned char ident[EI_NIDENT]){
	printf(" magic   ");
	for( int i = 0; i < EI_NIDENT; i++){
		printf("%02x  ", ident[i]);
	}
	printf("\n");

}

static const char *get_class_string(unsigned char c ){

	switch (c ){

		case ELFCLASS32: return "ELFE32";
		case ELFCLASS64: return "ELF64";
		default: return "Unknown";

	}

}

static const char *get_data_string(unsigned char d){

	switch (d){
		case ELFDATA2LSB: return "Little endian";
		case ELFDATA2MSB: return "Big endian";
		default: return "Unknown";
	}
}

static const char *get_type_string(uint16_t t ){

	switch (t){

		case ET_NONE: return "No file type";
		case ET_REL:  return "Relocatable";
		case ET_EXEC: return "Executable";
		case ET_DYN: return "Shared object";
		case ET_CORE: return "Core";
		default:  return "Unknown";
	}
}

static const char *get_machine_string(uint16_t m){

	switch(m){
		case EM_X86_64: return "x86-64";
		case EM_386: return "x86";
		case EM_AARCH64: return "ARM";
		default: return "Unknown";
	}
}

int main(int argc, char *argv[]){

	FILE *fp;
	Elf64_Ehdr ehdr;

	if (argc != 2){
		fprintf(stderr, "Usage:  %s <elf-file>\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "rb");
	if (fp == NULL){
		perror("fopen");
		return 1;
	   }

	if (fread(&ehdr , 1, sizeof(ehdr), fp ) != sizeof(ehdr)){
		fprintf(stderr, "Error: could not  read ELF header.\n");
		fclose(fp);
		return 1 ;
	}

	fclose(fp);

	if(ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
			ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
			ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
			ehdr.e_ident[EI_MAG3] != ELFMAG3 ){
		fprintf(stderr, "Error: not an elf file.\n");
		return 1;
	}

	print_magic(ehdr.e_ident);

	printf("Class: %s\n", get_class_string(ehdr.e_ident[EI_CLASS]));
	printf("Data: %s\n", get_data_string(ehdr.e_ident[EI_DATA]));
	printf("Type: %s\n",get_type_string(ehdr.e_type));
	printf("Machine: %s\n", get_machine_string(ehdr.e_machine));
	printf("Entry point: 0x%lx\n", (unsigned long )ehdr.e_entry);
	printf("Section header offset: %lu\n",(unsigned long) ehdr.e_shoff);
	printf("Number of program headers: %u\n",ehdr.e_phnum);
	printf("Number of section headers: %u\n", ehdr.e_shnum);
	printf("Section header string table index: %u\n", ehdr.e_shstrndx);

	return 0;
}
