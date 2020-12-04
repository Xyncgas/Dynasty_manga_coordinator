#pragma once
namespace LF {
    typedef bool el_end;
    template<typename _T1, typename _T2>
    struct el_nests {
        _T1 c;//current
        _T2 n;//next/nests
    };
    template<typename _T1>//initialize bool value for el_end (the lowest level)
    struct el_nests<_T1, el_end> {
        _T1 c;//current
        bool n = true;//next/nests
    };
    //example :
    //below codes would produce a big struct that has only 2 elements but many layers
    //el_nests<int,el_nests<int,el_nests<int,el_nests<int,el_nests<int,el_nests<int,el_nests<int,el_nests<int,el_nests<int,bool>>>>>>>>>
    template<typename _T1, int _C, typename _T2>
    struct managed_el_nests {
        int p = _C;//initialized and perhaps for labling
        _T1 c;
        _T2 n;
    };
    template <typename _T1, int _C>//initialize bool value for el_end (the lowest level)
    struct managed_el_nests<_T1, _C, el_end> {
        int p = _C;//initialized and perhaps for labling
        _T1 c;
        bool n = true;
    };
    //can automate code generation with helper
    //imagine a programing style where data is stored in one struct and has many levels of access
    //getting your access would perhpas be _A.n.n.n.n.n.n.n.n.n.c (10 levels) and can use a macro too -> #define el_n_10(_A) = _A.n.n.n.n.n.n.n.n.n.c
    //the struct is better be static, because if it's dynamic and grows big there's gonna be malloc that I wonder if it's gonna be unefficient, but usually even when you are generating stuffs at run time you would be using std vector classes or something so maybe it's alright
    //maybe this the way to avoid stack overflow too by optionally putting this struct in global and most variables inside, except those of local and fine grain variables.
}
namespace LF{
    using namespace std;
    typedef bool el_beg;
    typedef bool el_end;
    template<typename _T0, typename _T1, typename _T2 >
    struct el_steam {
        _T1 c;//current, in steam application, the most outer nest element can run with default constructor
        _T2 n=nullptr;//next/nests, you are dealing with the constructor of el_nests
        el_steam(_T0* steam) { c = _T1(steam); n = _T2(&c); }
        //so basically what's happening is T1 initiated and nothing happens, then T2 initiated and nothing happens, because the default constructor for these processors was to do nothing (still you are initializing structs only to throw away so be careful don't put too much non static variables in those structs (I guess just a pointer/storage and a function, maybe even discard the storage because you return from the current processor to the next processor for where it was called))
        //Then T1 is Initialized, so will T2 later in the same way
        //object destruction order always inside out
        el_steam() {}//Default constructor that does nothing :doesnt pass c address to n
        _T2* rs() { return &n; }//recursive seek
    };
    template<typename _T1, typename _T2 >
    struct el_steam<el_beg,_T1,_T2> {
        _T1 c;//current, in steam application, the most outer nest element can run with default constructor
        _T2 n;//same declaration too
        el_steam() { n = _T2(&c); }
    };//no seek, no assining to variable c, constructor param decide whether if the el_nest is chained(steam)
    template<typename _T0, typename _T1>//initialize bool value for el_end (the lowest level)
    struct el_steam<_T0,_T1,el_end> {
        _T1 c=nullptr;//current
        bool n = true;//next/nests
        el_steam(_T0* steam) { c =  _T1(steam); }
        el_steam() {}//Default constructor that does nothing :doesnt pass c address to n
        _T1* rs() { return &c; }//recursive seek
    };//no assigning to variable n
}
#if 0
namespace LF {
    typedef bool el_beg;
    typedef bool el_end;
    template<typename _T0, typename _T1, typename _T2 >
    struct el_steam_spread {
        std::make_shared<_T1> c = nullptr;//current, in steam application, the most outer nest element can run with default constructor
        std::make_shared<_T2> n = nullptr;//next/nests, you are dealing with the constructor of el_nests
        el_steam_spread(std::make_shared<_T0> steam) { c = std::make_shared<_T1>(steam); n = std::make_shared<_T2>(c); }
        //so basically what's happening is T1 initiated and nothing happens, then T2 initiated and nothing happens, because the default constructor for these processors was to do nothing (still you are initializing structs only to throw away so be careful don't put too much non static variables in those structs (I guess just a pointer/storage and a function, maybe even discard the storage because you return from the current processor to the next processor for where it was called))
        //Then T1 is Initialized, so will T2 later in the same way
        //object destruction order always inside out
        el_steam_spread() {}//Default constructor that does nothing :doesnt pass c address to n
    };
    template<typename _T1, typename _T2 >
    struct el_steam_spread<el_beg, _T1, _T2> {
        std::make_shared<_T1> c;//current, in steam application, the most outer nest element can run with default constructor
        std::make_shared<_T2> n;//same declaration too
        el_steam_spread() { c = std::make_shared<_T1>(); n = std::make_shared<_T2>(c); }
    };//no seek, no assining to variable c, constructor param decide whether if the el_nest is chained(steam)
    template<typename _T0, typename _T1>//initialize bool value for el_end (the lowest level)
    struct el_steam_spread<_T0, _T1, el_end> {
        std::make_shared<_T1> c = nullptr;//current
        bool n = true;//next/nests
        el_steam_spread(std::make_shared<_T0> steam) { c = std::make_shared<_T1>(steam); }
        el_steam_spread() {}//Default constructor that does nothing :doesnt pass c address to n
    };//no assigning to variable n
}
#endif//somehow it doesnt work if placed here for now, instead I pasted it in limbo.