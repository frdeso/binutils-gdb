/* as.h - global header file
   Copyright (C) 1987, 1990, 1991, 1992 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#define GAS 1
#include <ansidecl.h>
#include "host.h"
#include "flonum.h"

#ifndef __STDC__
#define	volatile		/**/
#ifndef const
#define	const			/**/
#endif /* const */
#endif /* __STDC__ */

#ifndef __LINE__
#define __LINE__ "unknown"
#endif /* __LINE__ */

#ifndef __FILE__
#define __FILE__ "unknown"
#endif /* __FILE__ */

/*
 * I think this stuff is largely out of date.  xoxorich.
 *
 * CAPITALISED names are #defined.
 * "lowercaseH" is #defined if "lowercase.h" has been #include-d.
 * "lowercaseT" is a typedef of "lowercase" objects.
 * "lowercaseP" is type "pointer to object of type 'lowercase'".
 * "lowercaseS" is typedef struct ... lowercaseS.
 *
 * #define DEBUG to enable all the "know" assertion tests.
 * #define SUSPECT when debugging.
 * #define COMMON as "extern" for all modules except one, where you #define
 *	COMMON as "".
 * If TEST is #defined, then we are testing a module: #define COMMON as "".
 */

/* These #defines are for parameters of entire assembler. */

/* #define SUSPECT JF remove for speed testing */
/* These #includes are for type definitions etc. */

#include <stdio.h>
#include <assert.h>

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free xfree

#define xfree free

#define BAD_CASE(value) \
{ \
      as_fatal("Case value %d unexpected at line %d of file \"%s\"\n", \
	       value, __LINE__, __FILE__); \
	   }


/* These are assembler-wide concepts */


#ifndef COMMON
#ifdef TEST
#define COMMON			/* declare our COMMONs storage here. */
#else
#define COMMON extern		/* our commons live elswhere */
#endif
#endif
/* COMMON now defined */
#define DEBUG			/* temporary */

#ifdef BROKEN_ASSERT
/* Turn off all assertion checks, on machines where the assert macro
   is buggy.  (For example, on the RS/6000, Reiser-cpp substitution is
   done to put the condition into a string, so if the condition contains
   a string, parse errors result.)  */
#undef DEBUG
#define NDEBUG
#endif

#ifdef DEBUG
#undef NDEBUG
#ifndef know
#define know(p) assert(p)	/* Verify our assumptions! */
#endif /* not yet defined */
#else
#define know(p)			/* know() checks are no-op.ed */
#endif

/* input_scrub.c */

/*
 * Supplies sanitised buffers to read.c.
 * Also understands printing line-number part of error messages.
 */


/* subsegs.c     Sub-segments. Also, segment(=expression type)s.*/

/*
 * This table describes the use of segments as EXPRESSION types.
 *
 *	X_seg	X_add_symbol  X_subtract_symbol	X_add_number
 * SEG_ABSENT						no (legal) expression
 * SEG_PASS1						no (defined) "
 * SEG_BIG					*	> 32 bits const.
 * SEG_ABSOLUTE				     	0
 * SEG_DATA		*		     	0
 * SEG_TEXT		*			0
 * SEG_BSS		*			0
 * SEG_UNKNOWN		*			0
 * SEG_DIFFERENCE	0		*	0
 * SEG_REGISTER					*
 *
 * The blank fields MUST be 0, and are nugatory.
 * The '0' fields MAY be 0. The '*' fields MAY NOT be 0.
 *
 * SEG_BIG: X_add_number is < 0 if the result is in
 *	generic_floating_point_number.  The value is -'c' where c is the
 *	character that introduced the constant.  e.g. "0f6.9" will have  -'f'
 *	as a X_add_number value.
 *	X_add_number > 0 is a count of how many littlenums it took to
 *	represent a bignum.
 * SEG_DIFFERENCE:
 * If segments of both symbols are known, they are the same segment.
 * X_add_symbol != X_sub_symbol (then we just cancel them, => SEG_ABSOLUTE).
 */


#ifdef MANY_SEGMENTS
#include "bfd.h"
#define N_SEGMENTS 10
#define SEG_NORMAL(x) ((x) >= SEG_E0 && (x) <= SEG_E9)
#define SEG_LIST SEG_E0,SEG_E1,SEG_E2,SEG_E3,SEG_E4,SEG_E5,SEG_E6,SEG_E7,SEG_E8,SEG_E9
#define SEG_DATA SEG_E1
#define SEG_TEXT SEG_E0
#define SEG_BSS SEG_E2
#else
#define N_SEGMENTS 3
#define SEG_NORMAL(x) ((x) == SEG_TEXT || (x) == SEG_DATA || (x) == SEG_BSS)
#define SEG_LIST SEG_TEXT,SEG_DATA,SEG_BSS
#endif

typedef enum _segT
  {
    SEG_ABSOLUTE = 0,
    SEG_LIST,
    SEG_UNKNOWN,
    SEG_ABSENT,			/* Mythical Segment (absent): NO expression seen. */
    SEG_PASS1,			/* Mythical Segment: Need another pass. */
    SEG_GOOF,			/* Only happens if AS has a logic error. */
    /* Invented so we don't crash printing */
    /* error message involving weird segment. */
    SEG_BIG,			/* Bigger than 32 bits constant. */
    SEG_DIFFERENCE,		/* Mythical Segment: absolute difference. */
    SEG_DEBUG,			/* Debug segment */
    SEG_NTV,			/* Transfert vector preload segment */
    SEG_PTV,			/* Transfert vector postload segment */
    SEG_REGISTER,		/* Mythical: a register-valued expression */
  } segT;

#define SEG_MAXIMUM_ORDINAL (SEG_REGISTER)

typedef int subsegT;

/* What subseg we are accreting now? */
COMMON subsegT now_subseg;

/* Segment our instructions emit to. */
/* Only OK values are SEG_TEXT or SEG_DATA. */
COMMON segT now_seg;

extern char *const seg_name[];
extern int section_alignment[];


/* relax() */

typedef enum _relax_state
  {
    /* Variable chars to be repeated fr_offset times.
       Fr_symbol unused. Used with fr_offset == 0 for a
       constant length frag. */
    rs_fill = 1,

    /* Align: Fr_offset: power of 2. 1 variable char: fill character. */
    rs_align,

    /* Org: Fr_offset, fr_symbol: address. 1 variable char: fill
       character. */
    rs_org,

    rs_machine_dependent,

#ifndef WORKING_DOT_WORD
    /* JF: gunpoint */
    rs_broken_word,
#endif
  } relax_stateT;

/* typedef unsigned char relax_substateT; */
/* JF this is more likely to leave the end of a struct frag on an align
   boundry.  Be very careful with this.  */
typedef unsigned long relax_substateT;

/* Enough bits for address, but still an integer type.
   Could be a problem, cross-assembling for 64-bit machines.  */
typedef unsigned long relax_addressT;


/* frags.c */

/*
 * A code fragment (frag) is some known number of chars, followed by some
 * unknown number of chars. Typically the unknown number of chars is an
 * instruction address whose size is yet unknown. We always know the greatest
 * possible size the unknown number of chars may become, and reserve that
 * much room at the end of the frag.
 * Once created, frags do not change address during assembly.
 * We chain the frags in (a) forward-linked list(s). The object-file address
 * of the 1st char of a frag is generally not known until after relax().
 * Many things at assembly time describe an address by {object-file-address
 * of a particular frag}+offset.

 BUG: it may be smarter to have a single pointer off to various different
 notes for different frag kinds. See how code pans
 */
struct frag			/* a code fragment */
{
  /* Object file address. */
  unsigned long fr_address;
  /* Chain forward; ascending address order. */
  /* Rooted in frch_root. */
  struct frag *fr_next;

  /* (Fixed) number of chars we know we have. */
  /* May be 0. */
  long fr_fix;
  /* (Variable) number of chars after above. */
  /* May be 0. */
  long fr_var;
  /* For variable-length tail. */
  struct symbol *fr_symbol;
  /* For variable-length tail. */
  long fr_offset;
  /*->opcode low addr byte,for relax()ation*/
  char *fr_opcode;
  /* What state is my tail in? */
  relax_stateT fr_type;
  relax_substateT fr_subtype;

  /* These are needed only on the NS32K machines */
  char fr_pcrel_adjust;
  char fr_bsr;
#ifndef NO_LISTING
  struct list_info_struct *line;
#endif
  /* Chars begin here.
     One day we will compile fr_literal[0]. */
  char fr_literal[1];
};

#define SIZEOF_STRUCT_FRAG \
((int)zero_address_frag.fr_literal-(int)&zero_address_frag)
/* We want to say fr_literal[0] above. */

typedef struct frag fragS;

/* Current frag we are building.  This frag is incomplete.  It is, however,
   included in frchain_now.  The fr_fix field is bogus; instead, use:
   obstack_next_free(&frags)-frag_now->fr_literal.  */
COMMON fragS *frag_now;

/* For foreign-segment symbol fixups. */
COMMON fragS zero_address_frag;
/* For local common (N_BSS segment) fixups. */
COMMON fragS bss_address_frag;

/* main program "as.c" (command arguments etc) */

/* ['x'] TRUE if "-x" seen. */
COMMON char flagseen[128];
COMMON unsigned char flag_readonly_data_in_text;
COMMON unsigned char flag_suppress_warnings;
COMMON unsigned char flag_always_generate_output;

/* name of emitted object file */
COMMON char *out_file_name;

/* TRUE if we need a second pass. */
COMMON int need_pass_2;

/* TRUE if we should do no relaxing, and
   leave lots of padding.  */
COMMON int linkrelax;

struct _pseudo_type
  {
    /* assembler mnemonic, lower case, no '.' */
    char *poc_name;
    /* Do the work */
    void (*poc_handler) ();
    /* Value to pass to handler */
    int poc_val;
  };

typedef struct _pseudo_type pseudo_typeS;

#ifndef NO_STDARG

int had_errors PARAMS ((void));
int had_warnings PARAMS ((void));
void as_bad PARAMS ((const char *Format,...));
void as_fatal PARAMS ((const char *Format,...));
void as_tsktsk PARAMS ((const char *Format,...));
void as_warn PARAMS ((const char *Format,...));

#else

int had_errors ();
int had_warnings ();
void as_bad ();
void as_fatal ();
void as_tsktsk ();
void as_warn ();

#endif /* __STDC__ & !NO_STDARG */

char *app_push PARAMS ((void));
char *atof_ieee PARAMS ((char *str, int what_kind, LITTLENUM_TYPE * words));
char *input_scrub_include_file PARAMS ((char *filename, char *position));
char *input_scrub_new_file PARAMS ((char *filename));
char *input_scrub_next_buffer PARAMS ((char **bufp));
char *strstr PARAMS ((const char *s, const char *wanted));
char *xmalloc PARAMS ((long size));
char *xrealloc PARAMS ((char *ptr, long n));
int do_scrub_next_char PARAMS ((int (*get) (), void (*unget) ()));
int gen_to_words PARAMS ((LITTLENUM_TYPE * words, int precision,
			  long exponent_bits));
int had_err PARAMS ((void));
int had_errors PARAMS ((void));
int had_warnings PARAMS ((void));
int ignore_input PARAMS ((void));
int scrub_from_file PARAMS ((void));
int scrub_from_file PARAMS ((void));
int scrub_from_string PARAMS ((void));
int seen_at_least_1_file PARAMS ((void));
void app_pop PARAMS ((char *arg));
void as_howmuch PARAMS ((FILE * stream));
void as_perror PARAMS ((char *gripe, char *filename));
void as_where PARAMS ((void));
void bump_line_counters PARAMS ((void));
void do_scrub_begin PARAMS ((void));
void input_scrub_begin PARAMS ((void));
void input_scrub_close PARAMS ((void));
void input_scrub_end PARAMS ((void));
void int_to_gen PARAMS ((long x));
void new_logical_line PARAMS ((char *fname, int line_number));
void scrub_to_file PARAMS ((int ch));
void scrub_to_string PARAMS ((int ch));
void subseg_change PARAMS ((segT seg, int subseg));
void subseg_new PARAMS ((segT seg, subsegT subseg));
void subsegs_begin PARAMS ((void));

/* this one starts the chain of target dependant headers */
#include "targ-env.h"

/* these define types needed by the interfaces */
#include "struc-symbol.h"

#include "write.h"
#include "expr.h"
#include "frags.h"
#include "hash.h"
#include "read.h"
#include "symbols.h"

#include "tc.h"
#include "obj.h"

#include "listing.h"

/* end of as.h */
