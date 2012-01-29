#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <elf.h>
#include "config.h"
#include "bytes.h"
#include "stack.h"

#define TOTAL_HEADER_SIZE   (sizeof(Elf32_Ehdr) + sizeof(Elf32_Phdr) * 2)
#define PROG_ENTRY          (MEMORY_ORG + TOTAL_HEADER_SIZE)
#define POINTER_START_SIZE  (1 + sizeof(uint32_t))

#define SAFE_WRITE(ptr, count, fd) if (fwrite(ptr, sizeof(uint8_t), count, fd) < count) { \
	PRINT_ERROR(MSG_WRITE_ERROR); \
	abort(); \
}

bytes_t *compile(FILE *fd);
bytes_t *set_pointer_start(uint32_t filesize);
Elf32_Ehdr *make_ehdr();
Elf32_Phdr *make_phdr_code(uint32_t filesize);
Elf32_Phdr *make_phdr_data(uint32_t filesize);

int main(int argc, char **argv) {
	bytes_t *code, *pointer_start;
	const char *ofname;
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *codehdr, *datahdr;
	FILE *infile, *outfile;
	uint32_t binsize, addr;
	struct stat info;

	if (argc < 2 || argc > 3) {
		printf("usage: %s <filename> [<output>]", argv[0]);
		return 0;
	}

	infile = fopen(argv[1], "r");
	code = compile(infile);
	fclose(infile);

	binsize = TOTAL_HEADER_SIZE + POINTER_START_SIZE + code->length;
	addr = binsize + (PAGE_SIZE - binsize % PAGE_SIZE);

	ehdr = make_ehdr();
	codehdr = make_phdr_code(binsize);
	datahdr = make_phdr_data(addr);
	pointer_start = set_pointer_start(addr);

	if (argc == 3) {
		ofname = argv[2];
		outfile = fopen(argv[2], "w");
	} else {
		ofname = "a.out";
		outfile = fopen("a.out", "w");
	}

	SAFE_WRITE((uint8_t *)ehdr,    sizeof(Elf32_Ehdr),    outfile);
	SAFE_WRITE((uint8_t *)codehdr, sizeof(Elf32_Phdr),    outfile);
	SAFE_WRITE((uint8_t *)datahdr, sizeof(Elf32_Phdr),    outfile);
	SAFE_WRITE(pointer_start->p,   pointer_start->length, outfile);
	SAFE_WRITE(code->p,            code->length,          outfile);

	fclose(outfile);
	stat(ofname, &info);
	chmod(ofname, info.st_mode | S_IXUSR | S_IXGRP | S_IXOTH);

	free(ehdr);
	free(codehdr);
	free(datahdr);
	bytes_del(code);
	bytes_del(pointer_start);

	return OK;
}

/*
 * Creates an Elf header.
 */
Elf32_Ehdr *make_ehdr() {
	int k;
	Elf32_Ehdr *ehdr;

	if ((ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr))) == NULL) {
		PRINT_ERROR(MSG_OUT_OF_MEMORY);
		abort();
	}

	ehdr->e_ident[EI_MAG0] = ELFMAG0;
	ehdr->e_ident[EI_MAG1] = ELFMAG1;
	ehdr->e_ident[EI_MAG2] = ELFMAG2;
	ehdr->e_ident[EI_MAG3] = ELFMAG3;
	ehdr->e_ident[EI_CLASS] = ELFCLASS32;
	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr->e_ident[EI_VERSION] = EV_CURRENT;
	ehdr->e_ident[EI_OSABI] = ELFOSABI_NONE;
	ehdr->e_ident[EI_ABIVERSION] = 0;
	for (k = EI_PAD; k < EI_NIDENT; k++)
		ehdr->e_ident[k] = 0;
	ehdr->e_type = ET_EXEC;
	ehdr->e_machine = EM_386;
	ehdr->e_version = EV_CURRENT;
	ehdr->e_entry = PROG_ENTRY;
	ehdr->e_phoff = sizeof(Elf32_Ehdr);
	ehdr->e_shoff = 0;
	ehdr->e_flags = 0;
	ehdr->e_ehsize = sizeof(Elf32_Ehdr);
	ehdr->e_phentsize = sizeof(Elf32_Phdr);
	ehdr->e_phnum = 2;
	ehdr->e_shentsize = sizeof(Elf32_Shdr);
	ehdr->e_shnum = 0;
	ehdr->e_shstrndx = SHN_UNDEF;
	return ehdr;
}

/*
 * Returns a segment header for the code
 */
Elf32_Phdr *make_phdr_code(uint32_t filesize) {
	Elf32_Phdr *phdr;

	if ((phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr))) == NULL) {
		PRINT_ERROR(MSG_OUT_OF_MEMORY);
		abort();
	}

	phdr->p_type = PT_LOAD;
	phdr->p_offset = 0;
	phdr->p_vaddr = MEMORY_ORG;
	phdr->p_paddr = MEMORY_ORG;
	phdr->p_filesz = filesize;
	phdr->p_memsz = filesize;
	phdr->p_flags = PF_X | PF_R;
	phdr->p_align = PAGE_SIZE;
	return phdr;
}

/*
 * Returns a segment header for the data
 */
Elf32_Phdr *make_phdr_data(uint32_t start) {
	Elf32_Phdr *phdr;

	if ((phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr))) == NULL) {
		PRINT_ERROR(MSG_OUT_OF_MEMORY);
		abort();
	}

	phdr->p_type = PT_LOAD;
	phdr->p_offset = start;
	phdr->p_vaddr = MEMORY_ORG + start;
	phdr->p_paddr = MEMORY_ORG + start;
	phdr->p_filesz = 0;
	phdr->p_memsz = 30000;
	phdr->p_flags = PF_X | PF_W | PF_R;
	phdr->p_align = PAGE_SIZE;
	return phdr;
}

uint8_t *serialize_uint32_little_endian(uint32_t val) {
	uint8_t *data;
	int k;
	if ((data = (uint8_t *)malloc(sizeof(uint32_t))) == NULL) {
		PRINT_ERROR(MSG_OUT_OF_MEMORY);
		abort();
	}
	for (k = 0; k < sizeof(uint32_t); k++)
		data[k] = (val >> (8 * k)) & 0xff;
	return data;
}

uint8_t *serialize_uint32_big_endian(uint32_t val) {
	uint8_t *data;
	int k = 3;
	if ((data = (uint8_t *)malloc(sizeof(uint32_t))) == NULL) {
		PRINT_ERROR(MSG_OUT_OF_MEMORY);
		abort();
	}
	while (k --> 0)
		data[k] = (val >> (8 * k)) & 0xff;
	return data;
}

/* because x86 */
#define SERIALIZE_UINT32 serialize_uint32_little_endian

void bytes_append_uint32(bytes_t *data, uint32_t val) {
	uint8_t *intdata;
	intdata = SERIALIZE_UINT32(val);
	bytes_extend(data, intdata, sizeof(uint32_t));
	free(intdata);
}

void bytes_replace_uint32(bytes_t *data, uint32_t val, size_t start) {
	uint8_t *intdata;
	int k;
	intdata = SERIALIZE_UINT32(val);
	for (k = 0; k < sizeof(uint32_t); k++)
		bytes_set(data, start + k, intdata[k]);
	free(intdata);
}

bytes_t *set_pointer_start(uint32_t filesize) {
	bytes_t *code;
	code = bytes_make();
	bytes_append(code, 0xb8);
	bytes_append_uint32(code, MEMORY_ORG + filesize);
	return code;
}

int token_p(int c) {
	return c == '<' || c == '>' || c == '+' || c == '-' || c == ',' || c == '.' || c == '[' || c == ']';
}

/* we assume that eax points to the beginning of the data section */
bytes_t *compile(FILE *fd) {
	bytes_t *code;
	int c;
	uint32_t seq, here, there;
	stack_t *jumps;

	jumps = NULL;
	code = bytes_make();

	while ((c = getc(fd)) != EOF) {
		switch (c) {
			case '<':
				seq = 1;
				while ((c = getc(fd)) == '<' || !token_p(c))
					if (c == '<')
						seq++;
				ungetc(c, fd);
				bytes_append(code, 0x2d); /* sub eax, [seq] */
				bytes_append_uint32(code, seq);
				break;
			case '>':
				seq = 1;
				while ((c = getc(fd)) == '>' || !token_p(c))
					if (c == '>')
						seq++;
				ungetc(c, fd);
				bytes_append(code, 0x05); /* add eax, [seq] */
				bytes_append_uint32(code, seq);
				break;
			case '+':
				seq = 1;
				while ((c = getc(fd)) == '+' || !token_p(c))
					if (c == '+')
						seq++;
				ungetc(c, fd);
				bytes_extend(code, (uint8_t *)"\x80\x00", 2); /* add [eax], [seq] */
				bytes_append(code, (uint8_t)seq);
				break;
			case '-':
				seq = 1;
				while ((c = getc(fd)) == '-' || !token_p(c))
					if (c == '-')
						seq++;
				ungetc(c, fd);
				bytes_extend(code, (uint8_t *)"\x80\x28", 2); /* sub [eax], [seq] */
				bytes_append(code, (uint8_t)seq);
				break;
			case ',':
				bytes_extend(code, (uint8_t *)"\x89\xc1", 2); /* mov ecx, eax ; move cell address into ecx */
				bytes_append(code, 0xb8);          /* mov eax, 3   ; select syscall read(2) */
				bytes_append_uint32(code, 3);
				bytes_append(code, 0xbb);          /* mov ebx, 0   ; stdin */
				bytes_append_uint32(code, 0);
				bytes_append(code, 0xba);          /* mov edx, 1   ; read one character */
				bytes_append_uint32(code, 1);
				bytes_extend(code, (uint8_t *)"\xcd\x80", 2); /* int 0x80     ; make syscall */
				bytes_extend(code, (uint8_t *)"\x89\xc8", 2); /* mov eax, ecx ; move cell address back to eax */
				break;
			case '.':
				bytes_extend(code, (uint8_t *)"\x89\xc1", 2); /* mov ecx, eax ; move cell address into ecx */
				bytes_append(code, 0xb8);          /* mov eax, 4   ; select syscall write(2) */
				bytes_append_uint32(code, 4);
				bytes_append(code, 0xbb);          /* mov ebx, 1   ; stdout */
				bytes_append_uint32(code, 1);
				bytes_append(code, 0xba);          /* mov edx, 1   ; write one character */
				bytes_append_uint32(code, 1);
				bytes_extend(code, (uint8_t *)"\xcd\x80", 2); /* int 0x80     ; make syscall */
				bytes_extend(code, (uint8_t *)"\x89\xc8", 2); /* mov eax, ecx ; move cell address back to eax */
				break;
			case '[':
				bytes_extend(code, (uint8_t *)"\x80\x38\x00", 3); /* cmp byte [eax], 0 */
				bytes_extend(code, (uint8_t *)"\x0f\x84", 2); /* je [forward]   ; jump to as yet unknown address */
				bytes_append_uint32(code, 0xdeadbeef);
				jumps = stack_push(jumps, code->length);
				break;
			case ']':
				if (jumps == NULL) {
					PRINT_ERROR(MSG_UNMATCHED_CLOSE_BRACKET);
					exit(COMPILE_ERROR);
				}
				bytes_extend(code, (uint8_t *)"\x80\x38\x00", 3); /* cmp byte [eax], 0 */
				bytes_extend(code, (uint8_t *)"\x0f\x85", 2); /* jne [backward] ; jump to corresponding bracket */
				bytes_append_uint32(code, 0xdeadbeef);
				here = PROG_ENTRY + POINTER_START_SIZE + code->length;
				there = PROG_ENTRY + POINTER_START_SIZE + jumps->v;
				bytes_replace_uint32(code, there - here, code->length - 4); 
				bytes_replace_uint32(code, here - there, jumps->v - 4);
				jumps = stack_pop(jumps);
				break;
		}
	}

	if ((c = stack_size(jumps)) != 0) {
		PRINT_ERROR(MSG_UNMATCHED_OPEN_BRACKETS, c);
		stack_liberate(jumps);
		exit(COMPILE_ERROR);
	}

	// cleanup code

	bytes_append(code, 0xb8);     /* mov eax, 1 */
	bytes_append_uint32(code, 1);
	bytes_append(code, 0xbb);     /* mov ebx, 0 */
	bytes_append_uint32(code, 0);
	bytes_extend(code, (uint8_t *)"\xcd\x80", 2); /* int 0x80 */

	return code;
}
