#ifndef __TEXTSIZE_H
#define __TEXTSIZE_H

#if defined __x86_64__ && !defined __ILP32__
#define __TEXTSIZE	64
#else
#define __TEXTSIZE	32
#endif

#endif