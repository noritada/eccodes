/*
 * (C) Copyright 2005- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities granted to it by
 * virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
 */

/**************************************
 *  Enrico Fucile
 **************************************/


#include "grib_api_internal.h"
/*
   This is used by make_class.pl

   START_CLASS_DEF
   CLASS      = accessor
   SUPER      = grib_accessor_class_unsigned
   IMPLEMENTS = unpack_long;pack_long; value_count
   IMPLEMENTS = init
   MEMBERS=const char*    productDefinitionTemplateNumber
   MEMBERS=int            instant
   END_CLASS_DEF

 */

/* START_CLASS_IMP */

/*

Don't edit anything between START_CLASS_IMP and END_CLASS_IMP
Instead edit values between START_CLASS_DEF and END_CLASS_DEF
or edit "accessor.class" and rerun ./make_class.pl

*/

static int pack_long(grib_accessor*, const long* val, size_t* len);
static int unpack_long(grib_accessor*, long* val, size_t* len);
static int value_count(grib_accessor*, long*);
static void init(grib_accessor*, const long, grib_arguments*);
static void init_class(grib_accessor_class*);

typedef struct grib_accessor_select_step_template
{
    grib_accessor att;
    /* Members defined in gen */
    /* Members defined in long */
    /* Members defined in unsigned */
    long nbytes;
    grib_arguments* arg;
    /* Members defined in select_step_template */
    const char*    productDefinitionTemplateNumber;
    int            instant;
} grib_accessor_select_step_template;

extern grib_accessor_class* grib_accessor_class_unsigned;

static grib_accessor_class _grib_accessor_class_select_step_template = {
    &grib_accessor_class_unsigned,                      /* super */
    "select_step_template",                      /* name */
    sizeof(grib_accessor_select_step_template),  /* size */
    0,                           /* inited */
    &init_class,                 /* init_class */
    &init,                       /* init */
    0,                  /* post_init */
    0,                    /* free mem */
    0,                       /* describes himself */
    0,                /* get length of section */
    0,              /* get length of string */
    &value_count,                /* get number of values */
    0,                 /* get number of bytes */
    0,                /* get offset to bytes */
    0,            /* get native type */
    0,                /* get sub_section */
    0,               /* grib_pack procedures long */
    0,                 /* grib_pack procedures long */
    &pack_long,                  /* grib_pack procedures long */
    &unpack_long,                /* grib_unpack procedures long */
    0,                /* grib_pack procedures double */
    0,              /* grib_unpack procedures double */
    0,                /* grib_pack procedures string */
    0,              /* grib_unpack procedures string */
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


grib_accessor_class* grib_accessor_class_select_step_template = &_grib_accessor_class_select_step_template;


static void init_class(grib_accessor_class* c)
{
    c->dump    =    (*(c->super))->dump;
    c->next_offset    =    (*(c->super))->next_offset;
    c->string_length    =    (*(c->super))->string_length;
    c->byte_count    =    (*(c->super))->byte_count;
    c->byte_offset    =    (*(c->super))->byte_offset;
    c->get_native_type    =    (*(c->super))->get_native_type;
    c->sub_section    =    (*(c->super))->sub_section;
    c->pack_missing    =    (*(c->super))->pack_missing;
    c->is_missing    =    (*(c->super))->is_missing;
    c->pack_double    =    (*(c->super))->pack_double;
    c->unpack_double    =    (*(c->super))->unpack_double;
    c->pack_string    =    (*(c->super))->pack_string;
    c->unpack_string    =    (*(c->super))->unpack_string;
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
    grib_accessor_select_step_template* self = (grib_accessor_select_step_template*)a;
    int n                                    = 0;

    self->productDefinitionTemplateNumber = grib_arguments_get_name(grib_handle_of_accessor(a), c, n++);
    self->instant                         = grib_arguments_get_long(grib_handle_of_accessor(a), c, n++);
}

static int unpack_long(grib_accessor* a, long* val, size_t* len)
{
    *val = 1;
    return GRIB_SUCCESS;
}

static int pack_long(grib_accessor* a, const long* val, size_t* len)
{
    grib_accessor_select_step_template* self = (grib_accessor_select_step_template*)a;
    long productDefinitionTemplateNumber     = 0;
    long productDefinitionTemplateNumberNew  = 0;

    grib_get_long(grib_handle_of_accessor(a), self->productDefinitionTemplateNumber, &productDefinitionTemplateNumber);

    if (self->instant) {
        /* Going from continuous or non-continuous interval to a point-in-time (instantaneous) */
        switch (productDefinitionTemplateNumber) {
            case 8:
                productDefinitionTemplateNumberNew = 0;
                break;
            case 9:
                productDefinitionTemplateNumberNew = 5;
                break;
            case 10:
                productDefinitionTemplateNumberNew = 6;
                break;
            case 11:
                productDefinitionTemplateNumberNew = 1;
                break;
            case 12:
                productDefinitionTemplateNumberNew = 2;
                break;
            case 13:
                productDefinitionTemplateNumberNew = 3;
                break;
            case 14:
                productDefinitionTemplateNumberNew = 4;
                break;
            case 42: /* non-EPS chemical */
                productDefinitionTemplateNumberNew = 40;
                break;
            case 43: /* EPS chemical */
                productDefinitionTemplateNumberNew = 41;
                break;
            case 46:                                     /* non-EPS aerosol */
                productDefinitionTemplateNumberNew = 48; /*44 is deprecated*/
                break;
            case 47: /* EPS aerosol */
                productDefinitionTemplateNumberNew = 45;
                break;
            case 67: /* non-EPS chemical distrib func */
                productDefinitionTemplateNumberNew = 57;
                break;
            case 68: /* EPS chemical distrib func */
                productDefinitionTemplateNumberNew = 58;
                break;
            case 72: /* non-EPS post-processing */
                productDefinitionTemplateNumberNew = 70;
                break;
            case 73: /* EPS post-processing */
                productDefinitionTemplateNumberNew = 71;
                break;
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 15:
                productDefinitionTemplateNumberNew = productDefinitionTemplateNumber;
                break;
            default:
                productDefinitionTemplateNumberNew = productDefinitionTemplateNumber;
                break;
        }
    }
    else {
        /* Going from point-in-time (instantaneous) to continuous or non-continuous interval */
        switch (productDefinitionTemplateNumber) {
            case 0:
                productDefinitionTemplateNumberNew = 8;
                break;
            case 1:
                productDefinitionTemplateNumberNew = 11;
                break;
            case 2:
                productDefinitionTemplateNumberNew = 12;
                break;
            case 3:
                productDefinitionTemplateNumberNew = 13;
                break;
            case 4:
                productDefinitionTemplateNumberNew = 14;
                break;
            case 5:
                productDefinitionTemplateNumberNew = 9;
                break;
            case 6:
                productDefinitionTemplateNumberNew = 10;
                break;
            case 40: /* non-EPS chemical */
                productDefinitionTemplateNumberNew = 42;
                break;
            case 41: /* EPS chemical */
                productDefinitionTemplateNumberNew = 43;
                break;
            case 48: /* non-EPS aerosol. Note template 44 is deprecated */
                productDefinitionTemplateNumberNew = 46;
                break;
            case 45: /* EPS aerosol */
                /*productDefinitionTemplateNumberNew = 47;   PDT deprecated */
                productDefinitionTemplateNumberNew = 85;
                break;
            case 57: /* non-EPS chemical distrib func */
                productDefinitionTemplateNumberNew = 67;
                break;
            case 58: /* EPS chemical distrib func */
                productDefinitionTemplateNumberNew = 68;
                break;
            case 70: /* non-EPS post-processing */
                productDefinitionTemplateNumberNew = 72;
                break;
            case 71: /* EPS post-processing */
                productDefinitionTemplateNumberNew = 73;
                break;
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
                productDefinitionTemplateNumberNew = productDefinitionTemplateNumber;
                break;
            default:
                productDefinitionTemplateNumberNew = productDefinitionTemplateNumber;
                break;
        }
    }

    if (productDefinitionTemplateNumber != productDefinitionTemplateNumberNew)
        grib_set_long(grib_handle_of_accessor(a), self->productDefinitionTemplateNumber, productDefinitionTemplateNumberNew);

    return 0;
}

static int value_count(grib_accessor* a, long* c)
{
    *c = 1;
    return 0;
}
