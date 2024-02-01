#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>
#include "nanoarrow.h"
#include "nanoarrow/nanoarrow_ipc.h"
#include "nanoarrow/nanoarrow_ipc.hpp"
#include "nanoarrow.hpp"
#include "converters.hpp"

enum TypeName {
    Int,
    Decimal,
    String,
    Boolean,
    Binary,
};


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

void ReadArrowData(
    std::vector<nanoarrow::UniqueArray>& ipcArrowArrayVec,
    std::vector<nanoarrow::UniqueArrayView>& ipcArrowArrayViewVec,
    nanoarrow::UniqueSchema& ipcArrowSchema,
    void (*converter_function)(ArrowArrayView*)
){
    converter_function(ipcArrowArrayViewVec[0]->children[0]);
}

int main() {
    std::string filePath("../test_data/arrow_perf_100_int");

    // load file and stored into bytes
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    std::vector<unsigned char> bytes;
    char byte;
    while (file.get(byte)) {
        bytes.push_back(static_cast<unsigned char>(byte));
    }
    char* charArray = reinterpret_cast<char*>(bytes.data());

    int iterCnt = 1000;
    bool testConsumeIPC = true;
    bool testReadData = true;

    std::vector<nanoarrow::UniqueArray> ipcArrowArrayVec;
    std::vector<nanoarrow::UniqueArrayView> ipcArrowArrayViewVec;
    nanoarrow::UniqueSchema ipcArrowSchema;

    // just consuming ipc data
    if(testConsumeIPC) {
        auto startTime = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < iterCnt; i++) {
            ReadArrowIpcStream(charArray, bytes.size(), ipcArrowArrayVec, ipcArrowArrayViewVec, ipcArrowSchema);
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "consume ipc avg duration: " << (duration.count() / iterCnt) << " milliseconds." << std::endl;
    }

    if(testReadData) {
        void (*functionPtr)(ArrowArrayView*);
        // TODO, assign function according to type
        functionPtr = IntValueConverter;
        auto startTime = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < iterCnt; i++) {
            ReadArrowData(ipcArrowArrayVec, ipcArrowArrayViewVec, ipcArrowSchema, functionPtr);
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "Read test avg duration: " << (duration.count() / iterCnt) << " milliseconds." << std::endl;
    }
    return 0;
}