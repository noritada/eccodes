/*
 * (C) Copyright 2005- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities granted to it by
 * virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
 */

#include "grib_api_internal.h"
#include <ctype.h>
/*
   This is used by make_class.pl

   START_CLASS_DEF
   CLASS      = dumper
   IMPLEMENTS = dump_long;dump_bits
   IMPLEMENTS = dump_double;dump_string;dump_string_array
   IMPLEMENTS = dump_bytes;dump_values
   IMPLEMENTS = dump_label;dump_section
   IMPLEMENTS = init;destroy
   IMPLEMENTS = header;footer
   MEMBERS = long section_offset
   MEMBERS = long empty
   MEMBERS = long end
   MEMBERS = long isLeaf
   MEMBERS = long isAttribute
   MEMBERS = grib_string_list* keys
   END_CLASS_DEF

 */


/* START_CLASS_IMP */

/*

Don't edit anything between START_CLASS_IMP and END_CLASS_IMP
Instead edit values between START_CLASS_DEF and END_CLASS_DEF
or edit "dumper.class" and rerun ./make_class.pl

*/

static void init_class      (grib_dumper_class*);
static int init            (grib_dumper* d);
static int destroy         (grib_dumper*);
static void dump_long       (grib_dumper* d, grib_accessor* a,const char* comment);
static void dump_bits       (grib_dumper* d, grib_accessor* a,const char* comment);
static void dump_double     (grib_dumper* d, grib_accessor* a,const char* comment);
static void dump_string     (grib_dumper* d, grib_accessor* a,const char* comment);
static void dump_string_array     (grib_dumper* d, grib_accessor* a,const char* comment);
static void dump_bytes      (grib_dumper* d, grib_accessor* a,const char* comment);
static void dump_values     (grib_dumper* d, grib_accessor* a);
static void dump_label      (grib_dumper* d, grib_accessor* a,const char* comment);
static void dump_section    (grib_dumper* d, grib_accessor* a,grib_block_of_accessors* block);
static void header         (grib_dumper*,grib_handle*);
static void footer         (grib_dumper*,grib_handle*);

typedef struct grib_dumper_bufr_encode_C {
    grib_dumper          dumper;
    /* Members defined in bufr_encode_C */
    long section_offset;
    long empty;
    long end;
    long isLeaf;
    long isAttribute;
    grib_string_list* keys;
} grib_dumper_bufr_encode_C;


static grib_dumper_class _grib_dumper_class_bufr_encode_C = {
    0,                              /* super                     */
    "bufr_encode_C",                              /* name                      */
    sizeof(grib_dumper_bufr_encode_C),     /* size                      */
    0,                                   /* inited */
    &init_class,                         /* init_class */
    &init,                               /* init                      */
    &destroy,                            /* free mem                       */
    &dump_long,                          /* dump long         */
    &dump_double,                        /* dump double    */
    &dump_string,                        /* dump string    */
    &dump_string_array,                        /* dump string array   */
    &dump_label,                         /* dump labels  */
    &dump_bytes,                         /* dump bytes  */
    &dump_bits,                          /* dump bits   */
    &dump_section,                       /* dump section      */
    &dump_values,                        /* dump values   */
    &header,                             /* header   */
    &footer,                             /* footer   */
};

grib_dumper_class* grib_dumper_class_bufr_encode_C = &_grib_dumper_class_bufr_encode_C;

/* END_CLASS_IMP */
static void dump_attributes(grib_dumper* d, grib_accessor* a, const char* prefix);

static int depth = 0;

static void init_class(grib_dumper_class* c) {}

static int init(grib_dumper* d)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    grib_context* c                 = d->context;
    self->section_offset            = 0;
    self->empty                     = 1;
    d->count                        = 1;
    self->isLeaf                    = 0;
    self->isAttribute               = 0;
    self->keys                      = (grib_string_list*)grib_context_malloc_clear(c, sizeof(grib_string_list));

    return GRIB_SUCCESS;
}

static int destroy(grib_dumper* d)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    grib_string_list* next          = self->keys;
    grib_string_list* cur           = NULL;
    grib_context* c                 = d->context;
    while (next) {
        cur  = next;
        next = next->next;
        grib_context_free(c, cur->value);
        grib_context_free(c, cur);
    }
    return GRIB_SUCCESS;
}

static char* lval_to_string(grib_context* c, long v)
{
    const size_t svalMaxLen = 40;
    char* sval = (char*)grib_context_malloc_clear(c, sizeof(char) * svalMaxLen);
    if (v == GRIB_MISSING_LONG)
        snprintf(sval, svalMaxLen, "CODES_MISSING_LONG");
    else
        snprintf(sval, svalMaxLen, "%ld", v);
    return sval;
}
static char* dval_to_string(grib_context* c, double v)
{
    const size_t svalMaxLen = 40;
    char* sval = (char*)grib_context_malloc_clear(c, sizeof(char) * svalMaxLen);
    if (v == GRIB_MISSING_DOUBLE)
        snprintf(sval, svalMaxLen, "CODES_MISSING_DOUBLE");
    else
        snprintf(sval, svalMaxLen, "%.18e", v);
    return sval;
}

static void dump_values(grib_dumper* d, grib_accessor* a)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    double value                    = 0;
    size_t size = 0, size2 = 0;
    double* values                  = NULL;
    int err                         = 0;
    int i, r, icount;
    int cols   = 2;
    long count = 0;
    char* sval;
    grib_context* c = a->context;
    grib_handle* h  = grib_handle_of_accessor(a);

    if ((a->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0 || (a->flags & GRIB_ACCESSOR_FLAG_READ_ONLY) != 0)
        return;

    grib_value_count(a, &count);
    size = size2 = count;

    if (size > 1) {
        values = (double*)grib_context_malloc_clear(c, sizeof(double) * size);
        err    = grib_unpack_double(a, values, &size2);
    }
    else {
        err = grib_unpack_double(a, &value, &size2);
    }
    Assert(size2 == size);

    self->empty = 0;

    if (size > 1) {
        fprintf(self->dumper.out, "  free(rvalues); rvalues = NULL;\n\n");
        fprintf(self->dumper.out, "  size = %lu;\n", (unsigned long)size);
        fprintf(self->dumper.out, "  rvalues = (double*)malloc(size * sizeof(double));\n");
        fprintf(self->dumper.out, "  if (!rvalues) { fprintf(stderr, \"Failed to allocate memory (%s).\\n\"); return 1; }", a->name);

        icount = 0;
        for (i = 0; i < size - 1; ++i) {
            if (icount > cols || i == 0) {
                fprintf(self->dumper.out, "\n  ");
                icount = 0;
            }
            sval = dval_to_string(c, values[i]);
            fprintf(self->dumper.out, "rvalues[%d]=%s; ", i, sval);
            grib_context_free(c, sval);
            icount++;
        }
        if (icount > cols || i == 0) {
            fprintf(self->dumper.out, "\n  ");
        }
        sval = dval_to_string(c, values[i]);
        fprintf(self->dumper.out, "rvalues[%d]=%s;", i, sval);
        grib_context_free(c, sval);

        depth -= 2;
        fprintf(self->dumper.out, "\n");
        grib_context_free(c, values);

        if ((r = compute_bufr_key_rank(h, self->keys, a->name)) != 0)
            fprintf(self->dumper.out, "  CODES_CHECK(codes_set_double_array(h, \"#%d#%s\",rvalues, size), 0);\n", r, a->name);
        else
            fprintf(self->dumper.out, "  CODES_CHECK(codes_set_double_array(h, \"%s\", rvalues, size), 0);\n", a->name);
    }
    else {
        r    = compute_bufr_key_rank(h, self->keys, a->name);
        sval = dval_to_string(c, value);
        if (r != 0)
            fprintf(self->dumper.out, "  CODES_CHECK(codes_set_double(h, \"#%d#%s\", %s), 0);\n", r, a->name, sval);
        else
            fprintf(self->dumper.out, "  CODES_CHECK(codes_set_double(h, \"%s\", %s), 0);\n", a->name, sval);
        grib_context_free(c, sval);
    }

    if (self->isLeaf == 0) {
        char* prefix;
        int dofree = 0;

        if (r != 0) {
            const size_t prefixMaxLen = strlen(a->name) + 10;
            prefix = (char*)grib_context_malloc_clear(c, sizeof(char) * prefixMaxLen);
            dofree = 1;
            snprintf(prefix, prefixMaxLen, "#%d#%s", r, a->name);
        }
        else
            prefix = (char*)a->name;

        dump_attributes(d, a, prefix);
        if (dofree)
            grib_context_free(c, prefix);
        depth -= 2;
    }

    (void)err; /* TODO */
}

static void dump_values_attribute(grib_dumper* d, grib_accessor* a, const char* prefix)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    double value                    = 0;
    size_t size = 0, size2 = 0;
    double* values                  = NULL;
    int err                         = 0;
    int i, icount;
    int cols   = 2;
    long count = 0;
    char* sval;
    grib_context* c = a->context;

    if ((a->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0 || (a->flags & GRIB_ACCESSOR_FLAG_READ_ONLY) != 0)
        return;

    grib_value_count(a, &count);
    size = size2 = count;

    if (size > 1) {
        values = (double*)grib_context_malloc_clear(c, sizeof(double) * size);
        err    = grib_unpack_double(a, values, &size2);
    }
    else {
        err = grib_unpack_double(a, &value, &size2);
    }
    Assert(size2 == size);

    self->empty = 0;

    if (size > 1) {
        fprintf(self->dumper.out, "  free(rvalues); rvalues = NULL;\n");
        fprintf(self->dumper.out, "  size = %lu;\n", (unsigned long)size);
        fprintf(self->dumper.out, "  rvalues = (double*)malloc(size * sizeof(double));\n");
        fprintf(self->dumper.out, "  if (!rvalues) { fprintf(stderr, \"Failed to allocate memory (%s).\\n\"); return 1; }", a->name);

        icount = 0;
        for (i = 0; i < size - 1; ++i) {
            if (icount > cols || i == 0) {
                fprintf(self->dumper.out, "\n  ");
                icount = 0;
            }
            sval = dval_to_string(c, values[i]);
            fprintf(self->dumper.out, "rvalues[%d]=%s; ", i, sval);
            grib_context_free(c, sval);
            icount++;
        }
        if (icount > cols || i == 0) {
            fprintf(self->dumper.out, "\n  ");
        }
        sval = dval_to_string(c, values[i]);
        fprintf(self->dumper.out, "rvalues[%d]=%s;", i, sval);
        grib_context_free(c, sval);

        depth -= 2;
        fprintf(self->dumper.out, "\n");
        grib_context_free(c, values);

        fprintf(self->dumper.out, "  CODES_CHECK(codes_set_double_array(h, \"%s->%s\", rvalues, size), 0);\n", prefix, a->name);
    }
    else {
        sval = dval_to_string(c, value);
        fprintf(self->dumper.out, "  CODES_CHECK(codes_set_double(h, \"%s->%s\", %s), 0);\n", prefix, a->name, sval);
        grib_context_free(c, sval);
    }

    if (self->isLeaf == 0) {
        const size_t prefix1MaxLen =  strlen(a->name) + strlen(prefix) + 5;
        char* prefix1 = (char*)grib_context_malloc_clear(c, sizeof(char) * prefix1MaxLen);
        snprintf(prefix1, prefix1MaxLen, "%s->%s", prefix, a->name);

        dump_attributes(d, a, prefix1);

        grib_context_free(c, prefix1);
        depth -= 2;
    }

    (void)err; /* TODO */
}

static int is_hidden(grib_accessor* a)
{
    return ( (a->flags & GRIB_ACCESSOR_FLAG_HIDDEN) != 0 );
}

static void dump_long(grib_dumper* d, grib_accessor* a, const char* comment)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    long value                      = 0;
    size_t size = 0, size2 = 0;
    long* values                    = NULL;
    int err                         = 0;
    int i, r, icount;
    int cols                        = 4;
    long count                      = 0;
    char* sval                      = NULL;
    grib_context* c                 = a->context;
    grib_handle* h                  = grib_handle_of_accessor(a);
    int doing_unexpandedDescriptors = 0;

    if ((a->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0) { /* key does not have the dump attribute */
        int skip = 1;
        /* See ECC-1107 */
        if (!is_hidden(a) && strcmp(a->name, "messageLength") == 0) skip = 0;
        if (skip) return;
    }

    doing_unexpandedDescriptors = (strcmp(a->name, "unexpandedDescriptors") == 0);
    grib_value_count(a, &count);
    size = size2 = count;

    if ((a->flags & GRIB_ACCESSOR_FLAG_READ_ONLY) != 0) {
        if (self->isLeaf == 0) {
            char* prefix;
            int dofree = 0;

            r = compute_bufr_key_rank(h, self->keys, a->name);
            if (r != 0) {
                const size_t prefixMaxLen = strlen(a->name) + 10;
                prefix = (char*)grib_context_malloc_clear(c, sizeof(char) * prefixMaxLen);
                dofree = 1;
                snprintf(prefix, prefixMaxLen, "#%d#%s", r, a->name);
            }
            else
                prefix = (char*)a->name;

            dump_attributes(d, a, prefix);
            if (dofree)
                grib_context_free(c, prefix);
            depth -= 2;
        }
        return;
    }

    if (size > 1) {
        values = (long*)grib_context_malloc_clear(a->context, sizeof(long) * size);
        err    = grib_unpack_long(a, values, &size2);
    }
    else {
        err = grib_unpack_long(a, &value, &size2);
    }
    Assert(size2 == size);

    self->empty = 0;

    if (size > 1) {
        fprintf(self->dumper.out, "  free(ivalues); ivalues = NULL;\n\n");
        fprintf(self->dumper.out, "  size = %lu;\n", (unsigned long)size);
        fprintf(self->dumper.out, "  ivalues = (long*)malloc(size * sizeof(long));\n");
        fprintf(self->dumper.out, "  if (!ivalues) { fprintf(stderr, \"Failed to allocate memory (%s).\\n\"); return 1; }", a->name);

        icount = 0;
        for (i = 0; i < size - 1; i++) {
            if (icount > cols || i == 0) {
                fprintf(self->dumper.out, "\n  ");
                icount = 0;
            }
            fprintf(self->dumper.out, "ivalues[%d]=%ld; ", i, values[i]);
            icount++;
        }
        if (icount > cols || i == 0) {
            fprintf(self->dumper.out, "\n  ");
        }
        fprintf(self->dumper.out, "ivalues[%d]=%ld;", i, values[i]);

        depth -= 2;
        fprintf(self->dumper.out, "\n");
        grib_context_free(a->context, values);

        if ((r = compute_bufr_key_rank(h, self->keys, a->name)) != 0) {
            fprintf(self->dumper.out, "  CODES_CHECK(codes_set_long_array(h, \"#%d#%s\", ivalues, size), 0);\n", r, a->name);
        }
        else {
            if (doing_unexpandedDescriptors) {
                fprintf(self->dumper.out, "\n  /* Create the structure of the data section */\n");
                /* fprintf(self->dumper.out,"  CODES_CHECK(codes_set_long(h, \"skipExtraKeyAttributes\", 1), 0);\n"); */
            }
            fprintf(self->dumper.out, "  CODES_CHECK(codes_set_long_array(h, \"%s\", ivalues, size), 0);\n", a->name);
            if (doing_unexpandedDescriptors)
                fprintf(self->dumper.out, "\n");
        }
    }
    else {
        r    = compute_bufr_key_rank(h, self->keys, a->name);
        sval = lval_to_string(c, value);
        if (r != 0) {
            fprintf(self->dumper.out, "  CODES_CHECK(codes_set_long(h, \"#%d#%s\", ", r, a->name);
        }
        else {
            if (doing_unexpandedDescriptors) {
                fprintf(self->dumper.out, "\n  /* Create the structure of the data section */\n");
                /* fprintf(self->dumper.out,"  CODES_CHECK(codes_set_long(h, \"skipExtraKeyAttributes\", 1), 0);\n"); */
            }
            fprintf(self->dumper.out, "  CODES_CHECK(codes_set_long(h, \"%s\", ", a->name);
        }

        fprintf(self->dumper.out, "%s), 0);\n", sval);
        grib_context_free(c, sval);
        if (doing_unexpandedDescriptors)
            fprintf(self->dumper.out, "\n");
    }

    if (self->isLeaf == 0) {
        char* prefix;
        int dofree = 0;

        if (r != 0) {
            const size_t prefixMaxLen = strlen(a->name) + 10;
            prefix = (char*)grib_context_malloc_clear(c, sizeof(char) * prefixMaxLen);
            dofree = 1;
            snprintf(prefix, prefixMaxLen, "#%d#%s", r, a->name);
        }
        else
            prefix = (char*)a->name;

        dump_attributes(d, a, prefix);
        if (dofree)
            grib_context_free(c, prefix);
        depth -= 2;
    }
    (void)err; /* TODO */
}

static void dump_long_attribute(grib_dumper* d, grib_accessor* a, const char* prefix)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    long value                      = 0;
    size_t size = 0, size2 = 0;
    long* values                    = NULL;
    int err                         = 0;
    int i, icount;
    int cols        = 4;
    long count      = 0;
    grib_context* c = a->context;

    if ((a->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0 || (a->flags & GRIB_ACCESSOR_FLAG_READ_ONLY) != 0)
        return;

    grib_value_count(a, &count);
    size = size2 = count;

    if (size > 1) {
        values = (long*)grib_context_malloc_clear(a->context, sizeof(long) * size);
        err    = grib_unpack_long(a, values, &size2);
    }
    else {
        err = grib_unpack_long(a, &value, &size2);
    }
    Assert(size2 == size);

    self->empty = 0;

    if (size > 1) {
        fprintf(self->dumper.out, "  free(ivalues); ivalues = NULL;\n");
        fprintf(self->dumper.out, "  size = %lu;\n", (unsigned long)size);
        fprintf(self->dumper.out, "  ivalues = (long*)malloc(size * sizeof(long));\n");
        fprintf(self->dumper.out, "  if (!ivalues) { fprintf(stderr, \"Failed to allocate memory (%s).\\n\"); return 1; }", a->name);

        icount = 0;
        for (i = 0; i < size - 1; i++) {
            if (icount > cols || i == 0) {
                fprintf(self->dumper.out, "\n  ");
                icount = 0;
            }
            fprintf(self->dumper.out, "ivalues[%d]=%ld; ", i, values[i]);
            icount++;
        }
        if (icount > cols || i == 0) {
            fprintf(self->dumper.out, "\n  ");
        }
        fprintf(self->dumper.out, "ivalues[%d]=%ld;", i, values[i]);

        depth -= 2;
        fprintf(self->dumper.out, "\n");
        grib_context_free(a->context, values);

        fprintf(self->dumper.out, "  CODES_CHECK(codes_set_long_array(h, \"%s->%s\", ivalues, size), 0);\n", prefix, a->name);
    }
    else {
        char* sval = lval_to_string(c, value);
        fprintf(self->dumper.out, "  CODES_CHECK(codes_set_long(h, \"%s->%s\", ", prefix, a->name);
        fprintf(self->dumper.out, "%s), 0);\n", sval);
        grib_context_free(c, sval);
    }

    if (self->isLeaf == 0) {
        const size_t prefix1MaxLen = strlen(a->name) + strlen(prefix) + 5;
        char* prefix1 = (char*)grib_context_malloc_clear(c, sizeof(char) * prefix1MaxLen);
        snprintf(prefix1, prefix1MaxLen, "%s->%s", prefix, a->name);

        dump_attributes(d, a, prefix1);

        grib_context_free(c, prefix1);
        depth -= 2;
    }
    (void)err; /* TODO */
}

static void dump_bits(grib_dumper* d, grib_accessor* a, const char* comment)
{
}

static void dump_double(grib_dumper* d, grib_accessor* a, const char* comment)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    double value                    = 0;
    size_t size                     = 1;
    int r;
    char* sval;
    grib_handle* h  = grib_handle_of_accessor(a);
    grib_context* c = h->context;

    if ((a->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0 || (a->flags & GRIB_ACCESSOR_FLAG_READ_ONLY) != 0)
        return;

    grib_unpack_double(a, &value, &size);
    self->empty = 0;

    r = compute_bufr_key_rank(h, self->keys, a->name);

    sval = dval_to_string(c, value);
    if (r != 0)
        fprintf(self->dumper.out, "  CODES_CHECK(codes_set_double(h, \"#%d#%s\", %s), 0);\n", r, a->name, sval);
    else
        fprintf(self->dumper.out, "  CODES_CHECK(codes_set_double(h, \"%s\", %s), 0);\n", a->name, sval);
    grib_context_free(c, sval);

    if (self->isLeaf == 0) {
        char* prefix;
        int dofree = 0;

        if (r != 0) {
            const size_t prefixMaxLen = strlen(a->name) + 10;
            prefix = (char*)grib_context_malloc_clear(c, sizeof(char) * prefixMaxLen);
            dofree = 1;
            snprintf(prefix, prefixMaxLen, "#%d#%s", r, a->name);
        }
        else
            prefix = (char*)a->name;

        dump_attributes(d, a, prefix);
        if (dofree)
            grib_context_free(c, prefix);
        depth -= 2;
    }
}

static void dump_string_array(grib_dumper* d, grib_accessor* a, const char* comment)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    char** values;
    size_t size = 0, i = 0;
    grib_context* c = a->context;
    int err         = 0;
    long count      = 0;
    int r           = 0;
    grib_handle* h  = grib_handle_of_accessor(a);

    if ((a->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0 || (a->flags & GRIB_ACCESSOR_FLAG_READ_ONLY) != 0)
        return;

    grib_value_count(a, &count);
    size = count;
    if (size == 1) {
        dump_string(d, a, comment);
        return;
    }

    fprintf(self->dumper.out, "  free(svalues);\n");
    fprintf(self->dumper.out, "  size = %lu;\n", (unsigned long)size);
    fprintf(self->dumper.out, "  svalues = (char**)malloc(size * sizeof(char*));\n");
    fprintf(self->dumper.out, "  if (!svalues) { fprintf(stderr, \"Failed to allocate memory (%s).\\n\"); return 1; }\n", a->name);

    self->empty = 0;
    values      = (char**)grib_context_malloc_clear(c, size * sizeof(char*));
    if (!values) {
        grib_context_log(c, GRIB_LOG_FATAL, "Memory allocation error: %zu bytes", size);
        return;
    }

    err = grib_unpack_string_array(a, values, &size);
    for (i = 0; i < size - 1; i++) {
        fprintf(self->dumper.out, "  svalues[%lu]=\"%s\"; \n", (unsigned long)i, values[i]);
    }
    fprintf(self->dumper.out, "  svalues[%lu]=\"%s\";\n", (unsigned long)i, values[i]);

    if (self->isLeaf == 0) {
        if ((r = compute_bufr_key_rank(h, self->keys, a->name)) != 0)
            fprintf(self->dumper.out, "  codes_set_string_array(h, \"#%d#%s\", (const char **)svalues, size);\n", r, a->name);
        else
            fprintf(self->dumper.out, "  codes_set_string_array(h, \"%s\", (const char **)svalues, size);\n", a->name);
    }

    if (self->isLeaf == 0) {
        char* prefix;
        int dofree = 0;

        if (r != 0) {
            const size_t prefixMaxLen = strlen(a->name) + 10;
            prefix = (char*)grib_context_malloc_clear(c, sizeof(char) * prefixMaxLen);
            dofree = 1;
            snprintf(prefix, prefixMaxLen, "#%d#%s", r, a->name);
        }
        else
            prefix = (char*)a->name;

        dump_attributes(d, a, prefix);
        if (dofree)
            grib_context_free(c, prefix);
        depth -= 2;
    }

    for (i = 0; i < size; i++) grib_context_free(c, values[i]);
    grib_context_free(c, values);
    (void)err; /* TODO */
}

static void dump_string(grib_dumper* d, grib_accessor* a, const char* comment)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    char* value                     = NULL;
    char* p                         = NULL;
    size_t size                     = 0;
    grib_context* c                 = a->context;
    int r = 0, err = 0;
    grib_handle* h = grib_handle_of_accessor(a);
    const char* acc_name = a->name;

    ecc__grib_get_string_length(a, &size);
    if (size == 0)
        return;

    if ((a->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0 || (a->flags & GRIB_ACCESSOR_FLAG_READ_ONLY) != 0)
        return;

    value = (char*)grib_context_malloc_clear(c, size);
    if (!value) {
        grib_context_log(c, GRIB_LOG_FATAL, "Memory allocation error: %zu bytes", size);
        return;
    }

    self->empty = 0;

    err = grib_unpack_string(a, value, &size);
    p   = value;
    r   = compute_bufr_key_rank(h, self->keys, acc_name);
    if (grib_is_missing_string(a, (unsigned char*)value, size)) {
        strcpy(value, ""); /* Empty string means MISSING string */
    }

    while (*p) {
        if (!isprint(*p))
            *p = '?';
        if (*p == '"')
            *p = '\''; /* ECC-1401 */
        p++;
    }

    fprintf(self->dumper.out, "  size = %lu;\n", (unsigned long)size);
    if (self->isLeaf == 0) {
        depth += 2;
        if (r != 0)
            fprintf(self->dumper.out, "  codes_set_string(h, \"#%d#%s\", ", r, acc_name);
        else
            fprintf(self->dumper.out, "  codes_set_string(h, \"%s\", ", acc_name);
    }
    fprintf(self->dumper.out, "\"%s\", &size);\n", value);

    if (self->isLeaf == 0) {
        char* prefix;
        int dofree = 0;

        if (r != 0) {
            const size_t prefixMaxLen = strlen(acc_name) + 10;
            prefix = (char*)grib_context_malloc_clear(c, sizeof(char) * prefixMaxLen);
            dofree = 1;
            snprintf(prefix, prefixMaxLen, "#%d#%s", r, acc_name);
        }
        else
            prefix = (char*)acc_name;

        dump_attributes(d, a, prefix);
        if (dofree)
            grib_context_free(c, prefix);
        depth -= 2;
    }

    grib_context_free(c, value);
    (void)err; /* TODO */
}

static void dump_bytes(grib_dumper* d, grib_accessor* a, const char* comment)
{
}

static void dump_label(grib_dumper* d, grib_accessor* a, const char* comment)
{
}

static void _dump_long_array(grib_handle* h, FILE* f, const char* key, const char* print_key)
{
    long* val;
    size_t size = 0, i;
    int cols = 9, icount = 0;

    if (grib_get_size(h, key, &size) == GRIB_NOT_FOUND)
        return;
    if (size == 0)
        return;

    fprintf(f, "  free(ivalues); ivalues = NULL;\n");
    fprintf(f, "  size = %lu;\n", (unsigned long)size);
    fprintf(f, "  ivalues = (long*)malloc(size * sizeof(long));\n");
    fprintf(f, "  if (!ivalues) { fprintf(stderr, \"Failed to allocate memory (%s).\\n\"); return 1; }", key);

    val = (long*)grib_context_malloc_clear(h->context, sizeof(long) * size);
    grib_get_long_array(h, key, val, &size);
    for (i = 0; i < size - 1; i++) {
        if (icount > cols || i == 0) {
            fprintf(f, "\n  ");
            icount = 0;
        }
        fprintf(f, "ivalues[%lu]=%ld; ", (unsigned long)i, val[i]);
        icount++;
    }
    if (icount > cols) {
        fprintf(f, "\n  ");
    }
    fprintf(f, "ivalues[%lu]=%ld;\n", (unsigned long)(size - 1), val[size - 1]);

    grib_context_free(h->context, val);
    fprintf(f, "  CODES_CHECK(codes_set_long_array(h, \"%s\", ivalues, size), 0);\n", print_key);
}

static void dump_section(grib_dumper* d, grib_accessor* a, grib_block_of_accessors* block)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    if (strcmp(a->name, "BUFR")==0 ||
        strcmp(a->name, "GRIB")==0 ||
        strcmp(a->name, "META")==0) {
        grib_handle* h = grib_handle_of_accessor(a);
        depth          = 2;
        self->empty    = 1;
        depth += 2;
        _dump_long_array(h, self->dumper.out, "dataPresentIndicator", "inputDataPresentIndicator");
        _dump_long_array(h, self->dumper.out, "delayedDescriptorReplicationFactor", "inputDelayedDescriptorReplicationFactor");
        _dump_long_array(h, self->dumper.out, "shortDelayedDescriptorReplicationFactor", "inputShortDelayedDescriptorReplicationFactor");
        _dump_long_array(h, self->dumper.out, "extendedDelayedDescriptorReplicationFactor", "inputExtendedDelayedDescriptorReplicationFactor");
        _dump_long_array(h, self->dumper.out, "inputOverriddenReferenceValues", "inputOverriddenReferenceValues");
        grib_dump_accessors_block(d, block);
        depth -= 2;
    }
    else if (strcmp(a->name, "groupNumber")==0) {
        if ((a->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0)
            return;
        self->empty = 1;
        depth += 2;
        grib_dump_accessors_block(d, block);
        depth -= 2;
    }
    else {
        grib_dump_accessors_block(d, block);
    }
}

static void dump_attributes(grib_dumper* d, grib_accessor* a, const char* prefix)
{
    int i                           = 0;
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    unsigned long flags;
    while (i < MAX_ACCESSOR_ATTRIBUTES && a->attributes[i]) {
        self->isAttribute = 1;
        if ((d->option_flags & GRIB_DUMP_FLAG_ALL_ATTRIBUTES) == 0 && (a->attributes[i]->flags & GRIB_ACCESSOR_FLAG_DUMP) == 0) {
            i++;
            continue;
        }
        self->isLeaf = a->attributes[i]->attributes[0] == NULL ? 1 : 0;
        flags        = a->attributes[i]->flags;
        a->attributes[i]->flags |= GRIB_ACCESSOR_FLAG_DUMP;
        switch (grib_accessor_get_native_type(a->attributes[i])) {
            case GRIB_TYPE_LONG:
                dump_long_attribute(d, a->attributes[i], prefix);
                break;
            case GRIB_TYPE_DOUBLE:
                dump_values_attribute(d, a->attributes[i], prefix);
                break;
            case GRIB_TYPE_STRING:
                break;
        }
        a->attributes[i]->flags = flags;
        i++;
    }
    self->isLeaf      = 0;
    self->isAttribute = 0;
}

static void header(grib_dumper* d, grib_handle* h)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    char sampleName[200]            = { 0 };
    long localSectionPresent, edition, bufrHeaderCentre, isSatellite;

    Assert(h->product_kind == PRODUCT_BUFR);

    grib_get_long(h, "localSectionPresent", &localSectionPresent);
    grib_get_long(h, "bufrHeaderCentre", &bufrHeaderCentre);
    grib_get_long(h, "edition", &edition);

    if (localSectionPresent && bufrHeaderCentre == 98) {
        grib_get_long(h, "isSatellite", &isSatellite);
        if (isSatellite)
            snprintf(sampleName, sizeof(sampleName), "BUFR%ld_local_satellite", edition);
        else
            snprintf(sampleName, sizeof(sampleName), "BUFR%ld_local", edition);
    }
    else {
        snprintf(sampleName, sizeof(sampleName), "BUFR%ld", edition);
    }

    if (d->count < 2) {
        fprintf(self->dumper.out, "/* This program was automatically generated with bufr_dump -EC */\n");
        fprintf(self->dumper.out, "/* Using ecCodes version: ");
        grib_print_api_version(self->dumper.out);
        fprintf(self->dumper.out, " */\n\n");
        fprintf(self->dumper.out, "#include \"eccodes.h\"\n");
        fprintf(self->dumper.out, "int main()\n");
        fprintf(self->dumper.out, "{\n");
        fprintf(self->dumper.out, "  size_t         size=0;\n");
        fprintf(self->dumper.out, "  const void*    buffer = NULL;\n");
        fprintf(self->dumper.out, "  FILE*          fout = NULL;\n");
        fprintf(self->dumper.out, "  codes_handle*  h = NULL;\n");
        fprintf(self->dumper.out, "  long*          ivalues = NULL;\n");
        fprintf(self->dumper.out, "  char**         svalues = NULL;\n");
        fprintf(self->dumper.out, "  double*        rvalues = NULL;\n");
        fprintf(self->dumper.out, "  const char*    sampleName = \"%s\";\n\n", sampleName);
    }

    fprintf(self->dumper.out, "  h = codes_bufr_handle_new_from_samples(NULL, sampleName);\n");
    fprintf(self->dumper.out, "  if (h == NULL) {\n");
    fprintf(self->dumper.out, "    fprintf(stderr, \"ERROR: Failed to create BUFR from %%s\\n\", sampleName);\n");
    fprintf(self->dumper.out, "    return 1;\n");
    fprintf(self->dumper.out, "  }\n");
}

static void footer(grib_dumper* d, grib_handle* h)
{
    grib_dumper_bufr_encode_C* self = (grib_dumper_bufr_encode_C*)d;
    fprintf(self->dumper.out, "\n  /* Encode the keys back in the data section */\n");
    fprintf(self->dumper.out, "  CODES_CHECK(codes_set_long(h, \"pack\", 1), 0);\n\n");
    if (d->count == 1)
        fprintf(self->dumper.out, "  fout = fopen(\"outfile.bufr\", \"w\");\n");
    else
        fprintf(self->dumper.out, "  fout = fopen(\"outfile.bufr\", \"a\");\n");

    /*fprintf(self->dumper.out,"  fout = fopen(\"outfile.bufr\", \"w\");");*/
    fprintf(self->dumper.out, "  if (!fout) {\n");
    fprintf(self->dumper.out, "    fprintf(stderr, \"ERROR: Failed to open output file 'outfile.bufr' for writing.\\n\");\n");
    fprintf(self->dumper.out, "    return 1;\n");
    fprintf(self->dumper.out, "  }\n");
    fprintf(self->dumper.out, "  CODES_CHECK(codes_get_message(h,&buffer,&size),0);\n");
    fprintf(self->dumper.out, "  if (fwrite(buffer,1,size,fout) != size) {\n");
    fprintf(self->dumper.out, "    fprintf(stderr, \"ERROR: Failed to write data.\\n\");\n");
    fprintf(self->dumper.out, "    return 1;\n");
    fprintf(self->dumper.out, "  }\n");
    fprintf(self->dumper.out, "  if (fclose(fout)!=0) {\n");
    fprintf(self->dumper.out, "    fprintf(stderr, \"ERROR: Failed to close output file handle.\\n\");\n");
    fprintf(self->dumper.out, "    return 1;\n");
    fprintf(self->dumper.out, "  }\n");
    fprintf(self->dumper.out, "  \n");
    fprintf(self->dumper.out, "  codes_handle_delete(h);\n");
    if (d->count == 1)
        fprintf(self->dumper.out, "  printf(\"Created output BUFR file 'outfile.bufr'.\\n\");\n");
    fprintf(self->dumper.out, "  free(ivalues); ivalues = NULL;\n");
    fprintf(self->dumper.out, "  free(rvalues); rvalues = NULL;\n");
    fprintf(self->dumper.out, "  free(svalues); svalues = NULL;\n\n");
}
