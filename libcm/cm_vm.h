/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_VM_H
#define CM_VM_H

#include <cassert>
#include <type_traits>

#include "emu_api_export.h"

#include "emu_log.h"

#ifdef CM_GENX
#define OFFSET ushort
#else
#define OFFSET uint
#endif /* CM_GENX */

#include "libcm_def.h"
#include "cm_common_macros.h"

#define __GLOBAL(V) thread_local V
#ifndef __GNUC__
    #define CM_NOINLINE_EMU __declspec(noinline)
#else
    #define CM_NOINLINE_EMU __attribute__((noinline))
#endif

template <typename T, uint R, uint C>
class matrix;
template <typename T, uint R, uint C>
class matrix_ref;
template <typename T, uint SZ>
class vector;
template <typename T, uint SZ>
class vector_ref;

/* Basic stream. Non template class. */
class basic_stream {
public:
    virtual int extract_data(void *buf, uint size = 0xffffffff) = 0;  /* extract datas to CmEmulSys::iobuffer */
    virtual void* get_addr(uint i) = 0;                         /* return address of data element */
    virtual void* get_addr_data() = 0;
    virtual void* get_addr_obj() = 0;                         /* return address of this */
    virtual uint get_size_of_element() const = 0;         /* return size of one element */
    virtual uint get_number_of_elements() const = 0;      /* return number of elements */
    virtual uint get_size_data() const = 0;
    virtual uint get_size_object() const =0;
    virtual ~basic_stream() = default;
};

// stream
template <typename T, uint SZ>
class stream: public basic_stream {
        static const bool type_conformable = cmtype<T>::value;
public:
        typedef  T _Type;

        CM_INLINE constexpr int n_elems() const { return SZ; }

        virtual T get(uint i) const = 0; // call to this virtual function won't appear in IL0
        virtual T& getref(uint i) = 0; // call to this virtual function won't appear in IL0
        virtual void* get_addr(uint i) = 0; // call to this virtual function won't appear in IL0
        virtual void* get_addr_data() = 0;
        virtual void* get_addr_obj() =0;
        int extract_data(void *buf,uint size = 0xffffffff);
        virtual uint get_size_of_element() const { return sizeof(T);};
        virtual uint get_number_of_elements() const {return SZ;};
        virtual uint get_size_data() const = 0;
        virtual uint get_size_object() const =0;

        /* merge */
        CM_NOINLINE void merge(const T x, const uint c);
        template <typename T1>  void CM_NOINLINE merge(const stream<T1,SZ> &x, const uint c);
        template <typename T1, typename T2> CM_NOINLINE void merge(const stream<T1,SZ> &x, const stream<T2,SZ> &c);
        template <typename T1> CM_NOINLINE void merge(const T x, const stream<T1,SZ> &c);
        template <typename T1, typename T2> CM_NOINLINE void merge(const stream<T1,SZ>& x, const stream<T2,SZ>& y, const uint c);
        template <typename T1> CM_NOINLINE void merge(const T x,              const stream<T1,SZ>& y, const uint c);
        template <typename T1> CM_NOINLINE  void merge(const stream<T1,SZ>& x, const T y,              const uint c);
        CM_NOINLINE void merge(const T x,              const T y,              const uint c);
        template <typename T1, typename T2, typename T3> CM_NOINLINE void merge(const stream<T1,SZ>& x, const stream<T2,SZ>& y, const stream<T3,SZ>& c);
        template <typename T1, typename T2>  CM_NOINLINE void merge(const T x,              const stream<T1,SZ>& y, const stream<T2,SZ>& c);
        template <typename T1, typename T2> CM_NOINLINE void merge(const stream<T1,SZ>& x, const T y,              const stream<T2,SZ>& c);
        template <typename T1> CM_NOINLINE void merge(const T x,              const T y,              const stream<T1,SZ>& c);

        // any,all
        CM_NOINLINE ushort any( void ) const;
        CM_NOINLINE ushort all( void ) const;

        // for debug
#ifdef CM_DEBUG
        virtual std::string type_name() const = 0;
        virtual std::string obj_name() const = 0;
#endif /* CM_DEBUG */
};

// matrix
template <typename T, uint R, uint C>
class matrix : public stream<T,R*C> {
public:
        template <typename T1, uint R1, uint C1> friend class matrix;
        template <typename T1, uint R1, uint C1> friend class matrix_ref;
        template <typename T1, uint SZ1> friend class vector;
        template <typename T1, uint SZ1> friend class vector_ref;

        enum { SZ = R*C };
        enum { ROWS=R, COLS=C, ELEMS=R*C };

        CM_INLINE constexpr int n_rows() const { return R; }
        CM_INLINE constexpr int n_cols() const { return C; }

        template <uint REP> CM_INLINE
        const vector<T, R*C*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,0,R*C,1>(ioff, joff); };
        template <uint REP, uint W> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,0,W,1>(ioff, joff); };
        template <uint REP, uint VS, uint W> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,VS,W,1>(ioff, joff); };
        template <uint REP, uint VS, uint W, uint HS> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,VS,W,HS>(ioff, joff); };

        virtual T get(uint i) const { return data[i]; }
        virtual T& getref(uint i) { return data[i]; }
        virtual void* get_addr(uint i) { return &data[i]; }
        virtual void* get_addr_data() {
                return this;
        }
        virtual void* get_addr_obj() { return this; }
        virtual uint get_size_data() const {
                return sizeof(*this);
        }
        virtual uint get_size_object() const { return sizeof(*this); }

        CM_NOINLINE T operator () (OFFSET i, OFFSET j) const {
            assert(i < R && j < C);
            return get(i*C+j);
        }

        CM_NOINLINE T& operator () (OFFSET i, OFFSET j) {
            assert(i < R && j < C);
            return data[i*C+j];
        }

        template <typename T1, uint R1, uint C1>
        class inner_hack {
            matrix* m_t;
            OFFSET _i;
        public:
            inner_hack(matrix* m, OFFSET i):m_t(m){_i=i;}
            CM_NOINLINE const T1 operator[](OFFSET j) const{return (*m_t)(_i,j);}
            CM_NOINLINE T1& operator[](OFFSET j){return (*m_t)(_i,j);}

        };

        CM_NOINLINE inner_hack<T,R,C> operator [] (OFFSET i) const {
            return inner_hack<T,R,C>(this,i);
        }

        CM_NOINLINE inner_hack<T,R,C> operator [] (OFFSET i) {
            return inner_hack<T,R,C>(this,i);
        }

        CM_NOINLINE_EMU GFX_EMU_API static void emuKernelParamInit__ (void *tgtObj = nullptr, void *src = nullptr, size_t size = 0) {
            if(tgtObj != nullptr) {
                assert(size == sizeof(T)*R*C);
                new(tgtObj) matrix<T,R,C>(static_cast<const T*>(src));
            }
        }

        // constructor for emu global variable, supporting global variables in emu mode
        CM_NOINLINE matrix(bool isGlobal);

        // constructors
        CM_NOINLINE matrix();
        // CM_NOINLINE matrix(void *ptr);                /* constructor for preload datas from URB */
        template <typename T2> CM_NOINLINE matrix(const T2 initArray[]); // constructor with array initializer
        CM_NOINLINE matrix(const matrix<T,R,C>& src); // copy constructor
        template <typename T2> CM_NOINLINE matrix(const T2& src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix(const matrix<T2,R2,C2>& src, const uint sat = 0 );
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix(const matrix_ref<T2,R2,C2>& src, const uint sat = 0);
        template <typename T2> CM_NOINLINE matrix(const vector<T2,R*C>& src)
        { new (this) matrix<T,R,C>((matrix<T2,1,R*C>&)src); }

        template <typename T2> CM_NOINLINE matrix(const vector_ref<T2,R*C>& src)
        { new (this) matrix<T,R,C>((matrix_ref<T2,1,R*C>&)src); }

        //operator =
        CM_NOINLINE matrix<T,R,C>& operator = (const matrix<T,R,C>& src); // assignment operator
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator = (const T2 src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix<T,R,C>& operator = (const matrix<T2,R2,C2>& src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix<T,R,C>& operator = (const matrix_ref<T2,R2,C2>& src);
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator = (const vector<T2,R*C>& src) { return this->operator=((const matrix<T2,1,R*C>&)src); };
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator = (const vector_ref<T2,R*C>& src) { return this->operator=((const matrix_ref<T2,1,R*C>&)src); };

        //selects
        template <typename T2> CM_NOINLINE vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> format(); // to vector
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T2,R2,C2> format();    // to matrix R2xC2
        template <typename T2> CM_NOINLINE const vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> format() const; // to vector
        template <typename T2, uint R2, uint C2> CM_NOINLINE const matrix_ref<T2,R2,C2> format() const;    // to matrix R2xC2
        vector_ref<T, C> CM_NOINLINE row(OFFSET i);
        matrix_ref<T,R,1> CM_NOINLINE column(OFFSET i);
        template <uint R2, uint RS, uint C2, uint CS> CM_NOINLINE matrix_ref<T,R2,C2> select(OFFSET ioff=0, OFFSET joff=0);
        template <uint R2, uint RS, uint C2, uint CS> CM_NOINLINE const matrix_ref<T,R2,C2> select(OFFSET ioff=0, OFFSET joff=0) const;

        //1D iselect
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index);
#if _MSC_VER >= 1700
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index, std::true_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index, std::false_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index, std::true_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index, std::false_type);
#endif

        //2D iselect
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index_x, const vector<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index_x, const vector_ref<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index_x, const vector<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index_x, const vector_ref<T2,WD>& index_y);
        //end of iselect

        template <uint R2, uint VS, uint WD, uint HS> CM_NOINLINE const vector<T, R2*WD> genx_select(OFFSET ioff=0, OFFSET joff=0);

        matrix_ref<T, R, C> CM_NOINLINE select_all();
        const matrix_ref<T, R, C> CM_NOINLINE select_all() const;

        // operators +=, -=, ...
        #define declare_operation(OP) \
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator OP (const T2 x);\
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix<T,R,C>& operator OP (const matrix<T2,R2,C2>& x);\
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix<T,R,C>& operator OP (const matrix_ref<T2,R2,C2>& x);\
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator OP (const vector<T2,SZ>& x);\
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator OP (const vector_ref<T2,SZ>& x);\

        declare_operation(+=)     // +=
        declare_operation(-=)     // -=
        declare_operation(*=)     // *=
        declare_operation(/=)     // /=
        declare_operation(%=)     // %=
        declare_operation(&=)     // &=
        declare_operation(|=)     // |=
        declare_operation(^=)     // ^=
        declare_operation(>>=)     // >>=
        declare_operation(<<=)     // <<=
        #undef declare_operation

        // for debug
        uint id() const { return number; }

#ifdef CM_DEBUG
        virtual std::string type_name() const {std::stringstream ss; ss << "M<" << typeid(T).name() << "," << R << "," << C << ">"; return ss.str();}
        virtual std::string obj_name() const {std::stringstream ss; ss << typeid(T).name() << "[" << /*id()*/SZ << "]"; return ss.str();}
#endif /* CM_DEBUG */

private:

        T data[SZ];
        CM_NOINLINE T operator () (uint i) const {
            assert(i < SZ);
            return get(i);
        }

        CM_NOINLINE T& operator () (uint i) {
            assert(i < SZ);
            return data[i];
        }
/*
        CM_NOINLINE T operator [] (uint i) const {
            assert(i < SZ);
            return get(i);
        }

        CM_NOINLINE T& operator [] (uint i) {
            assert(i < SZ);
            return data[i];
        }
*/
        // for debug
        uint number;
};

// matrix_ref
template <typename T, uint R, uint C>
class matrix_ref : public stream<T,R*C> {
public:
        template <typename T1, uint R1, uint C1> friend class matrix;
        template <typename T1, uint R1, uint C1> friend class matrix_ref;
        template <typename T1, uint SZ1> friend class vector;
        template <typename T1, uint SZ1> friend class vector_ref;

        enum { SZ = R*C };
        enum { ROWS=R, COLS=C, ELEMS=R*C };

        CM_INLINE constexpr int n_rows() const { return R; }
        CM_INLINE constexpr int n_cols() const { return C; }

        template <uint REP> CM_INLINE
        const vector<T, R*C*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,0,R*C,1>(ioff, joff); };
        template <uint REP, uint W> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,0,W,1>(ioff, joff); };
        template <uint REP, uint VS, uint W> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,VS,W,1>(ioff, joff); };
        template <uint REP, uint VS, uint W, uint HS> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,VS,W,HS>(ioff, joff); };

        virtual T get(uint i) const { return *data[i]; }
        virtual T& getref(uint i) { return *data[i]; }

        virtual void* get_addr(uint i) { return data[i]; }
        virtual void* get_addr_data() {
                return this;
        }
        virtual void* get_addr_obj() { return this; }
        void set_elem_ref(uint i, T* ptr) { data[i] = ptr; }
        virtual uint get_size_data() const {
                return sizeof(*this);
        }
        virtual uint get_size_object() const { return sizeof(*this); }

        CM_NOINLINE T operator () (OFFSET i, OFFSET j) const  {
            assert(i < R && j < C);
            return get(i*C+j);
        }

        CM_NOINLINE T& operator () (OFFSET i, OFFSET j) {
            assert(i < R && j < C);
            return *data[i*C+j];
        }

        template <typename T1, uint R1, uint C1>
        class inner_hack {
            matrix_ref* m_t;
            OFFSET _i;
        public:
            inner_hack(matrix_ref* m, OFFSET i):m_t(m){_i=i;}
            CM_NOINLINE const T1 operator[](OFFSET j) const{return (*m_t)(_i,j);}
            CM_NOINLINE T1& operator[](OFFSET j){return (*m_t)(_i,j);}

        };

        CM_NOINLINE inner_hack<T,R,C> operator [] (OFFSET i) const {
            return inner_hack<T,R,C>(this,i);
        }

        CM_NOINLINE inner_hack<T,R,C> operator [] (OFFSET i) {
            return inner_hack<T,R,C>(this,i);
        }

        // constructors
        CM_NOINLINE matrix_ref(const matrix_ref<T,R,C>& src); // copy constructor
        CM_NOINLINE matrix_ref(matrix<T,R,C>& src); // Point reference on matrix
#if defined(__CLANG_CM)
        CM_NOINLINE matrix_ref(const vector<T,R*C>& src);
        CM_NOINLINE matrix_ref(const vector_ref<T,R*C>& src);
#endif

        //operator =
        CM_NOINLINE matrix_ref<T,R,C>& operator = (const matrix<T,R,C>& src); // assignment operator
        CM_NOINLINE matrix_ref<T,R,C>& operator = (const matrix_ref<T,R,C>& src);
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const T2 src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const matrix<T2,R2,C2>& src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const matrix_ref<T2,R2,C2>& src);
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const vector<T2,R*C>& src) { return this->operator=((const matrix<T2,1,R*C>&)src); };
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const vector_ref<T2,R*C>& src) { return this->operator=((const matrix_ref<T2,1,R*C>&)src); };

        // operators +=, -=, ...
        #define declare_operation(OP) \
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const T2 x);\
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const matrix<T2,R2,C2>& x);\
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const matrix_ref<T2,R2,C2>& x);\
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const vector<T2,SZ>& x);\
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const vector_ref<T2,SZ>& x);\

        declare_operation(+=)     // +=
        declare_operation(-=)     // -=
        declare_operation(*=)     // *=
        declare_operation(/=)     // /=
        declare_operation(%=)     // %=
        declare_operation(&=)     // &=
        declare_operation(|=)     // |=
        declare_operation(^=)     // ^=
        declare_operation(>>=)     // >>=
        declare_operation(<<=)     // <<=
        #undef declare_operation

        bool is_contiguous() const;
        bool is_contiguous(const uint start, const uint end) const;
        //selects
        template <typename T2> CM_NOINLINE vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> format(); // to vector
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T2,R2,C2> format();    // to matrix R2xC2
        template <typename T2> CM_NOINLINE const vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> format() const; // to vector
        template <typename T2, uint R2, uint C2> CM_NOINLINE const matrix_ref<T2,R2,C2> format() const;    // to matrix R2xC2
        vector_ref<T, C> CM_NOINLINE row(OFFSET i);
        matrix_ref<T,R,1> CM_NOINLINE column(OFFSET i);
        template <uint R2, uint RS, uint C2, uint CS> CM_NOINLINE matrix_ref<T,R2,C2> select(OFFSET ioff=0, OFFSET joff=0);
        template <uint R2, uint RS, uint C2, uint CS> CM_NOINLINE const matrix_ref<T,R2,C2> select(OFFSET ioff=0, OFFSET joff=0) const;
        template <uint R2, uint VS, uint WD, uint HS> CM_NOINLINE const vector<T, R2*WD> genx_select(OFFSET ioff=0, OFFSET joff=0);

        matrix_ref<T, R, C> select_all() { return select<R, 1, C, 1>(0); }
        const matrix_ref<T, R, C> select_all() const { return select<R, 1, C, 1>(0); }

        //1D iselect
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index);
#if _MSC_VER >= 1700
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index, std::true_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index, std::false_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index, std::true_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index, std::false_type);
#endif

        //2D iselect
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index_x, const vector<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index_x, const vector_ref<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index_x, const vector<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index_x, const vector_ref<T2,WD>& index_y);
        //end of 2D iselect

        // for debug
        uint id() const { return number; }

        // EMU mode out-of-bounds support - pointer to dummy used to prevent illegal pointer deref
        // exceptions and allow EMU mode to work in a similar way to HW (albeit with undefined results
        // for out-of-bounds accesses)
        T* dummy() { return &_dummy; }

#ifdef CM_DEBUG
        virtual std::string type_name() const {std::stringstream ss; ss << "M<" << typeid(T).name() << "," << R << "," << C << ">"; return ss.str();}
        virtual std::string obj_name() const {std::stringstream ss; ss << typeid(T).name() << "[" << id() << "]"; return ss.str();}
#endif /* CM_DEBUG */

private:
        matrix_ref(const uint id) : number(id) {  } // id for debug
        T* data[SZ];

        // A dummy element that is used for EMU mode out-of-bounds select
        T _dummy;

        CM_NOINLINE T operator () (uint i) const {
            assert(i < SZ);
            return get(i);
        }

        CM_NOINLINE T& operator () (uint i) {
            assert(i < SZ);
            return *data[i];
        }
/*
        CM_NOINLINE T operator [] (uint i) const {
            assert(i < SZ);
            return get(i);
        }

        CM_NOINLINE T& operator [] (uint i) {
            assert(i < SZ);
            return *data[i];
        }
*/
        // for debug
        uint number;
};

// vector
template <typename T, uint SZ>
class vector : public matrix<T,1,SZ> {
        void assign(const stream<T, SZ> &src); //special member-function, not for CM language
        void assign_noSIMDCF(const stream<T, SZ> &src); //special member-function, not for CM language
public:
        template <typename T1, uint R1, uint C1> friend class matrix;
        template <typename T1, uint R1, uint C1> friend class matrix_ref;
        template <typename T1, uint SZ1> friend class vector_ref;
        template <typename T1, uint SZ1> friend class stream;

        template <uint REP> CM_INLINE
        vector<T,SZ*REP> replicate(OFFSET joff=0)
        {return ((matrix<T,1,SZ> *)this)->template replicate<REP>(0, joff);};
        template <uint REP, uint W> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff=0)
        {return ((matrix<T,1,SZ> *)this)->template replicate<REP,W>(0, joff);};
        template <uint REP, uint VS, uint W> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff=0)
        {return ((matrix<T,1,SZ> *)this)->template replicate<REP,VS,W>(0, joff);};
        template <uint REP, uint VS, uint W, uint HS> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff=0)
        {return ((matrix<T,1,SZ> *)this)->template replicate<REP,VS,W,HS>(0, joff);};

        CM_NOINLINE_EMU GFX_EMU_API static void emuKernelParamInit__ (void *tgtObj = nullptr, void *src = nullptr, size_t size = 0) {
            if(tgtObj != nullptr) {
                assert(size == sizeof(T)*SZ);
                new(tgtObj) vector<T,SZ>(static_cast<const T*>(src));
            }
        }

        // constructors: call base versions of constructors
        CM_NOINLINE vector() : matrix<T,1,SZ>() {
            emuKernelParamInit__();
        }

        // constructor for emu global variable
        CM_NOINLINE vector(bool isGlobal) : matrix<T, 1, SZ>(isGlobal) {}

        template <typename T2> CM_NOINLINE vector(const T2 initArray[]) : matrix<T,1,SZ>(initArray) {
            // constructor with array initializer
        }

        CM_NOINLINE vector(const vector<T,SZ>& src) : matrix<T,1,SZ>((const matrix<T,1,SZ>&)src) {} // copy constructor
        template <typename T2> CM_NOINLINE vector(const T2& src) : matrix<T,1,SZ>(src) {}
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector(const matrix<T2,R2,C2>& src, uint sat = 0) : matrix<T,1,SZ>(src, sat) {}
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector(const matrix_ref<T2,R2,C2>& src, uint sat = 0) : matrix<T,1,SZ>(src, sat) {}
        template <typename T2> CM_NOINLINE vector(const vector<T2,SZ>& src) : matrix<T,1,SZ>(src) {}
        template <typename T2> CM_NOINLINE vector(const vector_ref<T2,SZ>& src) : matrix<T,1,SZ>(src) {}

        CM_NOINLINE T operator () (OFFSET i) const {
            assert(i < SZ);
            return matrix<T,1,SZ>::get(i);
        }

        CM_NOINLINE T& operator () (OFFSET i) {
            assert(i < SZ);
            return matrix<T,1,SZ>::data[i];
        }

        CM_NOINLINE T operator [] (OFFSET i) const {
            assert(i < SZ);
            return matrix<T,1,SZ>::get(i);
        }

        CM_NOINLINE T& operator [] (OFFSET i) {
            assert(i < SZ);
            return matrix<T,1,SZ>::data[i];
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator () (const vector<T2,WD>& index) {
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator () (const vector_ref<T2,WD>& index) {
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator [] (const vector<T2,WD>& index) {
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator [] (const vector_ref<T2,WD>& index) {
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        //1D iselect only
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index) {
            CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index) {
            CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }
        //end of iselect

        //operator =: call base versions of operator =
        CM_NOINLINE vector<T,SZ>& operator = (const vector<T,SZ>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; } // assignment operator
        template <typename T2> CM_NOINLINE vector<T,SZ>& operator = (const T2 src) {
            ((matrix<T,1,SZ>*)this)->operator=(src); return *this;
        }
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector<T,SZ>& operator = (const matrix<T2,R2,C2>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector<T,SZ>& operator = (const matrix_ref<T2,R2,C2>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2> CM_NOINLINE vector<T,SZ>& operator = (const vector<T2,SZ>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2> CM_NOINLINE vector<T,SZ>& operator = (const vector_ref<T2,SZ>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; }

        //vector select
        template <uint C, uint CS> CM_NOINLINE const vector_ref<T,C> select(OFFSET joff=0) const {
            CM_STATIC_ERROR(((SZ) >= (C)), "select size is greater than source vector size");
            CM_STATIC_ERROR(((SZ) >= ((C-1)*(CS))+1) || (CS == 1), "select range exceeds source vector bounds");
            return ((matrix<T,1,SZ>*)this)->template select<1,1,C,CS>(0,joff);
        }
        template <uint C, uint CS> CM_NOINLINE vector_ref<T,C> select(OFFSET joff=0) {
            CM_STATIC_ERROR(((SZ) >= (C)), "select size is greater than source vector size");
            CM_STATIC_ERROR(((SZ) >= ((C-1)*(CS))+1) || (CS == 1), "select range exceeds source vector bounds");
            return ((matrix<T,1,SZ>*)this)->template select<1,1,C,CS>(0,joff);
        }

        vector_ref<T, SZ> CM_NOINLINE select_all() { return select<SZ, 1>(0); }
        const vector_ref<T, SZ> CM_NOINLINE select_all() const { return select<SZ, 1>(0); }

        //vector genx_select
        template <uint R, uint VS, uint WD, uint HS> CM_NOINLINE const vector<T, R*WD> genx_select(OFFSET joff=0) {
            CM_STATIC_ERROR((!std::is_same<T, double>::value), "genx_select is not supported for vectors with element type 'double'");
            CM_STATIC_ERROR(((SZ) >= (WD)), "genx_select width is greater than source vector size");
            return ((matrix<T,1,SZ>*)this)->template genx_select<R,VS,WD,HS>(0, joff);
        }
};

// vector_ref
template <typename T, uint SZ>
class vector_ref : public matrix_ref<T,1,SZ> {
public:
        template <typename T1, uint R1, uint C1> friend class matrix;
        template <typename T1, uint R1, uint C1> friend class matrix_ref;
        template <typename T1, uint SZ1> friend class vector;
        template <typename T1, uint SZ1> friend class vector_ref;

        template <uint REP> CM_INLINE
        vector<T,SZ*REP> replicate(OFFSET joff = 0)
        {return ((matrix_ref<T,1,SZ> *)this)->template replicate<REP>(0, joff);};
        template <uint REP, uint W> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff = 0)
        {return ((matrix_ref<T,1,SZ> *)this)->template replicate<REP,W>(0, joff);};
        template <uint REP, uint VS, uint W> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff = 0)
        {return ((matrix_ref<T,1,SZ> *)this)->template replicate<REP,VS,W>(0, joff);};
        template <uint REP, uint VS, uint W, uint HS> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff = 0)
        {return ((matrix_ref<T,1,SZ> *)this)->template replicate<REP,VS,W,HS>(0, joff);};

        // constructors
        CM_NOINLINE vector_ref(const vector_ref<T,SZ>& src) : matrix_ref<T,1,SZ>((const matrix_ref<T,1,SZ>&)src) {} // copy constructor
        CM_NOINLINE vector_ref(vector<T,SZ>& src) : matrix_ref<T,1,SZ>((matrix<T,1,SZ>&)src) {} //assign vector_ref to vector
        CM_NOINLINE vector_ref(const matrix_ref<T,1,SZ>& src) : matrix_ref<T,1,SZ>(src) {}
        CM_NOINLINE vector_ref(matrix<T,1,SZ>& src) : matrix_ref<T,1,SZ>(src) {}
#if defined(__CLANG_CM)
        CM_NOINLINE vector_ref(matrix<T,1,SZ> src) : matrix_ref<T,1,SZ>(src) {}
        CM_NOINLINE vector_ref(matrix_ref<T,1,SZ>& src) : matrix_ref<T,1,SZ>(src) {}
#endif // defined(__CLANG_CM)

        CM_NOINLINE T operator () (OFFSET i) const {
            assert(i < SZ);
            return matrix_ref<T,1,SZ>::get(i);
        }

        CM_NOINLINE T& operator () (OFFSET i) {
            assert(i < SZ);
            return *matrix_ref<T,1,SZ>::data[i];
        }

        CM_NOINLINE T operator [] (OFFSET i) const {
            assert(i < SZ);
            return matrix_ref<T,1,SZ>::get(i);
        }

        CM_NOINLINE T& operator [] (OFFSET i) {
            assert(i < SZ);
            return *matrix_ref<T,1,SZ>::data[i];
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator () (const vector<T2,WD>& index) const{
            return matrix_ref<T,1,SZ>::template iselect<T,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator () (const vector_ref<T2,WD>& index) const{
            return matrix_ref<T,1,SZ>::template iselect<T,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator [] (const vector<T2,WD>& index) const{
            return matrix_ref<T,1,SZ>::template iselect<T,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator [] (const vector_ref<T2,WD>& index) const{
            return matrix_ref<T,1,SZ>::template iselect<T,WD>(index);
        }

        //1D iselect only
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD >& index) {
            CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
            return matrix_ref<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index) {
            CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
            return matrix_ref<T,1,SZ>::template iselect<T2,WD>(index);
        }
        //end of iselect

        //operator =: call base versions of operator =
        CM_NOINLINE vector_ref<T,SZ>& operator = (const vector_ref<T,SZ>& src) { ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; } //assignment operator
        template <typename T2> CM_NOINLINE vector_ref<T,SZ>& operator = (const T2 src) { ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector_ref<T,SZ>& operator = (const matrix<T2,R2,C2>& src){ ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector_ref<T,SZ>& operator = (const matrix_ref<T2,R2,C2>& src){ ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2> CM_NOINLINE vector_ref<T,SZ>& operator = (const vector<T2,SZ>& src) { ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2> CM_NOINLINE vector_ref<T,SZ>& operator = (const vector_ref<T2,SZ>& src) { ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }

        //vector_ref select
        template <uint C, uint CS> CM_NOINLINE vector_ref<T,C> select(OFFSET joff=0) {return ((matrix_ref<T,1,SZ>*)this)->template select<1,1,C,CS>(0,joff);}
        template <uint C, uint CS> CM_NOINLINE const vector_ref<T,C> select(OFFSET joff=0) const {return ((matrix_ref<T,1,SZ>*)this)->template select<1,1,C,CS>(0,joff);}

        //vector_ref genx_select
        template <uint R, uint VS, uint WD, uint HS> CM_NOINLINE const vector<T, R*WD> genx_select(OFFSET joff=0) {
            CM_STATIC_WARNING((!std::is_same<T, double>::value), "genx_select is not supported for vectors with element type of 'double'");
            CM_STATIC_ERROR(((SZ) >= (WD)), "genx_select width is greater than source vector size");
            return ((matrix_ref<T,1,SZ>*)this)->template genx_select<R,VS,WD,HS>(0, joff);
        };

private:
        CM_NOINLINE vector_ref(const uint id) : matrix_ref<T,1,SZ>(id) {}
};

#ifdef CM_DEBUG
template <typename T, uint R, uint C>
std::ostream& operator << (std::ostream &out, const matrix<T,R,C>& m)
{
        out << "---" << m.obj_name() << ":" << std::endl;
        for (uint i=0; i<R; ++i) {
                for (uint j=0; j<C; ++j) {
                        out << m(i,j) << " ";
                }
                out << std::endl;
        }
        return out;
}
template <uint R, uint C>
std::ostream& operator << (std::ostream &out, const matrix<char,R,C>& m)
{
        out << "---" << m.obj_name() << ":" << std::endl;
        for (uint i=0; i<R; ++i) {
                for (uint j=0; j<C; ++j) {
                        out << (int)m(i,j) << " ";
                }
                out << std::endl;
        }
        return out;
}
#endif /* CM_DEBUG */

/*******************************************************************
/
/                         stream
/
*******************************************************************/

template <typename T, uint SZ>
int stream<T,SZ>::extract_data(void *buf, uint size)
{
    uint i;

    assert(SZ*sizeof(T) <= size);

    for (i=0; i< SZ; i++) {
        ((T*)buf)[i] = get(i);
    }

    return SZ*sizeof(T);
}

/*
 * Please do NOT move below MACRO in front of this headerfile
 * because the namespace "__CMInternal__" is defined in "cm_internal_emu.h". This file depends on matrix/vector declared before.
 */
#ifdef CM_GENX
#include "cm_internal_emu.h"    //using namespace __CMInternal__
#define SIMDCF_WRAPPER(X, SZ, i) \
    if (__CMInternal__::getWorkingStack() && !__CMInternal__::getWorkingStack()->isEmpty() && (SZ > 1))  { \
            if ((int)(__CMInternal__::getSIMDMarker() << (i)) < 0) \
                X;\
        } else { \
            X; \
        }
#else
#define SIMDCF_WRAPPER(X, SZ, i) X
#endif

#ifdef CM_GENX
#ifndef SIMDCF_ELEMENT_SKIP
#define SIMDCF_ELEMENT_SKIP(i) \
        if (__CMInternal__::getWorkingStack() && !__CMInternal__::getWorkingStack()->isEmpty()) \
            if ((int)(__CMInternal__::getSIMDMarker() << (i)) >= 0) \
                continue; \

#endif // SIMDCF_ELEMENT_SKIP
#else // CM_GENX
#define SIMDCF_ELEMENT_SKIP(i)
#endif // CM_GENX

/*
 *  merge
 */
template <typename T, uint SZ>
void stream<T,SZ>::merge(const T x, const uint c)
{
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            if (((c >> i) & 1) != 0) {
                p = (T*)this->get_addr(i);
                *p = x;
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const stream<T1,SZ> &x, const uint c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            if (((c >> i) & 1) != 0) {
                p = (T*)this->get_addr(i);
                *p = in_x.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2>
void stream<T,SZ>::merge(const stream<T1,SZ> &x, const stream<T2,SZ> &c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    vector<T2, SZ> in_c; in_c.assign(c);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            if ((in_c.get(i) & 1) != 0) {
                p = (T*)this->get_addr(i);
                *p = (T) in_x.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const T x, const stream<T1,SZ> &c)
{
    vector<T1, SZ> in_c; in_c.assign(c);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            if ((in_c.get(i) & 1) != 0) {
                p = (T*)this->get_addr(i);
                *p = x;
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2>
void stream<T,SZ>::merge(const stream<T1,SZ>& x, const stream<T2,SZ>& y,
                         const uint c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    vector<T2, SZ> in_y; in_y.assign(y);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if (((c >> i) & 1) != 0) {
                *p = in_x.get(i);
            } else {
                *p = in_y.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const T x, const stream<T1,SZ>& y, const uint c)
{
    vector<T1, SZ> in_y; in_y.assign(y);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if (((c >> i) & 1) != 0) {
                *p = x;
            } else {
                *p = in_y.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const stream<T1,SZ>& x, const T y, const uint c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if (((c >> i) & 1) != 0) {
                *p = in_x.get(i);
            } else {
                *p = y;
            }
    }
}

template <typename T, uint SZ>
void stream<T,SZ>::merge(const T x, const T y, const uint c)
{
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if (((c >> i) & 1) != 0) {
                *p = x;
            } else {
                *p = y;
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2, typename T3>
void stream<T,SZ>::merge(const stream<T1,SZ>& x, const stream<T2,SZ>& y, const stream<T3,SZ>& c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    vector<T2, SZ> in_y; in_y.assign(y);
    vector<T3, SZ> in_c; in_c.assign(c);

    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if ((in_c.get(i) & 1) != 0) {
                *p = in_x.get(i);
            } else
            {
                *p = in_y.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2>
void stream<T,SZ>::merge(const T x, const stream<T1,SZ>& y, const stream<T2,SZ>& c)
{
    vector<T1, SZ> in_y; in_y.assign(y);
    vector<T2, SZ> in_c; in_c.assign(c);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if ((in_c.get(i) & 1) != 0) {
                *p = x;
            } else
            {
                *p = in_y.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2>
void stream<T,SZ>::merge(const stream<T1,SZ>& x, const T y,
                         const stream<T2,SZ>& c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    vector<T2, SZ> in_c; in_c.assign(c);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if ((in_c.get(i) & 1) != 0) {
                *p = in_x.get(i);
            } else
            {
                *p = y;
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const T x, const T y, const stream<T1,SZ>& c)
{
    vector<T1, SZ> in_c; in_c.assign(c);
    T* p;
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if ((in_c.get(i) & 1) != 0) {
                *p = x;
            } else
            {
                *p = y;
            }
    }
}

/*******************************************************************
/
/                         matrix
/
*******************************************************************/

/*
 * matrix constructors
 */

/* Constuctor for data preload from URB. This function take effect on the data array */
/*
template <typename T, uint R, uint C>
matrix<T,R,C>::matrix(void *ptr)
{
    int i;
    for (i = 0; i < SZ; i++) {
        data[i] = ((T*)ptr)[i];
    }
}
*/

template <typename T, uint R, uint C>
matrix<T, R, C>::matrix(bool isGlobal)
{
    if (isGlobal)
    {
        uint32_t size = R * C * sizeof(T);
        initialize_global_var(this, this->data, size);
    }
}

// Matrix Initialization with array
template <typename T, uint R, uint C>
template <typename T2>
matrix<T,R,C>::matrix(const T2 initArray[]) {
    using argElT = std::remove_all_extents_t<T2>;
    for (int i = 0; i < SZ; i++) {
        const auto& el = reinterpret_cast<const argElT*>(initArray)[i];
        // SIMDCF_WRAPPER(data[i] = el, SZ, i);
        data[i] = el;
    }
}

template <typename T, uint R, uint C>
matrix<T,R,C>::matrix()
{
    //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
    emuKernelParamInit__();
}

//copy constructor
template <typename T, uint R, uint C>
matrix<T,R,C>::matrix(const matrix<T,R,C>& src) {
    //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
    *this = src;
}
template <typename T, uint R, uint C>
template <typename T2>
matrix<T,R,C>::matrix(const T2& src) {
    //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
    *this = src;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix<T,R,C>::matrix(const matrix<T2,R2,C2>& src, const uint sat)
{
    //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
    static const bool conformable = check_true<R*C == R2*C2>::value;
    assert(R*C == R2*C2);

    uint sat1 = 0;
    //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
    vector<T2, SZ> in_src; in_src.assign_noSIMDCF(src);

    for (uint i=0; i < SZ; i++) {
//          SIMDCF_WRAPPER((*this)(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat | sat1), SZ, i);
        (*this)(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat | sat1);
    }
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix<T,R,C>::matrix(const matrix_ref<T2,R2,C2>& src, const uint sat)
{
        //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign_noSIMDCF(src);

        for (uint i=0; i < SZ; i++) {
//          SIMDCF_WRAPPER((*this)(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat), SZ, i);
            (*this)(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat);
        }
}

//
// matrix operator =
//
template <typename T, uint R, uint C>
matrix<T,R,C>& matrix<T,R,C>::operator = (const matrix<T,R,C>& src)
{
        vector<T, SZ> in_src; in_src.assign(src);
        for (uint i=0; i<SZ; ++i) {
            SIMDCF_WRAPPER(this->getref(i) = in_src(i), SZ, i);
        }
        return *this;
}
template <typename T, uint R, uint C>
template <typename T2>
matrix<T,R,C>& matrix<T,R,C>::operator = (const T2 src)
{
        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();

        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(src, sat1), SZ, i);
        }

        return *this;
}

template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix<T,R,C>& matrix<T,R,C>::operator = (const matrix<T2,R2,C2>& src)
{
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);

        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat1), SZ, i);
        }

        return *this;
}

template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix<T,R,C>& matrix<T,R,C>::operator = (const matrix_ref<T2,R2,C2>& src)
{
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);

        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat1), SZ, i);
        }

        return *this;
}

//Should be inserted for GenX style of float->integer conversions
//sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
#define matrix_operation(OP) \
\
template <typename T, uint R, uint C> \
template <typename T2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const T2 x) \
{ \
        static const bool type_conformable = cmtype<T2>::value; \
        uint sat1 = 0; \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP x, sat1), SZ, i); \
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2, uint R2, uint C2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const matrix<T2,R2,C2>& x) \
{ \
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value; \
        assert(R*C == R2*C2); \
        uint sat1 = 0; \
        vector<T2, /*SZ*/R*C> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2, uint R2, uint C2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const matrix_ref<T2,R2,C2>& x) \
{ \
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value; \
        assert(R*C == R2*C2); \
        uint sat1 = 0; \
        vector<T2, /*SZ*/R*C> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const vector<T2,SZ>& x) \
{ \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const vector_ref<T2,SZ>& x) \
{ \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \

matrix_operation(+)     // +=
matrix_operation(-)     // -=
matrix_operation(*)     // *=
matrix_operation(/)     // /=
matrix_operation(%)     // %=
matrix_operation(&)     // &=
matrix_operation(|)     // |=
matrix_operation(^)     // ^=
matrix_operation(>>)     // >>=
matrix_operation(<<)     // <<=
#undef matrix_operation

//
// matrix selects
//
template <typename T, uint R, uint C>
template <typename T2>
vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> matrix<T,R,C>::format()
{
        CM_STATIC_ERROR(R>0, "format row size is zero");
        CM_STATIC_ERROR(C>0, "format column size is zero");
        CM_STATIC_WARNING(((R*C*sizeof(T)%sizeof(T2)) == 0), "source matrix size is not exactly divisible by format type size");
        const uint N = R*C*sizeof(T)/sizeof(T2);
        static const bool conformable = check_true<(R*C*sizeof(T))%sizeof(T2) == 0>::value;
        assert((R*C*sizeof(T))%sizeof(T2) == 0);
        vector_ref<T2,N> ret(id());
        for (uint i=0; i<N; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, ((T2*)data) + i), N, i);
        }

        return ret;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix_ref<T2,R2,C2> matrix<T,R,C>::format()
{
        CM_STATIC_ERROR(R2>0, "format row size is zero");
        CM_STATIC_ERROR(C2>0, "format column size is zero");
        CM_STATIC_ERROR((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C >= sizeof(T2)*R2*C2), "format result size is larger than source size");
        CM_STATIC_WARNING((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C <= sizeof(T2)*R2*C2), "format result size is smaller than source size");
        static const bool conformable = check_true<sizeof(T)*R*C == sizeof(T2)*R2*C2>::value;
        assert(sizeof(T)*R*C == sizeof(T2)*R2*C2);
        matrix_ref<T2,R2,C2> ret(id());
        for (uint i=0; i<R2*C2; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, ((T2*)data) + i), R2*C2, i);
        }

        return ret;
}
template <typename T, uint R, uint C>
template <typename T2>
const vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> matrix<T,R,C>::format() const
{
        CM_STATIC_ERROR(R>0, "format row size is zero");
        CM_STATIC_ERROR(C>0, "format column size is zero");
        CM_STATIC_WARNING(((R*C*sizeof(T)%sizeof(T2)) == 0), "source matrix size is not exactly divisible by format type size");
        const uint N = R*C*sizeof(T)/sizeof(T2);
        static const bool conformable = check_true<(R*C*sizeof(T))%sizeof(T2) == 0>::value;
        assert((R*C*sizeof(T))%sizeof(T2) == 0);
        vector_ref<T2,N> ret(id());
        for (uint i=0; i<N; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, ((T2*)data) + i), N, i);
        }

        return ret;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
const matrix_ref<T2,R2,C2> matrix<T,R,C>::format() const
{
        CM_STATIC_ERROR(R2>0, "format row size is zero");
        CM_STATIC_ERROR(C2>0, "format column size is zero");
        CM_STATIC_ERROR((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C >= sizeof(T2)*R2*C2), "format result size is larger than source size");
        CM_STATIC_WARNING((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C <= sizeof(T2)*R2*C2), "format result size is smaller than source size");
        static const bool conformable = check_true<sizeof(T)*R*C == sizeof(T2)*R2*C2>::value;
        assert(sizeof(T)*R*C == sizeof(T2)*R2*C2);
        matrix_ref<T2,R2,C2> ret(id());
        for (uint i=0; i<R2*C2; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, ((T2*)data) + i), R2*C2, i);
        }
        return ret;
}
template <typename T, uint R, uint C>
vector_ref<T, C> matrix<T,R,C>::row(OFFSET index)
{
        CM_STATIC_ERROR(R>0, "row size is zero");
        CM_STATIC_ERROR(C>0, "column size is zero");
        assert(index < R);

        vector_ref<T, C> ret(id());
        for (uint i=0; i<C; ++i) {
//          SIMDCF_WRAPPER(ret.set_elem_ref(i, data + C*index + i), C, i);
            ret.set_elem_ref(i, data + C * index + i);
        }
        return ret;
}
template <typename T, uint R, uint C>
matrix_ref<T,R,1> matrix<T,R,C>::column(OFFSET index)
{
        CM_STATIC_ERROR(R>0, "row size is zero");
        CM_STATIC_ERROR(C>0, "column size is zero");
        assert(index < C);

        matrix_ref<T,R,1> ret(id());
        for (uint i=0; i<R; ++i) {
//          SIMDCF_WRAPPER(ret.set_elem_ref(i, data + C*i + index), R, i);
            ret.set_elem_ref(i, data + C * i + index);
        }
        return ret;
}
template <typename T, uint R, uint C>
template <uint R2, uint RS, uint C2, uint CS>
matrix_ref<T,R2,C2> matrix<T,R,C>::select(OFFSET ioff, OFFSET joff)
{
        CM_STATIC_ERROR((RS > 0), "select does not support a row stride of 0");
        CM_STATIC_ERROR((CS > 0), "select does not support a column stride of 0");
        CM_STATIC_WARNING(!(R2 == 1 && RS != 1), "when row size is 1 the row stride must also be 1");
        CM_STATIC_WARNING(!(C2 == 1 && CS != 1), "when column size is 1 the column stride must also be 1");
        CM_STATIC_WARNING(((C2 - 1) * CS + 1 <= C), "new row must fit inside the source row (new row out of bounds wrt original)");
        CM_STATIC_WARNING(((R2 - 1) * RS + 1 <= R), "new matrix must fit inside the source matrix (new matrix out of bounds wrt original)");

        static const bool conformable1 = check_true<((R2 - 1) * RS < R)>::value;
        static const bool conformable2 = check_true<((C2 - 1) * CS < C)>::value;
        static const bool conformable3 = check_true<(RS > 0)>::value;
        static const bool conformable4 = check_true<(CS > 0)>::value;
        static const bool conformable5 = check_true<!(R2 == 1 && RS != 1)>::value;
        static const bool conformable6 = check_true<!(C2 == 1 && CS != 1)>::value;

        assert(ioff  < R - (R2 - 1) * RS);
        assert(joff  < C - (C2 - 1) * CS);

        matrix_ref<T,R2,C2> ret(id());
        for (uint i=0; i<R2; i++) {
            for (uint j=0; j<C2; j++) {
                if ((CS*j + joff) >= C) {
                    // We go off the end of the source row
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert( 0 && "select statement access is out-of-bounds on source matrix");
#endif
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ret.dummy()), R2 * C2, i * C2 + j);
                } else if ((C*(RS*i + ioff) + (CS*j) + joff) >= (C * R)) {
                    // We go off the end of the source matrix
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert( 0 && "select statement access is out-of-bounds on source matrix");
#endif
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ret.dummy()), R2 * C2, i * C2 + j );
                } else {
                    // Everything is within bounds
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ((T*)data) + C*(RS*i + ioff) + (CS*j) + joff), R2 * C2, i * C2 + j);
                }
            }
        }
        return ret;
}

template <typename T, uint R, uint C>
template <uint R2, uint RS, uint C2, uint CS>
const matrix_ref<T,R2,C2> matrix<T,R,C>::select(OFFSET ioff, OFFSET joff) const
{
        CM_STATIC_ERROR((RS > 0), "select does not support a row stride of 0");
        CM_STATIC_ERROR((CS > 0), "select does not support a column stride of 0");
        CM_STATIC_WARNING(!(R2 == 1 && RS != 1), "when row size is 1 the row stride must also be 1");
        CM_STATIC_WARNING(!(C2 == 1 && CS != 1), "when column size is 1 the column stride must also be 1");
        CM_STATIC_WARNING(((C2 - 1) * CS + 1 <= C), "new row must fit inside the source row (new row out of bounds wrt original)");
        CM_STATIC_WARNING(((R2 - 1) * RS + 1 <= R), "new matrix must fit inside the source matrix (new matrix out of bounds wrt original)");

        static const bool conformable1 = check_true<((R2 - 1) * RS < R)>::value;
        static const bool conformable2 = check_true<((C2 - 1) * CS < C)>::value;
        static const bool conformable3 = check_true<(RS > 0)>::value;
        static const bool conformable4 = check_true<(CS > 0)>::value;
        static const bool conformable5 = check_true<!(R2 == 1 && RS != 1)>::value;
        static const bool conformable6 = check_true<!(C2 == 1 && CS != 1)>::value;

        assert(ioff  < R - (R2 - 1) * RS);
        assert(joff  < C - (C2 - 1) * CS);

        matrix_ref<T,R2,C2> ret(id());
        for (uint i=0; i<R2; i++) {
            for (uint j=0; j<C2; j++) {
                if ((CS*j + joff) >= C) {
                    // We go off the end of the source row
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert( 0 && "select statement access is out-of-bounds on source matrix");
#endif
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ret.dummy()), R2 * C2, i * C2 + j);
                } else if ((C*(RS*i + ioff) + (CS*j) + joff) >= (C * R)) {
                    // We go off the end of the source matrix
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert( 0 && "select statement access is out-of-bounds on source matrix");
#endif
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ret.dummy()), R2 * C2, i * C2 + j );
                } else {
                    // Everything is within bounds
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ((T*)data) + C*(RS*i + ioff) + (CS*j) + joff), R2 * C2, i * C2 + j);
                }
            }
        }
        return ret;
}

template <typename T, uint R, uint C>
template <uint R2, uint VS, uint WD, uint HS>
const vector<T, R2*WD> matrix<T,R,C>::genx_select(OFFSET ioff, OFFSET joff)
{
        CM_STATIC_ERROR((!std::is_same<T, double>::value), "genx_select is not supported for matrices with element type of 'double'");
        static const bool conformable1 = check_true<(R2 > 0)>::value;
        static const bool conformable2 = check_true<(VS >= 0)>::value;
        static const bool conformable3 = check_true<(WD > 0)>::value;
        static const bool conformable4 = check_true<(HS >= 0)>::value;
        assert(R2>=0 && VS>=0 && WD>=0 && HS >=0);

        assert(ioff < R);
        assert(joff < C);

        vector<T,R2*WD> ret(id());
        for (uint i=0; i < R2*WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[C*ioff + joff + (i/WD)*VS + (i%WD)*HS], R2*WD, i);
        }
        return ret;
}

//1D iselect for matrix
#if _MSC_VER >= 1700
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index)
{
        return iselect(index, std::is_integral<T2>());
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index, std::true_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index, std::false_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            // in this case index doesn't have integral type elements
            // so can't be used - we will have already generated an error,
            // so just use 0 to allow compilation to continue (in case there
            // are more errors to find...)
            SIMDCF_WRAPPER(ret(i) = data[0], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index)
{
        return iselect(index, std::is_integral<T2>());
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index, std::true_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index, std::false_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            // in this case index doesn't have integral type elements
            // so can't be used - we will have already generated an error,
            // so just use 0 to allow compilation to continue (in case there
            // are more errors to find...)
            SIMDCF_WRAPPER(ret(i) = data[0], WD, i);
        }
        return ret;
}
#else
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
          SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index.get(i)], WD, i);
        }
        return ret;
}
#endif

//below are 2D iselect for matrix
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index_x, const vector<T2,WD>& index_y)
{
        CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index_x, const vector<T2,WD>& index_y)
{
        CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
          SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
          SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index_x, const vector_ref<T2,WD>& index_y)
{
        CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index_x, const vector_ref<T2,WD>& index_y)
{
        CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
          SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
          SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}
// end of iselect for 2D matrix

template <typename T, uint R, uint C>
matrix_ref<T, R, C> CM_NOINLINE matrix<T,R,C>::select_all()
{
        matrix_ref<T,R,C> ret = this->select<R, 1, C, 1>();
        return ret;
}

template <typename T, uint R, uint C>
const matrix_ref<T, R, C> CM_NOINLINE matrix<T,R,C>::select_all() const
{
        const matrix_ref<T,R,C> ret = this->select<R, 1, C, 1>();
        return ret;
}

/*******************************************************************
/
/                         matrix_ref
/
*******************************************************************/
template <typename T, uint R, uint C>
bool matrix_ref<T,R,C>::is_contiguous() const
{
        if (SZ == 1)
                return true;

        for (uint i=0; i<SZ-1; ++i) {
                if (data[i+1]-data[i] != 1)
                        return false;
        }
        return true;
}

template <typename T, uint R, uint C>
bool matrix_ref<T,R,C>::is_contiguous(const uint start, const uint end) const
{
        if (start == end)
            return true;

        if (SZ == 1)
            return true;

        for (uint i=start; i != end; ++i) {
                if (data[i+1]-data[i] != 1)
                        return false;
        }
        return true;
}

//
// matrix_ref copy constructor
//
template <typename T, uint R, uint C>
matrix_ref<T,R,C>::matrix_ref(const matrix_ref<T,R,C>& src)
{
        number = src.number;

        matrix_ref<T,R,C> in_src(id());
        memcpy(in_src.data, src.data, sizeof(T*) * SZ);

        memcpy(data, in_src.data,sizeof(T*)*SZ);
}

template <typename T, uint R, uint C>
matrix_ref<T,R,C>::matrix_ref(matrix<T,R,C>& src)
{
        number = src.id();
        for (uint i = 0; i < ROWS; i++)
            for (uint j = 0; j < COLS; j++) {
//              SIMDCF_WRAPPER(
//                      set_elem_ref(i * COLS + j, (T*)(src.get_addr(i * COLS + j))),
//                      SZ, j);
                set_elem_ref(i * COLS + j, (T*)(src.get_addr(i * COLS + j)));
            }
}
//
// matrix_ref assignment operator
//
template <typename T, uint R, uint C>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const matrix<T,R,C>& src)
{
        vector<T, SZ> in_src; in_src.assign(src);
        for (uint i=0; i<SZ; ++i) {
            SIMDCF_WRAPPER((*this)(i) = in_src(i), SZ, i);
        }
        return *this;
}

//
// matrix_ref operator =
//
template <typename T, uint R, uint C>
template <typename T2>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const T2 src)
{
        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(src, sat1), SZ, i);
        }

        return *this;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const matrix<T2,R2,C2>& src)
{
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);
        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat1), SZ, i);
        }

        return *this;
}

template <typename T, uint R, uint C>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const matrix_ref<T,R,C>& src)
{
        vector<T, SZ> in_src; in_src.assign(src);
        for (uint i=0; i<SZ; ++i) {
            SIMDCF_WRAPPER(this->getref(i) = T(in_src(i)), SZ, i);
        }
        return *this;
}

template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const matrix_ref<T2,R2,C2>& src)
{
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);
        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat1), SZ, i);
        }

        return *this;
}

//Should be inserted for GenX style of float->integer conversions
//sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
#define matrix_ref_operation(OP) \
\
template <typename T, uint R, uint C> \
template <typename T2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const T2 x) \
{ \
        static const bool type_conformable = cmtype<T2>::value; \
        uint sat1 = 0; \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP x, sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2, uint R2, uint C2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const matrix<T2,R2,C2>& x) \
{ \
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value; \
        assert(R*C == R2*C2); \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2, uint R2, uint C2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const matrix_ref<T2,R2,C2>& x) \
{ \
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value; \
        assert(R*C == R2*C2); \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const vector<T2,SZ>& x) \
{ \
        CM_STATIC_ERROR(R*C == SZ, "matrix and vector have a different number of elements"); \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const vector_ref<T2,SZ>& x) \
{ \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i); \
        } \
        return *this; \
} \

matrix_ref_operation(+)     // +=
matrix_ref_operation(-)     // -=
matrix_ref_operation(*)     // *=
matrix_ref_operation(/)     // /=
matrix_ref_operation(%)     // %=
matrix_ref_operation(&)     // &=
matrix_ref_operation(|)     // |=
matrix_ref_operation(^)     // ^=
matrix_ref_operation(>>)     // >>=
matrix_ref_operation(<<)     // <<=
#undef matrix_operation

//
// matrix_ref selects
//
template <typename T, uint R, uint C>
template <typename T2>
vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> matrix_ref<T,R,C>::format()
{
        CM_STATIC_ERROR(R>0, "format row size is zero");
        CM_STATIC_ERROR(C>0, "format column size is zero");
        CM_STATIC_WARNING(((R*C*sizeof(T)%sizeof(T2)) == 0), "source matrix size is not exactly divisible by format type size");

//        assert(is_contiguous());

        const uint N = R*C*sizeof(T)/sizeof(T2);
        static const bool conformable = check_true<(R*C*sizeof(T))%sizeof(T2) == 0>::value;
        assert((R*C*sizeof(T))%sizeof(T2) == 0);
        vector_ref<T2,N> ret(id());

        if (sizeof(T2) < sizeof(T))
        {
            uint ratio = sizeof(T) / sizeof(T2);
            for (uint i = 0; i < R*C; ++i)
            {
                for (uint j = 0; j < ratio; j++)
                {
                    SIMDCF_WRAPPER(ret.set_elem_ref(ratio* i + j, ((T2*)data[i]) + j), N, ratio* i + j);
                }
            }
        }
        else
        {

            for (uint i = 0; i < N; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, (T2*)(data[i * sizeof(T2) / sizeof(T)])), N, i);
            }
        }
        return ret;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix_ref<T2,R2,C2> matrix_ref<T,R,C>::format()
{
        CM_STATIC_ERROR(R2>0, "format row size is zero");
        CM_STATIC_ERROR(C2>0, "format column size is zero");
        CM_STATIC_ERROR((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C >= sizeof(T2)*R2*C2), "format result size is larger than source size");
        CM_STATIC_WARNING((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C <= sizeof(T2)*R2*C2), "format result size is smaller than source size");
        static const bool conformable = check_true<sizeof(T)*R*C == sizeof(T2)*R2*C2>::value;

//        assert(is_contiguous());

        assert(sizeof(T)*R*C == sizeof(T2)*R2*C2);
        matrix_ref<T2,R2,C2> ret(id());
        if (sizeof(T2) < sizeof(T))
        {
            uint ratio = sizeof(T) / sizeof(T2);
            for (uint i = 0; i < R*C; ++i)
            {
                for (uint j = 0; j < ratio; j++)
                {
                    SIMDCF_WRAPPER(ret.set_elem_ref(ratio* i + j, ((T2*)data[i]) + j), R2*C2, ratio* i + j);
                }
            }
        }
        else
        {
            for (uint i = 0; i<R2*C2; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, (T2*)(data[i * sizeof(T2) / sizeof(T)])), R2*C2, i);
            }
        }

        return ret;
}
template <typename T, uint R, uint C>
template <typename T2>
const vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> matrix_ref<T,R,C>::format() const
{
        CM_STATIC_ERROR(R>0, "format row size is zero");
        CM_STATIC_ERROR(C>0, "format column size is zero");
        CM_STATIC_WARNING(((R*C*sizeof(T)%sizeof(T2)) == 0), "source matrix size is not exactly divisible by format type size");

//        assert(is_contiguous());

        const uint N = R*C*sizeof(T)/sizeof(T2);
        static const bool conformable = check_true<(R*C*sizeof(T))%sizeof(T2) == 0>::value;
        assert((R*C*sizeof(T))%sizeof(T2) == 0);
        vector_ref<T2,N> ret(id());
        if (sizeof(T2) < sizeof(T))
        {
            uint ratio = sizeof(T) / sizeof(T2);
            for (uint i = 0; i < R*C; ++i)
            {
                for (uint j = 0; j < ratio; j++)
                {
                    SIMDCF_WRAPPER(ret.set_elem_ref(ratio* i + j, ((T2*)data[i]) + j), N, ratio* i + j);
                }
            }
        }
        else
        {

            for (uint i = 0; i < N; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, (T2*)(data[i * sizeof(T2) / sizeof(T)])), N, i);
            }
        }
        return ret;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
const matrix_ref<T2,R2,C2> matrix_ref<T,R,C>::format() const
{
        CM_STATIC_ERROR(R2>0, "format row size is zero");
        CM_STATIC_ERROR(C2>0, "format column size is zero");
        CM_STATIC_ERROR((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C >= sizeof(T2)*R2*C2), "format result size is larger than source size");
        CM_STATIC_WARNING((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C <= sizeof(T2)*R2*C2), "format result size is smaller than source size");
        static const bool conformable = check_true<sizeof(T)*R*C == sizeof(T2)*R2*C2>::value;

//        assert(is_contiguous());

        assert(sizeof(T)*R*C == sizeof(T2)*R2*C2);
        matrix_ref<T2,R2,C2> ret(id());
        if (sizeof(T2) < sizeof(T))
        {
            uint ratio = sizeof(T) / sizeof(T2);
            for (uint i = 0; i < R*C; ++i)
            {
                for (uint j = 0; j < ratio; j++)
                {
                    SIMDCF_WRAPPER(ret.set_elem_ref(ratio* i + j, ((T2*)data[i]) + j), R2*C2, ratio* i + j);
                }
            }
        }
        else
        {
            for (uint i = 0; i<R2*C2; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, (T2*)(data[i * sizeof(T2) / sizeof(T)])), R2*C2, i);
            }
        }

        return ret;
}
template <typename T, uint R, uint C>
vector_ref<T, C> matrix_ref<T,R,C>::row(OFFSET index)
{
        assert(index < R);

#ifdef CM_V1
        assert(is_contiguous());
#endif

        vector_ref<T, C> ret(id());
        for (uint i=0; i<C; ++i) {
//          SIMDCF_WRAPPER(ret.set_elem_ref(i, *(data + C*index + i)), C, i);
            ret.set_elem_ref(i, *(data + C * index + i));
        }
        return ret;
}
template <typename T, uint R, uint C>
matrix_ref<T,R,1> matrix_ref<T,R,C>::column(OFFSET index)
{
        assert(index < C);

#ifdef CM_V1
        assert(is_contiguous());
#endif

        matrix_ref<T,R,1> ret(id());
        for (uint i=0; i<R; ++i) {
//          SIMDCF_WRAPPER(ret.set_elem_ref(i, data[C*i + index]), R, i);
            ret.set_elem_ref(i, data[C*i + index]);
        }
        return ret;
}
template <typename T, uint R, uint C>
template <uint R2, uint RS, uint C2, uint CS>
matrix_ref<T,R2,C2> matrix_ref<T,R,C>::select(OFFSET ioff, OFFSET joff)
{
        CM_STATIC_ERROR((RS > 0), "select does not support a row stride of 0");
        CM_STATIC_ERROR((CS > 0), "select does not support a column stride of 0");
        CM_STATIC_WARNING(!(R2 == 1 && RS != 1), "when row size is 1 the row stride must also be 1");
        CM_STATIC_WARNING(!(C2 == 1 && CS != 1), "when column size is 1 the column stride must also be 1");
        CM_STATIC_WARNING(((C2 - 1) * CS + 1 <= C), "new row must fit inside the source row (new row out of bounds wrt original)");
        CM_STATIC_WARNING(((R2 - 1) * RS + 1 <= R), "new matrix must fit inside the source matrix (new matrix out of bounds wrt original)");

        static const bool conformable1 = check_true<((R2 - 1) * RS < R)>::value;
        static const bool conformable2 = check_true<((C2 - 1) * CS < C)>::value;
        static const bool conformable3 = check_true<(RS > 0)>::value;
        static const bool conformable4 = check_true<(CS > 0)>::value;
        static const bool conformable5 = check_true<!(R2 == 1 && RS != 1)>::value;
        static const bool conformable6 = check_true<!(C2 == 1 && CS != 1)>::value;

        assert(ioff  < R - (R2 - 1) * RS);
        assert(joff  < C - (C2 - 1) * CS);

#ifdef CM_V1
        assert(is_contiguous());
#endif

        matrix_ref<T,R2,C2> ret(id());
        for (uint i=0; i<R2; i++) {
            for (uint j=0; j<C2; j++) {
                if ((CS*j + joff) >= C) {
                    // We go off the end of the source row
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert(0 && "select statement access is out-of-bounds on source matrix_ref");
#endif
                    ret.set_elem_ref(C2*i + j, ret.dummy());
                } else if ((C*(RS*i + ioff) + (CS*j) + joff) >= (C * R)) {
                    // We go off the end of the source matrix
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert(0 && "select statement access is out-of-bounds on source matrix_ref");
#endif
                    ret.set_elem_ref(C2*i + j, ret.dummy());
                } else {
                    // Everything is within bounds
                    ret.set_elem_ref(C2*i + j, data[C*(RS*i + ioff) + (CS*j) + joff]);
                }
            }
        }
        return ret;
}

template <typename T, uint R, uint C>
template <uint R2, uint RS, uint C2, uint CS>
const matrix_ref<T,R2,C2> matrix_ref<T,R,C>::select(OFFSET ioff, OFFSET joff) const
{
        CM_STATIC_ERROR((RS > 0), "select does not support a row stride of 0");
        CM_STATIC_ERROR((CS > 0), "select does not support a column stride of 0");
        CM_STATIC_WARNING(!(R2 == 1 && RS != 1), "when row size is 1 the row stride must also be 1");
        CM_STATIC_WARNING(!(C2 == 1 && CS != 1), "when column size is 1 the column stride must also be 1");
        CM_STATIC_WARNING(((C2 - 1) * CS + 1 <= C), "new row must fit inside the source row (new row out of bounds wrt original)");
        CM_STATIC_WARNING(((R2 - 1) * RS + 1 <= R), "new matrix must fit inside the source matrix (new matrix out of bounds wrt original)");

        static const bool conformable1 = check_true<((R2 - 1) * RS < R)>::value;
        static const bool conformable2 = check_true<((C2 - 1) * CS < C)>::value;
        static const bool conformable3 = check_true<(RS > 0)>::value;
        static const bool conformable4 = check_true<(CS > 0)>::value;
        static const bool conformable5 = check_true<!(R2 == 1 && RS != 1)>::value;
        static const bool conformable6 = check_true<!(C2 == 1 && CS != 1)>::value;

        assert(ioff  < R - (R2 - 1) * RS);
        assert(joff  < C - (C2 - 1) * CS);

#ifdef CM_V1
        assert(is_contiguous());
#endif

        matrix_ref<T,R2,C2> ret(id());
        for (uint i=0; i<R2; i++) {
            for (uint j=0; j<C2; j++) {
                if ((CS*j + joff) >= C) {
                    // We go off the end of the source row
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert(0 && "select statement access is out-of-bounds on source matrix_ref");
#endif
                    ret.set_elem_ref(C2*i + j, ret.dummy());
                } else if ((C*(RS*i + ioff) + (CS*j) + joff) >= (C * R)) {
                    // We go off the end of the source matrix
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert(0 && "select statement access is out-of-bounds on source matrix_ref");
#endif
                    ret.set_elem_ref(C2*i + j, ret.dummy());
                } else {
                    // Everything is within bounds
                    ret.set_elem_ref(C2*i + j, data[C*(RS*i + ioff) + (CS*j) + joff]);
                }
            }
        }
        return ret;
}

template <typename T, uint R, uint C>
template <uint R2, uint VS, uint WD, uint HS>
const vector<T, R2*WD> matrix_ref<T,R,C>::genx_select(OFFSET ioff, OFFSET joff)
{
        static const bool conformable1 = check_true<(R2 > 0)>::value;
        static const bool conformable2 = check_true<(VS >= 0)>::value;
        static const bool conformable3 = check_true<(WD > 0)>::value;
        static const bool conformable4 = check_true<(HS >= 0)>::value;
        assert(R2>=0 && VS>=0 && WD>=0 && HS >=0);

        assert(ioff < R);
        assert(joff < C);

        vector<T,R2*WD> ret(id());
        for (uint i=0; i < R2*WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[C*ioff + joff + (i/WD)*VS + (i%WD)*HS], R2*WD, i);
        }
        return ret;
}

//below are 1D iselect for matrix_ref
#if _MSC_VER >= 1700
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index)
{
        return iselect(index, std::is_integral<T2>());
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index, std::true_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index, std::false_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            // in this case index doesn't have integral type elements
            // so can't be used - we will have already generated an error,
            // so just use 0 to allow compilation to continue (in case there
            // are more errors to find...)
            SIMDCF_WRAPPER(ret(i) = *data[0], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index)
{
        return iselect(index, std::is_integral<T2>());
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index, std::true_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index, std::false_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            // in this case index doesn't have integral type elements
            // so can't be used - we will have already generated an error,
            // so just use 0 to allow compilation to continue (in case there
            // are more errors to find...)
            SIMDCF_WRAPPER(ret(i) = *data[0], WD, i);
        }
        return ret;
}
#else
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index.get(i)], WD, i);
        }
        return ret;
}
#endif

//below are 2D iselect for matrix_ref
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index_x,
                                            const vector<T2,WD>& index_y)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index_x.get(i)*C+index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index_x,
                                            const vector<T2,WD>& index_y)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index_x,
                                            const vector_ref<T2,WD>& index_y)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index_x,
                                            const vector_ref<T2,WD>& index_y)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}
//end of 2D iselect for matrix_ref

/*******************************************************************
/
/                         vector
/
*******************************************************************/
template <typename T, uint SZ>
void vector<T, SZ>::assign(const stream<T, SZ> &src) {
    uint i;
    for (i = 0; i < SZ; i++) {
        SIMDCF_WRAPPER((*this)(i) = src.get(i), SZ, i);
    }
}

template <typename T, uint SZ>
void vector<T, SZ>::assign_noSIMDCF(const stream<T, SZ> &src) {
    uint i;
    for (i = 0; i < SZ; i++) {
        (*this)(i) = src.get(i);
    }
}

/*******************************************************************
/
/                  global functions/operators
/
*******************************************************************/
template <typename T, uint SZ>
CM_NOINLINE vector<typename restype<T,int>::type,SZ> operator + (const stream<T,SZ> &x) {
    vector<typename restype<T,int>::type, SZ> ret;

    for (uint i=0; i<SZ; ++i) {
        SIMDCF_WRAPPER(ret(i) =  x.get(i), SZ, i);
    }

    return ret;
}

template <typename T, uint SZ>
CM_NOINLINE vector<typename restype<T,int>::type, SZ> operator - (const stream<T,SZ>& x) {
    vector<typename restype<T,int>::type, SZ> ret;

    for (uint i=0; i<SZ; ++i) {
        SIMDCF_WRAPPER(ret(i) = - x.get(i), SZ, i);
    }

    return ret;
}

template <typename T, uint SZ>
CM_NOINLINE vector<typename restype<T,int>::type, SZ> operator ~ (const stream<T,SZ>& x) {
    vector<typename restype<T,int>::type, SZ> ret;

    for (uint i=0; i<SZ; ++i) {
        SIMDCF_WRAPPER(ret(i) = ~ x.get(i), SZ, i);
    }

    return ret;
}

template <typename T, uint SZ>
CM_NOINLINE vector<ushort, SZ> operator ! (const stream<T,SZ>& x) {
    vector<ushort, SZ> ret;

    for (uint i=0; i<SZ; ++i) {
        SIMDCF_WRAPPER(ret(i) = ! x.get(i), SZ, i);
    }

    return ret;
}

#define binary_arith_op(OP) \
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const stream<T2,SZ>& y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = RT(x.get(i) OP y.get(i)), SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const T2 y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        RT _y = y; \
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x.get(i) OP _y, SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const T1 x, const stream<T2,SZ>& y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        RT _x (x); \
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = _x OP y.get(i), SZ, i);\
        }\
        return ret;\
}\

binary_arith_op(+)
binary_arith_op(-)
binary_arith_op(*)
binary_arith_op(/)
binary_arith_op(%)
#undef binary_arith_op

#define binary_bitwise_op(OP) \
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename bitwise_restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const stream<T2,SZ>& y)\
{\
        typedef typename bitwise_restype<T1,T2>::type RT;\
        static const bool type_conformable = \
            check_true<is_inttype<T1>::value && is_inttype<T2>::value>::value; \
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = RT(x.get(i) OP y.get(i)), SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename bitwise_restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const T2 y)\
{\
        typedef typename bitwise_restype<T1,T2>::type RT;\
        static const bool type_conformable = \
            check_true<is_inttype<T1>::value && is_inttype<T2>::value>::value; \
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x.get(i) OP y, SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename bitwise_restype<T1,T2>::type,SZ> operator OP (const T1 x, const stream<T2,SZ>& y)\
{\
        typedef typename bitwise_restype<T1,T2>::type RT;\
        static const bool type_conformable = \
            check_true<is_inttype<T1>::value && is_inttype<T2>::value>::value; \
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x OP y.get(i), SZ, i); \
        }\
        return ret;\
}\

binary_bitwise_op(&)
binary_bitwise_op(|)
binary_bitwise_op(^)
#undef binary_bitwise_op

// our own enable_if implementation, as icl is not able to parse STL headers from later VS
template <bool, class T = void> struct cm_enable_if {};
template <class T> struct cm_enable_if<true, T> {
    typedef T type;
};

template <typename T, T v>
struct cm_integral_constant {
    typedef T value_type;
    static const value_type value = v;
    typedef cm_integral_constant<T, v> type;
    operator value_type() { return value; }
};

typedef cm_integral_constant<bool, true> cm_true_type;
typedef cm_integral_constant<bool, false> cm_false_type;

template<typename T, typename U> struct cm_is_same : public cm_false_type {};
template<typename T>             struct cm_is_same<T, T> : public cm_true_type{};

template <typename T> struct cm_remove_const          { typedef T type; };
template <typename T> struct cm_remove_const<const T> { typedef T type; };

template <typename T>
struct is_cm_scalar : cm_integral_constant <
    bool,
    cm_is_same<        float, typename cm_remove_const<T>::type>::value ||
    cm_is_same<       double, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         char, typename cm_remove_const<T>::type>::value ||
    cm_is_same<  signed char, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned char, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         short, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned short, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         int, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned int, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         long, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned long, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         long long, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned long long, typename cm_remove_const<T>::type>::value >
{};

#define binary_shift_op(OP) \
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename int_uint_type<T1>::type,SZ> operator OP (const stream<T1,SZ>& x, const stream<T2,SZ>& y)\
{\
        typedef typename int_uint_type<T1>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = RT(x.get(i) OP y.get(i)), SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE typename cm_enable_if<is_cm_scalar<T2>::value, vector<typename int_uint_type<T1>::type,SZ> >::type operator OP (const stream<T1,SZ>& x, const T2 y)\
{\
        typedef typename int_uint_type<T1>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x.get(i) OP y, SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename int_uint_type<T1>::type,SZ> operator OP (const T1 x, const stream<T2,SZ>& y)\
{\
        typedef typename int_uint_type<T1>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x OP y.get(i), SZ, i);\
        }\
        return ret;\
}\

binary_shift_op(>>)
binary_shift_op(<<)
#undef binary_shift_op

#define binary_compare_op(OP) \
\
template<typename T1, uint SZ, typename T2>\
CM_NOINLINE vector<typename ushort_type<T1,T2>::type, SZ> operator OP (const stream<T1, SZ>& x, const T2 y)\
{\
        static const bool type_conformable = cmtype<T2>::value; \
        vector<ushort, SZ> ret((ushort)0);\
        for (int i=0; i<SZ; i++) {\
            ret(i) = 0; \
            SIMDCF_ELEMENT_SKIP(i);\
            if (x.get(i) OP y) {\
                ret(i) = 1;\
            }\
        }\
        return ret;\
}\
\
template<typename T1, uint SZ, typename T2>\
CM_NOINLINE vector<typename ushort_type<T1,T2>::type, SZ> operator OP (const T1 x, const stream<T2, SZ>& y)\
{\
        static const bool type_conformable = cmtype<T1>::value; \
        vector<ushort, SZ> ret((ushort)0);\
        for (int i=0; i<SZ; i++) {\
            SIMDCF_ELEMENT_SKIP(i);\
            if (x OP y.get(i)) {\
                ret(i) = 1;\
            }\
        }\
        return ret;\
}\
\
template<typename T1, uint SZ, typename T2>\
CM_NOINLINE vector<ushort, SZ> operator OP (const stream<T1, SZ>& x, const stream<T2, SZ>& y)\
{\
        vector<ushort, SZ> ret((ushort)0);\
        for (int i=0; i<SZ; i++) {\
            SIMDCF_ELEMENT_SKIP(i);\
            if (x.get(i) OP y.get(i)) {\
                ret(i) = 1;\
            }\
        }\
        return ret;\
}\
\

binary_compare_op(<)
binary_compare_op(<=)
binary_compare_op(>)
binary_compare_op(>=)
binary_compare_op(==)
binary_compare_op(!=)

#define reduce_boolean_op(OP,ReduceOP,initValue)	\
\
template<typename T, uint SZ>\
CM_NOINLINE ushort stream<T, SZ>::OP ( void ) const	\
{\
        static const bool type_conformable = cmtype<T>::value; \
        ushort ret((ushort)initValue);\
        for (int i=0; i<SZ; i++) {\
            SIMDCF_WRAPPER(ret = (get(i) ReduceOP ret), SZ, i); 	\
            if ( ret!=initValue ) { return ret; } \
        }\
        return ret;\
}\
\

//SIMDCF_WRAPPER(ret = (get(i) ReduceOP ret), i);

reduce_boolean_op(any,||,0)
reduce_boolean_op(all,&&,1)

#endif /* CM_VM_H */
