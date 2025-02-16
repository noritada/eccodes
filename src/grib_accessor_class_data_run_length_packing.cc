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
   SUPER      = grib_accessor_class_values
   IMPLEMENTS = init
   IMPLEMENTS = unpack_double
   IMPLEMENTS = value_count
   MEMBERS=const char*  number_of_values
   MEMBERS=const char*  bits_per_value
   MEMBERS=const char*  max_level_value
   MEMBERS=const char*  number_of_level_values
   MEMBERS=const char*  decimal_scale_factor
   MEMBERS=const char*  level_values
   END_CLASS_DEF

 */

/* START_CLASS_IMP */

/*

Don't edit anything between START_CLASS_IMP and END_CLASS_IMP
Instead edit values between START_CLASS_DEF and END_CLASS_DEF
or edit "accessor.class" and rerun ./make_class.pl

*/

static int unpack_double(grib_accessor*, double* val, size_t* len);
static int value_count(grib_accessor*, long*);
static void init(grib_accessor*, const long, grib_arguments*);
static void init_class(grib_accessor_class*);

typedef struct grib_accessor_data_run_length_packing
{
    grib_accessor att;
    /* Members defined in gen */
    /* Members defined in values */
    int carg;
    const char* seclen;
    const char* offsetdata;
    const char* offsetsection;
    int dirty;
    /* Members defined in data_run_length_packing */
    const char* number_of_values;
    const char* bits_per_value;
    const char* max_level_value;
    const char* number_of_level_values;
    const char* decimal_scale_factor;
    const char* level_values;
} grib_accessor_data_run_length_packing;

extern grib_accessor_class* grib_accessor_class_values;

static grib_accessor_class _grib_accessor_class_data_run_length_packing = {
    &grib_accessor_class_values,                   /* super                     */
    "data_run_length_packing",                     /* name                      */
    sizeof(grib_accessor_data_run_length_packing), /* size                      */
    0,                                             /* inited */
    &init_class,                                   /* init_class */
    &init,                                         /* init                      */
    0,                                             /* post_init                      */
    0,                                             /* free mem                       */
    0,                                             /* describes himself         */
    0,                                             /* get length of section     */
    0,                                             /* get length of string      */
    &value_count,                                  /* get number of values      */
    0,                                             /* get number of bytes      */
    0,                                             /* get offset to bytes           */
    0,                                             /* get native type               */
    0,                                             /* get sub_section                */
    0,                                             /* grib_pack procedures long      */
    0,                                             /* grib_pack procedures long      */
    0,                                             /* grib_pack procedures long      */
    0,                                             /* grib_unpack procedures long    */
    0,                                             /* grib_pack procedures double    */
    &unpack_double,                                /* grib_unpack procedures double  */
    0,                                             /* grib_pack procedures string    */
    0,                                             /* grib_unpack procedures string  */
    0,                                             /* grib_pack array procedures string    */
    0,                                             /* grib_unpack array procedures string  */
    0,                                             /* grib_pack procedures bytes     */
    0,                                             /* grib_unpack procedures bytes   */
    0,                                             /* pack_expression */
    0,                                             /* notify_change   */
    0,                                             /* update_size   */
    0,                                             /* preferred_size   */
    0,                                             /* resize   */
    0,                                             /* nearest_smaller_value */
    0,                                             /* next accessor    */
    0,                                             /* compare vs. another accessor   */
    0,                                             /* unpack only ith value          */
    0,                                             /* unpack a subarray         */
    0,                                             /* clear          */
    0,                                             /* clone accessor          */
};


grib_accessor_class* grib_accessor_class_data_run_length_packing = &_grib_accessor_class_data_run_length_packing;


static void init_class(grib_accessor_class* c)
{
    c->dump                   = (*(c->super))->dump;
    c->next_offset            = (*(c->super))->next_offset;
    c->string_length          = (*(c->super))->string_length;
    c->byte_count             = (*(c->super))->byte_count;
    c->byte_offset            = (*(c->super))->byte_offset;
    c->get_native_type        = (*(c->super))->get_native_type;
    c->sub_section            = (*(c->super))->sub_section;
    c->pack_missing           = (*(c->super))->pack_missing;
    c->is_missing             = (*(c->super))->is_missing;
    c->pack_long              = (*(c->super))->pack_long;
    c->unpack_long            = (*(c->super))->unpack_long;
    c->pack_double            = (*(c->super))->pack_double;
    c->pack_string            = (*(c->super))->pack_string;
    c->unpack_string          = (*(c->super))->unpack_string;
    c->pack_string_array      = (*(c->super))->pack_string_array;
    c->unpack_string_array    = (*(c->super))->unpack_string_array;
    c->pack_bytes             = (*(c->super))->pack_bytes;
    c->unpack_bytes           = (*(c->super))->unpack_bytes;
    c->pack_expression        = (*(c->super))->pack_expression;
    c->notify_change          = (*(c->super))->notify_change;
    c->update_size            = (*(c->super))->update_size;
    c->preferred_size         = (*(c->super))->preferred_size;
    c->resize                 = (*(c->super))->resize;
    c->nearest_smaller_value  = (*(c->super))->nearest_smaller_value;
    c->next                   = (*(c->super))->next;
    c->compare                = (*(c->super))->compare;
    c->unpack_double_element  = (*(c->super))->unpack_double_element;
    c->unpack_double_subarray = (*(c->super))->unpack_double_subarray;
    c->clear                  = (*(c->super))->clear;
    c->make_clone             = (*(c->super))->make_clone;
}

/* END_CLASS_IMP */

static void init(grib_accessor* a, const long v, grib_arguments* args)
{
    grib_accessor_data_run_length_packing* self = (grib_accessor_data_run_length_packing*)a;
    grib_handle* gh                             = grib_handle_of_accessor(a);
    self->number_of_values                      = grib_arguments_get_name(gh, args, self->carg++);
    self->bits_per_value                        = grib_arguments_get_name(gh, args, self->carg++);
    self->max_level_value                       = grib_arguments_get_name(gh, args, self->carg++);
    self->number_of_level_values                = grib_arguments_get_name(gh, args, self->carg++);
    self->decimal_scale_factor                  = grib_arguments_get_name(gh, args, self->carg++);
    self->level_values                          = grib_arguments_get_name(gh, args, self->carg++);
    a->flags |= GRIB_ACCESSOR_FLAG_DATA;
}

static int value_count(grib_accessor* a, long* number_of_values)
{
    grib_accessor_data_run_length_packing* self = (grib_accessor_data_run_length_packing*)a;
    *number_of_values                           = 0;
    return grib_get_long_internal(grib_handle_of_accessor(a), self->number_of_values, number_of_values);
}

static int unpack_double(grib_accessor* a, double* val, size_t* len)
{
    grib_accessor_data_run_length_packing* self = (grib_accessor_data_run_length_packing*)a;
    grib_handle* gh                             = grib_handle_of_accessor(a);
    int err                                     = GRIB_SUCCESS;
    long seclen, number_of_values, bits_per_value, max_level_value, number_of_level_values, decimal_scale_factor;
    long* level_values = NULL;
    size_t level_values_size = 0;
    int i = 0;
    long number_of_compressed_values = 0, range = 0, offsetBeforeData = 0, pos = 0;
    long v, n, factor, k, j;
    long* compressed_values = NULL;
    double level_scale_factor = 0;
    double* levels = NULL;
    unsigned char* buf = NULL;

    if ((err = grib_get_long_internal(gh, self->seclen, &seclen)) != GRIB_SUCCESS)
        return err;
    if ((err = grib_get_long_internal(gh, self->number_of_values, &number_of_values)) != GRIB_SUCCESS)
        return err;
    if ((err = grib_get_long_internal(gh, self->bits_per_value, &bits_per_value)) != GRIB_SUCCESS)
        return err;
    if ((err = grib_get_long_internal(gh, self->max_level_value, &max_level_value)) != GRIB_SUCCESS)
        return err;
    if ((err = grib_get_long_internal(gh, self->number_of_level_values, &number_of_level_values)) != GRIB_SUCCESS)
        return err;
    if ((err = grib_get_long_internal(gh, self->decimal_scale_factor, &decimal_scale_factor)) != GRIB_SUCCESS)
        return err;

    level_values       = (long*)grib_context_malloc_clear(a->context, sizeof(long) * number_of_level_values);
    level_values_size = number_of_level_values;
    if ((err = grib_get_long_array_internal(gh, self->level_values, level_values, &level_values_size)) != GRIB_SUCCESS)
        return err;
    *len = number_of_values;
    i = 0;
    number_of_compressed_values = ((seclen - 5) * 8) / bits_per_value;
    if (number_of_compressed_values == 0 || max_level_value == 0) {
        for (i = 0; i < number_of_values; i++) {
            val[i] = GRIB_MISSING_DOUBLE;
        }
        return GRIB_SUCCESS;
    }
    range = (1 << bits_per_value) - 1 - max_level_value;
    if ((max_level_value <= 0) || (number_of_level_values <= 0) || (max_level_value > number_of_level_values) || (range <= 0)) {
        grib_context_log(a->context, GRIB_LOG_ERROR, "parameters are invalid: max_level_value=%ld(>0, <=number_of_level_values), number_of_level_values=%ld(>0, >=max_level_value), range=%ld(>0)", max_level_value, number_of_level_values, range);
        return GRIB_DECODING_ERROR;
    }
    if (decimal_scale_factor > 127) {
        decimal_scale_factor = -(decimal_scale_factor - 128);
    }
    level_scale_factor = grib_power(-decimal_scale_factor, 10.0);
    levels            = (double*)grib_context_malloc_clear(a->context, sizeof(double) * (number_of_level_values + 1));
    levels[0]                 = 0;
    for (i = 0; i < number_of_level_values; i++) {
        levels[i + 1] = level_values[i] * level_scale_factor;
    }
    compressed_values = (long*)grib_context_malloc_clear(a->context, sizeof(long) * number_of_compressed_values);
    buf      = (unsigned char*)grib_handle_of_accessor(a)->buffer->data;
    offsetBeforeData   = grib_byte_offset(a);
    buf += offsetBeforeData;
    pos = 0;
    grib_decode_long_array(buf, &pos, bits_per_value, number_of_compressed_values, compressed_values);
    j = 0;
    i = 0;
    while (i < number_of_compressed_values) {
        if (compressed_values[i] > max_level_value) {
            grib_context_log(a->context, GRIB_LOG_ERROR, "numberOfValues mismatch: i=%ld, compressed_values[i]=%ld, max_level_value=%ld", i, compressed_values[i], max_level_value);
            break;
        }
        v      = compressed_values[i++];
        n      = 1;
        factor = 1;
        while (i < number_of_compressed_values && compressed_values[i] > max_level_value) {
            n += factor * (compressed_values[i] - max_level_value - 1);
            factor = factor * range;
            i++;
        }
        if (n > number_of_values) {
            grib_context_log(a->context, GRIB_LOG_ERROR, "numberOfValues mismatch: n=%ld, number_of_values=%ld", n, number_of_values);
            break;
        }
        for (k = 0; k < n; k++) {
            val[j++] = levels[v];
        }
    }
    grib_context_free(a->context, level_values);
    grib_context_free(a->context, levels);
    grib_context_free(a->context, compressed_values);
    if (j != number_of_values) {
        grib_context_log(a->context, GRIB_LOG_ERROR, "numberOfValues mismatch: j=%ld, number_of_values=%ld", j, number_of_values);
        return GRIB_DECODING_ERROR;
    }
    return err;
}
