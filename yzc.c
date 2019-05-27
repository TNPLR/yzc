#include <stdio.h>
#include <stdlib.h>
/*
 * yzc: YZ Compiler
 * We don't support _Thread_local, _Static_assert
 * char as unsigned char
 */
/*****************************************************************************
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 * SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER SCANNER
 *****************************************************************************/
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
  fseek(g_fptr, -1, SEEK_CUR);
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
int isNonOrDigit(int c)
{
  return isNonDigit(c) || isDigit(c);
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
  } while (isNonOrDigit(c = nextChar()));
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
/*****************************************************************************
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 * PARSER PARSER PARSER PARSER PARSER PARSER PARSER PARSER
 *****************************************************************************/

// structure and enum define
/*************************************************
 * Parser Tree Nodes
 *************************************************/
enum NODE_ID {
  BASE, DECLARETION_LIST, FUNCTION_DEFINITION, EXTERNAL_DECLARATION,
  TRANSLATION_UNIT, COMPILATION_UNIT, JUMP_STATEMENT, FOR_EXPRESSION,
  FOR_DECLARATION, FOR_CONDITION, ITERATION_STATEMENT, SELECTION_STATEMENT,
  EXPRESSION_STATEMENT, BLOCKITEM, BLOCKITEM_LIST, COMPOUND_STATEMENT,
  LABELED_STATEMENT, STATEMENT, STATIC_ASSERT_DECLARATION, DESIGNATOR,
  DESIGNATOR_LIST, DESIGNATION, INITIALIZER_LIST, INITIALIZER, TYPEDEF_NAME,
  DIRECT_ABSTRACT_DECLARATOR, ABSTRACT_DECLARATOR, TYPENAME, IDENTIFIER_LIST,
  PARAMETER_DECLARATION, PARAMETER_LIST, PARAMETER_TYPE_LIST,
  TYPE_QUALIFIER_LIST, POINTER, DIRECT_DECLARATOR, DECLARATOR,
  ALIGNMENT_SPECIFIER, FUNCTION_SPECIFIER, TYPE_QUALIFIER,
  ATOMIC_TYPE_SPECIFIER, ENUMERATION_CONSTANT, ENUMERATOR, ENUMERATOR_LIST,
  ENUM_SPECIFIER, STRUCT_DECLARATOR, STRUCT_DECLARATOR_LIST,
  SPECIFIER_QUALIFIER_LIST, STRUCT_DECLARATION, STRUCT_DECLARATION_LIST,
  STRUCT_UNION, STRUCT_UNION_SPECIFIER, TYPE_SPECIFIER,
  STORAGE_CLASS_SPECIFIER, INIT_DECLARATOR, INIT_DECLARATOR_LIST,
  DECLARATION_SPECIFIER, DECLARATION_SPECIFIER_LIST, DECLARATION,
  CONSTANT_EXPRESSION, EXPRESSION, ASSIGNMENT_OPERATOR, ASSIGNMENT_EXPRESSION,
  CONDITIONAL_EXPRESSION, LOGICAL_OR_EXPRESSION, LOGICAL_AND_EXPRESSION,
  INCLUSIVE_OR_EXPRESSION, EXCLUSIVE_OR_EXPRESSION, AND_EXPRESSION,
  EQUALIFY_EXPRESSION, RELATIONAL_EXPRESSION, SHIFT_EXPRESSION,
  ADDITIVE_EXPRESSION, MULTIPLICATIVE_EXPRESSION, CAST_EXPRESSION,
  UNARY_OPERATOR, UNARY_EXPRESSION, ARGUMENT_EXPRESSION_LIST,
  POSTFIX_EXPRESSION, GENERIC_ASSOCIATION, GENERIC_ASSOCLIST,
  GENERIC_SELECTION, PRIMARY_EXPRESSION, END_OF_FILE, NOP, KEYWORD
};
struct node {
  int id;
  struct node *parent;
  unsigned int son_count;
  unsigned int max_son_count;
  struct node *son[];
};
struct keyword_node {
  int id;
  struct node *parent;
  unsigned int son_count;
  int keyword;
};
struct string_node {
  int id;
  struct node *parent;
  unsigned int son_count;
  unsigned int size;
  char str[];
};
struct number_node {
  int id;
  struct node *parent;
  unsigned int son_count;
  unsigned int value;
};

/* GLOBAL_VARIABLE */
struct node *g_compilation_unit;
void initTopNode(void) // translation_unit init with 8 node
{
  g_compilation_unit = malloc(sizeof(struct node) + sizeof(struct node *) * 2);
  g_compilation_unit->id = COMPILATION_UNIT;
  g_compilation_unit->parent = (struct node*)0;
  g_compilation_unit->max_son_count = 2;
  g_compilation_unit->son_count = 0;
}
void addNode(struct node *restrict n, struct node *restrict parent)
{
  n->parent = parent;
  if (parent->max_son_count - parent->son_count == 0) {
    parent = realloc(parent, sizeof(struct node) + sizeof(struct node *) * (parent->max_son_count + 8));
    if (parent == (struct node *)0) {
      errorExit("CANNOT ALLOC MEMORY");
    }
    parent->max_son_count += 8;
  }
  parent->son[parent->son_count++] = n;
}
struct node *newNode(struct node *parent, int type, int son_node)
{
  struct node *nd = malloc(sizeof(struct node) + sizeof(struct node *) * son_node);
  nd->id = type;
  nd->parent = parent;
  nd->max_son_count = son_node;
  nd->son_count = 0;
  addNode(nd, parent);
  return nd;
}
struct keyword_node *newKeywordNode(struct node *parent, int keyword_token)
{
  struct keyword_node *nd = malloc(sizeof(struct keyword_node));
  nd->id = KEYWORD;
  nd->parent = parent;
  nd->son_count = 0;
  nd->keyword = keyword_token;
  addNode((struct node *)nd, parent);
  return nd;
}
void freeSonNode(struct node *ptr)
{
  while (ptr->son_count--) {
    if (ptr->son[ptr->son_count]->son_count != 0) {
      freeSonNode(ptr->son[ptr->son_count]);
    }
    free(ptr->son[ptr->son_count]);
  }
}
// TODO Go through functions

/* TOKEN QUEUE*/
// My Queue function is weaker than normal
struct token_queue {
  int last_num;
  struct token buffer[4];
};
struct token_queue g_token_queue;
void initTokenQueue(void)
{
  g_token_queue.last_num = 0;
  do {
    nextToken();
    g_token_queue.buffer[g_token_queue.last_num] = g_current_token;
  } while (++g_token_queue.last_num < 4);
  --g_token_queue.last_num;
}
struct token *getCurrentTokenQueue(void)
{
  return &g_token_queue.buffer[(g_token_queue.last_num + 1) & 3];
}
struct token *nextTokenQueue(void)
{
  nextToken();
  g_token_queue.last_num = (g_token_queue.last_num + 1) & 3;
  g_token_queue.buffer[g_token_queue.last_num] = g_current_token;
  return &g_token_queue.buffer[(g_token_queue.last_num + 1) & 3];
}
struct token *lookAheadQueue(int nu)
{
  return &g_token_queue.buffer[(g_token_queue.last_num + nu + 1) & 3];
}
/* Check specifiers */
int checkStorageClassSpecifier(int tok)
{
  return 
    tok == TYPEDEF ||
    tok == EXTERN ||
    tok == STATIC ||
    tok == AUTO ||
    tok == REGISTER;
}
int checkStructOrUnion(int tok)
{
  return
    tok == STRUCT ||
    tok == UNION;
}
int checkTypeSpecifier(int tok)
{
  return
    tok == VOID ||
    tok == CHAR ||
    tok == SHORT ||
    tok == INT ||
    tok == LONG ||
    tok == FLOAT ||
    tok == DOUBLE ||
    tok == SIGNED ||
    tok == UNSIGNED ||
    checkTypeSpecifier(tok) ||
    tok == ENUM ||
    tok == IDENT;
}
int checkTypeQualifier(int tok)
{
  return
    tok == CONST ||
    tok == RESTRICT ||
    tok == VOLATILE;
}
int checkDeclarationSpecifier(int tok)
{
  return
    checkStorageClassSpecifier(tok) ||
    checkStructOrUnion(tok) ||
    checkTypeSpecifier(tok) ||
    checkTypeQualifier(tok) ||
    tok == INLINE;
}

/* PARSER */
void parseStorageClassSpecifier(struct node *storage_class_specifier)
{
  newKeywordNode(storage_class_specifier, getCurrentTokenQueue()->token_type);
}
void parseTypeSpecifier(struct node *type_specifier)
{
  newKeywordNode(type_specifier, getCurrentTokenQueue()->token_type);
}
void parseTypeQualifier(struct node *type_qualifier)
{
  newKeywordNode(type_qualifier, getCurrentTokenQueue()->token_type);
}
void parseFunctionSpecifier(struct node *function_specifier)
{
  newKeywordNode(function_specifier, getCurrentTokenQueue()->token_type);
}

void parseDeclarationSpecifier(struct node *declaration_specifier)
{
  int type = getCurrentTokenQueue()->token_type;
  if (checkStorageClassSpecifier(type)) {
    parseStorageClassSpecifier(newNode(declaration_specifier, STORAGE_CLASS_SPECIFIER, 1));
  } else if (checkTypeSpecifier(type)) {
    parseTypeSpecifier(newNode(declaration_specifier, TYPE_SPECIFIER, 1));
  } else if (checkTypeQualifier(type)) {
    parseTypeQualifier(newNode(declaration_specifier, TYPE_QUALIFIER, 1));
  } else if (type == INLINE) {
    parseStorageClassSpecifier(newNode(declaration_specifier, STORAGE_CLASS_SPECIFIER, 1));
  } else {
    errorExit("Unknown Declaration Specifier");
  }
}

// initDeclaratorList
// : initDeclarator
// | initDeclaratorList ',' initDeclarator
// ;
// initDeclarator
// : declarator
// | declarator '=' initializer
// ;
void parseInitDeclaratorList(struct node *init_declarator_list)
{
  while (1) {
    if (1) {
      // TODO
    }
  }
}

void parseDeclaration(struct node *declaration)
{
  while (1) {
    if (checkDeclarationSpecifier(getCurrentTokenQueue()->token_type)) {
      parseDeclarationSpecifier(newNode(declaration, DECLARATION_SPECIFIER, 1));
    } else if (getCurrentTokenQueue()->token_type == SEMILICON) {
      newNode(declaration, NOP, 0);
      return;
    } else { // initDeclaratorList
      parseInitDeclaratorList(newNode(declaration, INIT_DECLARATOR_LIST, 1));
    }
    nextTokenQueue();
  }
}

void parseFunctionDefinition(struct node *function_definition)
{
}


void parseExternalDeclaration(struct node *external_declaration)
{
  // Single semilicon ';'
  if (getCurrentTokenQueue()->token_type == SEMILICON) {
    newNode(external_declaration, NOP, 0);
    return;
  }

  // declaration
  // : declarationSpecifiers initDeclaratorList ';'
  // | declarationSpecifiers ';'
  // | staticAssertDeclaration
  //   : '_Static_assert' ...
  //   ;
  // ;
  //
  // declarationSpecifiers
  // : declarationSpecifier
  //   : storageClassSpecifier
  //     : typedef extern static _Thread_local auto register
  //   | typeSpecifier
  //     : void char short int long float double signed unsigned
  //     | atomicTypeSpecifier
  //     | structOrUnionSpecifier
  //       : structOrUnion Identifier? '{' structDeclarationList '}'
  //       | structOrUnion Identifier
  //         structOrUnion
  //         : struct union
  //     | enumSpecifier
  //       : enum XXX
  //     | typedefName
  //       : Identifier
  //     | typeSpecifier pointer
  //   | typeQualifier
  //     : const restrict volatile _Atomic
  //   | functionSpecifier
  //     : inline _Noreturn
  //   | alignmentSpecifier
  //     : _Alignas ...
  //   ;
  // ;
  if (checkDeclarationSpecifier(getCurrentTokenQueue()->token_type)) {
    parseDeclaration(newNode(external_declaration, DECLARATION, 3));
  } else {
    parseFunctionDefinition(newNode(external_declaration, FUNCTION_DEFINITION, 2));
  }

}

void parseTranslationUnit(struct node *translation_unit)
{
  while (1) {
    if (getCurrentTokenQueue()->token_type == EOFILE) {
      return;
    } else {
      parseExternalDeclaration(newNode(translation_unit, EXTERNAL_DECLARATION, 1));
    }
  }
}

/* PARSER MAIN BLOCK */
void parseCompilationUnit(void)
{
  while (1) {
    if (getCurrentTokenQueue()->token_type == EOFILE) {
      // EOF
      newNode(g_compilation_unit, END_OF_FILE, 0);
      return;
    } else {
      // TranslationUnit
      parseTranslationUnit(newNode(g_compilation_unit, TRANSLATION_UNIT, 1));
    }
  }
}


/* Main to test scanner*/

int main(int argc, char *argv[])
{
  openFile(argv[1]);
  do {
    nextToken();
    printf("%d\n", g_current_token.token_type);
  } while (g_current_token.token_type != EOFILE);
  closeFile();
  return 0;
}
