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
/*
   This is used by make_class.pl

   START_CLASS_DEF
   CLASS      = accessor
   SUPER      = grib_accessor_class_ascii
   IMPLEMENTS = pack_string;unpack_string;value_count
   IMPLEMENTS = init;string_length
   MEMBERS= grib_arguments* args
   END_CLASS_DEF

 */

/* START_CLASS_IMP */

/*

Don't edit anything between START_CLASS_IMP and END_CLASS_IMP
Instead edit values between START_CLASS_DEF and END_CLASS_DEF
or edit "accessor.class" and rerun ./make_class.pl

*/

static int pack_string(grib_accessor*, const char*, size_t* len);
static int unpack_string(grib_accessor*, char*, size_t* len);
static size_t string_length(grib_accessor*);
static int value_count(grib_accessor*, long*);
static void init(grib_accessor*, const long, grib_arguments*);
static void init_class(grib_accessor_class*);

typedef struct grib_accessor_sprintf
{
    grib_accessor att;
    /* Members defined in gen */
    /* Members defined in ascii */
    /* Members defined in sprintf */
    grib_arguments* args;
} grib_accessor_sprintf;

extern grib_accessor_class* grib_accessor_class_ascii;

static grib_accessor_class _grib_accessor_class_sprintf = {
    &grib_accessor_class_ascii,                      /* super */
    "sprintf",                      /* name */
    sizeof(grib_accessor_sprintf),  /* size */
    0,                           /* inited */
    &init_class,                 /* init_class */
    &init,                       /* init */
    0,                  /* post_init */
    0,                    /* free mem */
    0,                       /* describes himself */
    0,                /* get length of section */
    &string_length,              /* get length of string */
    &value_count,                /* get number of values */
    0,                 /* get number of bytes */
    0,                /* get offset to bytes */
    0,            /* get native type */
    0,                /* get sub_section */
    0,               /* grib_pack procedures long */
    0,                 /* grib_pack procedures long */
    0,                  /* grib_pack procedures long */
    0,                /* grib_unpack procedures long */
    0,                /* grib_pack procedures double */
    0,              /* grib_unpack procedures double */
    &pack_string,                /* grib_pack procedures string */
    &unpack_string,              /* grib_unpack procedures string */
    0,          /* grib_pack array procedures string */
    0,        /* grib_unpack array procedures string */
    0,                 /* grib_pack procedures bytes */
    0,               /* grib_unpack procedures bytes */
    0,            /* pack_expression */
    0,              /* notify_change */
    0,                /* update_size */
    0,             /* preferred_size */
    0,                     /* resize */
    0,      /* nearest_smaller_value */
    0,                       /* next accessor */
    0,                    /* compare vs. another accessor */
    0,      /* unpack only ith value */
    0,  /* unpack a given set of elements */
    0,     /* unpack a subarray */
    0,                      /* clear */
    0,                 /* clone accessor */
};


grib_accessor_class* grib_accessor_class_sprintf = &_grib_accessor_class_sprintf;


static void init_class(grib_accessor_class* c)
{
    c->dump    =    (*(c->super))->dump;
    c->next_offset    =    (*(c->super))->next_offset;
    c->byte_count    =    (*(c->super))->byte_count;
    c->byte_offset    =    (*(c->super))->byte_offset;
    c->get_native_type    =    (*(c->super))->get_native_type;
    c->sub_section    =    (*(c->super))->sub_section;
    c->pack_missing    =    (*(c->super))->pack_missing;
    c->is_missing    =    (*(c->super))->is_missing;
    c->pack_long    =    (*(c->super))->pack_long;
    c->unpack_long    =    (*(c->super))->unpack_long;
    c->pack_double    =    (*(c->super))->pack_double;
    c->unpack_double    =    (*(c->super))->unpack_double;
    c->pack_string_array    =    (*(c->super))->pack_string_array;
    c->unpack_string_array    =    (*(c->super))->unpack_string_array;
    c->pack_bytes    =    (*(c->super))->pack_bytes;
    c->unpack_bytes    =    (*(c->super))->unpack_bytes;
    c->pack_expression    =    (*(c->super))->pack_expression;
    c->notify_change    =    (*(c->super))->notify_change;
    c->update_size    =    (*(c->super))->update_size;
    c->preferred_size    =    (*(c->super))->preferred_size;
    c->resize    =    (*(c->super))->resize;
    c->nearest_smaller_value    =    (*(c->super))->nearest_smaller_value;
    c->next    =    (*(c->super))->next;
    c->compare    =    (*(c->super))->compare;
    c->unpack_double_element    =    (*(c->super))->unpack_double_element;
    c->unpack_double_element_set    =    (*(c->super))->unpack_double_element_set;
    c->unpack_double_subarray    =    (*(c->super))->unpack_double_subarray;
    c->clear    =    (*(c->super))->clear;
    c->make_clone    =    (*(c->super))->make_clone;
}

/* END_CLASS_IMP */

static void init(grib_accessor* a, const long l, grib_arguments* c)
{
    grib_accessor_sprintf* self = (grib_accessor_sprintf*)a;
    self->args                  = c;
    a->flags |= GRIB_ACCESSOR_FLAG_READ_ONLY;
}

static int pack_string(grib_accessor* a, const char* val, size_t* len)
{
    return GRIB_NOT_IMPLEMENTED;
}

static int unpack_string(grib_accessor* a, char* val, size_t* len)
{
    grib_accessor_sprintf* self = (grib_accessor_sprintf*)a;
    char result[1024];
    char tempBuffer[2048];
    char sres[1024];
    long ires            = 0;
    double dres          = 0;
    int i                = 0;
    size_t replen        = 1024;
    int ret              = GRIB_SUCCESS;
    int carg             = 0;
    int is_missing       = 0;
    const char* uname    = NULL;
    const char* tempname = NULL;
    size_t uname_len     = 0;

    uname = grib_arguments_get_string(grib_handle_of_accessor(a), self->args, carg++);
    snprintf(result, sizeof(result), "%s", "");
    uname_len = strlen(uname);

    for (i = 0; i < uname_len; i++) {
        if (uname[i] == '%') {
            int precision = 999;
            i++;
            if (uname[i] == '.') {
                char *theEnd = NULL, *start;
                start        = (char*)&(uname[++i]);
                precision    = strtol(start, &theEnd, 10);
                Assert(*theEnd != 0);
                while (uname[i] != *theEnd)
                    i++;
            }
            switch (uname[i]) {
                case 'd':
                    tempname = grib_arguments_get_name(grib_handle_of_accessor(a), self->args, carg++);

                    if ((ret = grib_get_long_internal(grib_handle_of_accessor(a), tempname, &ires)) != GRIB_SUCCESS)
                        return ret;
                    /* Bug GRIB-56: Check to see if the key is missing */
                    is_missing = grib_is_missing(grib_handle_of_accessor(a), tempname, &ret);
                    if (ret != GRIB_SUCCESS)
                        return ret;
                    if (is_missing) {
                        snprintf(tempBuffer, sizeof(tempBuffer), "%sMISSING", result);
                        strcpy(result, tempBuffer);
                    }
                    else {
                        /* Not missing so print it */
                        if (precision != 999) {
                            snprintf(tempBuffer, sizeof(tempBuffer), "%s%.*ld", result, precision, ires);
                            strcpy(result, tempBuffer);
                        }
                        else {
                            snprintf(tempBuffer, sizeof(tempBuffer), "%s%ld", result, ires);
                            strcpy(result, tempBuffer);
                        }
                    }
                    break;

                case 'g':
                    tempname = grib_arguments_get_name(grib_handle_of_accessor(a), self->args, carg++);
                    if ((ret = grib_get_double_internal(grib_handle_of_accessor(a), tempname, &dres)) != GRIB_SUCCESS)
                        return ret;
                    snprintf(tempBuffer, sizeof(tempBuffer), "%s%g", result, dres);
                    strcpy(result, tempBuffer);

                    break;

                case 's':
                    tempname = grib_arguments_get_name(grib_handle_of_accessor(a), self->args, carg++);
                    if ((ret = grib_get_string_internal(grib_handle_of_accessor(a), tempname, sres, &replen)) != GRIB_SUCCESS)
                        return ret;
                    snprintf(tempBuffer, sizeof(tempBuffer), "%s%s", result, sres);
                    strcpy(result, tempBuffer);
                    replen = 1024;
            }
        }
        else {
            snprintf(tempBuffer, sizeof(tempBuffer), "%s%c", result, uname[i]);
            strcpy(result, tempBuffer);
        }
    }

    replen = strlen(result) + 1;

    if (*len < replen) {
        *len = replen;
        return GRIB_ARRAY_TOO_SMALL;
    }
    *len = replen;

    snprintf(val, 1024, "%s", result);

    return GRIB_SUCCESS;
}

static int value_count(grib_accessor* a, long* count)
{
    *count = 1;
    return 0;
}

static size_t string_length(grib_accessor* a)
{
    return 1024;
}
