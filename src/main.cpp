/**
 * This is a prototype to test pure nanoarrow c performance
 * We identified that there are 4 primitive types that are used by python connector data converter
 *     1. Int: BooleanConverter, DateConverter, TimeConverter, TimesStampConverter
 *     2. Float: FloatConverter
 *     3. String: BinaryConverter, StringConverter
 *     4. Decimal: DecimalConverter
 *
 * The test focus on the following two aspects:
 *     1. time needed to read data from arrow ipc byte data
 *     2. time needed to get data from the arrow array
 *
 * In the future, the test should be extended to test:
 *     1. time needed to instantiate converters in connector
 *     2. data set composed of mixed data types
 */

#include <iostream>
#include <fstream>
#include <vector>
#include "nanoarrow.h"
#include "nanoarrow/nanoarrow_ipc.h"
#include "nanoarrow.hpp"
#include "converters.hpp"

enum TypeName {
    Int,  // Integer value used by BooleanConverter, DateConverter, TimeConverter, TimesStampConverter
    Float, // Float value used by FloatConverter
    String,  // String value used by BinaryConverter, StringConverter
    Decimal  // DecimalConverter is special reading raw bytes
};


std::string enumToString(TypeName typeName) {
    switch (typeName) {
        case Int:
            return "Int";
        case Float:
            return "Float";
        case String:
            return "String";
        case Decimal:
            return "Decimal";
    }
}

/**
 * using nanoarrow ipc to consume all the raw bytes into arrow array and arrow schema
 * @param arrow_bytes
 * @param arrow_bytes_size
 * @param ipcArrowArrayVec
 * @param ipcArrowArrayViewVec
 * @param ipcArrowSchema
 */
void ReadArrowIpcStream(
    char* arrow_bytes,
    int64_t arrow_bytes_size,
    std::vector<nanoarrow::UniqueArray>& ipcArrowArrayVec,
    std::vector<nanoarrow::UniqueArrayView>& ipcArrowArrayViewVec,
    nanoarrow::UniqueSchema& ipcArrowSchema

){
    int returnCode = 0;
    ArrowBuffer input_buffer;
    ArrowBufferInit(&input_buffer);
    returnCode = ArrowBufferAppend(&input_buffer, arrow_bytes, arrow_bytes_size);
    ArrowIpcInputStream input;
    returnCode = ArrowIpcInputStreamInitBuffer(&input, &input_buffer);
    ArrowArrayStream stream;
    returnCode = ArrowIpcArrayStreamReaderInit(&stream, &input, nullptr);
    returnCode = stream.get_schema(&stream, ipcArrowSchema.get());

    while (true) {
        nanoarrow::UniqueArray newUniqueArray;
        nanoarrow::UniqueArrayView newUniqueArrayView;
        auto retcode = stream.get_next(&stream, newUniqueArray.get());
        if (retcode == NANOARROW_OK && newUniqueArray->release != nullptr) {
            ipcArrowArrayVec.push_back(std::move(newUniqueArray));

            ArrowError error;
            returnCode = ArrowArrayViewInitFromSchema(newUniqueArrayView.get(),
                                                      ipcArrowSchema.get(), &error);

            returnCode = ArrowArrayViewSetArray(newUniqueArrayView.get(),
                                                newUniqueArray.get(), &error);
            ipcArrowArrayViewVec.push_back(std::move(newUniqueArrayView));
        } else {
            break;
        }
    }
    stream.release(&stream);
}

/**
 * read arrow data from all the batches
 * @param ipcArrowArrayVec
 * @param ipcArrowArrayViewVec
 * @param ipcArrowSchema
 * @param converter_function
 */
void ReadArrowData(
    std::vector<nanoarrow::UniqueArray>& ipcArrowArrayVec,
    std::vector<nanoarrow::UniqueArrayView>& ipcArrowArrayViewVec,
    nanoarrow::UniqueSchema& ipcArrowSchema,
    void (*converter_function)(ArrowArrayView*)
){

    for(int i = 0; i < ipcArrowArrayVec.size(); i+= 1){
        // n_children here is for column, currently we only test data with one column
        // in the future we can test data of multiple types/columns, we need to change the code accordingly
        for(int j = 0; j < ipcArrowArrayViewVec[i]->n_children; j += 1){
            converter_function(ipcArrowArrayViewVec[i]->children[j]);
        }
    }

}

/**
 * local data into memory from test data folder
 * @param typeName
 * @param bytes
 * @return
 */
int LoadDataFromFile(TypeName typeName, std::vector<unsigned char>& bytes){
    std::string filePath;
    switch (typeName) {
        case Int:
            filePath = "../test_data/arrow_perf_int";
            break;
        case String:
            filePath = "../test_data/arrow_perf_str";
            break;
        case Float:
            filePath = "../test_data/arrow_perf_float";
            break;
        case Decimal:
            filePath = "../test_data/arrow_perf_decimal";
            break;
    }
    // load file and stored into bytes
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open the file." << std::endl;
        return -1;
    }

    char byte;
    while (file.get(byte)) {
        bytes.push_back(static_cast<unsigned char>(byte));
    }
    return 0;
}

int TestDataType(TypeName testType, int iterCnt, bool testConsumeIPC, bool testReadData) {
    std::cout << "Test Data Type: " << enumToString(testType) << std::endl;

    std::vector<unsigned char> bytes;
    if(LoadDataFromFile(testType, bytes) == -1) {
        return -1;
    }
    char* charArray = reinterpret_cast<char*>(bytes.data());

    std::vector<nanoarrow::UniqueArray> ipcArrowArrayVec;
    std::vector<nanoarrow::UniqueArrayView> ipcArrowArrayViewVec;
    nanoarrow::UniqueSchema ipcArrowSchema;

    if(testConsumeIPC) {
        auto startTime = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < iterCnt; i++) {
            ReadArrowIpcStream(charArray, bytes.size(), ipcArrowArrayVec, ipcArrowArrayViewVec, ipcArrowSchema);
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        std::cout << "consume ipc avg duration: " << (duration.count() / iterCnt) << " microseconds." << std::endl;
    }

    if(testReadData) {
        void (*functionPtr)(ArrowArrayView*);
        switch (testType) {
            case Int:
                functionPtr = IntValueConverter;
                break;
            case String:
                functionPtr = StringValueConverter;
                break;
            case Float:
                functionPtr = FloatValueConverter;
                break;
            case Decimal:
                functionPtr = DecimalValueConverter;
                break;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < iterCnt; i++) {
            ReadArrowData(ipcArrowArrayVec, ipcArrowArrayViewVec, ipcArrowSchema, functionPtr);
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        std::cout << "Read test avg duration: " << (duration.count() / iterCnt) << " microseconds." << std::endl;
    }
    return 0;
}

int main() {
    int ret;
    int iterCnt = 1000;
    bool testConsumeIPC = true;
    bool testReadData = true;
    TestDataType(TypeName::Int, iterCnt, testConsumeIPC, testReadData);
    TestDataType(TypeName::String, iterCnt, testConsumeIPC, testReadData);
    TestDataType(TypeName::Float, iterCnt, testConsumeIPC, testReadData);
    TestDataType(TypeName::Decimal, iterCnt, testConsumeIPC, testReadData);
    return 0;
}
