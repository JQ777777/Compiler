/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser/SysY_parser.y"

#include <fstream>
#include "SysY_tree.h"
#include "type.h"
Program ast_root;

void yyerror(char *s, ...);
int yylex();
int error_num = 0;
extern int line_number;
extern std::ofstream fout;
extern IdTable id_table;

#line 85 "SysY_parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "SysY_parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_STR_CONST = 3,                  /* STR_CONST  */
  YYSYMBOL_IDENT = 4,                      /* IDENT  */
  YYSYMBOL_FLOAT_CONST = 5,                /* FLOAT_CONST  */
  YYSYMBOL_INT_CONST = 6,                  /* INT_CONST  */
  YYSYMBOL_LEQ = 7,                        /* LEQ  */
  YYSYMBOL_GEQ = 8,                        /* GEQ  */
  YYSYMBOL_EQ = 9,                         /* EQ  */
  YYSYMBOL_NE = 10,                        /* NE  */
  YYSYMBOL_AQ = 11,                        /* AQ  */
  YYSYMBOL_SQ = 12,                        /* SQ  */
  YYSYMBOL_MQ = 13,                        /* MQ  */
  YYSYMBOL_DQ = 14,                        /* DQ  */
  YYSYMBOL_MODQ = 15,                      /* MODQ  */
  YYSYMBOL_AND = 16,                       /* AND  */
  YYSYMBOL_OR = 17,                        /* OR  */
  YYSYMBOL_CONST = 18,                     /* CONST  */
  YYSYMBOL_IF = 19,                        /* IF  */
  YYSYMBOL_ELSE = 20,                      /* ELSE  */
  YYSYMBOL_WHILE = 21,                     /* WHILE  */
  YYSYMBOL_NONE_TYPE = 22,                 /* NONE_TYPE  */
  YYSYMBOL_INT = 23,                       /* INT  */
  YYSYMBOL_FLOAT = 24,                     /* FLOAT  */
  YYSYMBOL_FOR = 25,                       /* FOR  */
  YYSYMBOL_RETURN = 26,                    /* RETURN  */
  YYSYMBOL_BREAK = 27,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 28,                  /* CONTINUE  */
  YYSYMBOL_ERROR = 29,                     /* ERROR  */
  YYSYMBOL_TODO = 30,                      /* TODO  */
  YYSYMBOL_THEN = 31,                      /* THEN  */
  YYSYMBOL_32_ = 32,                       /* ';'  */
  YYSYMBOL_33_ = 33,                       /* '['  */
  YYSYMBOL_34_ = 34,                       /* ']'  */
  YYSYMBOL_35_ = 35,                       /* ','  */
  YYSYMBOL_36_ = 36,                       /* '('  */
  YYSYMBOL_37_ = 37,                       /* ')'  */
  YYSYMBOL_38_ = 38,                       /* '='  */
  YYSYMBOL_39_ = 39,                       /* '{'  */
  YYSYMBOL_40_ = 40,                       /* '}'  */
  YYSYMBOL_41_ = 41,                       /* '+'  */
  YYSYMBOL_42_ = 42,                       /* '-'  */
  YYSYMBOL_43_ = 43,                       /* '!'  */
  YYSYMBOL_44_ = 44,                       /* '*'  */
  YYSYMBOL_45_ = 45,                       /* '/'  */
  YYSYMBOL_46_ = 46,                       /* '%'  */
  YYSYMBOL_47_ = 47,                       /* '<'  */
  YYSYMBOL_48_ = 48,                       /* '>'  */
  YYSYMBOL_YYACCEPT = 49,                  /* $accept  */
  YYSYMBOL_Program = 50,                   /* Program  */
  YYSYMBOL_Comp_list = 51,                 /* Comp_list  */
  YYSYMBOL_CompUnit = 52,                  /* CompUnit  */
  YYSYMBOL_Decl = 53,                      /* Decl  */
  YYSYMBOL_VarDecl = 54,                   /* VarDecl  */
  YYSYMBOL_Array = 55,                     /* Array  */
  YYSYMBOL_ConstArray = 56,                /* ConstArray  */
  YYSYMBOL_ConstDecl = 57,                 /* ConstDecl  */
  YYSYMBOL_VarDef_list = 58,               /* VarDef_list  */
  YYSYMBOL_ConstDef_list = 59,             /* ConstDef_list  */
  YYSYMBOL_Array_list = 60,                /* Array_list  */
  YYSYMBOL_ConstArray_list = 61,           /* ConstArray_list  */
  YYSYMBOL_FuncDef = 62,                   /* FuncDef  */
  YYSYMBOL_VarDef = 63,                    /* VarDef  */
  YYSYMBOL_ConstDef = 64,                  /* ConstDef  */
  YYSYMBOL_ConstInitVal = 65,              /* ConstInitVal  */
  YYSYMBOL_VarInitVal = 66,                /* VarInitVal  */
  YYSYMBOL_ConstInitVal_list = 67,         /* ConstInitVal_list  */
  YYSYMBOL_VarInitVal_list = 68,           /* VarInitVal_list  */
  YYSYMBOL_FuncFParams = 69,               /* FuncFParams  */
  YYSYMBOL_FuncFParam = 70,                /* FuncFParam  */
  YYSYMBOL_Block = 71,                     /* Block  */
  YYSYMBOL_BlockItem_list = 72,            /* BlockItem_list  */
  YYSYMBOL_BlockItem = 73,                 /* BlockItem  */
  YYSYMBOL_Stmt = 74,                      /* Stmt  */
  YYSYMBOL_Exp = 75,                       /* Exp  */
  YYSYMBOL_Cond = 76,                      /* Cond  */
  YYSYMBOL_Lval = 77,                      /* Lval  */
  YYSYMBOL_PrimaryExp = 78,                /* PrimaryExp  */
  YYSYMBOL_IntConst = 79,                  /* IntConst  */
  YYSYMBOL_FloatConst = 80,                /* FloatConst  */
  YYSYMBOL_UnaryExp = 81,                  /* UnaryExp  */
  YYSYMBOL_FuncRParams = 82,               /* FuncRParams  */
  YYSYMBOL_Exp_list = 83,                  /* Exp_list  */
  YYSYMBOL_MulExp = 84,                    /* MulExp  */
  YYSYMBOL_AddExp = 85,                    /* AddExp  */
  YYSYMBOL_RelExp = 86,                    /* RelExp  */
  YYSYMBOL_EqExp = 87,                     /* EqExp  */
  YYSYMBOL_LAndExp = 88,                   /* LAndExp  */
  YYSYMBOL_LOrExp = 89,                    /* LOrExp  */
  YYSYMBOL_ConstExp = 90                   /* ConstExp  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  20
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   347

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  49
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  119
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  238

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   286


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    43,     2,     2,     2,    46,     2,     2,
      36,    37,    44,    41,    35,    42,     2,    45,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    32,
      47,    38,    48,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    33,     2,    34,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    39,     2,    40,     2,     2,     2,     2,
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
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    67,    67,    75,    80,    87,    91,    98,   102,   109,
     113,   121,   130,   140,   144,   153,   159,   168,   174,   184,
     189,   197,   202,   210,   215,   220,   225,   230,   235,   244,
     249,   254,   259,   269,   275,   285,   289,   293,   300,   304,
     308,   315,   319,   326,   330,   338,   342,   349,   353,   357,
     362,   367,   373,   381,   389,   395,   402,   410,   423,   427,
     435,   439,   447,   451,   459,   463,   467,   471,   476,   481,
     486,   491,   497,   502,   506,   510,   518,   523,   527,   531,
     535,   543,   548,   553,   557,   565,   569,   573,   577,   584,
     591,   599,   600,   604,   628,   632,   636,   644,   652,   656,
     664,   668,   672,   676,   684,   688,   692,   700,   704,   708,
     712,   716,   724,   728,   732,   740,   744,   752,   756,   764
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "STR_CONST", "IDENT",
  "FLOAT_CONST", "INT_CONST", "LEQ", "GEQ", "EQ", "NE", "AQ", "SQ", "MQ",
  "DQ", "MODQ", "AND", "OR", "CONST", "IF", "ELSE", "WHILE", "NONE_TYPE",
  "INT", "FLOAT", "FOR", "RETURN", "BREAK", "CONTINUE", "ERROR", "TODO",
  "THEN", "';'", "'['", "']'", "','", "'('", "')'", "'='", "'{'", "'}'",
  "'+'", "'-'", "'!'", "'*'", "'/'", "'%'", "'<'", "'>'", "$accept",
  "Program", "Comp_list", "CompUnit", "Decl", "VarDecl", "Array",
  "ConstArray", "ConstDecl", "VarDef_list", "ConstDef_list", "Array_list",
  "ConstArray_list", "FuncDef", "VarDef", "ConstDef", "ConstInitVal",
  "VarInitVal", "ConstInitVal_list", "VarInitVal_list", "FuncFParams",
  "FuncFParam", "Block", "BlockItem_list", "BlockItem", "Stmt", "Exp",
  "Cond", "Lval", "PrimaryExp", "IntConst", "FloatConst", "UnaryExp",
  "FuncRParams", "Exp_list", "MulExp", "AddExp", "RelExp", "EqExp",
  "LAndExp", "LOrExp", "ConstExp", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-178)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      92,    12,    16,    54,    80,    88,    92,  -178,  -178,  -178,
    -178,  -178,   127,   127,    18,     4,    50,  -178,    29,    87,
    -178,  -178,    19,   100,  -178,   111,    -5,   303,     2,   239,
    -178,    31,  -178,   132,    10,  -178,   303,   256,  -178,    35,
    -178,   127,  -178,   153,   160,    53,   117,  -178,   112,  -178,
    -178,   303,   303,   303,   303,   145,  -178,  -178,  -178,  -178,
    -178,   147,     3,    53,   128,   216,  -178,  -178,   239,  -178,
      61,  -178,    53,   171,     3,   148,   231,  -178,  -178,   256,
    -178,  -178,   178,   180,   162,  -178,    77,    53,   119,   193,
     201,  -178,  -178,  -178,  -178,   303,   303,   303,   303,   303,
    -178,    53,  -178,  -178,    93,  -178,  -178,    53,  -178,  -178,
    -178,    94,  -178,   243,   260,   203,   204,   132,   132,   205,
     283,   210,   214,  -178,  -178,  -178,  -178,   191,  -178,  -178,
     218,    66,  -178,  -178,  -178,  -178,   217,   228,  -178,  -178,
    -178,  -178,   147,   147,  -178,   239,  -178,  -178,   256,  -178,
     220,   234,   193,   235,   303,   303,    84,  -178,   219,  -178,
    -178,  -178,  -178,  -178,   303,   303,   303,   303,   303,   303,
    -178,   303,  -178,  -178,   300,   193,   250,   193,   257,   254,
       3,     8,   190,   277,   259,   263,   303,  -178,   278,   279,
     280,   281,   282,   284,  -178,  -178,   303,   303,    70,   303,
     303,   303,   303,   303,   303,   303,   303,    70,   285,  -178,
    -178,  -178,  -178,  -178,  -178,   286,   287,   298,     3,     3,
       3,     3,     8,     8,   190,   277,  -178,   318,  -178,  -178,
      70,   193,   289,  -178,   303,   291,    70,  -178
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     2,     3,     5,     7,
       8,     6,     0,     0,     0,    30,     0,    15,    30,     0,
       1,     4,     0,     0,    17,     0,     0,     0,     0,     0,
      19,    32,     9,     0,     0,    10,     0,     0,    21,     0,
      13,     0,    14,     0,     0,     0,     0,    45,    83,    90,
      89,     0,     0,     0,     0,     0,    86,    91,    87,    88,
     100,   104,    81,     0,     0,     0,    29,    38,     0,    20,
      30,    16,     0,     0,   119,     0,     0,    33,    35,     0,
      22,    18,    47,    48,     0,    28,     0,     0,     0,    84,
       0,    94,    95,    96,    11,     0,     0,     0,     0,     0,
      24,     0,    40,    43,     0,    31,    26,     0,    12,    37,
      41,     0,    34,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    64,    59,    62,    72,     0,    60,    63,
       0,    86,    46,    27,    93,    98,     0,    97,    85,   101,
     102,   103,   105,   106,    23,     0,    39,    25,     0,    36,
      51,     0,    54,     0,     0,     0,     0,    80,     0,    77,
      78,    58,    61,    65,     0,     0,     0,     0,     0,     0,
      92,     0,    44,    42,     0,    49,    52,    50,    53,     0,
     107,   112,   115,   117,    82,     0,     0,    79,     0,     0,
       0,     0,     0,     0,    99,    55,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    67,
      68,    69,    70,    71,    66,     0,     0,    73,   110,   111,
     108,   109,   113,   114,   116,   118,    75,     0,    56,    57,
       0,    83,     0,    74,     0,     0,     0,    76
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -178,  -178,  -178,   317,   -78,   173,   -26,   292,  -178,    -1,
     319,   -47,  -178,  -178,   297,   294,   -65,   -55,  -178,  -178,
     -11,   247,   -41,  -178,   211,  -177,   -27,  -143,   -77,  -178,
    -178,  -178,   -25,  -178,  -178,   126,   -28,    25,   135,   131,
    -178,   311
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     5,     6,     7,     8,     9,    30,    38,    10,    16,
      23,    31,    39,    11,    17,    24,    77,    66,   111,   104,
      46,    47,   126,   127,   128,   129,   130,   179,    56,    57,
      58,    59,    60,   136,   137,    61,    62,   181,   182,   183,
     184,    78
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      55,    89,    67,    19,    85,    69,   125,   131,    74,    74,
     103,   110,   185,   105,   112,   199,   200,    64,    43,    44,
      14,   217,   100,    73,    90,    43,    44,    91,    92,    93,
     226,   106,    45,    43,    44,    12,    13,    27,    67,    63,
      28,    67,    29,   208,    98,    99,   133,    72,    74,   125,
     131,    74,    36,   233,    26,   201,   202,    37,    15,   237,
     144,   135,    27,    69,    27,    34,   147,    29,    36,    68,
     139,   140,   141,    79,    48,    49,    50,   164,   165,   166,
     167,   168,    32,   173,    18,    33,   151,   153,    20,   115,
     172,   116,    84,   158,    27,   119,   120,   121,   122,    29,
      43,    44,   123,   175,   169,   177,    51,   117,   118,    84,
       1,    52,    53,    54,     2,     3,     4,    19,    67,    35,
      74,   131,    33,    48,    49,    50,   180,   180,   145,   148,
     131,    22,    40,   146,   149,    41,    70,   188,   189,   190,
     191,   192,   193,    42,   194,    27,    41,    55,    88,    69,
     232,    69,    86,   131,    87,    51,   134,    82,   180,   131,
      52,    53,    54,    86,    83,   101,    48,    49,    50,   215,
     216,   218,   219,   220,   221,   180,   180,   180,   180,    94,
       1,   115,   108,   116,    89,   117,   118,   119,   120,   121,
     122,    95,    96,    97,   123,    48,    49,    50,    51,   203,
     204,    84,   124,    52,    53,    54,    86,   235,   107,     1,
     115,   113,   116,   114,   117,   118,   119,   120,   121,   122,
      48,    49,    50,   123,   142,   143,    27,    51,   222,   223,
      84,   161,    52,    53,    54,    48,    49,    50,   138,   154,
     155,   156,   159,    48,    49,    50,   160,    48,    49,    50,
     163,   187,    51,   174,   170,    65,   102,    52,    53,    54,
      48,    49,    50,   171,    48,    49,    50,    51,   176,   178,
      76,   109,    52,    53,    54,    51,   206,   150,    65,    51,
      52,    53,    54,   196,    52,    53,    54,    48,    49,    50,
     197,   198,    51,   205,   152,    76,    51,    52,    53,    54,
     207,    52,    53,    54,    48,    49,    50,    48,    49,    50,
     209,   210,   211,   212,   213,   157,   214,   227,   230,    51,
     228,   229,   231,    21,    52,    53,    54,   234,   236,   186,
      71,    80,    25,   132,   195,    81,    51,   225,   162,    51,
     224,    52,    53,    54,    52,    53,    54,    75
};

static const yytype_uint8 yycheck[] =
{
      27,    48,    29,     4,    45,    31,    84,    84,    36,    37,
      65,    76,   155,    68,    79,     7,     8,    28,    23,    24,
       4,   198,    63,    34,    51,    23,    24,    52,    53,    54,
     207,    72,    37,    23,    24,    23,    24,    33,    65,    37,
      36,    68,    38,   186,    41,    42,    87,    37,    76,   127,
     127,    79,    33,   230,    36,    47,    48,    38,     4,   236,
     101,    88,    33,    89,    33,    36,   107,    38,    33,    38,
      95,    96,    97,    38,     4,     5,     6,    11,    12,    13,
      14,    15,    32,   148,     4,    35,   113,   114,     0,    19,
     145,    21,    39,   120,    33,    25,    26,    27,    28,    38,
      23,    24,    32,   150,    38,   152,    36,    23,    24,    39,
      18,    41,    42,    43,    22,    23,    24,   118,   145,    32,
     148,   198,    35,     4,     5,     6,   154,   155,    35,    35,
     207,     4,    32,    40,    40,    35,     4,   164,   165,   166,
     167,   168,   169,    32,   171,    33,    35,   174,    36,   175,
     227,   177,    35,   230,    37,    36,    37,     4,   186,   236,
      41,    42,    43,    35,     4,    37,     4,     5,     6,   196,
     197,   199,   200,   201,   202,   203,   204,   205,   206,    34,
      18,    19,    34,    21,   231,    23,    24,    25,    26,    27,
      28,    44,    45,    46,    32,     4,     5,     6,    36,     9,
      10,    39,    40,    41,    42,    43,    35,   234,    37,    18,
      19,    33,    21,    33,    23,    24,    25,    26,    27,    28,
       4,     5,     6,    32,    98,    99,    33,    36,   203,   204,
      39,    40,    41,    42,    43,     4,     5,     6,    37,    36,
      36,    36,    32,     4,     5,     6,    32,     4,     5,     6,
      32,    32,    36,    33,    37,    39,    40,    41,    42,    43,
       4,     5,     6,    35,     4,     5,     6,    36,    34,    34,
      39,    40,    41,    42,    43,    36,    17,    34,    39,    36,
      41,    42,    43,    33,    41,    42,    43,     4,     5,     6,
      33,    37,    36,    16,    34,    39,    36,    41,    42,    43,
      37,    41,    42,    43,     4,     5,     6,     4,     5,     6,
      32,    32,    32,    32,    32,    32,    32,    32,    20,    36,
      34,    34,     4,     6,    41,    42,    43,    38,    37,   156,
      33,    39,    13,    86,    34,    41,    36,   206,   127,    36,
     205,    41,    42,    43,    41,    42,    43,    36
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    18,    22,    23,    24,    50,    51,    52,    53,    54,
      57,    62,    23,    24,     4,     4,    58,    63,     4,    58,
       0,    52,     4,    59,    64,    59,    36,    33,    36,    38,
      55,    60,    32,    35,    36,    32,    33,    38,    56,    61,
      32,    35,    32,    23,    24,    37,    69,    70,     4,     5,
       6,    36,    41,    42,    43,    75,    77,    78,    79,    80,
      81,    84,    85,    37,    69,    39,    66,    75,    38,    55,
       4,    63,    37,    69,    85,    90,    39,    65,    90,    38,
      56,    64,     4,     4,    39,    71,    35,    37,    36,    60,
      75,    81,    81,    81,    34,    44,    45,    46,    41,    42,
      71,    37,    40,    66,    68,    66,    71,    37,    34,    40,
      65,    67,    65,    33,    33,    19,    21,    23,    24,    25,
      26,    27,    28,    32,    40,    53,    71,    72,    73,    74,
      75,    77,    70,    71,    37,    75,    82,    83,    37,    81,
      81,    81,    84,    84,    71,    35,    40,    71,    35,    40,
      34,    75,    34,    75,    36,    36,    36,    32,    75,    32,
      32,    40,    73,    32,    11,    12,    13,    14,    15,    38,
      37,    35,    66,    65,    33,    60,    34,    60,    34,    76,
      85,    86,    87,    88,    89,    76,    54,    32,    75,    75,
      75,    75,    75,    75,    75,    34,    33,    33,    37,     7,
       8,    47,    48,     9,    10,    16,    17,    37,    76,    32,
      32,    32,    32,    32,    32,    75,    75,    74,    85,    85,
      85,    85,    86,    86,    87,    88,    74,    32,    34,    34,
      20,     4,    77,    74,    38,    75,    37,    74
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    49,    50,    51,    51,    52,    52,    53,    53,    54,
      54,    55,    56,    57,    57,    58,    58,    59,    59,    60,
      60,    61,    61,    62,    62,    62,    62,    62,    62,    63,
      63,    63,    63,    64,    64,    65,    65,    65,    66,    66,
      66,    67,    67,    68,    68,    69,    69,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    71,    71,
      72,    72,    73,    73,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    75,    76,    77,    77,    78,    78,    78,    78,    79,
      80,    81,    81,    81,    81,    81,    81,    82,    83,    83,
      84,    84,    84,    84,    85,    85,    85,    86,    86,    86,
      86,    86,    87,    87,    87,    88,    88,    89,    89,    90
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     3,
       3,     3,     3,     4,     4,     1,     3,     1,     3,     1,
       2,     1,     2,     6,     5,     6,     5,     6,     5,     3,
       1,     4,     2,     3,     4,     1,     3,     2,     1,     3,
       2,     1,     3,     1,     3,     1,     3,     2,     2,     5,
       5,     4,     5,     5,     4,     6,     8,     8,     3,     2,
       1,     2,     1,     1,     1,     2,     4,     4,     4,     4,
       4,     4,     1,     5,     7,     5,    10,     2,     2,     3,
       2,     1,     1,     1,     2,     3,     1,     1,     1,     1,
       1,     1,     4,     3,     2,     2,     2,     1,     1,     3,
       1,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     1,     3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* Program: Comp_list  */
#line 68 "parser/SysY_parser.y"
{
    (yyloc) = (yylsp[0]); 
    ast_root = new __Program((yyvsp[0].comps));
    ast_root->SetLineNumber(line_number);
}
#line 1461 "SysY_parser.tab.c"
    break;

  case 3: /* Comp_list: CompUnit  */
#line 76 "parser/SysY_parser.y"
{
    (yyval.comps) = new std::vector<CompUnit>;
    ((yyval.comps))->push_back((yyvsp[0].comp_unit));
}
#line 1470 "SysY_parser.tab.c"
    break;

  case 4: /* Comp_list: Comp_list CompUnit  */
#line 81 "parser/SysY_parser.y"
{
    ((yyvsp[-1].comps))->push_back((yyvsp[0].comp_unit));
    (yyval.comps) = (yyvsp[-1].comps);
}
#line 1479 "SysY_parser.tab.c"
    break;

  case 5: /* CompUnit: Decl  */
#line 87 "parser/SysY_parser.y"
     {  //声明
    (yyval.comp_unit) = new CompUnit_Decl((yyvsp[0].decl)); 
    (yyval.comp_unit)->SetLineNumber(line_number);
}
#line 1488 "SysY_parser.tab.c"
    break;

  case 6: /* CompUnit: FuncDef  */
#line 91 "parser/SysY_parser.y"
        {  //函数定义
    (yyval.comp_unit) = new CompUnit_FuncDef((yyvsp[0].func_def)); 
    (yyval.comp_unit)->SetLineNumber(line_number);
}
#line 1497 "SysY_parser.tab.c"
    break;

  case 7: /* Decl: VarDecl  */
#line 98 "parser/SysY_parser.y"
        {
    (yyval.decl) = (yyvsp[0].decl); 
    (yyval.decl)->SetLineNumber(line_number);
}
#line 1506 "SysY_parser.tab.c"
    break;

  case 8: /* Decl: ConstDecl  */
#line 102 "parser/SysY_parser.y"
          {
    (yyval.decl) = (yyvsp[0].decl); 
    (yyval.decl)->SetLineNumber(line_number);
}
#line 1515 "SysY_parser.tab.c"
    break;

  case 9: /* VarDecl: INT VarDef_list ';'  */
#line 109 "parser/SysY_parser.y"
                    {
    (yyval.decl) = new VarDecl(Type::INT,(yyvsp[-1].defs)); 
    (yyval.decl)->SetLineNumber(line_number);
}
#line 1524 "SysY_parser.tab.c"
    break;

  case 10: /* VarDecl: FLOAT VarDef_list ';'  */
#line 113 "parser/SysY_parser.y"
                      {   //增加float类型
    (yyval.decl) = new VarDecl(Type::FLOAT,(yyvsp[-1].defs)); 
    (yyval.decl)->SetLineNumber(line_number);
}
#line 1533 "SysY_parser.tab.c"
    break;

  case 11: /* Array: '[' Exp ']'  */
#line 122 "parser/SysY_parser.y"
{
   (yyval.expression)=(yyvsp[-1].expression);
   (yyval.expression)->SetLineNumber(line_number);
}
#line 1542 "SysY_parser.tab.c"
    break;

  case 12: /* ConstArray: '[' ConstExp ']'  */
#line 131 "parser/SysY_parser.y"
{
    (yyval.expression) = (yyvsp[-1].expression);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 1551 "SysY_parser.tab.c"
    break;

  case 13: /* ConstDecl: CONST INT ConstDef_list ';'  */
#line 140 "parser/SysY_parser.y"
                            {   //增加float类型
    (yyval.decl) = new ConstDecl(Type::INT,(yyvsp[-1].defs)); 
    (yyval.decl)->SetLineNumber(line_number);
}
#line 1560 "SysY_parser.tab.c"
    break;

  case 14: /* ConstDecl: CONST FLOAT ConstDef_list ';'  */
#line 144 "parser/SysY_parser.y"
                              {
    (yyval.decl) = new ConstDecl(Type::FLOAT,(yyvsp[-1].defs)); 
    (yyval.decl)->SetLineNumber(line_number);
}
#line 1569 "SysY_parser.tab.c"
    break;

  case 15: /* VarDef_list: VarDef  */
#line 154 "parser/SysY_parser.y"
{  
    // 创建一个新的向量来存储变量定义  
    (yyval.defs) = new std::vector<Def>;  
    ((yyval.defs))->push_back((yyvsp[0].def)); 
}
#line 1579 "SysY_parser.tab.c"
    break;

  case 16: /* VarDef_list: VarDef_list ',' VarDef  */
#line 160 "parser/SysY_parser.y"
{  
    // 将新的 VarDef 添加到现有的列表中  
    ((yyvsp[-2].defs))->push_back((yyvsp[0].def));   
    (yyval.defs) = (yyvsp[-2].defs);  
}
#line 1589 "SysY_parser.tab.c"
    break;

  case 17: /* ConstDef_list: ConstDef  */
#line 169 "parser/SysY_parser.y"
{  
    // 创建一个新的向量来存储常量定义  
    (yyval.defs) = new std::vector<Def>; 
    ((yyval.defs))->push_back((yyvsp[0].def)); 
}
#line 1599 "SysY_parser.tab.c"
    break;

  case 18: /* ConstDef_list: ConstDef_list ',' ConstDef  */
#line 175 "parser/SysY_parser.y"
{  
    // 将新的 ConstDef 添加到现有的列表中  
    ((yyvsp[-2].defs))->push_back((yyvsp[0].def));    
    (yyval.defs) = (yyvsp[-2].defs);  
}
#line 1609 "SysY_parser.tab.c"
    break;

  case 19: /* Array_list: Array  */
#line 185 "parser/SysY_parser.y"
{
    (yyval.expressions) = new std::vector<Expression>;
    ((yyval.expressions))->push_back((yyvsp[0].expression));
}
#line 1618 "SysY_parser.tab.c"
    break;

  case 20: /* Array_list: Array_list Array  */
#line 190 "parser/SysY_parser.y"
{
    ((yyvsp[-1].expressions))->push_back((yyvsp[0].expression));
    (yyval.expressions) = (yyvsp[-1].expressions);
}
#line 1627 "SysY_parser.tab.c"
    break;

  case 21: /* ConstArray_list: ConstArray  */
#line 198 "parser/SysY_parser.y"
{
    (yyval.expressions) = new std::vector<Expression>;
    ((yyval.expressions))->push_back((yyvsp[0].expression));
}
#line 1636 "SysY_parser.tab.c"
    break;

  case 22: /* ConstArray_list: ConstArray_list ConstArray  */
#line 203 "parser/SysY_parser.y"
{
    ((yyvsp[-1].expressions))->push_back((yyvsp[0].expression));
    (yyval.expressions) = (yyvsp[-1].expressions);
}
#line 1645 "SysY_parser.tab.c"
    break;

  case 23: /* FuncDef: INT IDENT '(' FuncFParams ')' Block  */
#line 211 "parser/SysY_parser.y"
{
    (yyval.func_def) = new __FuncDef(Type::INT,(yyvsp[-4].symbol_token),(yyvsp[-2].formals),(yyvsp[0].block));
    (yyval.func_def)->SetLineNumber(line_number);
}
#line 1654 "SysY_parser.tab.c"
    break;

  case 24: /* FuncDef: INT IDENT '(' ')' Block  */
#line 216 "parser/SysY_parser.y"
{
    (yyval.func_def) = new __FuncDef(Type::INT,(yyvsp[-3].symbol_token),new std::vector<FuncFParam>(),(yyvsp[0].block)); 
    (yyval.func_def)->SetLineNumber(line_number);
}
#line 1663 "SysY_parser.tab.c"
    break;

  case 25: /* FuncDef: FLOAT IDENT '(' FuncFParams ')' Block  */
#line 221 "parser/SysY_parser.y"
{
    (yyval.func_def) = new __FuncDef(Type::FLOAT,(yyvsp[-4].symbol_token),(yyvsp[-2].formals),(yyvsp[0].block));
    (yyval.func_def)->SetLineNumber(line_number);
}
#line 1672 "SysY_parser.tab.c"
    break;

  case 26: /* FuncDef: FLOAT IDENT '(' ')' Block  */
#line 226 "parser/SysY_parser.y"
{
    (yyval.func_def) = new __FuncDef(Type::FLOAT,(yyvsp[-3].symbol_token),new std::vector<FuncFParam>(),(yyvsp[0].block)); 
    (yyval.func_def)->SetLineNumber(line_number);
}
#line 1681 "SysY_parser.tab.c"
    break;

  case 27: /* FuncDef: NONE_TYPE IDENT '(' FuncFParams ')' Block  */
#line 231 "parser/SysY_parser.y"
{
    (yyval.func_def) = new __FuncDef(Type::VOID, (yyvsp[-4].symbol_token), (yyvsp[-2].formals), (yyvsp[0].block));
    (yyval.func_def)->SetLineNumber(line_number);
}
#line 1690 "SysY_parser.tab.c"
    break;

  case 28: /* FuncDef: NONE_TYPE IDENT '(' ')' Block  */
#line 236 "parser/SysY_parser.y"
{
    (yyval.func_def) = new __FuncDef(Type::VOID, (yyvsp[-3].symbol_token), new std::vector<FuncFParam>(), (yyvsp[0].block));
    (yyval.func_def)->SetLineNumber(line_number);
}
#line 1699 "SysY_parser.tab.c"
    break;

  case 29: /* VarDef: IDENT '=' VarInitVal  */
#line 245 "parser/SysY_parser.y"
{
    (yyval.def) = new VarDef((yyvsp[-2].symbol_token),nullptr,(yyvsp[0].initval)); 
    (yyval.def)->SetLineNumber(line_number);
    }
#line 1708 "SysY_parser.tab.c"
    break;

  case 30: /* VarDef: IDENT  */
#line 250 "parser/SysY_parser.y"
{
    (yyval.def) = new VarDef_no_init((yyvsp[0].symbol_token),nullptr); 
    (yyval.def)->SetLineNumber(line_number);
    }
#line 1717 "SysY_parser.tab.c"
    break;

  case 31: /* VarDef: IDENT Array_list '=' VarInitVal  */
#line 255 "parser/SysY_parser.y"
{
    (yyval.def) =  new VarDef((yyvsp[-3].symbol_token),(yyvsp[-2].expressions),(yyvsp[0].initval)); 
    (yyval.def)->SetLineNumber(line_number);
}
#line 1726 "SysY_parser.tab.c"
    break;

  case 32: /* VarDef: IDENT Array_list  */
#line 260 "parser/SysY_parser.y"
{
    (yyval.def) = new VarDef_no_init((yyvsp[-1].symbol_token),(yyvsp[0].expressions)); 
    (yyval.def)->SetLineNumber(line_number);
}
#line 1735 "SysY_parser.tab.c"
    break;

  case 33: /* ConstDef: IDENT '=' ConstInitVal  */
#line 270 "parser/SysY_parser.y"
    {
        (yyval.def) = new ConstDef((yyvsp[-2].symbol_token),nullptr,(yyvsp[0].initval));
        (yyval.def)->SetLineNumber(line_number);
    }
#line 1744 "SysY_parser.tab.c"
    break;

  case 34: /* ConstDef: IDENT ConstArray_list '=' ConstInitVal  */
#line 276 "parser/SysY_parser.y"
    {
        (yyval.def) = new ConstDef((yyvsp[-3].symbol_token),(yyvsp[-2].expressions),(yyvsp[0].initval));
        (yyval.def)->SetLineNumber(line_number);
    }
#line 1753 "SysY_parser.tab.c"
    break;

  case 35: /* ConstInitVal: ConstExp  */
#line 285 "parser/SysY_parser.y"
         {   //常量表达式
    (yyval.initval) = new ConstInitVal_exp((yyvsp[0].expression)); 
    (yyval.initval)->SetLineNumber(line_number);
    }
#line 1762 "SysY_parser.tab.c"
    break;

  case 36: /* ConstInitVal: '{' ConstInitVal_list '}'  */
#line 289 "parser/SysY_parser.y"
                          {
    (yyval.initval) = new ConstInitVal((yyvsp[-1].initvals)); 
    (yyval.initval)->SetLineNumber(line_number);
    }
#line 1771 "SysY_parser.tab.c"
    break;

  case 37: /* ConstInitVal: '{' '}'  */
#line 293 "parser/SysY_parser.y"
        {   //空列表
    (yyval.initval) = new ConstInitVal(new std::vector<InitVal>());
     (yyval.initval)->SetLineNumber(line_number);
     }
#line 1780 "SysY_parser.tab.c"
    break;

  case 38: /* VarInitVal: Exp  */
#line 300 "parser/SysY_parser.y"
    {
    (yyval.initval) = new VarInitVal_exp((yyvsp[0].expression)); 
    (yyval.initval)->SetLineNumber(line_number);
    }
#line 1789 "SysY_parser.tab.c"
    break;

  case 39: /* VarInitVal: '{' VarInitVal_list '}'  */
#line 304 "parser/SysY_parser.y"
                        {
    (yyval.initval) = new VarInitVal((yyvsp[-1].initvals)); 
    (yyval.initval)->SetLineNumber(line_number);
    }
#line 1798 "SysY_parser.tab.c"
    break;

  case 40: /* VarInitVal: '{' '}'  */
#line 308 "parser/SysY_parser.y"
        {
    (yyval.initval) = new VarInitVal(new std::vector<InitVal>()); 
    (yyval.initval)->SetLineNumber(line_number);
    }
#line 1807 "SysY_parser.tab.c"
    break;

  case 41: /* ConstInitVal_list: ConstInitVal  */
#line 315 "parser/SysY_parser.y"
             {   //单个常量初始化值
    (yyval.initvals) = new std::vector<InitVal>;
    ((yyval.initvals))->push_back((yyvsp[0].initval));
}
#line 1816 "SysY_parser.tab.c"
    break;

  case 42: /* ConstInitVal_list: ConstInitVal_list ',' ConstInitVal  */
#line 319 "parser/SysY_parser.y"
                                   {   //多个常量初始化值
    ((yyvsp[-2].initvals))->push_back((yyvsp[0].initval));
    (yyval.initvals) = (yyvsp[-2].initvals);
}
#line 1825 "SysY_parser.tab.c"
    break;

  case 43: /* VarInitVal_list: VarInitVal  */
#line 326 "parser/SysY_parser.y"
           {
    (yyval.initvals) = new std::vector<InitVal>;
    ((yyval.initvals))->push_back((yyvsp[0].initval));
}
#line 1834 "SysY_parser.tab.c"
    break;

  case 44: /* VarInitVal_list: VarInitVal_list ',' VarInitVal  */
#line 330 "parser/SysY_parser.y"
                               {
    ((yyvsp[-2].initvals))->push_back((yyvsp[0].initval));
    (yyval.initvals) = (yyvsp[-2].initvals);
}
#line 1843 "SysY_parser.tab.c"
    break;

  case 45: /* FuncFParams: FuncFParam  */
#line 338 "parser/SysY_parser.y"
           {
    (yyval.formals) = new std::vector<FuncFParam>;
    ((yyval.formals))->push_back((yyvsp[0].formal));
}
#line 1852 "SysY_parser.tab.c"
    break;

  case 46: /* FuncFParams: FuncFParams ',' FuncFParam  */
#line 342 "parser/SysY_parser.y"
                           {
    ((yyvsp[-2].formals))->push_back((yyvsp[0].formal));
    (yyval.formals) = (yyvsp[-2].formals);
}
#line 1861 "SysY_parser.tab.c"
    break;

  case 47: /* FuncFParam: INT IDENT  */
#line 349 "parser/SysY_parser.y"
          {
    (yyval.formal) = new __FuncFParam(Type::INT,(yyvsp[0].symbol_token),nullptr);
    (yyval.formal)->SetLineNumber(line_number);
}
#line 1870 "SysY_parser.tab.c"
    break;

  case 48: /* FuncFParam: FLOAT IDENT  */
#line 353 "parser/SysY_parser.y"
            {
    (yyval.formal) = new __FuncFParam(Type::FLOAT,(yyvsp[0].symbol_token),nullptr);
    (yyval.formal)->SetLineNumber(line_number);
}
#line 1879 "SysY_parser.tab.c"
    break;

  case 49: /* FuncFParam: INT IDENT '[' ']' Array_list  */
#line 357 "parser/SysY_parser.y"
                             {    // 二维数组
    (yyvsp[0].expressions)->insert((yyvsp[0].expressions)->begin(),nullptr);
    (yyval.formal) = new __FuncFParam(Type::INT,(yyvsp[-3].symbol_token),(yyvsp[0].expressions));
    (yyval.formal)->SetLineNumber(line_number);
}
#line 1889 "SysY_parser.tab.c"
    break;

  case 50: /* FuncFParam: FLOAT IDENT '[' ']' Array_list  */
#line 362 "parser/SysY_parser.y"
                               {
    (yyvsp[0].expressions)->insert((yyvsp[0].expressions)->begin(),nullptr);
    (yyval.formal) = new __FuncFParam(Type::FLOAT,(yyvsp[-3].symbol_token),(yyvsp[0].expressions));
    (yyval.formal)->SetLineNumber(line_number);
}
#line 1899 "SysY_parser.tab.c"
    break;

  case 51: /* FuncFParam: INT IDENT '[' ']'  */
#line 367 "parser/SysY_parser.y"
                  {    // 一维数组
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(nullptr);
    (yyval.formal) = new __FuncFParam(Type::INT,(yyvsp[-2].symbol_token),temp);
    (yyval.formal)->SetLineNumber(line_number);
}
#line 1910 "SysY_parser.tab.c"
    break;

  case 52: /* FuncFParam: INT IDENT '[' Exp ']'  */
#line 374 "parser/SysY_parser.y"
{
    std::vector<Expression>* temp = new std::vector<Expression>; 
    temp->push_back(new Exp((yyvsp[-1].expression)));
    (yyval.formal) = new __FuncFParam(Type::INT,(yyvsp[-3].symbol_token),temp);
    (yyval.formal)->SetLineNumber(line_number); 

}
#line 1922 "SysY_parser.tab.c"
    break;

  case 53: /* FuncFParam: FLOAT IDENT '[' Exp ']'  */
#line 382 "parser/SysY_parser.y"
{
    std::vector<Expression>* temp = new std::vector<Expression>; 
    temp->push_back(new Exp((yyvsp[-1].expression)));
    (yyval.formal) = new __FuncFParam(Type::FLOAT,(yyvsp[-3].symbol_token),temp);
    (yyval.formal)->SetLineNumber(line_number); 

}
#line 1934 "SysY_parser.tab.c"
    break;

  case 54: /* FuncFParam: FLOAT IDENT '[' ']'  */
#line 389 "parser/SysY_parser.y"
                    {
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(nullptr);
    (yyval.formal) = new __FuncFParam(Type::FLOAT,(yyvsp[-2].symbol_token),temp);
    (yyval.formal)->SetLineNumber(line_number);
}
#line 1945 "SysY_parser.tab.c"
    break;

  case 55: /* FuncFParam: INT IDENT '[' ']' '[' ']'  */
#line 395 "parser/SysY_parser.y"
                          {
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(nullptr);
    //std::vector<Expression>* temp1 = new std::vector<Expression>;
    //temp1->push_back(nullptr);
    (yyval.formal) = new __FuncFParam(Type::FLOAT,(yyvsp[-4].symbol_token),temp);
    (yyval.formal)->SetLineNumber(line_number);
}
#line 1958 "SysY_parser.tab.c"
    break;

  case 56: /* FuncFParam: INT IDENT '[' Exp ']' '[' Exp ']'  */
#line 403 "parser/SysY_parser.y"
{
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(new Exp((yyvsp[-4].expression)));
    temp->push_back(new Exp((yyvsp[-1].expression)));
    (yyval.formal) = new __FuncFParam(Type::INT,(yyvsp[-6].symbol_token),temp);
    (yyval.formal)->SetLineNumber(line_number);  
}
#line 1970 "SysY_parser.tab.c"
    break;

  case 57: /* FuncFParam: FLOAT IDENT '[' Exp ']' '[' Exp ']'  */
#line 411 "parser/SysY_parser.y"
{
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(new Exp((yyvsp[-4].expression)));
    temp->push_back(new Exp((yyvsp[-1].expression)));
    (yyval.formal) = new __FuncFParam(Type::FLOAT,(yyvsp[-6].symbol_token),temp);
    (yyval.formal)->SetLineNumber(line_number);  
}
#line 1982 "SysY_parser.tab.c"
    break;

  case 58: /* Block: '{' BlockItem_list '}'  */
#line 423 "parser/SysY_parser.y"
                       { 
    (yyval.block) = new __Block((yyvsp[-1].block_items));
    (yyval.block)->SetLineNumber(line_number);
}
#line 1991 "SysY_parser.tab.c"
    break;

  case 59: /* Block: '{' '}'  */
#line 427 "parser/SysY_parser.y"
        { 
    (yyval.block) = new __Block(new std::vector<BlockItem>);
    (yyval.block)->SetLineNumber(line_number);
}
#line 2000 "SysY_parser.tab.c"
    break;

  case 60: /* BlockItem_list: BlockItem  */
#line 435 "parser/SysY_parser.y"
          { 
    (yyval.block_items) = new std::vector<BlockItem>;
    ((yyval.block_items))->push_back((yyvsp[0].block_item));
}
#line 2009 "SysY_parser.tab.c"
    break;

  case 61: /* BlockItem_list: BlockItem_list BlockItem  */
#line 439 "parser/SysY_parser.y"
                         {
    ((yyvsp[-1].block_items))->push_back((yyvsp[0].block_item));
    (yyval.block_items) = (yyvsp[-1].block_items);
}
#line 2018 "SysY_parser.tab.c"
    break;

  case 62: /* BlockItem: Decl  */
#line 447 "parser/SysY_parser.y"
     {
    (yyval.block_item) = new BlockItem_Decl((yyvsp[0].decl));
    (yyval.block_item)->SetLineNumber(line_number);
}
#line 2027 "SysY_parser.tab.c"
    break;

  case 63: /* BlockItem: Stmt  */
#line 451 "parser/SysY_parser.y"
     {
    (yyval.block_item) = new BlockItem_Stmt((yyvsp[0].stmt));
    (yyval.block_item)->SetLineNumber(line_number);
}
#line 2036 "SysY_parser.tab.c"
    break;

  case 64: /* Stmt: ';'  */
#line 459 "parser/SysY_parser.y"
    { //空
    (yyval.stmt) = new null_stmt();
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2045 "SysY_parser.tab.c"
    break;

  case 65: /* Stmt: Exp ';'  */
#line 463 "parser/SysY_parser.y"
        { //赋值语句
    (yyval.stmt) = new expr_stmt((yyvsp[-1].expression));
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2054 "SysY_parser.tab.c"
    break;

  case 66: /* Stmt: Lval '=' Exp ';'  */
#line 467 "parser/SysY_parser.y"
                 { //左值赋值
    (yyval.stmt) = new assign_stmt((yyvsp[-3].expression),(yyvsp[-1].expression));
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2063 "SysY_parser.tab.c"
    break;

  case 67: /* Stmt: Lval AQ Exp ';'  */
#line 471 "parser/SysY_parser.y"
                {
    auto add = new AddExp_plus((yyvsp[-3].expression),(yyvsp[-1].expression)); 
    (yyval.stmt) = new assign_stmt((yyvsp[-3].expression),add);
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2073 "SysY_parser.tab.c"
    break;

  case 68: /* Stmt: Lval SQ Exp ';'  */
#line 476 "parser/SysY_parser.y"
                {
    auto sub = new AddExp_sub((yyvsp[-3].expression),(yyvsp[-1].expression)); 
    (yyval.stmt) = new assign_stmt((yyvsp[-3].expression),sub);
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2083 "SysY_parser.tab.c"
    break;

  case 69: /* Stmt: Lval MQ Exp ';'  */
#line 481 "parser/SysY_parser.y"
                {
    auto mul = new MulExp_mul((yyvsp[-3].expression),(yyvsp[-1].expression));
    (yyval.stmt) = new assign_stmt((yyvsp[-3].expression),mul);
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2093 "SysY_parser.tab.c"
    break;

  case 70: /* Stmt: Lval DQ Exp ';'  */
#line 486 "parser/SysY_parser.y"
                {
    auto div = new MulExp_div((yyvsp[-3].expression),(yyvsp[-1].expression));
    (yyval.stmt) = new assign_stmt((yyvsp[-3].expression),div);
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2103 "SysY_parser.tab.c"
    break;

  case 71: /* Stmt: Lval MODQ Exp ';'  */
#line 491 "parser/SysY_parser.y"
                  {
    auto mod = new MulExp_mod((yyvsp[-3].expression),(yyvsp[-1].expression));
    (yyval.stmt) = new assign_stmt((yyvsp[-3].expression),mod);
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2113 "SysY_parser.tab.c"
    break;

  case 72: /* Stmt: Block  */
#line 497 "parser/SysY_parser.y"
      { //函数体
    (yyval.stmt) = new block_stmt((yyvsp[0].block));
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2122 "SysY_parser.tab.c"
    break;

  case 73: /* Stmt: IF '(' Cond ')' Stmt  */
#line 502 "parser/SysY_parser.y"
                                { //if语句，没有else语句所以返回空指针
    (yyval.stmt) = new if_stmt((yyvsp[-2].expression),(yyvsp[0].stmt));
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2131 "SysY_parser.tab.c"
    break;

  case 74: /* Stmt: IF '(' Cond ')' Stmt ELSE Stmt  */
#line 506 "parser/SysY_parser.y"
                               { //if-else语句
    (yyval.stmt) = new ifelse_stmt((yyvsp[-4].expression),(yyvsp[-2].stmt),(yyvsp[0].stmt));
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2140 "SysY_parser.tab.c"
    break;

  case 75: /* Stmt: WHILE '(' Cond ')' Stmt  */
#line 510 "parser/SysY_parser.y"
                        { //while语句
    (yyval.stmt) = new while_stmt((yyvsp[-2].expression),(yyvsp[0].stmt));
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2149 "SysY_parser.tab.c"
    break;

  case 76: /* Stmt: FOR '(' VarDecl Cond ';' Lval '=' Exp ')' Stmt  */
#line 518 "parser/SysY_parser.y"
                                                {
    auto n = new assign_stmt((yyvsp[-4].expression),(yyvsp[-2].expression));
    (yyval.stmt) = new for_stmt((yyvsp[-7].decl),(yyvsp[-6].expression),n,(yyvsp[0].stmt));
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2159 "SysY_parser.tab.c"
    break;

  case 77: /* Stmt: BREAK ';'  */
#line 523 "parser/SysY_parser.y"
          {
    (yyval.stmt) = new break_stmt();
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2168 "SysY_parser.tab.c"
    break;

  case 78: /* Stmt: CONTINUE ';'  */
#line 527 "parser/SysY_parser.y"
             {
    (yyval.stmt) = new continue_stmt();
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2177 "SysY_parser.tab.c"
    break;

  case 79: /* Stmt: RETURN Exp ';'  */
#line 531 "parser/SysY_parser.y"
               {
    (yyval.stmt) = new return_stmt((yyvsp[-1].expression));
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2186 "SysY_parser.tab.c"
    break;

  case 80: /* Stmt: RETURN ';'  */
#line 535 "parser/SysY_parser.y"
           {
    (yyval.stmt) = new return_stmt_void();
    (yyval.stmt)->SetLineNumber(line_number);
}
#line 2195 "SysY_parser.tab.c"
    break;

  case 81: /* Exp: AddExp  */
#line 543 "parser/SysY_parser.y"
       {(yyval.expression) = (yyvsp[0].expression); (yyval.expression)->SetLineNumber(line_number);}
#line 2201 "SysY_parser.tab.c"
    break;

  case 82: /* Cond: LOrExp  */
#line 548 "parser/SysY_parser.y"
       {(yyval.expression) = (yyvsp[0].expression); (yyval.expression)->SetLineNumber(line_number);}
#line 2207 "SysY_parser.tab.c"
    break;

  case 83: /* Lval: IDENT  */
#line 553 "parser/SysY_parser.y"
      { //标识符
    (yyval.expression) = new Lval((yyvsp[0].symbol_token),nullptr);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2216 "SysY_parser.tab.c"
    break;

  case 84: /* Lval: IDENT Array_list  */
#line 557 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new Lval((yyvsp[-1].symbol_token),(yyvsp[0].expressions));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2225 "SysY_parser.tab.c"
    break;

  case 85: /* PrimaryExp: '(' Exp ')'  */
#line 565 "parser/SysY_parser.y"
            {
    (yyval.expression) = new PrimaryExp_branch((yyvsp[-1].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2234 "SysY_parser.tab.c"
    break;

  case 86: /* PrimaryExp: Lval  */
#line 569 "parser/SysY_parser.y"
     {
    (yyval.expression) = (yyvsp[0].expression); 
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2243 "SysY_parser.tab.c"
    break;

  case 87: /* PrimaryExp: IntConst  */
#line 573 "parser/SysY_parser.y"
         {
    (yyval.expression) = (yyvsp[0].expression); 
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2252 "SysY_parser.tab.c"
    break;

  case 88: /* PrimaryExp: FloatConst  */
#line 577 "parser/SysY_parser.y"
           {
    (yyval.expression) = (yyvsp[0].expression); 
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2261 "SysY_parser.tab.c"
    break;

  case 89: /* IntConst: INT_CONST  */
#line 584 "parser/SysY_parser.y"
          {
    (yyval.expression) = new IntConst((yyvsp[0].int_token));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2270 "SysY_parser.tab.c"
    break;

  case 90: /* FloatConst: FLOAT_CONST  */
#line 591 "parser/SysY_parser.y"
            {
    (yyval.expression) = new FloatConst((yyvsp[0].float_token));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2279 "SysY_parser.tab.c"
    break;

  case 91: /* UnaryExp: PrimaryExp  */
#line 599 "parser/SysY_parser.y"
           {(yyval.expression) = (yyvsp[0].expression);}
#line 2285 "SysY_parser.tab.c"
    break;

  case 92: /* UnaryExp: IDENT '(' FuncRParams ')'  */
#line 600 "parser/SysY_parser.y"
                          {
    (yyval.expression) = new Func_call((yyvsp[-3].symbol_token),(yyvsp[-1].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2294 "SysY_parser.tab.c"
    break;

  case 93: /* UnaryExp: IDENT '(' ')'  */
#line 604 "parser/SysY_parser.y"
              {
    // 在sylib.h这个文件中,starttime()是一个宏定义
    // #define starttime() _sysy_starttime(__LINE__)
    // 我们在语法分析中将其替换为_sysy_starttime(line_number)
    // stoptime同理
    if((yyvsp[-2].symbol_token)->get_string() == "starttime"){
        auto params = new std::vector<Expression>;
        params->push_back(new IntConst(line_number));
        Expression temp = new FuncRParams(params);
        (yyval.expression) = new Func_call(id_table.add_id("_sysy_starttime"),temp);
        (yyval.expression)->SetLineNumber(line_number);
    }
    else if((yyvsp[-2].symbol_token)->get_string() == "stoptime"){
        auto params = new std::vector<Expression>;
        params->push_back(new IntConst(line_number));
        Expression temp = new FuncRParams(params);
        (yyval.expression) = new Func_call(id_table.add_id("_sysy_stoptime"),temp);
        (yyval.expression)->SetLineNumber(line_number);
    }
    else{
        (yyval.expression) = new Func_call((yyvsp[-2].symbol_token),nullptr);
        (yyval.expression)->SetLineNumber(line_number);
    }
}
#line 2323 "SysY_parser.tab.c"
    break;

  case 94: /* UnaryExp: '+' UnaryExp  */
#line 628 "parser/SysY_parser.y"
             {
    (yyval.expression) = new UnaryExp_plus((yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2332 "SysY_parser.tab.c"
    break;

  case 95: /* UnaryExp: '-' UnaryExp  */
#line 632 "parser/SysY_parser.y"
             {
    (yyval.expression) = new UnaryExp_neg((yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2341 "SysY_parser.tab.c"
    break;

  case 96: /* UnaryExp: '!' UnaryExp  */
#line 636 "parser/SysY_parser.y"
             {
    (yyval.expression) = new UnaryExp_not((yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2350 "SysY_parser.tab.c"
    break;

  case 97: /* FuncRParams: Exp_list  */
#line 644 "parser/SysY_parser.y"
         { //为空
    (yyval.expression) = new FuncRParams((yyvsp[0].expressions));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2359 "SysY_parser.tab.c"
    break;

  case 98: /* Exp_list: Exp  */
#line 652 "parser/SysY_parser.y"
    {
    (yyval.expressions) = new std::vector<Expression>;
    ((yyval.expressions))->push_back((yyvsp[0].expression));
}
#line 2368 "SysY_parser.tab.c"
    break;

  case 99: /* Exp_list: Exp_list ',' Exp  */
#line 656 "parser/SysY_parser.y"
                 {
    ((yyvsp[-2].expressions))->push_back((yyvsp[0].expression));
    (yyval.expressions) = (yyvsp[-2].expressions);
}
#line 2377 "SysY_parser.tab.c"
    break;

  case 100: /* MulExp: UnaryExp  */
#line 664 "parser/SysY_parser.y"
         {
    (yyval.expression) = (yyvsp[0].expression);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2386 "SysY_parser.tab.c"
    break;

  case 101: /* MulExp: MulExp '*' UnaryExp  */
#line 668 "parser/SysY_parser.y"
                    {
    (yyval.expression) = new MulExp_mul((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2395 "SysY_parser.tab.c"
    break;

  case 102: /* MulExp: MulExp '/' UnaryExp  */
#line 672 "parser/SysY_parser.y"
                    {
    (yyval.expression) = new MulExp_div((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2404 "SysY_parser.tab.c"
    break;

  case 103: /* MulExp: MulExp '%' UnaryExp  */
#line 676 "parser/SysY_parser.y"
                    {
    (yyval.expression) = new MulExp_mod((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2413 "SysY_parser.tab.c"
    break;

  case 104: /* AddExp: MulExp  */
#line 684 "parser/SysY_parser.y"
       {
    (yyval.expression) = (yyvsp[0].expression);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2422 "SysY_parser.tab.c"
    break;

  case 105: /* AddExp: AddExp '+' MulExp  */
#line 688 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new AddExp_plus((yyvsp[-2].expression),(yyvsp[0].expression)); 
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2431 "SysY_parser.tab.c"
    break;

  case 106: /* AddExp: AddExp '-' MulExp  */
#line 692 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new AddExp_sub((yyvsp[-2].expression),(yyvsp[0].expression)); 
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2440 "SysY_parser.tab.c"
    break;

  case 107: /* RelExp: AddExp  */
#line 700 "parser/SysY_parser.y"
       {
    (yyval.expression) = (yyvsp[0].expression);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2449 "SysY_parser.tab.c"
    break;

  case 108: /* RelExp: RelExp '<' AddExp  */
#line 704 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new RelExp_lt((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2458 "SysY_parser.tab.c"
    break;

  case 109: /* RelExp: RelExp '>' AddExp  */
#line 708 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new RelExp_gt((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2467 "SysY_parser.tab.c"
    break;

  case 110: /* RelExp: RelExp LEQ AddExp  */
#line 712 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new RelExp_leq((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2476 "SysY_parser.tab.c"
    break;

  case 111: /* RelExp: RelExp GEQ AddExp  */
#line 716 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new RelExp_geq((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2485 "SysY_parser.tab.c"
    break;

  case 112: /* EqExp: RelExp  */
#line 724 "parser/SysY_parser.y"
       {
    (yyval.expression) = (yyvsp[0].expression);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2494 "SysY_parser.tab.c"
    break;

  case 113: /* EqExp: EqExp EQ RelExp  */
#line 728 "parser/SysY_parser.y"
                {
    (yyval.expression) = new EqExp_eq((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2503 "SysY_parser.tab.c"
    break;

  case 114: /* EqExp: EqExp NE RelExp  */
#line 732 "parser/SysY_parser.y"
                {
    (yyval.expression) = new EqExp_neq((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2512 "SysY_parser.tab.c"
    break;

  case 115: /* LAndExp: EqExp  */
#line 740 "parser/SysY_parser.y"
      {
    (yyval.expression) = (yyvsp[0].expression);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2521 "SysY_parser.tab.c"
    break;

  case 116: /* LAndExp: LAndExp AND EqExp  */
#line 744 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new LAndExp_and((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2530 "SysY_parser.tab.c"
    break;

  case 117: /* LOrExp: LAndExp  */
#line 752 "parser/SysY_parser.y"
        {
    (yyval.expression) = (yyvsp[0].expression);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2539 "SysY_parser.tab.c"
    break;

  case 118: /* LOrExp: LOrExp OR LAndExp  */
#line 756 "parser/SysY_parser.y"
                  {
    (yyval.expression) = new LOrExp_or((yyvsp[-2].expression), (yyvsp[0].expression));
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2548 "SysY_parser.tab.c"
    break;

  case 119: /* ConstExp: AddExp  */
#line 764 "parser/SysY_parser.y"
       {
    (yyval.expression) = (yyvsp[0].expression);
    (yyval.expression)->SetLineNumber(line_number);
}
#line 2557 "SysY_parser.tab.c"
    break;


#line 2561 "SysY_parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 773 "parser/SysY_parser.y"
 

void yyerror(char* s, ...)
{
    ++error_num;
    fout<<"parser error in line "<<line_number<<"\n";
}
