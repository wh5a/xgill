/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_NAME = 258,
     TOK_INTLIT = 259,
     TOK_EMBEDDED_CODE = 260,
     TOK_LBRACE = 261,
     TOK_RBRACE = 262,
     TOK_SEMICOLON = 263,
     TOK_ARROW = 264,
     TOK_LPAREN = 265,
     TOK_RPAREN = 266,
     TOK_LANGLE = 267,
     TOK_RANGLE = 268,
     TOK_STAR = 269,
     TOK_AMPERSAND = 270,
     TOK_COMMA = 271,
     TOK_EQUALS = 272,
     TOK_COLON = 273,
     TOK_CLASS = 274,
     TOK_PUBLIC = 275,
     TOK_PRIVATE = 276,
     TOK_PROTECTED = 277,
     TOK_VERBATIM = 278,
     TOK_IMPL_VERBATIM = 279,
     TOK_CTOR = 280,
     TOK_DTOR = 281,
     TOK_PURE_VIRTUAL = 282,
     TOK_CUSTOM = 283,
     TOK_OPTION = 284,
     TOK_NEW = 285,
     TOK_ENUM = 286
   };
#endif
/* Tokens.  */
#define TOK_NAME 258
#define TOK_INTLIT 259
#define TOK_EMBEDDED_CODE 260
#define TOK_LBRACE 261
#define TOK_RBRACE 262
#define TOK_SEMICOLON 263
#define TOK_ARROW 264
#define TOK_LPAREN 265
#define TOK_RPAREN 266
#define TOK_LANGLE 267
#define TOK_RANGLE 268
#define TOK_STAR 269
#define TOK_AMPERSAND 270
#define TOK_COMMA 271
#define TOK_EQUALS 272
#define TOK_COLON 273
#define TOK_CLASS 274
#define TOK_PUBLIC 275
#define TOK_PRIVATE 276
#define TOK_PROTECTED 277
#define TOK_VERBATIM 278
#define TOK_IMPL_VERBATIM 279
#define TOK_CTOR 280
#define TOK_DTOR 281
#define TOK_PURE_VIRTUAL 282
#define TOK_CUSTOM 283
#define TOK_OPTION 284
#define TOK_NEW 285
#define TOK_ENUM 286




/* Copy the first part of user declarations.  */
#line 6 "agrampar.y"


#include "agrampar.h"       // agrampar_yylex, etc.

#include <stdlib.h>         // malloc, free

// enable debugging the parser
#ifndef NDEBUG
  #define YYDEBUG 1
#endif

// permit having other parser's codes in the same program
#define yyparse agrampar_yyparse



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 67 "agrampar.y"
{
  ASTSpecFile *file;
  ASTList<ToplevelForm> *formList;
  TF_class *tfClass;
  ASTList<CtorArg> *ctorArgList;
  ASTList<Annotation> *userDeclList;
  string *str;
  enum AccessCtl accessCtl;
  AccessMod *accessMod;
  ToplevelForm *verbatim;
  Annotation *annotation;
  TF_option *tfOption;
  ASTList<string> *stringList;
  TF_enum *tfEnum;
  ASTList<string> *enumeratorList;
  string *enumerator;
  ASTList<BaseClass> *baseClassList;
  BaseClass *baseClass;
  CustomCode *customCode;
}
/* Line 187 of yacc.c.  */
#line 195 "agrampar.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 208 "agrampar.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   111

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  72
/* YYNRULES -- Number of states.  */
#define YYNSTATES  114

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   286

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    12,    15,    18,    21,
      24,    31,    39,    40,    42,    46,    48,    49,    56,    65,
      68,    69,    71,    74,    78,    80,    84,    86,    89,    91,
      93,    97,    99,   101,   103,   105,   107,   111,   112,   115,
     118,   124,   126,   130,   133,   137,   139,   141,   143,   145,
     147,   149,   151,   156,   158,   162,   165,   168,   173,   174,
     177,   183,   190,   192,   196,   198,   199,   202,   204,   208,
     210,   212,   214
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      33,     0,    -1,    34,    -1,    -1,    34,    35,    -1,    34,
      52,    -1,    34,    53,    -1,    34,    55,    -1,    34,    47,
      -1,    34,     8,    -1,    36,    19,     3,    39,    58,    37,
      -1,    36,    19,     3,    40,    40,    58,    37,    -1,    -1,
      30,    -1,     6,    38,     7,    -1,     8,    -1,    -1,    38,
       9,     3,    39,    58,     8,    -1,    38,     9,     3,    39,
      58,     6,    45,     7,    -1,    38,    46,    -1,    -1,    40,
      -1,    10,    11,    -1,    10,    41,    11,    -1,    42,    -1,
      41,    16,    42,    -1,    43,    -1,    42,    43,    -1,     3,
      -1,     4,    -1,    12,    44,    13,    -1,    14,    -1,    15,
      -1,    17,    -1,    19,    -1,    42,    -1,    42,    16,    44,
      -1,    -1,    45,    46,    -1,    50,    48,    -1,    50,     5,
      17,     5,     8,    -1,    47,    -1,    28,     3,    48,    -1,
       5,     8,    -1,     6,     5,     7,    -1,    20,    -1,    21,
      -1,    22,    -1,    25,    -1,    26,    -1,    27,    -1,    49,
      -1,    49,    10,    51,    11,    -1,     3,    -1,    51,    16,
       3,    -1,    23,    48,    -1,    24,    48,    -1,    29,     3,
      54,     8,    -1,    -1,    54,     3,    -1,    31,     3,     6,
      56,     7,    -1,    31,     3,     6,    56,    16,     7,    -1,
      57,    -1,    56,    16,    57,    -1,     3,    -1,    -1,    18,
      59,    -1,    61,    -1,    59,    16,    61,    -1,    20,    -1,
      21,    -1,    22,    -1,    60,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   114,   114,   120,   121,   122,   123,   124,   125,   126,
     131,   135,   143,   144,   156,   158,   166,   167,   169,   171,
     179,   180,   186,   188,   193,   200,   205,   207,   213,   214,
     215,   216,   217,   218,   219,   223,   225,   232,   233,   239,
     241,   243,   249,   255,   257,   263,   264,   265,   266,   267,
     268,   272,   274,   279,   281,   286,   288,   293,   299,   300,
     305,   307,   312,   314,   319,   325,   326,   331,   333,   339,
     340,   341,   345
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_NAME", "TOK_INTLIT",
  "TOK_EMBEDDED_CODE", "\"{\"", "\"}\"", "\";\"", "\"->\"", "\"(\"",
  "\")\"", "\"<\"", "\">\"", "\"*\"", "\"&\"", "\",\"", "\"=\"", "\":\"",
  "\"class\"", "\"public\"", "\"private\"", "\"protected\"",
  "\"verbatim\"", "\"impl_verbatim\"", "\"ctor\"", "\"dtor\"",
  "\"pure_virtual\"", "\"custom\"", "\"option\"", "\"new\"", "\"enum\"",
  "$accept", "StartSymbol", "Input", "Class", "NewOpt", "ClassBody",
  "ClassMembersOpt", "CtorArgsOpt", "CtorArgs", "CtorArgList", "Arg",
  "ArgWord", "ArgList", "CtorMembersOpt", "Annotation", "CustomCode",
  "Embedded", "Public", "AccessMod", "StringList", "Verbatim", "Option",
  "OptionArgs", "Enum", "EnumeratorSeq", "Enumerator", "BaseClassesOpt",
  "BaseClassSeq", "BaseAccess", "BaseClass", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    32,    33,    34,    34,    34,    34,    34,    34,    34,
      35,    35,    36,    36,    37,    37,    38,    38,    38,    38,
      39,    39,    40,    40,    41,    41,    42,    42,    43,    43,
      43,    43,    43,    43,    43,    44,    44,    45,    45,    46,
      46,    46,    47,    48,    48,    49,    49,    49,    49,    49,
      49,    50,    50,    51,    51,    52,    52,    53,    54,    54,
      55,    55,    56,    56,    57,    58,    58,    59,    59,    60,
      60,    60,    61
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     2,     2,     2,     2,
       6,     7,     0,     1,     3,     1,     0,     6,     8,     2,
       0,     1,     2,     3,     1,     3,     1,     2,     1,     1,
       3,     1,     1,     1,     1,     1,     3,     0,     2,     2,
       5,     1,     3,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     3,     2,     2,     4,     0,     2,
       5,     6,     1,     3,     1,     0,     2,     1,     3,     1,
       1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,     9,     0,     0,     0,     0,    13,
       0,     4,     0,     8,     5,     6,     7,     0,     0,    55,
      56,     0,    58,     0,     0,    43,     0,    42,     0,     0,
      20,    44,    59,    57,    64,     0,    62,     0,    65,    21,
      60,     0,    28,    29,    22,     0,    31,    32,    33,    34,
       0,    24,    26,     0,     0,    65,    61,    63,    35,     0,
      23,     0,    27,    69,    70,    71,    66,     0,    67,    16,
      15,    10,     0,     0,    30,    25,     0,    72,     0,    11,
      36,    68,    14,     0,    45,    46,    47,    48,    49,    50,
      19,    41,    51,     0,    20,     0,     0,    39,    65,    21,
      53,     0,     0,     0,    52,     0,     0,    37,    17,    54,
      40,     0,    18,    38
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,    11,    12,    71,    78,    38,    39,    50,
      58,    52,    59,   111,    90,    91,    19,    92,    93,   101,
      14,    15,    28,    16,    35,    36,    54,    66,    67,    68
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -54
static const yytype_int8 yypact[] =
{
     -54,    12,     1,   -54,   -54,    13,    13,    23,    40,   -54,
      41,   -54,    66,   -54,   -54,   -54,   -54,    81,    85,   -54,
     -54,    13,   -54,    65,    88,   -54,    86,   -54,     3,    89,
      84,   -54,   -54,   -54,   -54,    -2,   -54,    46,    77,    84,
     -54,    52,   -54,   -54,   -54,    69,   -54,   -54,   -54,   -54,
      -3,    69,   -54,    48,    56,    77,   -54,   -54,    63,    83,
     -54,    69,   -54,   -54,   -54,   -54,    82,    94,   -54,   -54,
     -54,   -54,    56,    69,   -54,    69,    48,   -54,    14,   -54,
     -54,   -54,   -54,    96,   -54,   -54,   -54,   -54,   -54,   -54,
     -54,   -54,    90,    32,    84,    98,    -1,   -54,    77,   -54,
     -54,     6,    97,    68,   -54,   100,    99,   -54,   -54,   -54,
     -54,    26,   -54,   -54
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -54,   -54,   -54,   -54,   -54,    33,   -54,    10,   -38,   -54,
     -33,   -48,    35,   -54,    -5,   107,    -6,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   -54,    70,   -53,   -54,   -54,    34
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -13
static const yytype_int8 yytable[] =
{
      20,    55,    72,    62,    51,    40,    32,    25,    60,     4,
      62,    33,     3,    61,    41,    27,   102,   104,    17,    18,
     -12,    82,   105,    83,     5,     6,    21,    62,    75,     7,
       8,     9,    10,   112,    84,    85,    86,    96,    18,    87,
      88,    89,     7,    22,    23,   103,    84,    85,    86,    42,
      43,    87,    88,    89,     7,    34,    99,    44,    45,    56,
      46,    47,    69,    48,    70,    49,    42,    43,    63,    64,
      65,    29,    42,    43,   107,    45,   108,    46,    47,    73,
      48,    45,    49,    46,    47,    24,    48,    97,    49,    25,
      26,    30,    34,    31,    37,    53,    74,    77,    76,    94,
      95,   100,   106,   109,    98,    79,   113,   110,    80,    13,
      81,    57
};

static const yytype_uint8 yycheck[] =
{
       6,    39,    55,    51,    37,     7,     3,     8,    11,     8,
      58,     8,     0,    16,    16,    21,    17,    11,     5,     6,
      19,     7,    16,     9,    23,    24,     3,    75,    61,    28,
      29,    30,    31,     7,    20,    21,    22,     5,     6,    25,
      26,    27,    28,     3,     3,    98,    20,    21,    22,     3,
       4,    25,    26,    27,    28,     3,    94,    11,    12,     7,
      14,    15,     6,    17,     8,    19,     3,     4,    20,    21,
      22,     6,     3,     4,     6,    12,     8,    14,    15,    16,
      17,    12,    19,    14,    15,    19,    17,    93,    19,     8,
       5,     3,     3,     7,    10,    18,    13,     3,    16,     3,
      10,     3,     5,     3,    94,    72,   111,     8,    73,     2,
      76,    41
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    33,    34,     0,     8,    23,    24,    28,    29,    30,
      31,    35,    36,    47,    52,    53,    55,     5,     6,    48,
      48,     3,     3,     3,    19,     8,     5,    48,    54,     6,
       3,     7,     3,     8,     3,    56,    57,    10,    39,    40,
       7,    16,     3,     4,    11,    12,    14,    15,    17,    19,
      41,    42,    43,    18,    58,    40,     7,    57,    42,    44,
      11,    16,    43,    20,    21,    22,    59,    60,    61,     6,
       8,    37,    58,    16,    13,    42,    16,     3,    38,    37,
      44,    61,     7,     9,    20,    21,    22,    25,    26,    27,
      46,    47,    49,    50,     3,    10,     5,    48,    39,    40,
       3,    51,    17,    58,    11,    16,     5,     6,     8,     3,
       8,    45,     7,    46
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 115 "agrampar.y"
    { (yyval.file) = *((ASTSpecFile**)parseParam) = new ASTSpecFile((yyvsp[(1) - (1)].formList)); ;}
    break;

  case 3:
#line 120 "agrampar.y"
    { (yyval.formList) = new ASTList<ToplevelForm>; ;}
    break;

  case 4:
#line 121 "agrampar.y"
    { ((yyval.formList)=(yyvsp[(1) - (2)].formList))->append((yyvsp[(2) - (2)].tfClass)); ;}
    break;

  case 5:
#line 122 "agrampar.y"
    { ((yyval.formList)=(yyvsp[(1) - (2)].formList))->append((yyvsp[(2) - (2)].verbatim)); ;}
    break;

  case 6:
#line 123 "agrampar.y"
    { ((yyval.formList)=(yyvsp[(1) - (2)].formList))->append((yyvsp[(2) - (2)].tfOption)); ;}
    break;

  case 7:
#line 124 "agrampar.y"
    { ((yyval.formList)=(yyvsp[(1) - (2)].formList))->append((yyvsp[(2) - (2)].tfEnum)); ;}
    break;

  case 8:
#line 125 "agrampar.y"
    { ((yyval.formList)=(yyvsp[(1) - (2)].formList))->append(new TF_custom((yyvsp[(2) - (2)].customCode))); ;}
    break;

  case 9:
#line 126 "agrampar.y"
    { (yyval.formList)=(yyvsp[(1) - (2)].formList); ;}
    break;

  case 10:
#line 132 "agrampar.y"
    { ((yyval.tfClass)=(yyvsp[(6) - (6)].tfClass))->super->name = unbox((yyvsp[(3) - (6)].str)); 
           (yyval.tfClass)->super->args.steal((yyvsp[(4) - (6)].ctorArgList)); 
           (yyval.tfClass)->super->bases.steal((yyvsp[(5) - (6)].baseClassList)); ;}
    break;

  case 11:
#line 136 "agrampar.y"
    { ((yyval.tfClass)=(yyvsp[(7) - (7)].tfClass))->super->name = unbox((yyvsp[(3) - (7)].str));
           (yyval.tfClass)->super->args.steal((yyvsp[(4) - (7)].ctorArgList));
           (yyval.tfClass)->super->lastArgs.steal((yyvsp[(5) - (7)].ctorArgList));
           (yyval.tfClass)->super->bases.steal((yyvsp[(6) - (7)].baseClassList)); ;}
    break;

  case 12:
#line 143 "agrampar.y"
    {;}
    break;

  case 13:
#line 144 "agrampar.y"
    {;}
    break;

  case 14:
#line 157 "agrampar.y"
    { (yyval.tfClass)=(yyvsp[(2) - (3)].tfClass); ;}
    break;

  case 15:
#line 159 "agrampar.y"
    { (yyval.tfClass) = new TF_class(new ASTClass("(placeholder)", NULL, NULL, NULL, NULL), NULL); ;}
    break;

  case 16:
#line 166 "agrampar.y"
    { (yyval.tfClass) = new TF_class(new ASTClass("(placeholder)", NULL, NULL, NULL, NULL), NULL); ;}
    break;

  case 17:
#line 168 "agrampar.y"
    { ((yyval.tfClass)=(yyvsp[(1) - (6)].tfClass))->ctors.append(new ASTClass(unbox((yyvsp[(3) - (6)].str)), (yyvsp[(4) - (6)].ctorArgList), NULL, (yyvsp[(5) - (6)].baseClassList), NULL)); ;}
    break;

  case 18:
#line 170 "agrampar.y"
    { ((yyval.tfClass)=(yyvsp[(1) - (8)].tfClass))->ctors.append(new ASTClass(unbox((yyvsp[(3) - (8)].str)), (yyvsp[(4) - (8)].ctorArgList), NULL, (yyvsp[(5) - (8)].baseClassList), (yyvsp[(7) - (8)].userDeclList))); ;}
    break;

  case 19:
#line 172 "agrampar.y"
    { ((yyval.tfClass)=(yyvsp[(1) - (2)].tfClass))->super->decls.append((yyvsp[(2) - (2)].annotation)); ;}
    break;

  case 20:
#line 179 "agrampar.y"
    { (yyval.ctorArgList) = new ASTList<CtorArg>; ;}
    break;

  case 21:
#line 181 "agrampar.y"
    { (yyval.ctorArgList) = (yyvsp[(1) - (1)].ctorArgList); ;}
    break;

  case 22:
#line 187 "agrampar.y"
    { (yyval.ctorArgList) = new ASTList<CtorArg>; ;}
    break;

  case 23:
#line 189 "agrampar.y"
    { (yyval.ctorArgList) = (yyvsp[(2) - (3)].ctorArgList); ;}
    break;

  case 24:
#line 194 "agrampar.y"
    { (yyval.ctorArgList) = new ASTList<CtorArg>;
                 {
                   string tmp = unbox((yyvsp[(1) - (1)].str));
                   (yyval.ctorArgList)->append(parseCtorArg(tmp));
                 }
               ;}
    break;

  case 25:
#line 201 "agrampar.y"
    { ((yyval.ctorArgList)=(yyvsp[(1) - (3)].ctorArgList))->append(parseCtorArg(unbox((yyvsp[(3) - (3)].str)))); ;}
    break;

  case 26:
#line 206 "agrampar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 27:
#line 208 "agrampar.y"
    { (yyval.str) = appendStr((yyvsp[(1) - (2)].str), (yyvsp[(2) - (2)].str)); ;}
    break;

  case 28:
#line 213 "agrampar.y"
    { (yyval.str) = appendStr((yyvsp[(1) - (1)].str), box(" ")); ;}
    break;

  case 29:
#line 214 "agrampar.y"
    { (yyval.str) = appendStr((yyvsp[(1) - (1)].str), box(" ")); ;}
    break;

  case 30:
#line 215 "agrampar.y"
    { (yyval.str) = appendStr(box("<"), appendStr((yyvsp[(2) - (3)].str), box(">"))); ;}
    break;

  case 31:
#line 216 "agrampar.y"
    { (yyval.str) = box("*"); ;}
    break;

  case 32:
#line 217 "agrampar.y"
    { (yyval.str) = box("&"); ;}
    break;

  case 33:
#line 218 "agrampar.y"
    { (yyval.str) = box("="); ;}
    break;

  case 34:
#line 219 "agrampar.y"
    { (yyval.str) = box("class "); ;}
    break;

  case 35:
#line 224 "agrampar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 36:
#line 226 "agrampar.y"
    { (yyval.str) = appendStr((yyvsp[(1) - (3)].str), appendStr(box(","), (yyvsp[(3) - (3)].str))); ;}
    break;

  case 37:
#line 232 "agrampar.y"
    { (yyval.userDeclList) = new ASTList<Annotation>; ;}
    break;

  case 38:
#line 234 "agrampar.y"
    { ((yyval.userDeclList)=(yyvsp[(1) - (2)].userDeclList))->append((yyvsp[(2) - (2)].annotation)); ;}
    break;

  case 39:
#line 240 "agrampar.y"
    { (yyval.annotation) = new UserDecl((yyvsp[(1) - (2)].accessMod), unbox((yyvsp[(2) - (2)].str)), ""); ;}
    break;

  case 40:
#line 242 "agrampar.y"
    { (yyval.annotation) = new UserDecl((yyvsp[(1) - (5)].accessMod), unbox((yyvsp[(2) - (5)].str)), unbox((yyvsp[(4) - (5)].str))); ;}
    break;

  case 41:
#line 244 "agrampar.y"
    { (yyval.annotation) = (yyvsp[(1) - (1)].customCode); ;}
    break;

  case 42:
#line 250 "agrampar.y"
    { (yyval.customCode) = new CustomCode(unbox((yyvsp[(2) - (3)].str)), unbox((yyvsp[(3) - (3)].str))); ;}
    break;

  case 43:
#line 256 "agrampar.y"
    { (yyval.str) = (yyvsp[(1) - (2)].str); ;}
    break;

  case 44:
#line 258 "agrampar.y"
    { (yyval.str) = (yyvsp[(2) - (3)].str); ;}
    break;

  case 45:
#line 263 "agrampar.y"
    { (yyval.accessCtl) = AC_PUBLIC; ;}
    break;

  case 46:
#line 264 "agrampar.y"
    { (yyval.accessCtl) = AC_PRIVATE; ;}
    break;

  case 47:
#line 265 "agrampar.y"
    { (yyval.accessCtl) = AC_PROTECTED; ;}
    break;

  case 48:
#line 266 "agrampar.y"
    { (yyval.accessCtl) = AC_CTOR; ;}
    break;

  case 49:
#line 267 "agrampar.y"
    { (yyval.accessCtl) = AC_DTOR; ;}
    break;

  case 50:
#line 268 "agrampar.y"
    { (yyval.accessCtl) = AC_PUREVIRT; ;}
    break;

  case 51:
#line 273 "agrampar.y"
    { (yyval.accessMod) = new AccessMod((yyvsp[(1) - (1)].accessCtl), NULL); ;}
    break;

  case 52:
#line 275 "agrampar.y"
    { (yyval.accessMod) = new AccessMod((yyvsp[(1) - (4)].accessCtl), (yyvsp[(3) - (4)].stringList)); ;}
    break;

  case 53:
#line 280 "agrampar.y"
    { (yyval.stringList) = new ASTList<string>((yyvsp[(1) - (1)].str)); ;}
    break;

  case 54:
#line 282 "agrampar.y"
    { ((yyval.stringList)=(yyvsp[(1) - (3)].stringList))->append((yyvsp[(3) - (3)].str)); ;}
    break;

  case 55:
#line 287 "agrampar.y"
    { (yyval.verbatim) = new TF_verbatim(unbox((yyvsp[(2) - (2)].str))); ;}
    break;

  case 56:
#line 289 "agrampar.y"
    { (yyval.verbatim) = new TF_impl_verbatim(unbox((yyvsp[(2) - (2)].str))); ;}
    break;

  case 57:
#line 294 "agrampar.y"
    { (yyval.tfOption) = new TF_option(unbox((yyvsp[(2) - (4)].str)), (yyvsp[(3) - (4)].stringList)); ;}
    break;

  case 58:
#line 299 "agrampar.y"
    { (yyval.stringList) = new ASTList<string>; ;}
    break;

  case 59:
#line 301 "agrampar.y"
    { ((yyval.stringList)=(yyvsp[(1) - (2)].stringList))->append((yyvsp[(2) - (2)].str)); ;}
    break;

  case 60:
#line 306 "agrampar.y"
    { (yyval.tfEnum) = new TF_enum(unbox((yyvsp[(2) - (5)].str)), (yyvsp[(4) - (5)].enumeratorList)); ;}
    break;

  case 61:
#line 308 "agrampar.y"
    { (yyval.tfEnum) = new TF_enum(unbox((yyvsp[(2) - (6)].str)), (yyvsp[(4) - (6)].enumeratorList)); ;}
    break;

  case 62:
#line 313 "agrampar.y"
    { (yyval.enumeratorList) = new ASTList<string>((yyvsp[(1) - (1)].enumerator)); ;}
    break;

  case 63:
#line 315 "agrampar.y"
    { ((yyval.enumeratorList)=(yyvsp[(1) - (3)].enumeratorList))->append((yyvsp[(3) - (3)].enumerator)); ;}
    break;

  case 64:
#line 320 "agrampar.y"
    { (yyval.enumerator) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 65:
#line 325 "agrampar.y"
    { (yyval.baseClassList) = new ASTList<BaseClass>; ;}
    break;

  case 66:
#line 327 "agrampar.y"
    { (yyval.baseClassList) = (yyvsp[(2) - (2)].baseClassList); ;}
    break;

  case 67:
#line 332 "agrampar.y"
    { (yyval.baseClassList) = new ASTList<BaseClass>((yyvsp[(1) - (1)].baseClass)); ;}
    break;

  case 68:
#line 334 "agrampar.y"
    { ((yyval.baseClassList)=(yyvsp[(1) - (3)].baseClassList))->append((yyvsp[(3) - (3)].baseClass)); ;}
    break;

  case 69:
#line 339 "agrampar.y"
    { (yyval.accessCtl) = AC_PUBLIC; ;}
    break;

  case 70:
#line 340 "agrampar.y"
    { (yyval.accessCtl) = AC_PRIVATE; ;}
    break;

  case 71:
#line 341 "agrampar.y"
    { (yyval.accessCtl) = AC_PROTECTED; ;}
    break;

  case 72:
#line 346 "agrampar.y"
    { (yyval.baseClass) = new BaseClass((yyvsp[(1) - (2)].accessCtl), unbox((yyvsp[(2) - (2)].str))); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 1876 "agrampar.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 349 "agrampar.y"


/* ----------------- extra C code ------------------- */


