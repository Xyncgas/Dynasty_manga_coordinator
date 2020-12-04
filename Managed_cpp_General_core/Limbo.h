#pragma once
#ifdef ROOT_MODEL_nests
auto ROOT_ =
std::make_shared<
	LF::el_nests<P0,
	LF::el_nests<P1,
	LF::el_nests<Px_end,
	LF::el_end>>>
	>();//in function.h you now have the possibility to take a pointer of the last prcessor instance
//You can have another style that's like reversed order, although you gotta change templated_nest too, so instead of next object taking pointer of current object you are taking pointer of the next object in current object. The first case is more traditional and is taking return and stuff, so the second case I guess is more functional and would be calling next function from the last function...
#endif
//can't be both on though
///After declaring ROOT_, it would be responsible to make and maintian some sort of macros too.
#define ROOT_L0 (*ROOT).c
#define ROOT_L1 (*ROOT)(.n).c
#define ROOT_L2 (*ROOT)(.n)(.n).c
#undef el_snx