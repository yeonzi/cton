
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * CTON memory hook structure.
 * These function is supposed to have the same function with malloc/free in the
 * stdlib.
 */
typedef struct {
    void *      pool;
    void *    (*palloc)(void *pool, size_t size);
    void *    (*prealloc)(void *pool, void *ptr, size_t size);
    void      (*pfree)(void *pool, void *ptr);
    void      (*pdestroy)(void *pool);
} cton_memhook;

extern cton_memhook cton_std_hook;

/*
 * CTON object types
 */

typedef struct cton_obj_s       cton_obj;
typedef struct cton_string_s    cton_string;
typedef struct cton_array_s     cton_array;
typedef struct cton_hash_item_s cton_hash_item;
typedef struct cton_hash_s      cton_hash;

enum cton_type_e {
    CTON_INVALID = 0,
    CTON_OBJECT  = 1,
    CTON_NULL    = 2,
    CTON_BOOL    = 3,
    CTON_BINARY  = 4,
    CTON_STRING  = 5,
    CTON_ARRAY   = 6,
    CTON_HASH    = 7,
    CTON_INT8    = 8,
    CTON_INT16   = 9,
    CTON_INT32   = 10,
    CTON_INT64   = 11,
    CTON_UINT8   = 12,
    CTON_UINT16  = 13,
    CTON_UINT32  = 14,
    CTON_UINT64  = 15,
    CTON_FLOAT8  = 16,
    CTON_FLOAT16 = 17,
    CTON_FLOAT32 = 18,
    CTON_FLOAT64 = 19,
};

#define CTON_TYPE_CNT 20

enum cton_bool_e {
    CTON_FALSE = 0,
    CTON_TRUE  = 1
};

struct cton_string_s {
    size_t   len;
    size_t   used;
    uint8_t *ptr;
};

struct cton_array_s {
    size_t       len;
    size_t       used;
    cton_obj   **ptr;
    enum cton_type_e    sub_type;
};

struct cton_hash_item_s {
    cton_obj        *key;
    cton_obj        *value;
    cton_hash_item  *next;
};

struct cton_hash_s {
    cton_hash_item *root;
    cton_hash_item *last;
};

struct cton_obj_s {
    uint64_t  magic;  /* Magic number for debug. */

    union {
        enum cton_bool_e   b;
        cton_string str;
        cton_array  arr;
        cton_hash   hash;
        int8_t      i8;
        int16_t     i16;
        int32_t     i32;
        int64_t     i64;
        uint8_t     u8;
        uint16_t    u16;
        uint32_t    u32;
        uint64_t    u64;
        float       f32;
        double      f64;
    } payload;

    /* For object pool */
    struct cton_obj_s *next;
    struct cton_obj_s *prev;

    enum cton_type_e type;   /* CTON type */
};

typedef enum   cton_type_e      cton_type;
typedef enum   cton_bool_e      cton_bool;

#define CTON_STRUCT_MAGIC 0x4E4F5443




/*******************************************************************************
 * CTON context types
 * 用来保存内存池结构体，对应的错误指，节点池，以及全局参数。
 * 同一个上下文结构体不保证写线程安全。
 * TODO 是否保留一个内存池的互斥锁接口，通过锁住整个上下文的方式保证线程安全？
 * 
 * 不同上下文结构体之间应保证线程安全。（避免static变量的使用）
 * 是否要求所有函数都使用上下文结构体作为第一个传入参数？
 ******************************************************************************/

typedef struct cton_ctx_s       cton_ctx;

enum cton_error_e {
    CTON_OK            = 0,
    CTON_ERROR_EMHOOK  = 1,   /* Invalid Memory hook */
    CTON_ERROR_EALLOC  = 2,   /* Memory allocation failed */
    CTON_ERROR_TYPE    = 3,
    CTON_ERROR_IMPLEM  = 4,
    CTON_ERROR_OVF     = 5,
    CTON_ERROR_REPLACE = 6,
    CTON_ERROR_SUBTYPE = 7,
    CTON_ERROR_INDEX,
    CTON_ERROR_UNKNOWN = 127  /* Unknown error */
};

typedef enum cton_error_e       cton_err;

struct cton_ctx_s {
    cton_memhook memhook;

    cton_obj *root; /* 根节点 */

    /* Object pool */
    cton_obj *nodes;
    cton_obj *nodes_last;

    cton_err  err;
};

/* cton_alloc.c */
cton_memhook* cton_memhook_init (void * pool,
    void *    (*palloc)(void *pool, size_t size),
    void *    (*prealloc)(void *pool, void *ptr, size_t size),
    void      (*pfree)(void *pool, void *ptr),
    void      (*pdestroy)(void *pool));

static void * cton_alloc(cton_ctx *ctx, size_t size);
static void cton_free(cton_ctx *ctx, void *ptr);
static void * cton_realloc(cton_ctx *ctx, void *ptr, size_t size_ori, size_t size_new);
static void cton_pdestroy(cton_ctx *ctx);

/* cton.c */
cton_ctx *cton_init(cton_memhook *hook);
int cton_destory(cton_ctx *ctx);
void cton_seterr(cton_ctx *ctx, cton_err err);
cton_err cton_geterr(cton_ctx *ctx);
char * cton_strerr(cton_err err);
#define cton_err_clear(ctx) {cton_seterr((ctx), CTON_OK)}

int cton_tree_setroot(cton_ctx *ctx, cton_obj *obj);
cton_obj *cton_tree_getroot(cton_ctx *ctx);
cton_obj *cton_tree_get_by_path(cton_ctx *ctx, cton_obj *path);

/* cton_object.c */
cton_obj * cton_object_create(cton_ctx *ctx, cton_type type);
void cton_object_delete(cton_ctx *ctx, cton_obj *obj);
cton_type cton_object_gettype(cton_ctx *ctx, cton_obj *obj);
void * cton_object_getvalue(cton_ctx *ctx, cton_obj *obj);

static void cton_string_init(cton_ctx *ctx, cton_obj *str);
uint8_t * cton_str_getptr(cton_ctx *ctx, cton_obj *obj);
size_t cton_str_getlen(cton_ctx *ctx, cton_obj *obj);
int cton_str_setlen(cton_ctx *ctx, cton_obj *obj, size_t len);
void * cton_binary_getptr(cton_ctx *ctx, cton_obj *obj);

size_t cton_array_getlen(cton_ctx *ctx, cton_obj *arr);
size_t cton_array_setlen(cton_ctx *ctx, cton_obj *arr, size_t len);
int cton_array_settype(cton_ctx *ctx, cton_obj *arr, cton_type type);
cton_type cton_array_gettype(cton_ctx *ctx, cton_obj *arr);
cton_obj * cton_array_get(cton_ctx *ctx, cton_obj *arr, size_t index);
int cton_array_set(cton_ctx *ctx, cton_obj *arr, cton_obj *obj, size_t index);
void * cton_array_getptr(cton_ctx *ctx, cton_obj *obj);
static void cton_array_init(cton_ctx *ctx, cton_obj *obj);

cton_obj * cton_hash_set(cton_ctx *ctx, cton_obj *h, cton_obj *k, cton_obj *v);
cton_obj * cton_hash_get(cton_ctx *ctx, cton_obj *h, cton_obj *k);

int64_t cton_numeric_getint(cton_ctx *ctx, cton_obj *obj);
uint64_t cton_numeric_getuint(cton_ctx *ctx, cton_obj *obj);
double cton_numeric_getfloat(cton_ctx *ctx, cton_obj *obj);

/* cton_util.c */
static size_t cton_util_align(size_t size, size_t align);
static void * cton_util_memcpy(void *dst, const void *src, size_t n);
static void * cton_util_memset(void *b, int c, size_t len);
cton_obj * cton_str_fromcstr(cton_ctx *ctx,
    const char *str, char end, char quote);
cton_obj * cton_str_new_cstr(cton_ctx *ctx, const char *cstr);
