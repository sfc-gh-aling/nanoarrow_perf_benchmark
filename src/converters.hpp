#include "nanoarrow.hpp"

void IntValueConverter(ArrowArrayView* array){
    for(int i = 0; i < array->length; i+= 1) {
        ArrowArrayViewGetIntUnsafe(array, i);
    }
}

void FloatValueConverter(ArrowArrayView* array){
    for(int i = 0; i < array->length; i+= 1) {
        ArrowArrayViewGetDoubleUnsafe(array, i);
    }
}

void StringValueConverter(ArrowArrayView* array){
    for(int i = 0; i < array->length; i+= 1) {
        ArrowArrayViewGetStringUnsafe(array, i);
    }
}

void DecimalValueConverter(ArrowArrayView* array){
    for(int i = 0; i < array->length; i+= 1) {
        int64_t bytes_start = 16 * (array->array->offset + i);
        const char* ptr_start = array->buffer_views[1].data.as_char;
    }
}
