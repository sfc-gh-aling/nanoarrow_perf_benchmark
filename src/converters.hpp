#include "nanoarrow.hpp"

void IntValueConverter(ArrowArrayView* array){
    for(int i = 0; i < array->length; i+= 1) {
        ArrowArrayViewGetIntUnsafe(array, i);
    }
}
