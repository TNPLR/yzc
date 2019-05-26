#include <stdio.h>
#include <stdlib.h>
/*
 * yzc: YZ Compiler
 *
 * char as unsigned char
 */
/*****************************************************************************
 * Function List
 *****************************************************************************/
/*************************************************************
 * Common Functions
 *************************************************************/
/******************************************
 * File Handler
 ******************************************/
void openFile(const char *s);
void closeFile(void);
int nextChar(void);
void backChar(void);
/******************************************
 * Check Character Type
 ******************************************/
int isHexDigit(int c);
int isOctDigit(int c);
int isDigit(int c);
int isNonDigit(int c);
/******************************************
 * Compare String
 ******************************************/
int streq(const char *s1, const char *s2);
/******************************************
 * Error Handler
 ******************************************/
void errorExit(const char *s);
/*************************************************************
 * Scanner Functions
 *************************************************************/
/******************************************
 * Main Token Scanner
 ******************************************/
void nextToken(void);
/******************************************
 * Number Literal Token
 ******************************************/
void hexNumToken(void);
void octNumToken(int c);
void decNumToken(int c);
void numToken(int c); // main function
/******************************************
 * Keyword and Identifier Token
 ******************************************/
void checkKeyword(void);
void nonDigitToken(int c); // main function
/******************************************
 * Comment Handler
 ******************************************/
void jmpLine(void);
void jmpBlockComment(void);
/******************************************
 * Char Literal Handler
 ******************************************/
int getCharLiteral(void);
/*************************************************************
 * parser Functions
 *************************************************************/

/*****************************************************************************
 * Enum
 *****************************************************************************/
enum ALL_TOKEN {
  COLON, SEMILICON, LPAREN, RPAREN, LSQR_BRAC, RSQR_BRAC, LCUR_BRAC, RCUR_BRAC,
  INT_NUM, UINT_NUM, STRING_LIT, AUTO, BREAK, CASE, CHAR, CONST, CONTINUE,
  DEFAULT, DO, DOUBLE, ELSE, ENUM, EXTERN, FLOAT, FOR, GOTO, IF, INLINE,
  INT, LONG, REGISTER, RESTRICT, RET, SHORT, SIGNED,
  SIZEOF, STATIC, STRUCT, SWITCH, TYPEDEF, UNION, UNSIGNED, VOID,
  VOLATILE, WHILE, IDENT, MEMBER, PTR_MEMBER,
  INCRESE, DECRESE, BIT_COMPLEMENT, LOG_NEG,
  MUL, DIV, MOD, PLUS, MINUS, SHIFT_LEFT, SHIFT_RIGHT,
  GT, LT, GE, LE, EQ, NEQ, BIT_AND, BIT_EOR, BIT_OR,
  LOG_AND, LOG_OR, COND, ASSIGN, PLUS_ASN, MINUS_ASN,
  MUL_ASN, DIV_ASN, MOD_ASN, SHL_ASN, SHR_ASN, BITAND_ASN,
  BITEOR_ASN, BITOR_ASN, EOFILE, COMMA
};
/*****************************************************************************
 * Structure
 *****************************************************************************/
struct token {
  int token_type;
  unsigned int value;
  int number;
  char idname[64];
  char string[4096];
};
/*****************************************************************************
 * Global Variable List
 *****************************************************************************/
FILE *g_fptr;
unsigned int g_line = 1;// store line number
struct token g_current_token;// store current token


// file handler
void openFile(const char *s)
{
  g_fptr = fopen(s, "r");
}
void closeFile(void)
{
  fclose(g_fptr);
}
int nextChar(void)
{
  return fgetc(g_fptr);
}
void backChar(void)
{
  fseek(g_fptr, -1, SEEK_SET);
}

// check char type
int isHexDigit(int c)
{
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}
int isOctDigit(int c)
{
  return (c >= '0' && c <= '7');
}
int isDigit(int c)
{
  return (c >= '0' && c <= '9');
}
int isNonDigit(int c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// compare two string
int streq(const char *s1, const char *s2)
{
  while (*s1 != '\0' && *s2 != '\0') {
    if (*s1++ != *s2++) {
      return 0;
    }
  }
  return 1;
}


// Error message maker
void errorExit(const char *s)
{
  printf("yzc error: Line %u\n%s\n", g_line, s);
  exit(EXIT_FAILURE);
}

// num literal
void hexNumToken(void)
{
  unsigned int ui = 0;
  int c = 0;
  while (isHexDigit(c = nextChar())) {
    ui <<= 4;
    if (c >= '0' && c <= '9') {
      ui |= (c - '0') & 0xF;
    } else if (c >= 'A' && c <= 'F') {
      ui |= (c - 'A' + 10) & 0xF;
    } else {
      ui |= (c - 'a' + 10) & 0xF;
    }
  }
  if (c == 'U' || c == 'u') {
    g_current_token.token_type = UINT_NUM;
  } else {
    g_current_token.token_type = INT_NUM;
    backChar();
  }
  g_current_token.value = ui;
}
void octNumToken(int c)
{
  unsigned int ui = 0;
  while (isOctDigit(c = nextChar())) {
    ui <<= 3;
    ui |= (c - '0') & 07;
  }
  if (c == 'U' || c == 'u') {
    g_current_token.token_type = UINT_NUM;
  } else {
    g_current_token.token_type = INT_NUM;
    backChar();
  }
  g_current_token.value = ui;
}
void decNumToken(int c)
{
  unsigned int ui = 0;
  while (isDigit(c = nextChar())) {
    ui *= 10;
    ui += (c - '0');
  }
  if (c == 'U' || c == 'u') {
    g_current_token.token_type = UINT_NUM;
  } else {
    g_current_token.token_type = INT_NUM;
    backChar();
  }
  g_current_token.value = ui;
}

void numToken(int c)
{
  if (c == '0') {
    c = nextChar();
    if (c == 'x' || c == 'X') { // Hex
      hexNumToken();
    } else { // Oct
      backChar();
      octNumToken(c);
    }
  } else { // Dec
    decNumToken(c);
  }
}

void checkKeyword(void)
{
  if (streq("auto", g_current_token.idname)) {
    g_current_token.token_type = AUTO;
  } else if (streq("break", g_current_token.idname)) {
    g_current_token.token_type = BREAK;
  } else if (streq("case", g_current_token.idname)) {
    g_current_token.token_type = CASE;
  } else if (streq("char", g_current_token.idname)) {
    g_current_token.token_type = CHAR;
  } else if (streq("const", g_current_token.idname)) {
    g_current_token.token_type = CONST;
  } else if (streq("continue", g_current_token.idname)) {
    g_current_token.token_type = CONTINUE;
  } else if (streq("default", g_current_token.idname)) {
    g_current_token.token_type = DEFAULT;
  } else if (streq("do", g_current_token.idname)) {
    g_current_token.token_type = DO;
  } else if (streq("double", g_current_token.idname)) {
    g_current_token.token_type = DOUBLE;
  } else if (streq("else", g_current_token.idname)) {
    g_current_token.token_type = ELSE;
  } else if (streq("enum", g_current_token.idname)) {
    g_current_token.token_type = ENUM;
  } else if (streq("extern", g_current_token.idname)) {
    g_current_token.token_type = EXTERN;
  } else if (streq("float", g_current_token.idname)) {
    g_current_token.token_type = FLOAT;
  } else if (streq("for", g_current_token.idname)) {
    g_current_token.token_type = FOR;
  } else if (streq("goto", g_current_token.idname)) {
    g_current_token.token_type = GOTO;
  } else if (streq("if", g_current_token.idname)) {
    g_current_token.token_type = IF;
  } else if (streq("inline", g_current_token.idname)) {
    g_current_token.token_type = INLINE;
  } else if (streq("int", g_current_token.idname)) {
    g_current_token.token_type = INT;
  } else if (streq("long", g_current_token.idname)) {
    g_current_token.token_type = LONG;
  } else if (streq("register", g_current_token.idname)) {
    g_current_token.token_type = REGISTER;
  } else if (streq("restrict", g_current_token.idname)) {
    g_current_token.token_type = RESTRICT;
  } else if (streq("return", g_current_token.idname)) {
    g_current_token.token_type = RET;
  } else if (streq("short", g_current_token.idname)) {
    g_current_token.token_type = SHORT;
  } else if (streq("signed", g_current_token.idname)) {
    g_current_token.token_type = SIGNED;
  } else if (streq("sizeof", g_current_token.idname)) {
    g_current_token.token_type = SIZEOF;
  } else if (streq("static", g_current_token.idname)) {
    g_current_token.token_type = STATIC;
  } else if (streq("struct", g_current_token.idname)) {
    g_current_token.token_type = STRUCT;
  } else if (streq("switch", g_current_token.idname)) {
    g_current_token.token_type = SWITCH;
  } else if (streq("typedef", g_current_token.idname)) {
    g_current_token.token_type = TYPEDEF;
  } else if (streq("union", g_current_token.idname)) {
    g_current_token.token_type = UNION;
  } else if (streq("unsigned", g_current_token.idname)) {
    g_current_token.token_type = UNSIGNED;
  } else if (streq("void", g_current_token.idname)) {
    g_current_token.token_type = VOID;
  } else if (streq("volatile", g_current_token.idname)) {
    g_current_token.token_type = VOLATILE;
  } else if (streq("while", g_current_token.idname)) {
    g_current_token.token_type = WHILE;
  } else {
    g_current_token.token_type = IDENT;
  }
}

void nonDigitToken(int c)
{
  char *ptr = g_current_token.idname;
  do {
    *ptr++ = c;
  } while (isNonDigit(c = nextChar()));
  backChar();
  checkKeyword();
}

void jmpLine(void)
{
  int c = 0;
  while ((c = nextChar()) != '\n' && (c != -1)) {
    // DO NOTHING
  }
  ++g_line;
}

void jmpBlockComment(void)
{
  int c;
  while (1) {
    while ((c = nextChar()) != '*') {
      if (c == '\n') {
        ++g_line;
      }
    }
    if ((c = nextChar()) == '/') {
      return;
    }
    if (c == -1) {
      errorExit("Block Comment without end");
      // WON'T BE HERE
    }
  }
}

int getCharLiteral(void)
{
  int c = nextChar();
  if (c == '\\') {
    c = nextChar();
    if (isOctDigit(c)) {
      unsigned int ui = 0;
      while (isOctDigit(c = nextChar())) {
        ui <<= 3;
        ui |= (c - '0') & 07;
      }
      c = (int)ui;
    } else if (c == 'x') {
      unsigned int ui = 0;
      while (isHexDigit(c = nextChar())) {
        ui <<= 4;
        if (c >= '0' && c <= '9') {
          ui |= (c - '0') & 0xF;
        } else if (c >= 'A' && c <= 'F') {
          ui |= (c - 'A' + 10) & 0xF;
        } else {
          ui |= (c - 'a' + 10) & 0xF;
        }
      }
      c = (int)ui;
    }
    switch (c) {
      case '\'':
        c = '\'';
        break;
      case '\"':
        c = '\"';
        break;
      case '\?':
        c = '\?';
        break;
      case '\\':
        c = '\\';
        break;
      case 'a':
        c = '\a';
        break;
      case 'b':
        c = '\b';
        break;
      case 'f':
        c = '\f';
        break;
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      case 't':
        c = '\t';
        break;
      case 'v':
        c = '\v';
        break;
      default:
        break;
    }
  }
  return c;
}

void nextToken(void)
{
  while (1) {
    int c = nextChar();
    if (c >= '0' && c <= '9') {
      numToken(c);
      return;
    }
    if (isNonDigit(c)) {
      nonDigitToken(c);
      return;
    }
    if (c == '#') {
      // If we have a preprocessor, we don't need this thing
      // TODO: Preprocessor
      jmpLine();
      continue;
    }
    if (c == '/') {
      c = nextChar();
      if (c == '/') {
        jmpLine();
      } else if (c == '*') {
        jmpBlockComment();
      } else if (c == '=') {
        g_current_token.token_type = DIV_ASN;
        return;
      } else {
        backChar();
        g_current_token.token_type = DIV;
        return;
      }
      continue;
    }
    if (c == '\'') {
      g_current_token.value = (unsigned int)getCharLiteral();
      g_current_token.token_type = UINT_NUM;
      c = nextChar();
      if (c != '\'') {
        errorExit("Character Literal DOES NOT END");
      }
      return;
    }
    if (c == '\"') {
      unsigned string_count = 0;
      while ((c = getCharLiteral()) != '\"' && string_count <= 4095) {
        g_current_token.string[string_count++] = (char)c;
      }
      if (c != '\"') {
        errorExit("String Literal Too Long");
      }
      return;
    }
    //
    // TODO float literal
    //
    switch (c) {
      case -1:
        g_current_token.token_type = EOFILE;
        return;
      case ':':
        g_current_token.token_type = COLON;
        return;
      case ';':
        g_current_token.token_type = SEMILICON;
        return;
      case '(':
        g_current_token.token_type = LPAREN;
        return;
      case ')':
        g_current_token.token_type = RPAREN;
        return;
      case '[':
        g_current_token.token_type = LSQR_BRAC;
        return;
      case ']':
        g_current_token.token_type = RSQR_BRAC;
        return;
      case '{':
        g_current_token.token_type = LCUR_BRAC;
        return;
      case '}':
        g_current_token.token_type = RCUR_BRAC;
        return;
      case ',':
        g_current_token.token_type = COMMA;
        return;
      case '\n':
        ++g_line;
        break;
      case ' ':
        break;
      case '.':
        g_current_token.token_type = MEMBER;
        return;
      case '-':
        c = nextChar();
        if (c == '>') {
          g_current_token.token_type = MEMBER;
        } else if (c == '-') {
          g_current_token.token_type = DECRESE;
        } else if (c == '=') {
          g_current_token.token_type = MINUS_ASN;
        } else {
          backChar();
          g_current_token.token_type = MINUS;
        }
        return;
      case '+':
        c = nextChar();
        if (c == '+') {
          g_current_token.token_type = INCRESE;
        } else if (c == '=') {
          g_current_token.token_type = PLUS_ASN;
        } else {
          backChar();
          g_current_token.token_type = PLUS;
        }
        return;
      case '~':
        g_current_token.token_type = BIT_COMPLEMENT;
        return;
      case '!':
        if ((c = nextChar()) == '=') {
          g_current_token.token_type = NEQ;
        } else {
          backChar();
          g_current_token.token_type = LOG_NEG;
        }
        return;
      case '*':
        if ((c = nextChar()) == '=') {
          g_current_token.token_type = MUL_ASN;
        } else {
          backChar();
          g_current_token.token_type = MUL;
        }
        return;
      case '%':
        if ((c = nextChar()) == '=') {
          g_current_token.token_type = MOD_ASN;
        } else {
          backChar();
          g_current_token.token_type = MOD;
        }
        return;
      case '<':
        c = nextChar();
        if (c == '<') {
          c = nextChar();
          if (c == '=') {
            g_current_token.token_type = SHL_ASN;
          } else {
            backChar();
            g_current_token.token_type = SHIFT_LEFT;
          }
        } else if (c == '=') {
          g_current_token.token_type = LE;
        } else {
          backChar();
          g_current_token.token_type = LT;
        }
        return;
      case '>':
        c = nextChar();
        if (c == '>') {
          c = nextChar();
          if (c == '=') {
            g_current_token.token_type = SHR_ASN;
          } else {
            backChar();
            g_current_token.token_type = SHIFT_RIGHT;
          }
        } else if (c == '=') {
          g_current_token.token_type = GE;
        } else {
          backChar();
          g_current_token.token_type = GT;
        }
        return;
      case '=':
        c = nextChar();
        if (c == '=') {
          g_current_token.token_type = EQ;
        } else {
          backChar();
          g_current_token.token_type = ASSIGN;
        }
        return;
      case '&':
        c = nextChar();
        if (c == '&') {
          g_current_token.token_type = LOG_AND;
        } else if (c == '=') {
          g_current_token.token_type = BITAND_ASN;
        } else {
          backChar();
          g_current_token.token_type = BIT_AND;
        }
        return;
      case '^':
        c = nextChar();
        if (c == '=') {
          g_current_token.token_type = BITEOR_ASN;
        } else {
          backChar();
          g_current_token.token_type = BIT_EOR;
        }
        return;
      case '|':
        c = nextChar();
        if (c == '|') {
          g_current_token.token_type = LOG_OR;
        } else if (c == '=') {
          g_current_token.token_type = BITOR_ASN;
        } else {
          backChar();
          g_current_token.token_type = BIT_OR;
        }
        return;
      case '?':
        g_current_token.token_type = COND;
        return;
      default:
        errorExit("Unknown Character");
        break;
    }
  }
}
