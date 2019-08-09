#include <stdio.h>
#include <ctype.h>
#include <string.h>

// MD5 Hash
#include <gcrypt.h>

// Big Num
#include <gmp.h>

/* NYZC -- No YZ Compiler */
enum Token {
	// Keywords
	AUTO, BREAK, CASE, CHAR, CONST, CONTINUE, DEFAULT, DO, DOUBLE, ELSE,
	ENUM, EXTERN, FLOAT, FOR, GOTO, IF, INT, LONG, REGISTER, RETURN,
	SHORT, SIGNED, SIZEOF, STATIC, STRUCT, SWITCH, TYPEDEF, UNION,
	UNSIGNED, VOID, VOLATILE, WHILE,

	// Operator
	L_BRACKET, R_BRACKET, L_PARE, R_PARE, PERIOD, ARROW, DEC,
	SUB_ASSIGN, SUB, ADD, INC, ADD_ASSIGN, AND, LOGIC_AND, AND_ASSIGN,
	MUL_ASSIGN, MUL, BIT_COMPLEMENT, BIT_COMPL_ASSIGN, NOT_EQ,
	NOT, DIV_ASSIGN, DIV, MOD_ASSIGN, MOD, LESS_EQU, L_SHIFT_ASSIGN,
	L_SHIFT, LESS_THAN, GREAT_EQU, R_SHIFT_ASSIGN, R_SHIFT,
	GREATER_THAN, EQU, ASSIGN, XOR_ASSIGN, XOR, BIT_OR_ASSIGN, BIT_OR,
	LOGIC_OR, CONDITION, COLON, COMMA, L_CURLY, R_CURLY, SEMILICON,

	// Numeric Constant
	Integer_Constant, Float_Constant,

	Identifier, String_Literal,
	Header_Name, PP_Number, Character_Constant, End_Of_File,
	CANNOT_HANDLE,
};

struct ID {
	char name[65];
	char hash[16]; // MD5 Hash
};

struct token {
	enum Token t;
	union {
		struct ID id;
		// Remember to clean it
		mpf_t f;

		mpz_t z;

		char c;

		// Remember to free it
		char *s;
	};
};

void get_keyword_or_id(FILE *file, struct token *tok)
{
	tok->t = Identifier;
	int len = 0;
	while (len < 64) {
		char c = fgetc(file);

		if (feof(file)) {
			gcry_md_hash_buffer(GCRY_MD_MD5, tok->id.hash, tok->id.name, sizeof(tok->id.name));
			return;
		}

		if (!(isalpha(c) || isdigit(c) || c == '_')) {
			fseek(file, -1, SEEK_CUR);
			gcry_md_hash_buffer(GCRY_MD_MD5, tok->id.hash, tok->id.name, len);
			return;
		}

		tok->id.name[len++] = c;
		tok->id.name[len] = '\0';
		if (!strcmp("auto", tok->id.name)) {
			tok->t = AUTO;
		
		} else if (!strcmp("break", tok->id.name)) {
			tok->t = BREAK;
		
		} else if (!strcmp("case", tok->id.name)) {
			tok->t = CASE;
		
		} else if (!strcmp("char", tok->id.name)) {
			tok->t = CHAR;
		
		} else if (!strcmp("const", tok->id.name)) {
			tok->t = CONST;
		
		} else if (!strcmp("continue", tok->id.name)) {
			tok->t = CONTINUE;
		
		} else if (!strcmp("default", tok->id.name)) {
			tok->t = DEFAULT;
		
		} else if (!strcmp("do", tok->id.name)) {
			tok->t = DO;
		
		} else if (!strcmp("double", tok->id.name)) {
			tok->t = DOUBLE;
		
		} else if (!strcmp("else", tok->id.name)) {
			tok->t = ELSE;
		
		} else if (!strcmp("enum", tok->id.name)) {
			tok->t = ENUM;
		
		} else if (!strcmp("extern", tok->id.name)) {
			tok->t = EXTERN;
		
		} else if (!strcmp("float", tok->id.name)) {
			tok->t = FLOAT;
		
		} else if (!strcmp("for", tok->id.name)) {
			tok->t = FOR;
		
		} else if (!strcmp("goto", tok->id.name)) {
			tok->t = GOTO;
		
		} else if (!strcmp("if", tok->id.name)) {
			tok->t = IF;
		
		} else if (!strcmp("int", tok->id.name)) {
			tok->t = INT;
		
		} else if (!strcmp("long", tok->id.name)) {
			tok->t = LONG;
		
		} else if (!strcmp("register", tok->id.name)) {
			tok->t = REGISTER;
		
		} else if (!strcmp("return", tok->id.name)) {
			tok->t = RETURN;
		
		} else if (!strcmp("short", tok->id.name)) {
			tok->t = SHORT;
		
		} else if (!strcmp("signed", tok->id.name)) {
			tok->t = SIGNED;
		
		} else if (!strcmp("sizeof", tok->id.name)) {
			tok->t = SIZEOF;
		
		} else if (!strcmp("static", tok->id.name)) {
			tok->t = STATIC;
		
		} else if (!strcmp("struct", tok->id.name)) {
			tok->t = STRUCT;
		
		} else if (!strcmp("switch", tok->id.name)) {
			tok->t = SWITCH;
		
		} else if (!strcmp("typedef", tok->id.name)) {
			tok->t = TYPEDEF;
		
		} else if (!strcmp("union", tok->id.name)) {
			tok->t = UNION;
		
		} else if (!strcmp("unsigned", tok->id.name)) {
			tok->t = UNSIGNED;
		
		} else if (!strcmp("void", tok->id.name)) {
			tok->t = VOID;
		
		} else if (!strcmp("volatile", tok->id.name)) {
			tok->t = VOLATILE;
		
		} else if (!strcmp("while", tok->id.name)) {
			tok->t = WHILE;
		} else {
			tok->t = Identifier;
		}
	}
	tok->t = CANNOT_HANDLE;
}

void get_num_constant(FILE *file, struct token *tok)
{
	int isfloat = 0;
	int ishex = 0;
	int isoct = 0;
	int isexp = 0;

	char buffer[256] = {0};
	char c = fgetc(file);

	if (c == '0') {
		c = fgetc(file);
		if (c == 'x' || c == 'X') {
			ishex = 1;
		} else {
			fseek(file, -1, SEEK_CUR);
			isoct = 1;
		}
	} else {
		fseek(file, -1, SEEK_CUR);
	}

	for (int i = 0; i < 256; ++i) {
		c = fgetc(file);
		if (ishex && isxdigit(c)) {
			buffer[i] = c;
		} else if (isoct && isdigit(c) && c != '9' && c != '8') {
			buffer[i] = c;
		} else if (!ishex && !isoct && isdigit(c)) {
			buffer[i] = c;
		} else if (!isfloat && c == '.') {
			isfloat = 1;
			buffer[i] = c;
		} else if ((c == 'e' || c == 'E') && isexp == 0) {
			buffer[i] = 'e';
			c = fgetc(file);
			if ((c == '+' || c == '-') && ++i < 256) {
				buffer[i] = c;
			} else {
				fseek(file, -1, SEEK_CUR);
			}
		} else if (ishex && (c == 'p' || c == 'P') && isexp == 0) {
			buffer[i] = '@';
			c = fgetc(file);
			if ((c == '+' || c == '-') && ++i < 256) {
				buffer[i] = c;
			} else {
				fseek(file, -1, SEEK_CUR);
			}
		} else {
			break;
		}
	}
	if (isfloat) {
		tok->t = Float_Constant;
		mpf_init(tok->f);
		if (ishex) {
			mpf_set_str(tok->f, buffer, 16);
		} else {
			mpf_set_str(tok->f, buffer, 10);
		}
	} else {
		tok->t = Integer_Constant;
		mpz_init(tok->z);
		if (ishex) {
			mpz_set_str(tok->z, buffer, 16);
		} else if (isoct) {
			mpz_set_str(tok->z, buffer, 8);
		} else {
			mpz_set_str(tok->z, buffer, 10);
		}
	}
}

int get_escape_character(FILE *file)
{
	int ret;
	char c = fgetc(file);
	switch (c) {
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case '\'':
		return '\'';
	case '\"':
		return '\"';
	case '\?':
		return '\?';
	case '\\':
		return '\\';
	case 'x':
		c = fgetc(file);
		while (isxdigit(c)) {
			if (isalpha(c)) {
				ret = (ret << 4) | (toupper(c) - 'A');
			} else {
				ret = (ret << 4) | (c - '0');
			}
			c = fgetc(file);
		}
		fseek(file, -1, SEEK_CUR);
		return ret;
	default:
		if (isdigit(c) && c != '9' && c != '8') {
			ret = c - '0';
			c = fgetc(file);
			if (isdigit(c) && c != '9' && c != '8') {
				ret = (ret << 3) | (c - '0');
				c = fgetc(file);
				if (isdigit(c) && c != '9' && c != '8') {
					ret = (ret << 3) | (c - '0');
				} else {
					fseek(file, -1, SEEK_CUR);
				}
			} else {
				fseek(file, -1, SEEK_CUR);
			}
			return ret;
		}
		return -1;
	}
}

void get_char_constant(FILE *file, struct token *tok)
{
	char c;
	tok->t = Character_Constant;

	fgetc(file); // the '
	c = fgetc(file);
	if (c == '\\') {
		int k = get_escape_character(file);
		if (k == -1) {
			tok->t = CANNOT_HANDLE;
		} else {
			tok->c = (char)k;
		}
	} else {
		tok->c = c;
	}
	fgetc(file); // the '
}

void get_string_literal(FILE *file, struct token *tok)
{
	int i;
	char c;
	fgetc(file); // "
	tok->s = malloc(4096);
	for (i = 0; i < 4095; ++i) {
		c = fgetc(file);
		if (c == '\"') {
			tok->t = String_Literal;
			tok->s[i] = '\0';
			return;
		}

		if (c == '\\') {
			tok->s[i] = get_escape_character(file);
		} else {
			tok->s[i] = c;
		}
	}
	tok->t = CANNOT_HANDLE;
}

void scanner(FILE *file, struct token *tok)
{
	while (1) {
		int c = fgetc(file);
		if (feof(file)) {
			tok->t = End_Of_File;
			return;
		}

		if (isalpha(c) || c == '_') {
			fseek(file, -1, SEEK_CUR);
			get_keyword_or_id(file, tok);
			return;
		}

		if (isdigit(c)) {
			fseek(file, -1, SEEK_CUR);
			get_num_constant(file, tok);
			return;
		}

		if (c == '\'') {
			fseek(file, -1, SEEK_CUR);
			get_char_constant(file, tok);
			return;
		}

		if (c == '\"') {
			fseek(file, -1, SEEK_CUR);
			get_string_literal(file, tok);
			return;
		}

		// Operators or Punctuators
		switch (c) {
		case '[':
			tok->t = L_BRACKET;
			return;
		case ']':
			tok->t = R_BRACKET;
			return;
		case '(':
			tok->t = L_PARE;
			return;
		case ')':
			tok->t = R_PARE;
			return;
		case '.':
			// member or float constant
			c = fgetc(file);
			if (isdigit(c)) {
				fseek(file, -2, SEEK_CUR);
				get_num_constant(file, tok);
				return;
			} else {
				fseek(file, -1, SEEK_CUR);
				tok->t = PERIOD;
				return;
			}
		case '-':
			// -> - -- -=
			c = fgetc(file);
			switch (c) {
			case '>':
				tok->t = ARROW;
				return;
			case '-':
				tok->t = DEC;
				return;
			case '=':
				tok->t = SUB_ASSIGN;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = SUB;
				return;
			}
		case '+':
			// + ++ +=
			c = fgetc(file);
			switch (c) {
			case '+':
				tok->t = INC;
				return;
			case '=':
				tok->t = ADD_ASSIGN;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = ADD;
				return;
			}
		case '&':
			// & && &=
			c = fgetc(file);
			switch (c) {
			case '&':
				tok->t = LOGIC_AND;
				return;
			case '=':
				tok->t = AND_ASSIGN;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = AND;
				return;
			}
		case '*':
			// * *=
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = MUL_ASSIGN;
				return;
			default:
				fseek(file, -1, SEEK_CUR);

				tok->t = MUL;
				return;
			}
		case '~':
			// ~ ~=
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = BIT_COMPL_ASSIGN;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = BIT_COMPLEMENT;
				return;
			}
		case '!':
			// ! !=
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = NOT_EQ;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = NOT;
				return;
			}
		case '/':
			// / /= // /*
			c = fgetc(file);
			switch (c) {
			case '/':
				do {
					c = fgetc(file);
				} while (!feof(file) && c != '\n');
				break;
			case '*':
				do {
					c = fgetc(file);
					if (c == '*') {
						c = fgetc(file);
						if (feof(file) || c == '/') {
							break;
						}
						fseek(file, -1, SEEK_CUR);
					}
				} while (!feof(file));
				break;
			case '=':
				tok->t = DIV_ASSIGN;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = DIV;
				return;
			}
			break;
		case '%':
			// % %=
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = MOD_ASSIGN;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = MOD;
				return;
			}
		case '<':
			// < <= << <<=
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = LESS_EQU;
				return;
			case '<':
				c = fgetc(file);
				switch (c) {
				case '=':
					tok->t = L_SHIFT_ASSIGN;
					return;
				default:
					fseek(file, -1, SEEK_CUR);
					tok->t = L_SHIFT;
					return;
				}
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = LESS_THAN;
				return;
			}
		case '>':
			// > >= >> >>=
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = GREAT_EQU;
				return;
			case '>':
				c = fgetc(file);
				switch (c) {
				case '=':
					tok->t = R_SHIFT_ASSIGN;
					return;
				default:
					fseek(file, -1, SEEK_CUR);
					tok->t = R_SHIFT;
					return;
				}
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = GREATER_THAN;
				return;
			}
		case '=':
			// = ==
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = EQU;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = ASSIGN;
				return;
			}
		case '^':
			// ^ ^=
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = XOR_ASSIGN;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = XOR;
				return;
				
			}
		case '|':
			// | || |=
			c = fgetc(file);
			switch (c) {
			case '=':
				tok->t = BIT_OR_ASSIGN;
				return;
			case '|':
				tok->t = LOGIC_OR;
				return;
			default:
				fseek(file, -1, SEEK_CUR);
				tok->t = BIT_OR;
				return;
			}
		case '?':
			tok->t = CONDITION;
			return;
		case ':':
			tok->t = COLON;
			return;
		case ',':
			tok->t = COMMA;
			return;
		case '{':
			tok->t = L_CURLY;
			return;
		case '}':
			tok->t = R_CURLY;
			return;
		case ';':
			tok->t = SEMILICON;
			return;
		case '#':
			do {
				c = fgetc(file);
			} while (!feof(file) && c != '\n');
			break;
		}
	}
}

int main()
{
	FILE *f = fopen("a.c", "r");
	struct token tok;
	do {
		scanner(f, &tok);
		switch (tok.t) {
		case Integer_Constant:
			gmp_printf("Integer: %Zd\n", tok.z);
			mpz_clear(tok.z);
			break;
		case Float_Constant:
			gmp_printf("Float: %Ff\n", tok.f);
			mpf_clear(tok.f);
			break;
		case Character_Constant:
			printf("Char: %d\n", tok.c);
			break;
		case String_Literal:
			printf("String: %s\n", tok.s);
			free(tok.s);
			break;
		case CANNOT_HANDLE:
			puts("Something Went Wrong");
			break;
		case Identifier:
			printf("ID: Name: %s Hash: %02hhX:%02hhX:%02hhX"
					":%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX"
					":%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n",
					tok.id.name, tok.id.hash[0],
					tok.id.hash[1], tok.id.hash[2],
					tok.id.hash[3], tok.id.hash[4],
					tok.id.hash[5], tok.id.hash[6],
					tok.id.hash[7], tok.id.hash[8],
					tok.id.hash[9], tok.id.hash[10],
					tok.id.hash[11], tok.id.hash[12],
					tok.id.hash[13], tok.id.hash[14],
					tok.id.hash[15]);
			break;
		case End_Of_File:
			puts("File Ends");
			break;
		default:
			if (tok.t <= WHILE) {
				printf("Keyword %s ID: %d\n", tok.id.name, tok.t);
			}
			printf("Operator ID: %d\n", tok.t);
		}
	} while (tok.t != End_Of_File);
}
